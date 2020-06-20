/*
 * ScannedDevice.cpp
 *
 *  Created on: 6 jun. 2020
 *      Author: gusa
 */

#include <src/hci/ScannedDevice.h>

ScannedDevice::ScannedDevice(struct BtMessageGenerator::bd_addr_t bd_addr,
                             uint8_t psrm, uint16_t clkofs) {
  this->bd_addr = bd_addr;
  this->psrm = psrm;
  this->clkofs = clkofs;
}

ScannedDevice::ScannedDevice() {
  // TODO(Jorge): define bd_addr
  // this->bd_addr = {.addr = -1};
  this->psrm = 0;
  this->clkofs = 0;
}

ScannedDevice::~ScannedDevice() {}
