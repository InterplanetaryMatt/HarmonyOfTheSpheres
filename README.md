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
