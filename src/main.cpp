#include <M5Unified.h>

#include "PetDisplay.h"
#include "PetGame.h"

namespace {
PetDisplay display;
PetGame game;
uint32_t lastDraw = 0;
constexpr uint32_t kFrameIntervalMs = 33;
constexpr uint32_t kDisplayIdleTimeoutMs = 60000;
uint32_t lastInteraction = 0;
bool displaySleeping = false;
}  // namespace

void setup() {
  auto config = M5.config();
  M5.begin(config);
  M5.Display.setRotation(1);
  M5.Display.setBrightness(170);
  M5.Speaker.setVolume(72);
  M5.Power.setVibration(0);

  Serial.begin(115200);
  display.begin();
  game.begin();
  lastDraw = millis();
  lastInteraction = lastDraw;
  display.draw(game.state(), lastDraw);
}

void loop() {
  M5.update();
  const uint32_t now = millis();
  const uint8_t touchCount = M5.Touch.getCount();
  const auto touch = M5.Touch.getDetail();

  if (touchCount == 1 && touch.wasPressed()) {
    if (displaySleeping) {
      M5.Display.wakeup();
      displaySleeping = false;
      lastInteraction = now;
      lastDraw = now;
      display.draw(game.state(), now);
    } else {
      lastInteraction = now;
      game.handleAction(display.handleTouch(touch.x, touch.y, game.state()));
    }
  }
  game.update(now);

  if (!displaySleeping &&
      static_cast<uint32_t>(now - lastInteraction) >= kDisplayIdleTimeoutMs) {
    M5.Display.sleep();
    displaySleeping = true;
  }

  if (!displaySleeping &&
      static_cast<uint32_t>(now - lastDraw) >= kFrameIntervalMs) {
    display.draw(game.state(), now);
    lastDraw = now;
  }
  delay(10);
}
