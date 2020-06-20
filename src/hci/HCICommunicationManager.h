/*
 * HCICommunicationManager.h
 *
 *  Created on: 5 jun. 2020
 *      Author: gusa
 */

#ifndef SRC_HCI_HCICOMMUNICATIONMANAGER_H_
#define SRC_HCI_HCICOMMUNICATIONMANAGER_H_

#include <src/DataQueue.h>
#include <src/hci/ScannedDeviceList.h>
#include <src/l2Cap/L2CapCommunicationManager.h>
#include <stdint.h>

class HCICommunicationManager {
 public:
  static const uint8_t HCI_ID = 0x04;

  HCICommunicationManager(DataQueue* rxQueue, DataQueue* txQueue,
                          L2CapCommunicationManager* l2CapManager,
                          BtMessageGenerator* btMessageGenerator);
  void ProcessHciEvent(uint8_t event_code, uint8_t len, uint8_t* data);
  virtual ~HCICommunicationManager();

 private:
  DataQueue* rxQueue;
  DataQueue* txQueue;
  L2CapCommunicationManager* l2CapManager;
  ScannedDeviceList* sDeviceList;
  BtMessageGenerator* btMessageGenerator;
  bool wiimoteConnected = false;
  void ProcessCommandCompleteEvent(uint8_t len, uint8_t* data);
  void ProcessCommandStatusEvent(uint8_t len, uint8_t* data);
  void ProcessInquiryCompleteEvent(uint8_t len, uint8_t* data);
  void ProcessInquiryResultEvent(uint8_t len, uint8_t* data);
  void ProcessConnectionCompleteEvent(uint8_t len, uint8_t* data);
  void ProcessDisconnectionCompleteEvent(uint8_t len, uint8_t* data);
  void ProcessRemoteNameRequestCompleteEvent(uint8_t len, uint8_t* data);
  void StartScan();
};

#endif /* SRC_HCI_HCICOMMUNICATIONMANAGER_H_ */
