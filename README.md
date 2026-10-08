# Tiny Friend for M5Stack Core2

A small touch-controlled pet game for the M5Stack Core2. Feed, play with, wash,
and put your pet to sleep to keep its needs balanced. Progress is stored in the
Core2's non-volatile preferences.

## Controls

Tap one of the four buttons along the bottom of the screen:

- **Feed** lowers hunger and gives a small happiness boost.
- **Play** boosts happiness, but uses energy and increases hunger.
- **Wash** improves cleanliness.
- **Sleep / Wake** restores energy over time while the pet sleeps.
- **Shake** the Core2 to wake the pet if it is asleep and make it excited.
- Tilt the Core2 to make the pet lean and slide with gravity.
- To reset the pet and its age, tap its name, tap **RESET** on the rename
  screen, then confirm.
- Pet age is counted in accumulated powered-on time, persists across restarts,
  and is shown in days in the top bar.
- The food meter decreases over time; if it stays at zero for five accumulated
  powered-on days, the pet dies. Feeding before then restores the meter and
  cancels the starvation countdown. Reset the game from the death screen to
  start over.
- Tap the pet's name in the top bar to rename it with the on-screen keyboard.
- Tap the pet to cycle between the round-eared, cat, and bunny avatars; the
  selected look is saved.
- Tap **SND / MUTE** in the top bar to toggle sound; the setting is remembered.
- The top bar shows battery percentage and a lightning bolt while charging.
- Care actions and shake reactions give a short vibration.
- The display sleeps after one minute without touch. Tap anywhere to wake it;
  the wake tap does not also activate a control.

The pet plays short sound cues when you care for it or shake the device. Shake
detection requires two quick movements and has a short cooldown to avoid
accidental repeated reactions.

Battery percentage is estimated from voltage, so it can vary with charging and
load. Needs change once per minute while the device is running. Hunger,
happiness, energy, and cleanliness are shown as meters on the screen.

## Build and upload

1. Install [PlatformIO Core](https://platformio.org/install/cli) or the
   PlatformIO IDE extension.
2. Connect the M5Stack Core2 over USB.
3. From this directory, run `pio run -t upload`.
4. To view serial messages, run `pio device monitor`.

The PlatformIO environment uses the `m5stack-core2` board definition and
M5Unified library.
