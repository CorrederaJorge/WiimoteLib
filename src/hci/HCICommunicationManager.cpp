/*
 * HCICommunicationManager.cpp
 *
 *  Created on: 5 jun. 2020
 *      Author: gusa
 */

#include <esp32-hal-log.h>
#include <src/BtMessageGenerator.h>
#include <src/hci/HCICommunicationManager.h>
#include <src/hci/ScannedDeviceList.h>
#include <src/utils/Utils.h>

#define PSM_HID_Control_11 0x0011
#define PSM_HID_Interrupt_13 0x0013

HCICommunicationManager::HCICommunicationManager(
    DataQueue* rxQueue, DataQueue* txQueue,
    L2CapCommunicationManager* l2CapManager,
    BtMessageGenerator* btMessageGenerator) {
  this->rxQueue = rxQueue;
  this->txQueue = txQueue;
  this->l2CapManager = l2CapManager;
  this->sDeviceList = new ScannedDeviceList();
  this->btMessageGenerator = btMessageGenerator;
}

HCICommunicationManager::~HCICommunicationManager() {
  delete this->sDeviceList;
}

void HCICommunicationManager::ProcessHciEvent(uint8_t event_code, uint8_t len,
                                              uint8_t* data) {
  if (event_code != 0x02) {  // suppress inquiry_result_event
    log_d("**** HCI_EVENT code=%02X len=%d data=%s", event_code, len,
          Utils::FormatHex(data, len));
  }

  if (event_code == 0x01) {
    this->ProcessInquiryCompleteEvent(len, data);
  } else if (event_code == 0x02) {
    this->ProcessInquiryResultEvent(len, data);
  } else if (event_code == 0x03) {
    this->ProcessConnectionCompleteEvent(len, data);
  } else if (event_code == 0x05) {
    this->ProcessDisconnectionCompleteEvent(len, data);
  } else if (event_code == 0x07) {
    this->ProcessRemoteNameRequestCompleteEvent(len, data);
  } else if (event_code == 0x0E) {
    this->ProcessCommandCompleteEvent(len, data);
  } else if (event_code == 0x0F) {
    this->ProcessCommandStatusEvent(len, data);
  } else if (event_code == 0x10) {
    log_e("Hardware error");
  } else if (event_code == 0x13) {
    log_d("  (Number Of Completed Packets Event)");
  } else if (event_code == 0x0D) {
    log_d("  (QoS Setup Complete Event)");
  } else {
    log_d("!!! ProcessHciEvent no impl !!!");
  }
}

void HCICommunicationManager::ProcessRemoteNameRequestCompleteEvent(
    uint8_t len, uint8_t* data) {
  uint8_t status = data[0];
  log_d("remote_name_request_complete status=%02X", status);
  struct BtMessageGenerator::bd_addr_t bd_addr;
  STREAM_TO_BDADDR(bd_addr.addr, data + 1);
  log_d(
      "  BD_ADDR = %s",
      Utils::FormatHex(reinterpret_cast<uint8_t*>(&bd_addr.addr), BD_ADDR_LEN));

  char* name = reinterpret_cast<char*>(data + 7);
  log_d("  REMOTE_NAME = %s", name);

  int idx = this->sDeviceList->ScannedDeviceFind(&bd_addr);
  if (0 <= idx && strcmp("Nintendo RVL-CNT-01", name) == 0) {
    {
      uint8_t tmp_data[256];
      uint16_t len =
          this->btMessageGenerator->make_cmd_inquiry_cancel(tmp_data);
      log_d("queued inquiry_cancel.");
      this->txQueue->Queue(tmp_data, len, Data::Type::INQUIRY_CANCEL);
    }

    ScannedDevice scannedDevice = this->sDeviceList->Get(idx);

    uint16_t pt = 0x0008;
    uint8_t ars = 0x00;
    uint8_t tmp_data[256];
    uint16_t len = this->btMessageGenerator->make_cmd_create_connection(
        tmp_data, scannedDevice.bd_addr, pt, scannedDevice.psrm,
        scannedDevice.clkofs, ars);
    log_d("queued create_connection.");
    this->txQueue->Queue(tmp_data, len, Data::Type::CREATE_CONNECTION);
  }
}

void HCICommunicationManager::ProcessDisconnectionCompleteEvent(uint8_t len,
                                                                uint8_t* data) {
  uint8_t status = data[0];
  log_d("disconnection_complete status=%02X", status);

  uint16_t ch = data[2] << 8 | data[1];  // Connection_Handle
  uint8_t reason = data[3];              // Reason

  log_d("  Connection_Handle  = 0x%04X", ch);
  log_d("  Reason             = %02X", reason);

  wiimoteConnected = false;
  StartScan();
}

