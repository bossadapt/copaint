#include "copaint_common.hpp"
#include "olcPGEX_Network.h"
#include <SFML/Graphics/Color.hpp>
#include <SFML/Graphics/Image.hpp>
#include <cstdio>
#include <deque>
#include <memory>
#include <mutex>
#include <stack>
#include <unordered_map>
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
      std::shared_ptr<olc::net::connection<NetMessage>> client) override {
    canvasClientsLock.lock();

    activeClients.erase(client->GetID());
    canvasClientsLock.unlock();
  }
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
      canvasClientsLock.lock();

      if (!activeClients.empty()) {
        printf("sending existing canvas to %d\n", client->GetID());
        // requesting canvas status
        olc::net::message<NetMessage> canvasRequest;
        canvasRequest.header.id = NetMessage::Canvas_RequestCanvas;
        clientsNeedingCanvas.push(client);
        for (auto curClient : activeClients) {
          MessageClient(curClient.second, canvasRequest);
        }
        // sending a request to all that were already connected;
      } else {
        printf("sending empty canvas to %d\n", client->GetID());
        sf::Image emptyCanvas;
        emptyCanvas.create(WINDOW_WIDTH, WINDOW_HEIGHT, sf::Color::White);
        // Prepare message
        olc::net::message<NetMessage> canvasResponse;
        canvasResponse.header.id = NetMessage::Canvas_AddCanvas;

        // Get raw pixel data
        const sf::Uint8 *pixels = emptyCanvas.getPixelsPtr();
        uint32_t width = WINDOW_WIDTH;
        uint32_t height = WINDOW_HEIGHT;
        uint32_t size = width * height * 4; // 4 bytes per pixel (RGBA)

        // Send pixel data in reverse (because olc::net::message is a stack)
        for (int i = size - 1; i >= 0; --i) {
          canvasResponse << pixels[i];
        }
        canvasResponse << height << width << size;

        MessageClient(client, canvasResponse);
        activeClients[client->GetID()] = client;
      }
      canvasClientsLock.unlock();

      break;
    }
    case NetMessage::Client_UnregisterWithServer: {
      break;
    }
    case NetMessage::Canvas_AddCanvas: {
      printf("add canvas called\n");
      canvasClientsLock.lock();
      while (!clientsNeedingCanvas.empty()) {
        printf("sending recieved canvas to %d\n",
               clientsNeedingCanvas.top()->GetID());
        MessageClient(clientsNeedingCanvas.top(), msg);
        printf("finsihed sending that\n");
        activeClients[clientsNeedingCanvas.top()->GetID()] =
            clientsNeedingCanvas.top();
        clientsNeedingCanvas.pop();
        printf("popped and added\n");
      }
      canvasClientsLock.unlock();
      break;
    }
    case NetMessage::Canvas_AddStrokes: {
      MessageAllClients(msg, client);
      break;
    }
    }
  }

private:
  std::pmr::unordered_map<uint32_t,
                          std::shared_ptr<olc::net::connection<NetMessage>>>
      activeClients;
  std::mutex canvasClientsLock;
  std::stack<std::shared_ptr<olc::net::connection<NetMessage>>>
      clientsNeedingCanvas;
};

int main() {
  CoopPaint_server server(60000);
  server.Start();

  while (1) {
    server.Update(-1, true);
  }
  return 0;
}