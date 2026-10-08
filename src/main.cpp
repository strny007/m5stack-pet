#include <M5Unified.h>

#include "PetDisplay.h"
#include "PetGame.h"

namespace {
PetDisplay display;
PetGame game;
uint32_t lastDraw = 0;
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
  display.draw(game.state(), lastDraw);
}

void loop() {
  M5.update();
  const uint32_t now = millis();
  const auto touch = M5.Touch.getDetail();
  if (touch.wasPressed()) {
    game.handleAction(display.handleTouch(touch.x, touch.y, game.state()));
  }
  game.update(now);

  if (static_cast<uint32_t>(now - lastDraw) >= 400) {
    display.draw(game.state(), now);
    lastDraw = now;
  }
  delay(10);
}
