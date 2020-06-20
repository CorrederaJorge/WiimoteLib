/*
 * ScannedDevice.h
 *
 *  Created on: 6 jun. 2020
 *      Author: gusa
 */

#ifndef SRC_HCI_SCANNEDDEVICE_H_
#define SRC_HCI_SCANNEDDEVICE_H_

#include <src/BtMessageGenerator.h>
#include <stdint.h>

class ScannedDevice {
 public:
  ScannedDevice(struct BtMessageGenerator::bd_addr_t bd_addr, uint8_t psrm,
                uint16_t clkofs);
  ScannedDevice();
  struct BtMessageGenerator::bd_addr_t bd_addr;
  uint8_t psrm;
  uint16_t clkofs;
  virtual ~ScannedDevice();
};

#endif /* SRC_HCI_SCANNEDDEVICE_H_ */
