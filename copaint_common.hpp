#include "olcPGEX_Network.h"
#include <SFML/Graphics/Color.hpp>
#include <SFML/System/Vector2.hpp>
#include <cstdint>
#include <sys/types.h>
#include <vector>
enum class NetMessage : int32_t {
  Client_Accepted,

  Client_AssignID,
  Client_RegisterWithServer,
  Client_UnregisterWithServer,

  Canvas_AddPainter,
  Canvas_RemovePainter,

  Canvas_AddStrokes
};
struct Stroke {
  sf::Vector2<int> start;
  sf::Vector2<int> finish;
  sf::Color color;
  sf::Uint8 size;
  friend olc::net::message<NetMessage> &
  operator<<(olc::net::message<NetMessage> &msg, const Stroke &stroke) {
    msg << stroke.start.x << stroke.start.y;
    msg << stroke.finish.x << stroke.finish.y;
    msg << stroke.color.r << stroke.color.g << stroke.color.b << stroke.color.a;
    msg << stroke.size;
    return msg;
  }

  friend olc::net::message<NetMessage> &
  operator>>(olc::net::message<NetMessage> &msg, Stroke &stroke) {
    msg >> stroke.size;
    msg >> stroke.color.a >> stroke.color.b >> stroke.color.g >> stroke.color.r;
    msg >> stroke.finish.y >> stroke.finish.x;
    msg >> stroke.start.y >> stroke.start.x;
    return msg;
  }
};
struct StrokeBuffer {
  u_int32_t nUniqueID;
  std::vector<Stroke> strokes;
  friend olc::net::message<NetMessage> &
  operator<<(olc::net::message<NetMessage> &msg, const StrokeBuffer &buffer) {

    for (const auto &stroke : buffer.strokes) {
      msg << stroke;
    }
    msg << uint32_t(buffer.strokes.size());
    msg << buffer.nUniqueID;

    return msg;
  }

  friend olc::net::message<NetMessage> &
  operator>>(olc::net::message<NetMessage> &msg, StrokeBuffer &buffer) {
    msg >> buffer.nUniqueID;
    uint32_t strokeCount;
    msg >> strokeCount;
    buffer.strokes.resize(strokeCount);
    for (auto &stroke : buffer.strokes) {
      msg >> stroke;
    }
    return msg;
  }
};