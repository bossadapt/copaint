#include "copaint_common.hpp"
#include <SFML/Config.hpp>
#include <SFML/Graphics.hpp>
#include <SFML/Graphics/Color.hpp>
#include <SFML/Graphics/Font.hpp>
#include <SFML/Graphics/RectangleShape.hpp>
#include <SFML/Graphics/RenderWindow.hpp>
#include <SFML/Graphics/Sprite.hpp>
#include <SFML/Graphics/Texture.hpp>
#include <SFML/System/Clock.hpp>
#include <SFML/System/Mutex.hpp>
#include <SFML/System/Time.hpp>
#include <SFML/System/Utf.hpp>
#include <SFML/System/Vector2.hpp>
#include <SFML/Window/Keyboard.hpp>
#include <SFML/Window/Mouse.hpp>
#include <iostream>
#include <ostream>
#include <sys/socket.h>
#include <vector>
#define WINDOW_WIDTH 800
#define WINDOW_HEIGHT 600
// actions that can be taken
//  click
class CoopPaint_client : olc::net::client_interface<NetMessage> {
public:
  CoopPaint_client()
      : window(sf::VideoMode(WINDOW_WIDTH, WINDOW_HEIGHT), "copaint") {

    canvas.create(WINDOW_WIDTH, WINDOW_HEIGHT, sf::Color::White);
  }
  void start() {

    // send 100 messages over the network a second with buffered stroke lengths
    sf::Clock bufferNetworkClock;
    sf::Time bufferLen = sf::milliseconds(10);
    // if server did not initialize in 5 seconds close with terminal message
    sf::Clock initializationTimeoutClock;
    sf::Time timeoutLength = sf::milliseconds(5000);
    sf::Font font;
    if (!networkPrep()) {
      std::cout << "Failed to set up client socket\n";
      return;
    }
    if (!font.loadFromFile("./assets/Freedom-10eM.ttf")) {
      std::cout << "Failed to load font from assets\n";
      return;
    }
    while (window.isOpen()) {
      // check all the window's events that were triggered since the last
      // iteration of the loop
      sf::Event event;
      updateFromNetwork();

      while (window.pollEvent(event)) {
        // "close requested" event: we close the window
        if (event.type == sf::Event::Closed)
          window.close();
      }
      if (initializationFinished) {
        std::cout << "Initialization finished";

        getLocalInput();
        if (bufferNetworkClock.getElapsedTime() > bufferLen &&
            !bufferedLocalStrokes.strokes.empty()) {
          updateNetwork();
          bufferNetworkClock.restart();
        }

        updateCanvas();

      } else if (initializationTimeoutClock.getElapsedTime() > timeoutLength) {
        std::cout << "Connection to server for initialization failed \n";
        return;
      } else {
        drawLoadingScreen(font);
      }
    }
  }
  bool networkPrep() {
    if (Connect("127.0.0.1", 60000)) {
      return true;
    }
    return false;
  }
  void updateNetwork() {
    olc::net::message<NetMessage> strokesUpdate;
    strokesUpdate.header.id = NetMessage::Canvas_AddStrokes;
    strokesUpdate << bufferedLocalStrokes;
    Send(strokesUpdate);
    bufferedLocalStrokes.strokes.clear();
  }
  // TODO: add brush size and color
  // TODO: send the canvas image to new joinees
  // TODO: set up rooms  to join
  // TODO: possibly add more tools
  void updateFromNetwork() {
    if (IsConnected()) {
      while (!Incoming().empty()) {
        auto msg = Incoming().pop_front().msg;

        switch (msg.header.id) {
        case NetMessage::Client_Accepted: {
          std::cout << "Server accepted client - you're in!\n";
          olc::net::message<NetMessage> reply;
          reply.header.id = NetMessage::Client_RegisterWithServer;
          Send(reply);
          break;
        }
        case NetMessage::Client_AssignID: {
          msg >> nUniqueID;
          std::cout << "Assigned CLient ID Recieved :" << nUniqueID << "\n";
          initializationFinished = true;
          bufferedLocalStrokes.nUniqueID = nUniqueID;
          break;
        }
        // case NetMessageTypes::Client_RegisterWithServer: handled by server
        // case NetMessageTypes::Client_UnregisterWithServer: handled by server
        case NetMessage::Canvas_AddPainter: {
          std::cout << "Painter has joined\n";
          break;
        }
        case NetMessage::Canvas_RemovePainter: {
          std::cout << "Painter has Left\n";
          break;
        }
        case NetMessage::Canvas_AddStrokes: {
          StrokeBuffer remoteStrokeBuffer;
          std::cout << "Trying to pull data out" << std::endl;
          msg >> remoteStrokeBuffer;
          std::cout << remoteStrokeBuffer.strokes[0].start.x << "|"
                    << remoteStrokeBuffer.strokes[0].start.y << std::endl;
          for (Stroke stroke : remoteStrokeBuffer.strokes) {
            drawPixels(stroke.start, stroke.finish, stroke.color, stroke.size);
          }
          break;
        }
        }
      }
    }
  }
  void getLocalInput() {
    if (sf::Mouse::isButtonPressed(sf::Mouse::Left)) {
      sf::Vector2<int> mouse = sf::Mouse::getPosition(window);
      if (mouse.x > 0 && mouse.y > 0 && mouse.x < canvas.getSize().x &&
          mouse.y < canvas.getSize().y) {
        if (isHolding) {
          bufferedLocalStrokes.strokes.push_back(
              {oldLocation, mouse, sf::Color::Black, 1});
          drawPixels(oldLocation, mouse, sf::Color::Black, 1);
        }
        isHolding = true;
        oldLocation = mouse;
      }
    } else {
      isHolding = false;
    }
  }
  void drawLoadingScreen(sf::Font font) {
    window.clear(sf::Color::White);
    sf::Text text;
    text.setString("Loading Canvas Connection...");
    text.setFont(font);
    text.setCharacterSize(24);
    text.setFillColor(sf::Color::Black);
    text.setStyle(sf::Text::Bold);
    text.setPosition(200, 200);
    window.draw(text);
    window.display();
  }
  void updateCanvas() {
    window.clear(sf::Color::White);
    canvasTexture.loadFromImage(canvas);
    canvasSprite.setTexture(canvasTexture);
    window.draw(canvasSprite);
    window.display();
  }
  // yoinked Bresenham's line algorithm
  void drawPixels(sf::Vector2<int> start, sf::Vector2<int> finish,
                  sf::Color color, sf::Uint8 size) {
    int m_new = 2 * (finish.y - start.y);
    int slope_error_new = m_new - (finish.x - start.x);
    for (int x = start.x, y = start.y; x <= finish.x; x++) {
      canvas.setPixel(x, y, color);

      // Add slope to increment angle formed
      slope_error_new += m_new;

      // Slope error reached limit, time to
      // increment y and update slope error.
      if (slope_error_new >= 0) {
        y++;
        slope_error_new -= 2 * (finish.x - start.x);
      }
    }
  }

  // void increaseWidth() {
  //   if (width < 255) {
  //     width++;
  //   }
  // }
  // void decreateWidth() {
  //   if (width > 1) {
  //     width--;
  //   }
  // }

private:
  bool isHolding;
  sf::Vector2<int> oldLocation;
  // graphics
  sf::RenderWindow window;
  sf::Image canvas;
  sf::Texture canvasTexture;
  sf::Sprite canvasSprite;
  //  client side variables
  sf::Uint8 strokeSize;
  u_int32_t nUniqueID = 0;

  // network
  bool initializationFinished = false;
  StrokeBuffer bufferedLocalStrokes;
};
int main() {
  CoopPaint_client coop;
  coop.start();

  return 0;
}
