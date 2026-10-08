#pragma once

#include <Preferences.h>

#include "PetDisplay.h"
#include "PetState.h"

class PetGame {
 public:
  void begin();
  void update(uint32_t now);
  void handleAction(PetAction action);
  void reset();
  PetState& state();

 private:
  struct Note {
    uint16_t frequency;
    uint16_t durationMs;
  };

  void load();
  void save();
  void updateShakeDetection(uint32_t now);
  void reactToShake(uint32_t now);
  void updateNeeds(uint32_t now);
  void updateSound(uint32_t now);
  void updateVibration(uint32_t now);
  void refreshBattery(uint32_t now);
  void startSound(const Note* notes, size_t count);
  void pulseVibration(uint8_t level = 180);

  Preferences preferences_;
  PetState pet_;
  uint32_t lastNeedTick_ = 0;
  uint32_t lastSave_ = 0;
  uint32_t lastAgeUpdate_ = 0;
  uint32_t lastBatteryRefresh_ = 0;
  uint32_t hapticUntil_ = 0;
  uint32_t lastShakeImpulse_ = 0;
  uint32_t lastShakeReaction_ = 0;
  uint32_t nextNoteAt_ = 0;
  float previousAccelX_ = 0;
  float previousAccelY_ = 0;
  float previousAccelZ_ = 0;
  float gravityAccelX_ = 0;
  float gravityAccelY_ = 0;
  bool gravityReady_ = false;
  const Note* activeNotes_ = nullptr;
  size_t activeNoteCount_ = 0;
  size_t activeNoteIndex_ = 0;
  bool accelReady_ = false;
  bool storageReady_ = false;
};