void HCICommunicationManager::ProcessConnectionCompleteEvent(uint8_t len,
                                                             uint8_t* data) {
  uint8_t status = data[0];
  log_d("connection_complete status=%02X", status);

  uint16_t connection_handle = data[2] << 8 | data[1];
  struct BtMessageGenerator::bd_addr_t bd_addr;
  STREAM_TO_BDADDR(bd_addr.addr, data + 3);
  uint8_t lt = data[9];   // Link_Type
  uint8_t ee = data[10];  // Encryption_Enabled

  log_d("  Connection_Handle  = 0x%04X", connection_handle);
  log_d(
      "  BD_ADDR            = %s",
      Utils::FormatHex(reinterpret_cast<uint8_t*>(&bd_addr.addr), BD_ADDR_LEN));
  log_d("  Link_Type          = %02X", lt);
  log_d("  Encryption_Enabled = %02X", ee);

  this->l2CapManager->L2capConnect(connection_handle, PSM_HID_Control_11);
}

void HCICommunicationManager::ProcessInquiryResultEvent(uint8_t len,
                                                        uint8_t* data) {
  uint8_t num = data[0];
  log_d("inquiry_result num=%d", num);

  for (int i = 0; i < num; i++) {
    int pos = 1 + (6 + 1 + 2 + 3 + 2) * i;

    struct BtMessageGenerator::bd_addr_t bd_addr;
    STREAM_TO_BDADDR(bd_addr.addr, data + pos);

    log_d("**** inquiry_result BD_ADDR(%d/%d) = %s", i, num,
          Utils::FormatHex(reinterpret_cast<uint8_t*>(&bd_addr.addr),
                           BD_ADDR_LEN));

    int idx = this->sDeviceList->ScannedDeviceFind(&bd_addr);
    if (idx == -1) {
      log_d("    Page_Scan_Repetition_Mode = %02X", data[pos + 6]);
      // data[pos+7] data[pos+8] // Reserved
      log_d("    Class_of_Device = %02X %02X %02X", data[pos + 9],
            data[pos + 10], data[pos + 11]);
      log_d("    Clock_Offset = %02X %02X", data[pos + 12], data[pos + 13]);

      ScannedDevice* scanned_device =
          new ScannedDevice(bd_addr, data[pos + 6],
                            ((0x80 | data[pos + 12]) << 8) | (data[pos + 13]));
      idx = this->sDeviceList->ScannedDeviceAdd(*scanned_device);
      if (0 <= idx) {
        if (data[pos + 9] == 0x04 && data[pos + 10] == 0x25 &&
            data[pos + 11] == 0x00) {  // Filter for Wiimote [04 25 00]
          uint8_t tmp_data[256];
          uint16_t len = this->btMessageGenerator->make_cmd_remote_name_request(
              tmp_data, scanned_device->bd_addr, scanned_device->psrm,
              scanned_device->clkofs);
          log_d("queued remote_name_request.");
          this->txQueue->Queue(tmp_data, len, Data::Type::REMOTE_NAME_REQUEST);
        } else {
          log_d("skiped to remote_name_request. (not Wiimote COD)");
        }
      } else {
        log_d("failed to scanned_list_add.");
      }
    } else {
      log_d(" (dup idx=%d)", idx);
    }
  }
}

static bool IsEqual(uint8_t* data, uint16_t command) {
  return (data[1] == (uint8_t)command) && (data[2] == (uint8_t)(command >> 8));
}

void HCICommunicationManager::StartScan() {
  log_e("Start scan");
  uint8_t tmp_data[256];
  uint16_t len = this->btMessageGenerator->make_cmd_reset(tmp_data);
  this->txQueue->Queue(tmp_data, len, Data::Type::RESET);
  log_d("queued reset => %s", Utils::FormatHex(tmp_data, len));
}

void HCICommunicationManager::ProcessInquiryCompleteEvent(uint8_t len,
                                                          uint8_t* data) {
  uint8_t status = data[0];
  log_d("inquiry_complete status=%02X", status);
  StartScan();
}

