/*
 * BtMessageGenerator.cpp
 *
 *  Created on: 13 jun. 2020
 *      Author: gusa
 */

#include <src/BtMessageGenerator.h>
#include <stdint.h>

#define UINT16_TO_STREAM(p, u16)    \
  {                                 \
    *(p)++ = (uint8_t)(u16);        \
    *(p)++ = (uint8_t)((u16) >> 8); \
  }

#define UINT8_TO_STREAM(p, u8) \
  { *(p)++ = (uint8_t)(u8); }

#define BDADDR_TO_STREAM(p, a)                    \
  {                                               \
    int ijk;                                      \
    for (ijk = 0; ijk < BD_ADDR_LEN; ijk++)       \
      *(p)++ = (uint8_t)a[BD_ADDR_LEN - 1 - ijk]; \
  }

#define ARRAY_TO_STREAM(p, a, len)                            \
  {                                                           \
    int ijk;                                                  \
    for (ijk = 0; ijk < len; ijk++) *(p)++ = (uint8_t)a[ijk]; \
  }

enum {
  H4_TYPE_COMMAND = 1,
  H4_TYPE_ACL = 2,
  H4_TYPE_SCO = 3,
  H4_TYPE_EVENT = 4
};

BtMessageGenerator::BtMessageGenerator() {}

BtMessageGenerator::~BtMessageGenerator() {}

uint16_t BtMessageGenerator::make_cmd_read_bd_addr(uint8_t *buf) {
  UINT8_TO_STREAM(buf, H4_TYPE_COMMAND);
  UINT16_TO_STREAM(buf, HCI_READ_BD_ADDR);
  UINT8_TO_STREAM(buf, 0);
  return HCI_H4_CMD_PREAMBLE_SIZE;
}

uint16_t BtMessageGenerator::make_cmd_reset(uint8_t *buf) {
  UINT8_TO_STREAM(buf, H4_TYPE_COMMAND);
  UINT16_TO_STREAM(buf, HCI_RESET);
  UINT8_TO_STREAM(buf, 0);
  return HCI_H4_CMD_PREAMBLE_SIZE;
}

uint16_t BtMessageGenerator::make_cmd_write_local_name(uint8_t *buf,
                                                       uint8_t *name,
                                                       uint8_t len) {
  // name ends with null. TODO check len<=248
  UINT8_TO_STREAM(buf, H4_TYPE_COMMAND);
  UINT16_TO_STREAM(buf, HCI_WRITE_LOCAL_NAME);
  UINT8_TO_STREAM(buf, 248);
  ARRAY_TO_STREAM(buf, name, len);
  for (uint8_t i = len; i < 248; i++) {
    UINT8_TO_STREAM(buf, 0);
  }
  return HCI_H4_CMD_PREAMBLE_SIZE + 248;
}

uint16_t BtMessageGenerator::make_cmd_write_class_of_device(uint8_t *buf,
                                                            uint8_t *cod) {
  UINT8_TO_STREAM(buf, H4_TYPE_COMMAND);
  UINT16_TO_STREAM(buf, HCI_WRITE_CLASS_OF_DEVICE);
  UINT8_TO_STREAM(buf, 3);
  for (uint8_t i = 0; i < 3; i++) {
    UINT8_TO_STREAM(buf, cod[i]);
  }
  return HCI_H4_CMD_PREAMBLE_SIZE + 3;
}

uint16_t BtMessageGenerator::make_cmd_write_scan_enable(uint8_t *buf,
                                                        uint8_t mode) {
  UINT8_TO_STREAM(buf, H4_TYPE_COMMAND);
  UINT16_TO_STREAM(buf, HCI_WRITE_SCAN_ENABLE);
  UINT8_TO_STREAM(buf, 1);

  UINT8_TO_STREAM(buf, mode);
  return HCI_H4_CMD_PREAMBLE_SIZE + 1;
}

