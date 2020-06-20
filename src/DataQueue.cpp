/*
 * DataQueue.cpp
 *
 *  Created on: 31 may. 2020
 *      Author: gusa
 */

#include <esp32-hal-log.h>
#include <freertos/FreeRTOS.h>
#include <freertos/queue.h>
#include <freertos/task.h>
#include <src/DataQueue.h>
#include <string.h>

#define QUEUE_SIZE 32

DataQueue::DataQueue(int queueId) {
  this->queueId = queueId;
  this->queue = xQueueCreate(QUEUE_SIZE, sizeof(Data *));
  if (this->queue == NULL) {
    log_e("xQueueCreate(_tx_queue) failed");
    return;
  }
}

bool DataQueue::DeQueueData(Data **dataObj) {
  bool result = pdTRUE == xQueueReceive(queue, dataObj, 0);

  return result;
}

bool DataQueue::ContainsDataBlocking() {
  UBaseType_t result = uxQueueMessagesWaiting(queue);

  return result != 0;
}

esp_err_t DataQueue::Queue(uint8_t *data, uint16_t len, Data::Type type) {
  if (!data || !len) {
    log_w("No data provided");
    return ESP_OK;
  }

  Data *dataObj = new Data(data, len, type);

  if (queue != NULL && xQueueSend(queue, &dataObj, portMAX_DELAY) != pdPASS) {
    log_e("xQueueSend failed");
    delete dataObj;
    return ESP_FAIL;
  }

  return ESP_OK;
}

esp_err_t DataQueue::QueueFromISR(uint8_t *data, uint16_t len,
                                  Data::Type type) {
  if (!data || !len) {
    log_w("No data provided");
    return ESP_OK;
  }

  Data *dataObj = new Data(data, len, type);

  BaseType_t xHigherPriorityTaskWoken = pdFALSE;

  if (queue != NULL &&
      xQueueSendFromISR(queue, &dataObj, &xHigherPriorityTaskWoken) != pdPASS) {
    log_e("xQueueSend failed");
    delete dataObj;
    if (xHigherPriorityTaskWoken) {
      taskYIELD();
    }
    return ESP_FAIL;
  }

  if (xHigherPriorityTaskWoken) {
    taskYIELD();
  }
  return ESP_OK;
}

DataQueue::~DataQueue() { log_d("Deleted DataQueue"); }
