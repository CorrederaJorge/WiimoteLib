/*
 * L2CapConnectionList.h
 *
 *  Created on: 11 jun. 2020
 *      Author: gusa
 */

#ifndef SRC_L2CAP_L2CAPCONNECTIONLIST_H_
#define SRC_L2CAP_L2CAPCONNECTIONLIST_H_

#include <src/l2Cap/L2CapConnection.h>

class L2CapConnectionList {
#define L2CAP_CONNECTION_LIST_SIZE 8

 public:
  L2CapConnectionList();
  int l2cap_connection_add(L2CapConnection *l2cap_connection);
  int l2cap_connection_find_by_psm(uint16_t connection_handle, uint16_t psm);
  int l2cap_connection_find_by_local_cid(uint16_t connection_handle,
                                         uint16_t local_cid);
  L2CapConnection *Get(int id);

  virtual ~L2CapConnectionList();

 private:
  int l2cap_connection_size = 0;
  L2CapConnection *l2cap_connection_list[L2CAP_CONNECTION_LIST_SIZE];

  void Clear();
};

#endif /* SRC_L2CAP_L2CAPCONNECTIONLIST_H_ */
