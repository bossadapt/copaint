#include "olcPGEX_Network.h"
#include <SFML/Graphics/Color.hpp>
#include <SFML/System/Vector2.hpp>
#include <cstdint>
#include <sys/types.h>
#include <vector>
enum class NetMessage : int32_t {
  Client_Accepted,

  Client_AssignID,
  // TODO: Client_AssignInitialCanvas,
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
  // Serialization
  friend olc::net::message<NetMessage> &
  operator<<(olc::net::message<NetMessage> &msg, const Stroke &stroke) {
    msg << stroke.start.x << stroke.start.y;
    msg << stroke.finish.x << stroke.finish.y;
    msg << stroke.color.r << stroke.color.g << stroke.color.b << stroke.color.a;
    msg << stroke.size;
    return msg;
  }

  // Deserialization (must reverse the order!)
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
    std::cout << "sending unique of:" << buffer.nUniqueID << std::endl;
    std::cout << "Sending a buffer size of" << uint32_t(buffer.strokes.size())
              << std::endl;

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
    std::cout << "Recieving a unique of" << buffer.nUniqueID << std::endl;
    std::cout << "Recieving a buffer size of" << strokeCount << std::endl;
    buffer.strokes.resize(strokeCount);
    for (auto &stroke : buffer.strokes) {
      msg >> stroke;
    }
    return msg;
  }
};