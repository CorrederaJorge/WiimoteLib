/*
 * L2CapManager.cpp
 *
 *  Created on: 6 jun. 2020
 *      Author: gusa
 */

#include <src/l2Cap/L2CapCommunicationManager.h>
#include <src/l2Cap/L2CapConnection.h>
#include <src/utils/Utils.h>

#define PSM_HID_Control_11 0x0011
#define PSM_HID_Interrupt_13 0x0013

static uint8_t _g_identifier = 1;
static uint16_t _g_local_cid = 0x0040;

L2CapCommunicationManager::L2CapCommunicationManager(
    DataQueue* rxQueue, DataQueue* txQueue,
    BtMessageGenerator* btMessageGenerator) {
  this->rxQueue = rxQueue;
  this->txQueue = txQueue;
  this->l2CapConnectionList = new L2CapConnectionList();
  this->btMessageGenerator = btMessageGenerator;
}

uint8_t L2CapCommunicationManager::AddressSpace_(address_space_t as) {
  switch (as) {
    case EEPROM_MEMORY:
      return 0x00;
    case CONTROL_REGISTER:
      return 0x04;
  }
  return 0xFF;
}

void L2CapCommunicationManager::L2capConnect(uint16_t connection_handle,
                                             uint16_t psm) {
  this->L2capConnect_(connection_handle, psm, _g_local_cid++);
}

void L2CapCommunicationManager::L2capConnect_(uint16_t connection_handle,
                                              uint16_t psm,
                                              uint16_t source_cid) {
  uint8_t packet_boundary_flag = 0b10;  // Packet_Boundary_Flag
  uint8_t broadcast_flag = 0b00;        // Broadcast_Flag
  uint16_t channel_id = 0x0001;
  uint8_t data[] = {
      0x02,             // CONNECTION REQUEST
      _g_identifier++,  // Identifier
      0x04,
      0x00,  // Length:     0x0004
      (uint8_t)(psm & (uint8_t)0xFF),
      (uint8_t)(psm >> 8),  // PSM: HID_Control=0x0011, HID_Interrupt=0x0013
      (uint8_t)(source_cid & 0xFF),
      (uint8_t)(source_cid >> 8)  // Source CID: 0x0040+
  };
  uint16_t data_len = 8;
  uint8_t tmp_data[256];

  uint16_t len = this->btMessageGenerator->make_acl_l2cap_single_packet(
      tmp_data, connection_handle, packet_boundary_flag, broadcast_flag,
      channel_id, data, data_len);
  log_d("queued acl_l2cap_single_packet(CONNECTION REQUEST)");
  this->txQueue->Queue(tmp_data, len, Data::Type::L2CAP_CONNECTION_REQUEST);

  L2CapConnection* l2cap_connection =
      new L2CapConnection(connection_handle, psm, source_cid, 0);
  int idx = this->l2CapConnectionList->l2cap_connection_add(l2cap_connection);
  if (idx == -1) {
    log_e("!!! l2cap_connection_add failed.");
  }
}

void L2CapCommunicationManager::SetLed(uint8_t leds) {
  // TODO(Jorge): Check for multiconnection from list
  this->SetLed_(this->l2CapConnectionList->Get(0)->connection_handle, leds);
}

void L2CapCommunicationManager::ProcessAclData(uint8_t* data, size_t len) {
  if (!wiimoteConnected) {
    log_d("**** ACL_DATA len=%d data=%s", len, Utils::FormatHex(data, len));
  }

  uint16_t connection_handle = ((data[1] & 0x0F) << 8) | data[0];
  uint8_t packet_boundary_flag = (data[1] & 0x30) >> 4;  // Packet_Boundary_Flag
  uint8_t broadcast_flag = (data[1] & 0xC0) >> 6;        // Broadcast_Flag
  if (packet_boundary_flag != 0b10) {
    log_d("!!! packet_boundary_flag = 0b%02B", packet_boundary_flag);
    return;
  }
  if (broadcast_flag != 0b00) {
    log_d("!!! broadcast_flag = 0b%02B", broadcast_flag);
    return;
  }
  uint16_t l2cap_len = (data[5] << 8) | data[4];
  uint16_t channel_id = (data[7] << 8) | data[6];

  this->ProcessL2capData_(connection_handle, channel_id, data + 8, l2cap_len);
}

