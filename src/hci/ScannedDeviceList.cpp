/*
 * ScannedDeviceList.cpp
 *
 *  Created on: 6 jun. 2020
 *      Author: gusa
 */
#include <src/hci/ScannedDevice.h>
#include <src/hci/ScannedDeviceList.h>
#include <string.h>

ScannedDeviceList::ScannedDeviceList() {}

ScannedDeviceList::~ScannedDeviceList() {
  for (int i = 0; i < scanned_device_list_size; i++) {
    if (&scanned_device_list[i] != NULL) {
      delete &scanned_device_list[i];
    }
  }

  scanned_device_list_size = 0;
}

ScannedDevice ScannedDeviceList::Get(int idx) {
  return this->scanned_device_list[idx];
}

int ScannedDeviceList::ScannedDeviceAdd(ScannedDevice scanned_device) {
  if (SCANNED_DEVICE_LIST_SIZE == scanned_device_list_size) {
    return -1;
  }
  scanned_device_list[scanned_device_list_size++] = scanned_device;
  return scanned_device_list_size;
}

int ScannedDeviceList::ScannedDeviceFind(
    BtMessageGenerator::bd_addr_t *bd_addr) {
  for (int i = 0; i < scanned_device_list_size; i++) {
    ScannedDevice *c = &scanned_device_list[i];
    if (memcmp(&bd_addr->addr, c->bd_addr.addr, BD_ADDR_LEN) == 0) {
      return i;
    }
  }

  return -1;
}
