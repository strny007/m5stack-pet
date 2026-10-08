#include "PetGame.h"

#include <M5Unified.h>

namespace {

constexpr uint32_t kNeedTickMs = 60000;
constexpr uint32_t kSaveIntervalMs = 60000;
constexpr uint32_t kShakePairWindowMs = 900;
constexpr uint32_t kShakeCooldownMs = 2500;
constexpr uint32_t kReactionDurationMs = 2200;
constexpr uint32_t kBatteryRefreshMs = 15000;
constexpr uint32_t kHapticDurationMs = 55;
constexpr uint8_t kPetNameMaxLength = 12;

uint8_t clampNeed(int value) {
  if (value < 0) return 0;
  if (value > 100) return 100;
  return static_cast<uint8_t>(value);
}

}  // namespace

void PetGame::begin() {
  storageReady_ = preferences_.begin("tinyfriend", false);
  if (!storageReady_) {
    Serial.println("Could not open pet storage; progress will not be saved.");
  }
  load();
  M5.Speaker.setVolume(pet_.muted ? 0 : 72);
  lastNeedTick_ = millis();
  lastSave_ = lastNeedTick_;
  lastAgeUpdate_ = lastNeedTick_;
  refreshBattery(lastNeedTick_);
  Serial.println("Tiny Friend ready.");
}

void PetGame::load() {
  if (!storageReady_) return;
  pet_.hunger = clampNeed(preferences_.getUChar("hunger", pet_.hunger));
  pet_.happiness = clampNeed(preferences_.getUChar("happy", pet_.happiness));
  pet_.energy = clampNeed(preferences_.getUChar("energy", pet_.energy));
  pet_.cleanliness = clampNeed(preferences_.getUChar("clean", pet_.cleanliness));
  pet_.sleeping = preferences_.getBool("sleep", false);
  pet_.muted = preferences_.getBool("muted", false);
  pet_.avatar = preferences_.getUChar("avatar", pet_.avatar);
  if (pet_.avatar >= 3) pet_.avatar = 0;
  pet_.aliveMs = preferences_.getULong64("alive", pet_.aliveMs);
  if (preferences_.isKey("name")) {
    pet_.name = preferences_.getString("name", pet_.name);
  }
  if (pet_.name.length() > kPetNameMaxLength) {
    pet_.name.remove(kPetNameMaxLength);
  }
}

void PetGame::save() {
  if (!storageReady_) return;
  preferences_.putUChar("hunger", pet_.hunger);
  preferences_.putUChar("happy", pet_.happiness);
  preferences_.putUChar("energy", pet_.energy);
  preferences_.putUChar("clean", pet_.cleanliness);
  preferences_.putBool("sleep", pet_.sleeping);
  preferences_.putBool("muted", pet_.muted);
  preferences_.putUChar("avatar", pet_.avatar);
  preferences_.putString("name", pet_.name);
  preferences_.putULong64("alive", pet_.aliveMs);
}

PetState& PetGame::state() {
  return pet_;
}

void PetGame::reset() {
  const int batteryLevel = pet_.batteryLevel;
  const bool batteryLevelValid = pet_.batteryLevelValid;
  const bool batteryCharging = pet_.batteryCharging;
  const float tiltX = pet_.tiltX;
  const float tiltY = pet_.tiltY;
  pet_ = PetState{};
  pet_.batteryLevel = batteryLevel;
  pet_.batteryLevelValid = batteryLevelValid;
  pet_.batteryCharging = batteryCharging;
  pet_.tiltX = tiltX;
  pet_.tiltY = tiltY;

  const uint32_t now = millis();
  lastNeedTick_ = now;
  lastAgeUpdate_ = now;
  lastSave_ = now;
  activeNotes_ = nullptr;
  hapticUntil_ = 0;
  M5.Power.setVibration(0);
  M5.Speaker.setVolume(72);
  save();
  Serial.println("Pet game reset.");
}

void PetGame::startSound(const Note* notes, size_t count) {
  if (pet_.muted) return;
  activeNotes_ = notes;
  activeNoteCount_ = count;
  activeNoteIndex_ = 0;
  nextNoteAt_ = millis();
}

void PetGame::pulseVibration(uint8_t level) {
  M5.Power.setVibration(level);
  hapticUntil_ = millis() + kHapticDurationMs;
}

