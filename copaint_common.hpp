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
};
struct StrokeBuffer {
  u_int32_t nUniqueID;
  std::vector<Stroke> strokes;
};