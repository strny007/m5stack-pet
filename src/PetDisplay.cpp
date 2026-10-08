#include "PetDisplay.h"

#include <cstring>

namespace {

constexpr int kScreenWidth = 320;
constexpr int kScreenHeight = 240;
constexpr uint8_t kPetNameMaxLength = 12;

constexpr uint16_t kBackground = 0x0841;
constexpr uint16_t kPanel = 0x10A2;
constexpr uint16_t kTrack = 0x2945;
constexpr uint16_t kMint = 0x6F5D;
constexpr uint16_t kYellow = 0xFEA0;
constexpr uint16_t kCoral = 0xF986;
constexpr uint16_t kWhite = 0xFFFF;
constexpr uint16_t kMuted = 0x9CF3;
constexpr uint16_t kGreen = 0x4E68;
constexpr uint16_t kBrown = 0x8A22;
constexpr uint16_t kBlack = 0x0000;

bool reacting(const PetState& pet, uint32_t now) {
  return static_cast<int32_t>(pet.reactionUntil - now) > 0;
}

const char* moodText(const PetState& pet, uint32_t now) {
  if (reacting(pet, now)) return "Wheee!";
  if (pet.sleeping) return "Zzz...";
  if (pet.hunger < 20) return "Feed me!";
  if (pet.energy < 20) return "So sleepy";
  if (pet.cleanliness < 20) return "Bath time!";
  if (pet.happiness < 25) return "Play with me";
  return "Feeling good!";
}

}  // namespace

void PetDisplay::begin() {
  canvas_.createSprite(kScreenWidth, kScreenHeight);
}

void PetDisplay::drawNeed(const char* label, uint8_t value, int x, int y,
                          uint16_t color) {
  canvas_.setTextColor(kWhite, kBackground);
  canvas_.setCursor(x, y);
  canvas_.print(label);
  canvas_.setTextColor(kMuted, kBackground);
  canvas_.setCursor(x + 48, y);
  canvas_.printf("%u%%", value);

  constexpr int barWidth = 105;
  canvas_.fillRoundRect(x, y + 15, barWidth, 7, 3, kTrack);
  const int fillWidth = barWidth * value / 100;
  if (fillWidth > 0) {
    canvas_.fillRoundRect(x, y + 15, fillWidth, 7, 3, color);
  }
}

void PetDisplay::drawPet(const PetState& pet, uint32_t now) {
  const bool excited = reacting(pet, now);
  const int bounce = pet.sleeping
      ? 0
      : excited ? static_cast<int>((now / 75) % 2) * 5
                : static_cast<int>((now / 450) % 2);
  const int centerX =
      160 + (excited ? static_cast<int>((now / 75) % 2) * 5 - 2 : 0);
  const int centerY = 150 + bounce;
  const uint16_t bodyColor = pet.sleeping ? kMuted : kYellow;

  canvas_.fillRoundRect(217, 130, 93, 28, 9, kPanel);
  canvas_.setTextColor(kWhite, kPanel);
  canvas_.setCursor(226, 139);
  canvas_.print(moodText(pet, now));

  canvas_.fillCircle(centerX - 23, centerY - 21, 12, bodyColor);
  canvas_.fillCircle(centerX + 23, centerY - 21, 12, bodyColor);
  canvas_.fillCircle(centerX, centerY, 32, bodyColor);
  canvas_.fillCircle(centerX - 23, centerY - 21, 5, kCoral);
  canvas_.fillCircle(centerX + 23, centerY - 21, 5, kCoral);

  if (pet.sleeping) {
    canvas_.drawLine(centerX - 13, centerY - 3, centerX - 6, centerY - 3,
                     kBlack);
    canvas_.drawLine(centerX + 6, centerY - 3, centerX + 13, centerY - 3,
                     kBlack);
    canvas_.setTextColor(kMint, kBackground);
    canvas_.setCursor(centerX + 37, centerY - 28);
    canvas_.print("Z");
    canvas_.setCursor(centerX + 47, centerY - 38);
    canvas_.print("z");
  } else {
    canvas_.fillCircle(centerX - 10, centerY - 4, 3, kBlack);
    canvas_.fillCircle(centerX + 10, centerY - 4, 3, kBlack);
    canvas_.drawLine(centerX - 5, centerY + 10, centerX, centerY + 13, kBrown);
    canvas_.drawLine(centerX, centerY + 13, centerX + 5, centerY + 10, kBrown);
  }

  canvas_.fillCircle(centerX - 20, centerY + 7, 4, kCoral);
  canvas_.fillCircle(centerX + 20, centerY + 7, 4, kCoral);
}

