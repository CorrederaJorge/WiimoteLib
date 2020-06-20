/*
 * ScannedDeviceList.h
 *
 *  Created on: 6 jun. 2020
 *      Author: gusa
 */

#ifndef SRC_HCI_SCANNEDDEVICELIST_H_
#define SRC_HCI_SCANNEDDEVICELIST_H_

#include <src/BtMessageGenerator.h>
#include <src/hci/ScannedDevice.h>

class ScannedDeviceList {
#define SCANNED_DEVICE_LIST_SIZE 16

 public:
  ScannedDeviceList();
  int ScannedDeviceFind(BtMessageGenerator::bd_addr_t *bd_addr);
  int ScannedDeviceAdd(ScannedDevice scanned_device);
  ScannedDevice Get(int idx);
  virtual ~ScannedDeviceList();

 private:
  int scanned_device_list_size = 0;

  ScannedDevice scanned_device_list[SCANNED_DEVICE_LIST_SIZE];
};

#endif /* SRC_HCI_SCANNEDDEVICELIST_H_ */