void L2CapCommunicationManager::ProcessL2capConnectionResponse_(
    uint16_t connection_handle, uint8_t* data) {
  uint8_t identifier = data[1];
  uint16_t destination_cid = (data[5] << 8) | data[4];
  uint16_t source_cid = (data[7] << 8) | data[6];
  uint16_t result = (data[9] << 8) | data[8];
  uint16_t status = (data[11] << 8) | data[10];

  log_d("L2CAP CONNECTION RESPONSE");
  log_d("  identifier      = %02X", identifier);
  log_d("  destination_cid = %04X", destination_cid);
  log_d("  source_cid      = %04X", source_cid);
  log_d("  result          = %04X", result);
  log_d("  status          = %04X", status);

  if (result == 0x0000) {
    int idx = this->l2CapConnectionList->l2cap_connection_find_by_local_cid(
        connection_handle, source_cid);
    L2CapConnection* l2capConnection = this->l2CapConnectionList->Get(idx);
    l2capConnection->remote_cid = destination_cid;

    uint8_t packet_boundary_flag = 0b10;  // Packet_Boundary_Flag
    uint8_t broadcast_flag = 0b00;        // Broadcast_Flag
    uint16_t channel_id = 0x0001;
    uint8_t data[] = {
        0x04,             // CONFIGURATION REQUEST
        _g_identifier++,  // Identifier
        0x08,
        0x00,  // Length: 0x0008
        (uint8_t)(destination_cid & 0xFF),
        (uint8_t)(destination_cid >> 8),  // Destination CID
        0x00,
        0x00,  // Flags
        0x01,
        0x02,
        0x40,
        0x00  // type=01 len=02 value=00 40
    };
    uint16_t data_len = 12;
    uint8_t tmp_data[256];

    uint16_t len = this->btMessageGenerator->make_acl_l2cap_single_packet(
        tmp_data, connection_handle, packet_boundary_flag, broadcast_flag,
        channel_id, data, data_len);
    log_d("queued acl_l2cap_single_packet(CONFIGURATION REQUEST)");
    this->txQueue->Queue(tmp_data, len,
                         Data::Type::L2CAP_CONFIGURATION_REQUEST);
  }
}

static void process_l2cap_configuration_response(uint16_t connection_handle,
                                                 uint8_t* data) {
  uint8_t identifier = data[1];
  uint16_t len = (data[3] << 8) | data[2];
  uint16_t source_cid = (data[5] << 8) | data[4];
  uint16_t flags = (data[7] << 8) | data[6];
  uint16_t result = (data[9] << 8) | data[8];
  // config = data[10..]

  log_d("L2CAP CONFIGURATION RESPONSE");
  log_d("  identifier      = %02X", identifier);
  log_d("  len             = %04X", len);
  log_d("  source_cid      = %04X", source_cid);
  log_d("  flags           = %04X", flags);
  log_d("  result          = %04X", result);
  log_d("  config          = %s", Utils::FormatHex(data + 10, len - 6));
}

