/*
 * L2CapManager.h
 *
 *  Created on: 6 jun. 2020
 *      Author: gusa
 */

#ifndef SRC_L2CAP_L2CAPCOMMUNICATIONMANAGER_H_
#define SRC_L2CAP_L2CAPCOMMUNICATIONMANAGER_H_

#include <esp32-hal-log.h>
#include <src/BtMessageGenerator.h>
#include <src/DataQueue.h>
#include <src/l2Cap/L2CapConnectionList.h>
#include <stdint.h>

class L2CapCommunicationManager {
 public:
  typedef void (*WiimoteCallback)(uint8_t number, uint8_t*, size_t);

  enum address_space_t { EEPROM_MEMORY, CONTROL_REGISTER };

  L2CapCommunicationManager(DataQueue* rxQueue, DataQueue* txQueue,
                            BtMessageGenerator* btMessageGenerator);
  void L2capConnect(uint16_t connection_handle, uint16_t psm);
  void ProcessAclData(uint8_t* data, size_t len);
  void RegisterCallback(uint8_t number, WiimoteCallback cb);
  virtual ~L2CapCommunicationManager();

  void SetLed(uint8_t leds);

 private:
  DataQueue* rxQueue;
  DataQueue* txQueue;
  L2CapConnectionList* l2CapConnectionList;
  BtMessageGenerator* btMessageGenerator;
  WiimoteCallback WiimoteCallbackList[4];
  bool wiimoteConnected = false;
  void ProcessL2capData_(uint16_t connection_handle, uint16_t channel_id,
                         uint8_t* data, uint16_t len);
  void ProcessL2capConnectionResponse_(uint16_t connection_handle,
                                       uint8_t* data);
  void ProcessL2capConfigurationRequest_(uint16_t connection_handle,
                                         uint8_t* data);
  void ProcessExtensionControllerReports_(uint16_t connection_handle,
                                          uint16_t channel_id, uint8_t* data,
                                          uint16_t len);
  void SetLed_(uint16_t connection_handle, uint8_t leds);
  void SetReportingMode_(uint16_t connection_handle, uint8_t reporting_mode,
                         bool continuous);
  void WriteMemory_(uint16_t connection_handle, address_space_t as,
                    uint32_t offset, uint8_t size, const uint8_t* d);
  uint8_t AddressSpace_(address_space_t as);
  void ReadMemory_(uint16_t connection_handle, address_space_t as,
                   uint32_t offset, uint16_t size);
  void L2capConnect_(uint16_t connection_handle, uint16_t psm,
                     uint16_t source_cid);
  void ProcessReport_(uint8_t* data, uint16_t len);
};

#endif /* SRC_L2CAP_L2CAPCOMMUNICATIONMANAGER_H_ */