void HCICommunicationManager::ProcessCommandCompleteEvent(uint8_t len,
                                                          uint8_t* data) {
  // data[0] Num_HCI_Command_Packets

  if (IsEqual(data, HCI_RESET)) {
    if (data[3] == HCI_OK) {  // OK
      uint8_t tmp_data[256];
      uint16_t len = this->btMessageGenerator->make_cmd_read_bd_addr(tmp_data);
      log_d("queued read_bd_addr.");
      this->txQueue->Queue(tmp_data, len, Data::Type::READ_ADDRESS);
    } else {
      log_d("reset failed.");
    }
  } else if (IsEqual(data, HCI_READ_BD_ADDR)) {  // read_bd_addr
    if (data[3] == HCI_OK) {
      log_d("read_bd_addr OK. BD_ADDR=%s", Utils::FormatHex(data + 4, 6));
      char name[] = "ESP32-BT-L2CAP";

      uint8_t tmp_data[256];
      uint16_t len = this->btMessageGenerator->make_cmd_write_local_name(
          tmp_data, reinterpret_cast<uint8_t*>(name), sizeof(name));
      this->txQueue->Queue(tmp_data, len, Data::Type::WRITE_LOCAL_NAME);
      log_d("queued write_local_name.");
    } else {
      log_d("read_bd_addr failed.");
    }

  } else if (IsEqual(data, HCI_WRITE_LOCAL_NAME)) {  // write_local_name
    // data[0] Num_HCI_Command_Packets
    if (data[3] == HCI_OK) {  // OK
      log_d("write_local_name OK.");
      uint8_t cod[3] = {0x04, 0x05, 0x00};

      uint8_t tmp_data[256];

      uint16_t len = this->btMessageGenerator->make_cmd_write_class_of_device(
          tmp_data, cod);
      this->txQueue->Queue(tmp_data, len, Data::Type::WRITE_DEVICE_CLASS);
      log_d("queued write_class_of_device.");
    } else {
      log_d("write_local_name failed.");
    }
  } else if (IsEqual(data,
                     HCI_WRITE_CLASS_OF_DEVICE)) {  // write_class_of_device
    // data[0] Num_HCI_Command_Packets
    if (data[3] == HCI_OK) {  // OK
      log_d("write_class_of_device OK.");
      uint8_t tmp_data[256];

      uint16_t len =
          this->btMessageGenerator->make_cmd_write_scan_enable(tmp_data, 3);
      this->txQueue->Queue(tmp_data, len, Data::Type::SCAN_ENABLE);
      log_d("queued write_scan_enable.");
    } else {
      log_d("write_class_of_device failed.");
    }
  } else if (IsEqual(data, HCI_WRITE_SCAN_ENABLE)) {  // write_scan_enable
    // data[0] Num_HCI_Command_Packets
    if (data[3] == HCI_OK) {  // OK
      log_d("write_scan_enable OK.");
      uint8_t tmp_data[256];
      // TODO(Jorge) : Clear scanned device
      // scanned_device_clear();
      uint16_t len = this->btMessageGenerator->make_cmd_inquiry(
          tmp_data, 0x9E8B33, 0x05 /*0x30*/, 0x00);
      this->txQueue->Queue(tmp_data, len, Data::Type::INQUIRY);
      log_d("queued inquiry.");
    } else {
      log_d("write_scan_enable failed.");
    }
  } else if (IsEqual(data, HCI_INQUIRY_CANCEL)) {  // inquiry_cancel
    // data[0] Num_HCI_Command_Packets
    if (data[3] == HCI_OK) {  // OK
      log_d("inquiry_cancel OK.");
    } else {
      log_d("inquiry_cancel failed.");
    }
  } else {
    log_d("!!! process_command_complete_event no impl !!!");
  }
}

void HCICommunicationManager::ProcessCommandStatusEvent(uint8_t len,
                                                        uint8_t* data) {
  if (data[2] == 0x01 && data[3] == 0x04) {  // inquiry
    // data[1] Num_HCI_Command_Packets
    if (data[0] == HCI_OK) {  // 0x00=pending
      log_d("inquiry pending!");
    } else {
      log_d("inquiry failed. error=%02X", data[0]);
    }
  } else if (data[2] == 0x19 && data[3] == 0x04) {
    // data[1] Num_HCI_Command_Packets
    if (data[0] == HCI_OK) {  // 0x00=pending
      log_d("remote_name_request pending!");
    } else {
      log_d("remote_name_request failed. error=%02X", data[0]);
    }
  } else if (data[2] == 0x05 && data[3] == 0x04) {
    // data[1] Num_HCI_Command_Packets
    if (data[0] == HCI_OK) {  // 0x00=pending
      log_d("create_connection pending!");
    } else {
      log_d("create_connection failed. error=%02X", data[0]);
    }
  } else {
    log_d("!!! process_command_status_event no impl !!!");
  }
}
