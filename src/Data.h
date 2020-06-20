/*
 * Data.h
 *
 *  Created on: 1 jun. 2020
 *      Author: gusa
 */

#ifndef SRC_DATA_H_
#define SRC_DATA_H_

#include <esp32-hal-log.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

class Data {
 public:
  enum Type : uint8_t {
    RESET = 0,
    FROM_BT = 1,
    READ_ADDRESS = 2,
    WRITE_LOCAL_NAME = 3,
    WRITE_DEVICE_CLASS = 4,
    SCAN_ENABLE = 5,
    INQUIRY = 6,
    REMOTE_NAME_REQUEST = 7,
    INQUIRY_CANCEL = 8,
    CREATE_CONNECTION = 9,
    L2CAP_CONNECTION_REQUEST = 10,
    L2CAP_SET_LEDS = 11,
    L2CAP_SET_REPORTING_MODE = 12,
    L2CAP_WRITE_MEMORY = 13,
    L2CAP_READ_MEMORY = 14,
    L2CAP_CONFIGURATION_REQUEST = 15,
    L2CAP_CONFIGURATION_RESPONSE = 16
  };

  uint8_t *data;
  uint16_t len;
  Type type;

  Data();
  Data(uint8_t *data, uint16_t len, Type type);
  virtual ~Data();
};

#endif /* SRC_DATA_H_ */
