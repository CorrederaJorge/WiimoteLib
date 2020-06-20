/*
 * L2CapConnection.h
 *
 *  Created on: 11 jun. 2020
 *      Author: gusa
 */

#ifndef SRC_L2CAP_L2CAPCONNECTION_H_
#define SRC_L2CAP_L2CAPCONNECTION_H_

#include <stdint.h>

class L2CapConnection {
 public:
  uint16_t connection_handle;
  uint16_t psm;
  uint16_t local_cid;
  uint16_t remote_cid;

  L2CapConnection(uint16_t connection_handle, uint16_t psm, uint16_t local_cid,
                  uint16_t remote_cid);
  virtual ~L2CapConnection();

 private:
};

#endif /* SRC_L2CAP_L2CAPCONNECTION_H_ */