void PetGame::updateVibration(uint32_t now) {
  if (hapticUntil_ != 0 && static_cast<int32_t>(now - hapticUntil_) >= 0) {
    M5.Power.setVibration(0);
    hapticUntil_ = 0;
  }
}

void PetGame::updateSound(uint32_t now) {
  if (pet_.muted || activeNotes_ == nullptr ||
      activeNoteIndex_ >= activeNoteCount_ ||
      static_cast<int32_t>(now - nextNoteAt_) < 0) {
    return;
  }

  const Note note = activeNotes_[activeNoteIndex_++];
  M5.Speaker.tone(note.frequency, note.durationMs);
  nextNoteAt_ = now + note.durationMs + 25;
  if (activeNoteIndex_ >= activeNoteCount_) {
    activeNotes_ = nullptr;
  }
}

void PetGame::reactToShake(uint32_t now) {
  static constexpr Note kShakeSound[] = {{784, 65}, {1047, 65}, {1568, 170}};
  if (pet_.sleeping) pet_.sleeping = false;
  pet_.happiness = clampNeed(pet_.happiness + 12);
  pet_.reactionUntil = now + kReactionDurationMs;
  lastShakeReaction_ = now;
  startSound(kShakeSound, sizeof(kShakeSound) / sizeof(kShakeSound[0]));
  pulseVibration(220);
  save();
  lastSave_ = now;
  Serial.println("Pet reacted to a shake.");
}

void PetGame::updateShakeDetection(uint32_t now) {
  float accelX;
  float accelY;
  float accelZ;
  if (!M5.Imu.getAccel(&accelX, &accelY, &accelZ)) {
    accelReady_ = false;
    gravityReady_ = false;
    pet_.tiltX += (0.0f - pet_.tiltX) * 0.12f;
    pet_.tiltY += (0.0f - pet_.tiltY) * 0.12f;
    return;
  }

  if (!gravityReady_) {
    gravityAccelX_ = accelX;
    gravityAccelY_ = accelY;
    gravityReady_ = true;
  } else {
    constexpr float kGravityFilter = 0.08f;
    gravityAccelX_ += (accelX - gravityAccelX_) * kGravityFilter;
    gravityAccelY_ += (accelY - gravityAccelY_) * kGravityFilter;
  }

  const float horizontalTilt = gravityAccelX_ * -22.0f;
  const float verticalTilt = gravityAccelY_ * -9.0f;
  const float targetX =
      horizontalTilt < -22 ? -22 : horizontalTilt > 22 ? 22 : horizontalTilt;
  const float targetY =
      verticalTilt < -7 ? -7 : verticalTilt > 7 ? 7 : verticalTilt;
  pet_.tiltX += (targetX - pet_.tiltX) * 0.12f;
  pet_.tiltY += (targetY - pet_.tiltY) * 0.12f;

  if (!accelReady_) {
    previousAccelX_ = accelX;
    previousAccelY_ = accelY;
    previousAccelZ_ = accelZ;
    accelReady_ = true;
    return;
  }

  const float deltaX = accelX - previousAccelX_;
  const float deltaY = accelY - previousAccelY_;
  const float deltaZ = accelZ - previousAccelZ_;
  previousAccelX_ = accelX;
  previousAccelY_ = accelY;
  previousAccelZ_ = accelZ;

  constexpr float kShakeDeltaThreshold = 1.25f;
  if (deltaX * deltaX + deltaY * deltaY + deltaZ * deltaZ <
      kShakeDeltaThreshold * kShakeDeltaThreshold) {
    return;
  }

  if (static_cast<uint32_t>(now - lastShakeImpulse_) <= kShakePairWindowMs &&
      (lastShakeReaction_ == 0 ||
       static_cast<uint32_t>(now - lastShakeReaction_) >= kShakeCooldownMs)) {
    lastShakeImpulse_ = 0;
    reactToShake(now);
    return;
  }
  lastShakeImpulse_ = now;
}

