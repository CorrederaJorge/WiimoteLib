/*
 * L2CapConnectionList.cpp
 *
 *  Created on: 11 jun. 2020
 *      Author: gusa
 */

#include <esp32-hal-log.h>
#include <src/l2Cap/L2CapConnection.h>
#include <src/l2Cap/L2CapConnectionList.h>
#include <stddef.h>
#include <stdint.h>


L2CapConnectionList::L2CapConnectionList() {}

L2CapConnectionList::~L2CapConnectionList() { this->Clear(); }

int L2CapConnectionList::l2cap_connection_find_by_psm(
    uint16_t connection_handle, uint16_t psm) {
  for (int i = 0; i < L2CAP_CONNECTION_LIST_SIZE; i++) {
    L2CapConnection* c = this->l2cap_connection_list[i];
    if (connection_handle == c->connection_handle && psm == c->psm) {
      return i;
    }
  }

  log_d("Connection not found");
  return -1;
}

void L2CapConnectionList::Clear() {
  for (int i = 0; i < l2cap_connection_size; i++) {
    if (l2cap_connection_list[i] != NULL) {
      delete l2cap_connection_list[i];
      l2cap_connection_list[i] = NULL;
    }
  }
  l2cap_connection_size = 0;
}

int L2CapConnectionList::l2cap_connection_find_by_local_cid(
    uint16_t connection_handle, uint16_t local_cid) {
  for (int i = 0; i < L2CAP_CONNECTION_LIST_SIZE; i++) {
    L2CapConnection* c = l2cap_connection_list[i];
    if (connection_handle == c->connection_handle &&
        local_cid == c->local_cid) {
      return i;
    }
  }
  return -1;
}

L2CapConnection* L2CapConnectionList::Get(int id) {
  return this->l2cap_connection_list[id];
}

int L2CapConnectionList::l2cap_connection_add(
    L2CapConnection* l2cap_connection) {
  if (L2CAP_CONNECTION_LIST_SIZE == l2cap_connection_size) {
    return -1;
  }
  l2cap_connection_list[l2cap_connection_size++] = l2cap_connection;
  return l2cap_connection_size;
}
