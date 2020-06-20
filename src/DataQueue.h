/*
 * DataQueue.h
 *
 *  Created on: 31 may. 2020
 *      Author: gusa
 */

#ifndef SRC_DATAQUEUE_H_
#define SRC_DATAQUEUE_H_

#include <esp_err.h>
#include <freertos/FreeRTOS.h>
#include <freertos/semphr.h>
#include <src/Data.h>
#include <stddef.h>
#include <stdint.h>


class DataQueue {
 public:
  explicit DataQueue(int queueId);
  bool ContainsDataBlocking();
  esp_err_t Queue(uint8_t *data, uint16_t len, Data::Type type);
  esp_err_t QueueFromISR(uint8_t *data, uint16_t len, Data::Type type);

  bool DeQueueData(Data **dataObj);
  virtual ~DataQueue();

 private:
  int queueId;
  xQueueHandle queue = NULL;
};

#endif /* SRC_DATAQUEUE_H_ */
