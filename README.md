# Harmony of the Spheres v0.5 - Interplanetary branding

JUCE VST3 instrument and MIDI generator for REAPER, Windows and macOS.

## Version 0.5 changes
- Adds the supplied Interplanetary Podcast artwork at the top-right of the plug-in editor.
- Embeds the artwork in the compiled plug-in using JUCE BinaryData. No external image file is needed at runtime.
- Replaces non-ASCII punctuation in the editor strings, preventing broken symbols on some builds.
- Clarifies the on-screen keyboard and orbital-modulation instructions.
- Retains the orbital visual, individual planet MIDI channels, keyboard gate, four beats per orbit, and rotation CC modulation of v0.4.

## Updating an existing GitHub repository

Upload the *contents* of this folder to the root of your existing GitHub repository, replacing the old files. Preserve the `Assets/interplanetary_logo.jpg` path and `.github/workflows/build.yml` workflow. The `CMakeLists.txt` now builds the logo into the VST3. Commit; then check the Actions build on macOS and Windows.

This is an uncompiled source update. Verify the results in REAPER before using on stage.

## Notes
- The original logo is copyright its respective owner; included as provided by the user.
- Each planet still has its own MIDI channel selector. Standard MIDI pitch bend and CC messages are channel-wide, so use different channels for independent tuning/modulation.
- Rotations use sidereal periods, not solar days.

## v0.6: Independent orbital CC switches

- **Orbital distance -> CC11** independently switches on/off continuous Expression messages representing distance from the Sun. The *Orbit modulation* slider controls depth. No longer depends on Orbital modulation tuning mode, pitch bend, or the 4-beat note gate.
- **Orbital position -> CC10** independently switches on/off continuous pan messages corresponding to orbital angular position. Also uses the *Orbit modulation* depth slider and is independent of the tuning mode / gate.
- **Microtonal pitch bend** now controls pitch-bend output only.
- **Rotation CC** stays separate and can be mapped to CC1, CC11, CC74, or CC71. **Do not choose CC11 for rotation while Orbital distance -> CC11 is enabled**, as both sources would compete.
- MIDI CCs are sent while a key is held, MIDI output is enabled, and the corresponding planet is enabled, including between note gate pulses. They cease when the last key is released.
- MIDI CC messages are channel-wide. For independent planet modulation, choose a distinct output MIDI channel for each planet.

This is a source update; test in a REAPER MIDI logger before relying on it for performance.
