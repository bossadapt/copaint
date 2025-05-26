#include "copaint_common.hpp"
#include "network/net_server.h"
class CoopPaint_server : public olc::net::server_interface<NetMessage> {
public:
  CoopPaint_server(uint16_t nport)
      : olc::net::server_interface<NetMessage>(nport) {}

protected:
  bool OnClientConnect(
      std::shared_ptr<olc::net::connection<NetMessage>> client) override {
    return true;
  }
  void OnClientValidated(
      std::shared_ptr<olc::net::connection<NetMessage>> client) override {
    olc::net::message<NetMessage> msg;
    msg.header.id = NetMessage::Client_Accepted;
    client->Send(msg);
  }
  void OnClientDisconnect(
      std::shared_ptr<olc::net::connection<NetMessage>> client) override {}
  void OnMessage(std::shared_ptr<olc::net::connection<NetMessage>> client,
                 olc::net::message<NetMessage> &msg) override {
    switch (msg.header.id) {
    case NetMessage::Client_RegisterWithServer: {
      olc::net::message<NetMessage> msgWithID;
      msgWithID.header.id = NetMessage::Client_AssignID;
      msgWithID << client->GetID();
      MessageClient(client, msgWithID);

      olc::net::message<NetMessage> msgWithNewPerson;
      msgWithNewPerson.header.id = NetMessage::Canvas_AddPainter;
      msgWithNewPerson << client->GetID();
      // sends to all including this person
      MessageAllClients(msgWithNewPerson);

      // TODO: send them the existing canvas(maybe keep state from the host)
      break;
    }
    case NetMessage::Client_UnregisterWithServer: {
      break;
    }
    case NetMessage::Canvas_AddStrokes: {
      MessageAllClients(msg, client);
      break;
    }
    }
  }
};

int main() {
  CoopPaint_server server(60000);
  server.Start();

  while (1) {
    server.Update(-1, true);
  }
  return 0;
}