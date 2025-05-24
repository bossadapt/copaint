#include "./network/net_common.h"
#include "network/net_client.h"
#include <iostream>

enum class CustomMsgTypes : uint32_t { FireBullet, MovePlayer };
class CustomClient : public olc::net::client_interface<CustomMsgTypes> {
public:
  bool FireBullet(float x, float y) {
    olc::net::message<CustomMsgTypes> msg;
    msg.header.id = CustomMsgTypes::FireBullet;
    msg << x << y;
    Send(msg);
  }
};
int main() {
  olc::net::message<CustomMsgTypes> msg;
  msg.header.id = CustomMsgTypes::FireBullet;

  int a = 1;
  bool b = true;
  float c = 3.13149f;
  struct {
    float x;
    float y;
  } d[5];
  msg << a << b << c << d;
  a = 99;
  b = false;
  c = 99.0f;
  msg << d << c << b << a;
}