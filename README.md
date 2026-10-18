## v0.3 — MIDI-keyed Solar System

Play a note into the plug-in to start the eight planets. Incoming MIDI note 69 (A4) makes Earth 440 Hz; C4 makes Earth 261.626 Hz, etc. Releasing the final held MIDI key stops all planet MIDI notes and smoothly silences the internal audio. With overlapping keys, the most recently pressed held note sets Earth's pitch, reverting to a prior held note on release. Incoming note MIDI is consumed, not forwarded to the downstream Roland instrument; the planetary notes are emitted instead.

The Earth frequency knob is disabled in this MIDI-keyed version. MIDI input must be enabled in the JUCE VST3 definition; v0.3 includes that change.

**Output modulation (only if both Pitch bend + orbit CC and Orbital modulation are selected):** CC11 = orbital expression/relative level, CC10 = stereo pan. Sent per active planet on its assigned channel, at most ~25 times per second and only if values change. MIDI pitch bend is sent when notes start if Pitch bend + orbit CC is on; it assumes a +/-2 semitone range on your Roland instrument. Turn off this option if the receiving patch ignores CCs or does not use the same pitch-bend range.

**Important:** assign every planet to a different MIDI channel when using pitch bend or CCs; these are channel-wide MIDI messages. The channel-specific CC11 and CC10 are creative orbital mappings, not physical acoustic properties of the planets.

**Known limitation:** note-on/off are currently block-boundary events rather than sample-accurate, and the plugin does not capture sustain-pedal behaviour. The release gate responds to received note-offs; some controllers/hosts may keep notes active until sustain pedal is lifted.

# Harmony of the Spheres — JUCE VST3 (source prototype)

A Windows/macOS planetary sonification synth designed for REAPER, based on the eight planetary *sidereal* orbital periods. This package contains **source code, not precompiled binaries**. It has not yet been compiled or host-tested. Built using JUCE 8 (fetched automatically by CMake). A generic JUCE editor exposes the parameters; the animated orbital display from the browser edition has **not** yet been ported.

## Audio and mapping

- 8 sine/triangle/soft-saw oscillators; individually enabled and level-controlled.
- **True ratios**: Earth frequency divided by each planet's orbital period in Earth years; outer pitches can be infrasonic.
- **Octave-folded**: repeatedly transpose by octaves into roughly 90–1200 Hz, retaining pitch class but not absolute ratios.
- **Compressed**: exponent 0.4 applied to period ratios, changing the relative intervals.
- **Orbital modulation**: octave-folded tuning plus Kepler's equation (nonuniform orbital motion, eccentricity). Position modulates amplitude and stereo pan. Kepler orbital periods are accelerated by the **One Earth year (seconds)** control.
- The eccentricity exaggeration is explicitly an **artistic departure** from measured ellipticity.
- MIDI notes are sent for the rounded pitches. Most MIDI instruments cannot reproduce the precise orbital ratios through ordinary MIDI notes.
- **Independent MIDI channels + pitch bend** uses MIDI channels 1–8, with one planet per channel and pitch bend assuming the receiving instruments' range is ±2 semitones. In orbital modulation mode it also emits expression CC11 and pan CC10. A synth receiving only channel 1 will play Mercury only in this mode. With this option *off*, all planets send rounded notes on channel 1 without per-planet pitch bend or CC modulation. Some synths will not respond to the CC mappings.

### REAPER and Roland instrument routing

1. Add **Harmony of the Spheres (VST3)** to a track FX chain.
2. Add your **Roland VST3 instrument after it** in the **same FX chain**. If Roland is not hearing generated notes, check the REAPER FX pin connector's MIDI bus input/output and whether the VST3 wrapper exposes MIDI output.
3. Disable `Internal sound on` if you only want Roland audio. Leave `MIDI notes out` enabled.
4. Start with `Independent MIDI channels + pitch bend` **off** (shared MIDI channel 1). Check that all eight notes are sounding on the Roland instrument. Enable independent channels only if the destination is multitimbral or set to MIDI omni, and configured for pitch-bend ±2 semitones.
5. MIDI notes remain held while the planet is enabled. Disable a planet to send note-off. Note events update when pitches/tuning modes change.
6. For separate instrument instances, put them on separate tracks and route specific MIDI channels from the source track, with independent channel mode enabled.

**Host caveat:** MIDI output from VST3 instruments can vary with host and wrapper support. This JUCE project advertises MIDI output, but the actual routing with your specific REAPER and Roland versions **needs testing**. An AU build on macOS is not requested; VST3 is the cross-platform format.

## Build locally

Prerequisites: CMake 3.22+, Git, Visual Studio 2022 C++ development tools on Windows or Xcode Command Line Tools on macOS. Internet required the first time for JUCE download.

```bash
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build --config Release --parallel 3
```

The resulting `.vst3` bundle will be in `build/HarmonyOfTheSpheres_artefacts/Release/VST3` (or `build/HarmonyOfTheSpheres_artefacts/VST3`, depending on platform/build configuration).