void PetDisplay::drawButton(int x, const char* label, uint16_t color) {
  canvas_.fillRoundRect(x, 190, 70, 39, 8, kPanel);
  canvas_.drawRoundRect(x, 190, 70, 39, 8, color);
  canvas_.setTextColor(color, kPanel);
  canvas_.setCursor(x + 9, 205);
  canvas_.print(label);
}

void PetDisplay::drawKey(int x, int y, int width, const char* label) {
  canvas_.fillRoundRect(x, y, width, 29, 5, kPanel);
  canvas_.drawRoundRect(x, y, width, 29, 5, kMint);
  canvas_.setTextColor(kWhite, kPanel);
  canvas_.setCursor(x + (width - static_cast<int>(strlen(label)) * 6) / 2,
                    y + 10);
  canvas_.print(label);
}

void PetDisplay::drawNameEditor() {
  canvas_.fillScreen(kBackground);
  canvas_.setTextColor(kMint, kBackground);
  canvas_.setCursor(12, 12);
  canvas_.print("NAME YOUR PET");

  canvas_.fillRoundRect(10, 38, 300, 30, 6, kPanel);
  canvas_.setTextColor(kWhite, kPanel);
  canvas_.setCursor(19, 48);
  canvas_.print(editedName_);
  canvas_.setTextColor(kMuted, kPanel);
  canvas_.setCursor(276, 48);
  canvas_.printf("%u/%u", static_cast<unsigned>(editedName_.length()),
                 kPetNameMaxLength);

  constexpr char kRows[][11] = {"QWERTYUIOP", "ASDFGHJKL", "ZXCVBNM"};
  constexpr uint8_t kRowLengths[] = {10, 9, 7};
  constexpr int kRowY[] = {76, 110, 144};
  for (uint8_t row = 0; row < 3; ++row) {
    const int startX = (kScreenWidth - kRowLengths[row] * 30) / 2;
    for (uint8_t key = 0; key < kRowLengths[row]; ++key) {
      char label[] = {kRows[row][key], '\0'};
      drawKey(startX + key * 30, kRowY[row], 28, label);
    }
  }

  drawKey(8, 190, 70, "SPACE");
  drawKey(86, 190, 70, "DELETE");
  drawKey(164, 190, 70, "CANCEL");
  drawKey(242, 190, 70, "SAVE");
  canvas_.pushSprite(0, 0);
}

