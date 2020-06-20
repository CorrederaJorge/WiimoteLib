/**
 * Creative Common - BY: Attribution Jorge Corredera 2020
 * Data.cpp
 *
 *  Created on: 1 jun. 2020
 *      Author: Jorge Corredera
 */

#include <src/Data.h>

Data::Data() {
  this->data = NULL;
  this->len = -1;
  this->type = Data::Type::RESET;
}

Data::Data(uint8_t *data, uint16_t len, Type type) {
  this->data = reinterpret_cast<uint8_t *>(malloc(sizeof(uint8_t) * len));
  this->len = len;
  this->type = type;
  if (!this->data) {
    log_e("lendata Malloc Failed!");
  }

  memcpy(this->data, data, len);
}

Data::~Data() {
  if (this->data != NULL) {
    free(this->data);
    this->data = NULL;
    this->len = 0;
  }
}
