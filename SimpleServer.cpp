#include "./network/net_common.h"
#include "network/net_connection.h"
#include "network/net_server.h"
#include <cstdint>
#include <cstdio>
#include <iostream>
#include <memory>
enum class CustomMsgTypes : uint32_t {
  ServerAccept,
  ServerDeny,
  ServerPing,
  MessageAll,
  ServerMessage,
};
class CustomServer : public olc::net::server_interface<CustomMsgTypes> {
public:
  CustomServer(uint32_t nPort)
      : olc::net::server_interface<CustomMsgTypes>(nPort) {}

protected:
  virtual bool OnClientConnect(
      std::shared_ptr<olc::net::connection<CustomMsgTypes>> client) {
    return true;
  }
  virtual void OnClientDisconnect(
      std::shared_ptr<olc::net::connection<CustomMsgTypes>> client,
      olc::net::message<CustomMsgTypes> msg) {}
};

int main() {
  printf("started");
  CustomServer server(60000);
  server.Start();
  while (1) {
    server.Update(-1, true);
  }
}