void PetGame::updateNeeds(uint32_t now) {
  while (static_cast<uint32_t>(now - lastNeedTick_) >= kNeedTickMs) {
    lastNeedTick_ += kNeedTickMs;
    pet_.hunger = clampNeed(pet_.hunger + 1);
    pet_.cleanliness = clampNeed(pet_.cleanliness - 1);
    pet_.happiness = clampNeed(pet_.happiness - 1);

    if (pet_.sleeping) {
      pet_.energy = clampNeed(pet_.energy + 3);
    } else {
      pet_.energy = clampNeed(pet_.energy - 1);
    }

    if (pet_.hunger < 20 && pet_.happiness > 0) {
      pet_.happiness = clampNeed(pet_.happiness - 1);
    }
  }
}

void PetGame::refreshBattery(uint32_t now) {
  const int batteryVoltageMv = M5.Power.getBatteryVoltage();
  pet_.batteryLevelValid = batteryVoltageMv > 0;
  if (pet_.batteryLevelValid) {
    constexpr int kEmptyBatteryMv = 3300;
    constexpr int kFullBatteryMv = 4200;
    const int level =
        (batteryVoltageMv - kEmptyBatteryMv) * 100 /
        (kFullBatteryMv - kEmptyBatteryMv);
    pet_.batteryLevel = level < 0 ? 0 : level > 100 ? 100 : level;
  }
  pet_.batteryCharging =
      M5.Power.isCharging() == M5.Power.is_charging_t::is_charging;
  lastBatteryRefresh_ = now;
}

void PetGame::handleAction(PetAction action) {
  static constexpr Note kFeedSound[] = {{880, 90}, {1175, 120}};
  static constexpr Note kPlaySound[] = {{1047, 75}, {1319, 75}, {1568, 140}};
  static constexpr Note kWashSound[] = {{740, 100}, {988, 100}};
  static constexpr Note kSleepSound[] = {{659, 130}, {523, 180}};
  const bool wasSleeping = pet_.sleeping;

  switch (action) {
    case PetAction::feed:
      pet_.hunger = clampNeed(pet_.hunger - 28);
      pet_.happiness = clampNeed(pet_.happiness + 4);
      startSound(kFeedSound, sizeof(kFeedSound) / sizeof(kFeedSound[0]));
      pulseVibration(170);
      break;
    case PetAction::play:
      if (pet_.sleeping) return;
      pet_.happiness = clampNeed(pet_.happiness + 22);
      pet_.energy = clampNeed(pet_.energy - 12);
      pet_.hunger = clampNeed(pet_.hunger + 5);
      startSound(kPlaySound, sizeof(kPlaySound) / sizeof(kPlaySound[0]));
      pulseVibration(170);
      break;
    case PetAction::wash:
      pet_.cleanliness = clampNeed(pet_.cleanliness + 35);
      pet_.happiness = clampNeed(pet_.happiness + 3);
      startSound(kWashSound, sizeof(kWashSound) / sizeof(kWashSound[0]));
      pulseVibration(170);
      break;
    case PetAction::toggleSleep:
      pet_.sleeping = !pet_.sleeping;
      startSound(kSleepSound, sizeof(kSleepSound) / sizeof(kSleepSound[0]));
      pulseVibration(120);
      if (wasSleeping) pet_.reactionUntil = millis() + kReactionDurationMs;
      break;
    case PetAction::toggleMute:
      pet_.muted = !pet_.muted;
      activeNotes_ = nullptr;
      M5.Speaker.setVolume(pet_.muted ? 0 : 72);
      break;
    case PetAction::cycleAvatar:
      pet_.avatar = (pet_.avatar + 1) % 3;
      break;
    case PetAction::rename:
      break;
    case PetAction::resetGame:
      reset();
      return;
    case PetAction::none:
      return;
  }

  save();
  lastSave_ = millis();
}

void PetGame::update(uint32_t now) {
  pet_.aliveMs += static_cast<uint32_t>(now - lastAgeUpdate_);
  lastAgeUpdate_ = now;
  updateShakeDetection(now);
  updateSound(now);
  updateVibration(now);
  updateNeeds(now);

  if (static_cast<uint32_t>(now - lastBatteryRefresh_) >= kBatteryRefreshMs) {
    refreshBattery(now);
  }

  if (storageReady_ &&
      static_cast<uint32_t>(now - lastSave_) >= kSaveIntervalMs) {
    save();
    lastSave_ = now;
  }
}
