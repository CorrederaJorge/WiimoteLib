/*
 * Utils.cpp
 *
 *  Created on: 5 jun. 2020
 *      Author: gusa
 */

#include <src/utils/Utils.h>
#include <stdio.h>

Utils::Utils() {}

Utils::~Utils() {}

#define FORMAT_HEX_MAX_BYTES 30

static char formatHexBuffer[FORMAT_HEX_MAX_BYTES * 3 + 3 + 1];

char* Utils::FormatHex(uint8_t* data, uint16_t len) {
  for (uint16_t i = 0; i < len && i < FORMAT_HEX_MAX_BYTES; i++) {
    snprintf(formatHexBuffer + 3 * i, len, "%02X ", data[i]);
  }

  if (FORMAT_HEX_MAX_BYTES < len) {
    snprintf(formatHexBuffer + 3 * FORMAT_HEX_MAX_BYTES, FORMAT_HEX_MAX_BYTES,
             "...");
  }

  return formatHexBuffer;
}