uint16_t BtMessageGenerator::make_cmd_inquiry(uint8_t *buf, uint32_t lap,
                                              uint8_t len, uint8_t num) {
  UINT8_TO_STREAM(buf, H4_TYPE_COMMAND);
  UINT16_TO_STREAM(buf, HCI_INQUIRY);
  UINT8_TO_STREAM(buf, 5);

  UINT8_TO_STREAM(buf, (lap & 0xFF));          // lap 0x33 <- 0x9E8B33
  UINT8_TO_STREAM(buf, ((lap >> 8) & 0xFF));   // lap 0x8B
  UINT8_TO_STREAM(buf, ((lap >> 16) & 0xFF));  // lap 0x9E
  UINT8_TO_STREAM(buf, len);
  UINT8_TO_STREAM(buf, num);
  return HCI_H4_CMD_PREAMBLE_SIZE + 5;
}

uint16_t BtMessageGenerator::make_cmd_inquiry_cancel(uint8_t *buf) {
  UINT8_TO_STREAM(buf, H4_TYPE_COMMAND);
  UINT16_TO_STREAM(buf, HCI_INQUIRY_CANCEL);
  UINT8_TO_STREAM(buf, 0);

  return HCI_H4_CMD_PREAMBLE_SIZE + 0;
}

uint16_t BtMessageGenerator::make_cmd_remote_name_request(
    uint8_t *buf, struct bd_addr_t bd_addr, uint8_t psrm, uint16_t clkofs) {
  UINT8_TO_STREAM(buf, H4_TYPE_COMMAND);
  UINT16_TO_STREAM(buf, HCI_REMOTE_NAME_REQUEST);
  UINT8_TO_STREAM(buf, 6 + 1 + 1 + 2);

  BDADDR_TO_STREAM(buf, bd_addr.addr);
  UINT8_TO_STREAM(buf, psrm);     // Page_Scan_Repetition_Mode
  UINT8_TO_STREAM(buf, 0);        // Reserved
  UINT16_TO_STREAM(buf, clkofs);  // Clock_Offset
  return HCI_H4_CMD_PREAMBLE_SIZE + 10;
}

uint16_t BtMessageGenerator::make_cmd_create_connection(
    uint8_t *buf, struct bd_addr_t bd_addr, uint16_t pt, uint8_t psrm,
    uint16_t clkofs, uint8_t ars) {
  UINT8_TO_STREAM(buf, H4_TYPE_COMMAND);
  UINT16_TO_STREAM(buf, HCI_CREATE_CONNECTION);
  UINT8_TO_STREAM(buf, 6 + 2 + 1 + 1 + 2 + 1);

  BDADDR_TO_STREAM(buf, bd_addr.addr);
  UINT16_TO_STREAM(buf, pt);      // Packet_Type
  UINT8_TO_STREAM(buf, psrm);     // Page_Scan_Repetition_Mode
  UINT8_TO_STREAM(buf, 0);        // Reserved
  UINT16_TO_STREAM(buf, clkofs);  // Clock_Offset
  UINT8_TO_STREAM(buf, ars);      // Allow_Role_Switch
  return HCI_H4_CMD_PREAMBLE_SIZE + 13;
}

uint16_t BtMessageGenerator::make_l2cap_single_packet(uint8_t *buf,
                                                      uint16_t channel_id,
                                                      uint8_t *data,
                                                      uint16_t len) {
  UINT16_TO_STREAM(buf, len);
  UINT16_TO_STREAM(buf, channel_id);  // 0x0001=Signaling channel
  ARRAY_TO_STREAM(buf, data, len);
  return 2 + 2 + len;
}

uint16_t BtMessageGenerator::make_acl_l2cap_single_packet(
    uint8_t *buf, uint16_t connection_handle, uint8_t packet_boundary_flag,
    uint8_t broadcast_flag, uint16_t channel_id, uint8_t *data, uint8_t len) {
  uint8_t *l2cap_buf = buf + HCI_H4_ACL_PREAMBLE_SIZE;
  uint16_t l2cap_len =
      make_l2cap_single_packet(l2cap_buf, channel_id, data, len);

  UINT8_TO_STREAM(buf, H4_TYPE_ACL);
  UINT8_TO_STREAM(buf, connection_handle & 0xFF);
  UINT8_TO_STREAM(buf, ((connection_handle >> 8) & 0x0F) |
                           packet_boundary_flag << 4 | broadcast_flag << 6);
  UINT16_TO_STREAM(buf, l2cap_len);

  return HCI_H4_ACL_PREAMBLE_SIZE + l2cap_len;
}