void PetDisplay::draw(const PetState& pet, uint32_t now) {
  if (nameEditorOpen_) {
    drawNameEditor();
    return;
  }

  canvas_.fillScreen(kBackground);
  canvas_.fillRoundRect(8, 7, 304, 27, 8, kPanel);
  canvas_.setTextColor(kMint, kPanel);
  canvas_.setCursor(17, 14);
  canvas_.print(pet.name);

  const uint16_t batteryColor =
      pet.batteryLevelValid && pet.batteryLevel < 20 ? kCoral : kMint;
  canvas_.drawRoundRect(198, 13, 15, 11, 2, batteryColor);
  canvas_.fillRect(213, 16, 2, 5, batteryColor);
  if (pet.batteryLevelValid && pet.batteryLevel > 0) {
    canvas_.fillRect(200, 15, 1 + pet.batteryLevel * 9 / 100, 7,
                     batteryColor);
  }
  canvas_.setTextColor(kMuted, kPanel);
  canvas_.setCursor(219, 14);
  if (pet.batteryLevelValid) {
    canvas_.printf("%s%u%%", pet.batteryCharging ? "+" : "",
                   static_cast<unsigned>(pet.batteryLevel));
  } else {
    canvas_.print("BAT --");
  }
  canvas_.setTextColor(pet.muted ? kCoral : kMint, kPanel);
  canvas_.setCursor(270, 14);
  canvas_.print(pet.muted ? "MUTE" : "SND");

  drawNeed("FOOD", pet.hunger, 14, 43, kCoral);
  drawNeed("JOY", pet.happiness, 166, 43, kMint);
  drawNeed("REST", pet.energy, 14, 78, kYellow);
  drawNeed("CLEAN", pet.cleanliness, 166, 78, kGreen);

  canvas_.drawFastHLine(12, 113, 296, kTrack);
  drawPet(pet, now);
  drawButton(10, "FEED", kCoral);
  drawButton(88, "PLAY", kMint);
  drawButton(166, "WASH", kGreen);
  drawButton(244, pet.sleeping ? "WAKE" : "SLEEP", kYellow);
  canvas_.pushSprite(0, 0);
}

PetAction PetDisplay::handleNameEditorTouch(int x, int y, PetState& pet) {
  if (y >= 76 && y < 173) {
    constexpr char kRows[][11] = {"QWERTYUIOP", "ASDFGHJKL", "ZXCVBNM"};
    constexpr uint8_t kRowLengths[] = {10, 9, 7};
    constexpr int kRowY[] = {76, 110, 144};
    for (uint8_t row = 0; row < 3; ++row) {
      if (y < kRowY[row] || y >= kRowY[row] + 29) continue;
      const int startX = (kScreenWidth - kRowLengths[row] * 30) / 2;
      const int offset = x - startX;
      if (offset < 0 || offset >= kRowLengths[row] * 30 || offset % 30 >= 28) {
        return PetAction::none;
      }
      if (editedName_.length() < kPetNameMaxLength) {
        editedName_ += kRows[row][offset / 30];
      }
      return PetAction::none;
    }
  }

  if (y < 190 || y >= 219) return PetAction::none;
  if (x >= 8 && x < 78) {
    if (editedName_.length() < kPetNameMaxLength) editedName_ += ' ';
  } else if (x >= 86 && x < 156) {
    if (!editedName_.isEmpty()) editedName_.remove(editedName_.length() - 1);
  } else if (x >= 164 && x < 234) {
    nameEditorOpen_ = false;
  } else if (x >= 242 && x < 312) {
    editedName_.trim();
    if (editedName_.isEmpty()) {
      editedName_ = pet.name;
    } else {
      pet.name = editedName_;
      nameEditorOpen_ = false;
      return PetAction::rename;
    }
  }
  return PetAction::none;
}

PetAction PetDisplay::handleTouch(int x, int y, PetState& pet) {
  if (nameEditorOpen_) {
    return handleNameEditorTouch(x, y, pet);
  }

  if (y >= 7 && y < 34) {
    if (x >= 264) return PetAction::toggleMute;
    if (x < 195) {
      editedName_ = pet.name;
      nameEditorOpen_ = true;
    }
    return PetAction::none;
  }
  if (y < 190 || y >= 229) return PetAction::none;

  const int buttonOffset = x - 10;
  if (buttonOffset < 0 || buttonOffset >= 304 || buttonOffset % 78 >= 70) {
    return PetAction::none;
  }
  switch (buttonOffset / 78) {
    case 0: return PetAction::feed;
    case 1: return PetAction::play;
    case 2: return PetAction::wash;
    case 3: return PetAction::toggleSleep;
    default: return PetAction::none;
  }
}
