#ifndef _WIIMOTE_H_
#define _WIIMOTE_H_

#include <esp_bt.h>
#include <src/hci/HCICommunicationManager.h>

#include <cstdint>

#include "src/DataQueue.h"

typedef void (*wiimote_callback_t)(uint8_t number, uint8_t *, size_t);

class Wiimote {
 public:
  Wiimote();
  void Init();
  void Handle();
  virtual ~Wiimote();
  int NotifyHostRecv(uint8_t *data, uint16_t len);
  void NotifyHostSendAvailable(void);
  void RegisterCallback(uint8_t number,
                        L2CapCommunicationManager::WiimoteCallback cb);
  void SetLed(uint8_t led);

 private:
  HCICommunicationManager *HCICommManager;
  L2CapCommunicationManager *L2CapManager;
  void StartScan();
};

#endif
