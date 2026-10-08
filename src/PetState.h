#pragma once

#include <Arduino.h>

struct PetState {
  uint8_t hunger = 72;
  uint8_t happiness = 78;
  uint8_t energy = 74;
  uint8_t cleanliness = 82;
  bool sleeping = false;
  bool muted = false;
  uint8_t avatar = 0;
  String name = "Mochi";
  int batteryLevel = 0;
  bool batteryLevelValid = false;
  bool batteryCharging = false;
  uint32_t reactionUntil = 0;
  float tiltX = 0;
  float tiltY = 0;
  uint64_t aliveMs = 0;
};
