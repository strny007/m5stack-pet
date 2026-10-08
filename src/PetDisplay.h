#pragma once

#include <M5Unified.h>

#include "PetState.h"

enum class PetAction {
  none,
  feed,
  play,
  wash,
  toggleSleep,
  toggleMute,
  cycleAvatar,
  rename,
};

class PetDisplay {
 public:
  void begin();
  void draw(const PetState& pet, uint32_t now);
  PetAction handleTouch(int x, int y, PetState& pet);

 private:
  void drawNeed(const char* label, uint8_t value, int x, int y, uint16_t color);
  void drawPet(const PetState& pet, uint32_t now);
  void drawButton(int x, const char* label, uint16_t color);
  void drawKey(int x, int y, int width, const char* label);
  void drawNameEditor();
  PetAction handleNameEditorTouch(int x, int y, PetState& pet);

  M5Canvas canvas_{&M5.Display};
  String editedName_;
  bool nameEditorOpen_ = false;
};
