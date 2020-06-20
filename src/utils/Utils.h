/*
 * Utils.h
 *
 *  Created on: 5 jun. 2020
 *      Author: gusa
 */

#ifndef SRC_UTILS_UTILS_H_
#define SRC_UTILS_UTILS_H_

#include <stdint.h>

class Utils {
 public:
  Utils();
  static char* FormatHex(uint8_t* data, uint16_t len);

  virtual ~Utils();
};

#endif /* SRC_UTILS_UTILS_H_ */