void L2CapCommunicationManager::ProcessL2capConfigurationRequest_(
    uint16_t connection_handle, uint8_t* data) {
  uint8_t identifier = data[1];
  uint16_t len = (data[3] << 8) | data[2];
  uint16_t destination_cid = (data[5] << 8) | data[4];
  uint16_t flags = (data[7] << 8) | data[6];
  // config = data[8..]

  log_d("L2CAP CONFIGURATION REQUEST");
  log_d("  identifier      = %02X", identifier);
  log_d("  len             = %02X", len);
  log_d("  destination_cid = %04X", destination_cid);
  log_d("  flags           = %04X", flags);
  log_d("  config          = %s", Utils::FormatHex(data + 8, len - 4));

  if (flags != 0x0000) {
    log_d("!!! flags!=0x0000");
    return;
  }
  if (len != 0x08) {
    log_d("!!! len!=0x08");
    return;
  }
  if (data[8] == 0x01 && data[9] == 0x02) {  // MTU
    uint16_t mtu = (data[11] << 8) | data[10];
    log_d("  MTU=%d", mtu);

    int idx = this->l2CapConnectionList->l2cap_connection_find_by_local_cid(
        connection_handle, destination_cid);
    L2CapConnection* l2capConnection = this->l2CapConnectionList->Get(idx);

    uint8_t packet_boundary_flag = 0b10;  // Packet_Boundary_Flag
    uint8_t broadcast_flag = 0b00;        // Broadcast_Flag
    uint16_t channel_id = 0x0001;
    uint16_t source_cid = l2capConnection->remote_cid;
    uint8_t data[] = {
        0x05,        // CONFIGURATION RESPONSE
        identifier,  // Identifier
        0x0A,
        0x00,  // Length: 0x000A
        (uint8_t)(source_cid & 0xFF),
        (uint8_t)(source_cid >> 8),  // Source CID
        0x00,
        0x00,  // Flags
        0x00,
        0x00,  // Res
        0x01,
        0x02,
        (uint8_t)(mtu & 0xFF),
        (uint8_t)(mtu >> 8)  // type=01 len=02 value=xx xx
    };
    uint16_t data_len = 14;
    uint8_t tmp_data[256];
    uint16_t len = this->btMessageGenerator->make_acl_l2cap_single_packet(
        tmp_data, connection_handle, packet_boundary_flag, broadcast_flag,
        channel_id, data, data_len);
    log_d("queued acl_l2cap_single_packet(CONFIGURATION RESPONSE)");
    this->txQueue->Queue(tmp_data, len,
                         Data::Type::L2CAP_CONFIGURATION_RESPONSE);

    if (l2capConnection->psm == PSM_HID_Control_11) {
      L2capConnect_(connection_handle, PSM_HID_Interrupt_13, _g_local_cid++);
    }
  }
}

void L2CapCommunicationManager::SetLed_(uint16_t connection_handle,
                                        uint8_t leds) {
  int idx = this->l2CapConnectionList->l2cap_connection_find_by_psm(
      connection_handle, PSM_HID_Interrupt_13);
  L2CapConnection* l2capConnection = this->l2CapConnectionList->Get(idx);

  uint8_t packet_boundary_flag = 0b10;  // Packet_Boundary_Flag
  uint8_t broadcast_flag = 0b00;        // Broadcast_Flag
  uint16_t channel_id = l2capConnection->remote_cid;
  uint8_t data[] = {
      0xA2, 0x11,
      (uint8_t)(leds << 4)  // 0x0? - 0xF?
  };
  uint16_t data_len = 3;
  uint8_t tmp_data[256];

  uint16_t len = this->btMessageGenerator->make_acl_l2cap_single_packet(
      tmp_data, connection_handle, packet_boundary_flag, broadcast_flag,
      channel_id, data, data_len);
  log_d("queued acl_l2cap_single_packet(Set LEDs)");
  this->txQueue->Queue(tmp_data, len, Data::Type::L2CAP_SET_LEDS);
}

void L2CapCommunicationManager::WriteMemory_(uint16_t connection_handle,
                                             address_space_t as,
                                             uint32_t offset, uint8_t size,
                                             const uint8_t* d) {
  int idx = this->l2CapConnectionList->l2cap_connection_find_by_psm(
      connection_handle, PSM_HID_Interrupt_13);
  L2CapConnection* l2capConnection = this->l2CapConnectionList->Get(idx);

  uint8_t packet_boundary_flag = 0b10;  // Packet_Boundary_Flag
  uint8_t broadcast_flag = 0b00;        // Broadcast_Flag
  uint16_t channel_id = l2capConnection->remote_cid;
  // (a2) 16 MM FF FF FF SS DD DD DD DD DD DD DD DD DD DD DD DD DD DD DD DD
  uint8_t data[] = {0xA2,
                    0x16,               // Write
                    AddressSpace_(as),  // MM 0x00=EEPROM, 0x04=ControlRegister
                    (uint8_t)((offset >> 16) & 0xFF),  // FF
                    (uint8_t)((offset >> 8) & 0xFF),   // FF
                    (uint8_t)((offset)&0xFF),          // FF
                    size,                              // SS size 1..16
                    0x00,
                    0x00,
                    0x00,
                    0x00,
                    0x00,
                    0x00,
                    0x00,
                    0x00,
                    0x00,
                    0x00,
                    0x00,
                    0x00,
                    0x00,
                    0x00,
                    0x00,
                    0x00};
  memcpy(data + 7, d, size);
  uint8_t tmp_data[256];

  uint16_t data_len = 7 + 16;
  uint16_t len = this->btMessageGenerator->make_acl_l2cap_single_packet(
      tmp_data, connection_handle, packet_boundary_flag, broadcast_flag,
      channel_id, data, data_len);
  log_d("queued acl_l2cap_single_packet(write memory)");
  this->txQueue->Queue(tmp_data, len, Data::Type::L2CAP_WRITE_MEMORY);
}

