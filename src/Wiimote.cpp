#include <esp32-hal-bt.h>
#include <esp32-hal-log.h>
#include <esp_bt.h>
#include <freertos/FreeRTOS.h>
#include <freertos/queue.h>
#include <src/BtMessageGenerator.h>
#include <src/Data.h>
#include <src/DataQueue.h>
#include <src/Wiimote.h>
#include <src/utils/Utils.h>

static bool hciInit = false;
static DataQueue* rxQueue = NULL;
static DataQueue* txQueue = NULL;
static BtMessageGenerator* btMessageGenerator = NULL;

static void StartScan() {
  log_e("Start scan");
  uint8_t tmp_data[256];
  uint16_t len = btMessageGenerator->make_cmd_reset(tmp_data);
  txQueue->Queue(tmp_data, len, Data::Type::RESET);
  log_d("queued reset => %s", Utils::FormatHex(tmp_data, len));
}

/**
 * callback
 */
static void NotifyHostSendAvailable(void) {
  if (!hciInit) {
    StartScan();
    hciInit = true;
  }
}

static int NotifyHostRecv(uint8_t* data, uint16_t len) {
  // return rxQueue->QueueFromISR(data, len, Data::Type::FROM_BT);
  return rxQueue->Queue(data, len, Data::Type::FROM_BT);
}

Wiimote::Wiimote() {
  rxQueue = new DataQueue(1);
  txQueue = new DataQueue(2);
  btMessageGenerator = new BtMessageGenerator();
  this->L2CapManager =
      new L2CapCommunicationManager(rxQueue, txQueue, btMessageGenerator);

  // TODO(Jorge): REMOVE THIS DEPENDENCY
  this->HCICommManager = new HCICommunicationManager(
      rxQueue, txQueue, this->L2CapManager, btMessageGenerator);
}

static const esp_vhci_host_callback_t callback = {NotifyHostSendAvailable,
                                                  NotifyHostRecv};

void Wiimote::RegisterCallback(uint8_t number,
                               L2CapCommunicationManager::WiimoteCallback cb) {
  this->L2CapManager->RegisterCallback(number, cb);
}

void Wiimote::SetLed(uint8_t led) { this->L2CapManager->SetLed(led); }

void Wiimote::Init() {
  if (!btStart()) {
    log_e("btStart failed");
    return;
  }

  esp_err_t ret = esp_bt_sleep_disable();
  if (ret != ESP_OK) {
    log_e("Bluetooth Sleep Disable Failed: %s", esp_err_to_name(ret));
    return;
  }

  ret = esp_vhci_host_register_callback(&callback);

  if (ret != ESP_OK) {
    log_e("esp_vhci_host_register_callback failed: %d %s", ret,
          esp_err_to_name(ret));
    return;
  }
}

void Wiimote::Handle() {
  if (!btStarted()) {
    log_d("btStarted no");
    return;
  }
  Data* dataOb = NULL;
  if (txQueue != NULL && txQueue->ContainsDataBlocking()) {
    if (esp_vhci_host_check_send_available()) {
      if (txQueue->DeQueueData(&dataOb)) {
        log_d("SEND %d => %s ", dataOb->type,
              Utils::FormatHex(dataOb->data, dataOb->len));
        esp_vhci_host_send_packet(dataOb->data, dataOb->len);
        if (dataOb != NULL) {
          delete dataOb;
          dataOb = NULL;
        }
      }
    }
  }

  if (rxQueue != NULL && rxQueue->ContainsDataBlocking()) {
    if (rxQueue->DeQueueData(&dataOb)) {
      switch (dataOb->data[0]) {
        case HCICommunicationManager::HCI_ID:
          this->HCICommManager->ProcessHciEvent(
              dataOb->data[1], dataOb->data[2], dataOb->data + 3);
          break;
        case 0x02:
          this->L2CapManager->ProcessAclData(dataOb->data + 1, dataOb->len - 1);
          break;
        default:
          log_d("**** !!! Not HCI Event !!! ****");
          log_d("Not HCI Event len=%d data=%s", dataOb->len,
                Utils::FormatHex(dataOb->data, dataOb->len));
      }
      if (dataOb != NULL) {
        delete dataOb;
        dataOb = NULL;
      }
    }
  }
}

Wiimote::~Wiimote() {
  log_d("Deleted Wiimote");

  if (rxQueue != NULL) {
    delete rxQueue;
    rxQueue = NULL;
  }

  if (txQueue != NULL) {
    delete txQueue;
    txQueue = NULL;
  }

  if (btMessageGenerator != NULL) {
    delete btMessageGenerator;
    btMessageGenerator = NULL;
  }
}
