/*
 * L2CapConnection.cpp
 *
 *  Created on: 11 jun. 2020
 *      Author: gusa
 */

#include <src/l2Cap/L2CapConnection.h>

L2CapConnection::L2CapConnection(uint16_t connection_handle, uint16_t psm,
                                 uint16_t local_cid, uint16_t remote_cid) {
  this->connection_handle = connection_handle;
  this->psm = psm;
  this->local_cid = local_cid;
  this->remote_cid = remote_cid;
}

L2CapConnection::~L2CapConnection() {}