void L2CapCommunicationManager::ProcessExtensionControllerReports_(
    uint16_t connection_handle, uint16_t channel_id, uint8_t* data,
    uint16_t len) {
  static int controller_query_state = 0;

  switch (controller_query_state) {
    case 0:
      // 0x20 Status
      // (a1) 20 BB BB LF 00 00 VV
      if (data[1] == 0x20) {
        if (data[4] & 0x02) {  // extension controller is connected
          this->WriteMemory_(connection_handle, CONTROL_REGISTER, 0xA400F0, 1,
                             (const uint8_t[]){0x55});
          controller_query_state = 1;
        } else {  // extension controller is NOT connected
          SetReportingMode_(connection_handle, 0x30,
                            false);  // 0x30: Core Buttons : 30 BB BB
          // _set_reporting_mode(connection_handle, 0x31, false); -- 0x31: Core
          // Buttons and Accelerometer : 31 BB BB AA AA AA
        }
      }
      break;
    case 1:
      // A1 22 00 00 16 00 => OK
      // A1 22 00 00 16 04 => NG
      if (data[1] == 0x22 && data[4] == 0x16) {
        if (data[5] == 0x00) {
          this->WriteMemory_(connection_handle, CONTROL_REGISTER, 0xA400FB, 1,
                             (const uint8_t[]){0x00});
          controller_query_state = 2;
        } else {
          controller_query_state = 0;
        }
      }
      break;
    case 2:
      if (data[1] == 0x22 && data[4] == 0x16) {
        if (data[5] == 0x00) {
          this->ReadMemory_(connection_handle, CONTROL_REGISTER, 0xA400FA,
                            6);  // read controller type
          controller_query_state = 3;
        } else {
          controller_query_state = 0;
        }
      }
      break;
    case 3:
      // 0x21 Read response
      // (a1) 21 BB BB SE FF FF DD DD DD DD DD DD DD DD DD DD DD DD DD DD DD DD
      if (data[1] == 0x21) {
        if (memcmp(data + 5, (const uint8_t[]){0x00, 0xFA}, 2) == 0) {
          if (memcmp(data + 7,
                     (const uint8_t[]){0x00, 0x00, 0xA4, 0x20, 0x00, 0x00},
                     6) == 0) {  // Nunchuck
            SetReportingMode_(
                connection_handle, 0x32,
                false);  // 0x32: Core Buttons with 8 Extension bytes : 32 BB BB
                         // EE EE EE EE EE EE EE EE
          }
          controller_query_state = 0;
        }
      }
      break;
  }
}

