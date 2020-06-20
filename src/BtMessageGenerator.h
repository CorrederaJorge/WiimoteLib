/*
 * BtMessageGenerator.h
 *
 *  Created on: 13 jun. 2020
 *      Author: gusa
 */

#ifndef SRC_BTMESSAGEGENERATOR_H_
#define SRC_BTMESSAGEGENERATOR_H_

#include <stdint.h>

class BtMessageGenerator {
#define HCI_OK 0x00
#define HCI_H4_CMD_PREAMBLE_SIZE 4
#define HCI_H4_ACL_PREAMBLE_SIZE (5)

/*  HCI Command opcode group field(OGF) */
#define HCI_GRP_LINK_CONT_CMDS (0x01 << 10)          /* 0x0400 */
#define HCI_GRP_HOST_CONT_BASEBAND_CMDS (0x03 << 10) /* 0x0C00 */
#define HCI_GRP_INFO_PARAMS_CMDS (0x04 << 10)

// OGF + OCF
#define HCI_RESET (0x0003 | HCI_GRP_HOST_CONT_BASEBAND_CMDS)
#define HCI_READ_BD_ADDR (0x0009 | HCI_GRP_INFO_PARAMS_CMDS)
#define HCI_WRITE_LOCAL_NAME (0x0013 | HCI_GRP_HOST_CONT_BASEBAND_CMDS)
#define HCI_WRITE_CLASS_OF_DEVICE (0x0024 | HCI_GRP_HOST_CONT_BASEBAND_CMDS)
#define HCI_WRITE_SCAN_ENABLE (0x001A | HCI_GRP_HOST_CONT_BASEBAND_CMDS)
#define HCI_INQUIRY (0x0001 | HCI_GRP_LINK_CONT_CMDS)
#define HCI_INQUIRY_CANCEL (0x0002 | HCI_GRP_LINK_CONT_CMDS)
#define HCI_REMOTE_NAME_REQUEST (0x0019 | HCI_GRP_LINK_CONT_CMDS)
#define HCI_CREATE_CONNECTION (0x0005 | HCI_GRP_LINK_CONT_CMDS)

#define BD_ADDR_LEN (6)

#define STREAM_TO_BDADDR(a, p)              \
  {                                         \
    int ijk;                                \
    for (ijk = 0; ijk < BD_ADDR_LEN; ijk++) \
      a[BD_ADDR_LEN - 1 - ijk] = (p)[ijk];  \
  }

 public:
  struct bd_addr_t {
    uint8_t addr[BD_ADDR_LEN];
  };
  BtMessageGenerator();
  virtual ~BtMessageGenerator();

  uint16_t make_cmd_read_bd_addr(uint8_t *buf);

  uint16_t make_cmd_reset(uint8_t *buf);
  uint16_t make_cmd_write_local_name(uint8_t *buf, uint8_t *name, uint8_t len);

  uint16_t make_cmd_write_class_of_device(uint8_t *buf, uint8_t *cod);

  uint16_t make_cmd_write_scan_enable(uint8_t *buf, uint8_t mode);

  uint16_t make_cmd_inquiry(uint8_t *buf, uint32_t lap, uint8_t len,
                            uint8_t num);

  uint16_t make_cmd_inquiry_cancel(uint8_t *buf);

  uint16_t make_cmd_remote_name_request(uint8_t *buf, struct bd_addr_t bd_addr,
                                        uint8_t psrm, uint16_t clkofs);

  uint16_t make_cmd_create_connection(uint8_t *buf, struct bd_addr_t bd_addr,
                                      uint16_t pt, uint8_t psrm,
                                      uint16_t clkofs, uint8_t ars);

  uint16_t make_l2cap_single_packet(uint8_t *buf, uint16_t channel_id,
                                    uint8_t *data, uint16_t len);

  uint16_t make_acl_l2cap_single_packet(
      uint8_t *buf, uint16_t connection_handle, uint8_t packet_boundary_flag,
      uint8_t broadcast_flag, uint16_t channel_id, uint8_t *data, uint8_t len);
};

#endif /* SRC_BTMESSAGEGENERATOR_H_ */