**Install**: copy the `.vst3` bundle to `%COMMONPROGRAMFILES%\\VST3` (Windows) or `~/Library/Audio/Plug-Ins/VST3/` (macOS). Rescan VST3 plug-ins in REAPER. A downloaded, unsigned macOS plug-in may require dealing with Gatekeeper and signing; use trusted builds only.

## Automatic builds (GitHub)

Create an empty repository, upload this folder (including `.github/workflows/build.yml`), and trigger **Actions → Build Windows and macOS plug-ins → Run workflow**. Download the build artifacts from the workflow run. macOS builds target the runner's default architecture, currently Apple Silicon. For Intel Mac support, adjust CMake for a universal build and test both architectures.

## Scientific caveats / next steps

The animated orbit display is not included in this first JUCE prototype. Planetary frequencies are *mapped from revolutions per year*, not real audible sounds emitted by planets. The real orbital motion is governed by Kepler's equation; stereo location, level and timbre are musical interpretations. The per-block orbital updates are not sample-accurate and long-period planets move slowly unless simulation time is accelerated substantially. The generic parameter interface is deliberately utilitarian, pending a bespoke orbit visualization and performance UI.

Next priority: build/test with REAPER + Roland plugin, then add custom planetary orbital editor, independent MIDI CC mapping UI, note quantization choice and a panic/reset control.

## Version 0.2 — orbital graphic and per-planet MIDI channels

* The custom JUCE interface draws eight Keplerian ellipses with the Sun at a focus, and animates each planet according to its period and eccentricity. Planet and orbit sizes are illustrative, not a scale model.
* Each planet has an `MIDI channel` selector (1–16), `On` toggle and level control. All parameters are saved in REAPER project state.
* Changing a planet's channel sends its note-off on the **previous** channel before the replacement note-on on the **new** channel.
* For independent microtonal pitch bend and CC10/CC11 orbital modulation, give each planet a **unique MIDI channel** and enable `Pitch bend + orbit CC`. MIDI pitch bend assumes the receiving instrument has a ±2-semitone pitch-bend range.
* Shared channels work for ordinary equal-tempered note playback, but pitch bend and MIDI CC are per-channel and can interfere when two planets use the same channel. The instrument doesn't prevent duplicates.
* Orbital visuals animate while REAPER is processing audio, not as a standalone astronomical clock. Audio oscillator pitches remain exact within their tuning mode, while outgoing standard MIDI notes are rounded to semitones (with optional pitch bend).
* The panel's orbit animation compresses time and intentionally exaggerates eccentricity if the user sets `Ellipse exaggeration` above 1.

### REAPER routing

Place Harmony of the Spheres before a multitimbral instrument and configure instrument parts to receive the planets' selected channels, or send the track's MIDI to separate Roland instrument tracks (disable master/parent audio send on the generator track if necessary). For each receiving Roland instrument, choose the appropriate MIDI source channel in REAPER's track send configuration. Turn off `Internal audio` in Harmony to hear only the Roland instruments.

### Build check

Push this folder's **contents** to the root of your existing GitHub repository, including `.github/workflows/build.yml`. The workflow attempts Windows and macOS builds independently. **This release is source only and has not been compiled or host-tested here**; use GitHub Actions to verify its build and inspect any compiler errors.


## v0.4: orbit rhythm and planetary rotation CC

* **4 beats per orbit:** when enabled, each planet gates its note on four times per sidereal orbital revolution, at equal intervals in **time (mean anomaly)**. Pulse length determines each beat's duty cycle. A note is only generated while an incoming keyboard note is held. This is **not** a shared 4/4 tempo: every planet has its own orbital bar duration. Note events are currently resolved to the audio-buffer boundary.
* **Rotation CC:** outputs a continuous controller oscillating once per planet's **sidereal rotation**. Negative rotation periods preserve Venus and Uranus's retrograde direction. Choices: CC1, CC11, CC74 or CC71. The modulation is an artistic mapping, not an acoustic property of the planet. 200-Hz target update rate is evaluated once per audio block; rapid rotations at extremely short simulated years can alias.
* **Critical:** use a unique MIDI channel per planet, particularly when spin CC, pitch bend or ellipse CC are active: MIDI 1.0 CCs affect an entire channel. When both ellipse expression and spin CC11 are enabled on the same channel, they will compete; choose CC74 or CC1 instead.
* **Timing:** year length is simulated seconds per Earth year. Four Earth note pulses per simulated Earth year; Earth's rotation makes ~366.26 **sidereal** cycles per Earth orbit (not 365), due to the distinction between sidereal and solar days.
* **Incoming MIDI:** a held key triggers the system and sets Earth's pitch; releasing all keys stops output notes.
* **Status:** source-level prototype, builds and actual MIDI behavior have not been verified in REAPER or Roland VSTs.