void L2CapCommunicationManager::SetReportingMode_(uint16_t connection_handle,
                                                  uint8_t reporting_mode,
                                                  bool continuous) {
  int idx = this->l2CapConnectionList->l2cap_connection_find_by_psm(
      connection_handle, PSM_HID_Interrupt_13);
  L2CapConnection* l2capConnection = this->l2CapConnectionList->Get(idx);

  uint8_t packet_boundary_flag = 0b10;  // Packet_Boundary_Flag
  uint8_t broadcast_flag = 0b00;        // Broadcast_Flag
  uint16_t channel_id = l2capConnection->remote_cid;
  uint8_t data[] = {0xA2, 0x12,
                    (uint8_t)(continuous ? 0x04 : 0x00),  // 0x00, 0x04
                    reporting_mode};
  uint16_t data_len = 4;
  uint8_t tmp_data[256];

  uint16_t len = this->btMessageGenerator->make_acl_l2cap_single_packet(
      tmp_data, connection_handle, packet_boundary_flag, broadcast_flag,
      channel_id, data, data_len);
  log_d("queued acl_l2cap_single_packet(Set reporting mode)");
  this->txQueue->Queue(tmp_data, len, Data::Type::L2CAP_SET_REPORTING_MODE);
}

void L2CapCommunicationManager::RegisterCallback(uint8_t number,
                                                 WiimoteCallback cb) {
  if (number < 1 || 4 < number) {
    return;
  }

  WiimoteCallbackList[number - 1] = cb;
}

void L2CapCommunicationManager::ProcessReport_(uint8_t* data, uint16_t len) {
  log_d("REPORT len=%d data=%s", len, Utils::FormatHex(data, len));
  uint8_t number = 0;
  WiimoteCallback callBack = *this->WiimoteCallbackList[number];
  if (callBack) {
    callBack(number, data, len);
  }
}

void L2CapCommunicationManager::ProcessL2capData_(uint16_t connection_handle,
                                                  uint16_t channel_id,
                                                  uint8_t* data, uint16_t len) {
  if (data[0] == 0x03) {  // CONNECTION RESPONSE
    ProcessL2capConnectionResponse_(connection_handle, data);
  } else if (data[0] == 0x05) {  // CONFIGURATION RESPONSE
    process_l2cap_configuration_response(connection_handle, data);
  } else if (data[0] == 0x04) {  // CONFIGURATION REQUEST
    ProcessL2capConfigurationRequest_(connection_handle, data);
  } else if (data[0] == 0xA1) {  // HID 0xA1
    if (!wiimoteConnected) {
      SetLed_(connection_handle, 0b0001);
      wiimoteConnected = true;
    }
    ProcessExtensionControllerReports_(connection_handle, channel_id, data,
                                       len);
    this->ProcessReport_(data, len);
  } else {
    log_d("!!! process_l2cap_data no impl !!!");
    log_d("  L2CAP len=%d data=%s", len, Utils::FormatHex(data, len));
  }
}

void L2CapCommunicationManager::ReadMemory_(uint16_t connection_handle,
                                            address_space_t as, uint32_t offset,
                                            uint16_t size) {
  int idx = this->l2CapConnectionList->l2cap_connection_find_by_psm(
      connection_handle, PSM_HID_Interrupt_13);
  L2CapConnection* l2capConnection = this->l2CapConnectionList->Get(idx);

  uint8_t packet_boundary_flag = 0b10;  // Packet_Boundary_Flag
  uint8_t broadcast_flag = 0b00;        // Broadcast_Flag
  uint16_t channel_id = l2capConnection->remote_cid;
  // (a2) 17 MM FF FF FF SS SS
  uint8_t data[] = {
      0xA2,
      0x17,               // Read
      AddressSpace_(as),  // MM 0x00=EEPROM, 0x04=ControlRegister
      (uint8_t)((offset >> 16) & 0x00FF),  // FF
      (uint8_t)((offset >> 8) & 0xFF),     // FF
      (uint8_t)((offset)&0xFF),            // FF
      (uint8_t)((size >> 8) & 0xFF),       // SS
      (uint8_t)((size)&0xFF)               // SS
  };
  uint16_t data_len = 8;
  uint8_t tmp_data[256];
  uint16_t len = this->btMessageGenerator->make_acl_l2cap_single_packet(
      tmp_data, connection_handle, packet_boundary_flag, broadcast_flag,
      channel_id, data, data_len);
  log_d("queued acl_l2cap_single_packet(read memory)");
  this->txQueue->Queue(tmp_data, len, Data::Type::L2CAP_READ_MEMORY);
}

L2CapCommunicationManager::~L2CapCommunicationManager() {}
