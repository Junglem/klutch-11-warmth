# KLUTCH 11 – WARMTH

Professional analogue-inspired vocal saturation VST3 plugin.

## Product target

WARMTH is designed as a complete stereo saturation and tone-shaping processor for FL Studio 21 and other VST3 hosts.

### Control set

1. **DRIVE** – overall saturation intensity
2. **TUBE** – tube-inspired harmonic character
3. **TAPE** – tape-inspired saturation and soft compression
4. **BODY** – low-mid fullness
5. **PRESENCE** – upper-mid clarity
6. **AIR** – high-frequency lift
7. **TONE** – overall tonal tilt
8. **DENSITY** – harmonic thickness
9. **MIX** – dry/wet blend
10. **INPUT** – input gain
11. **OUTPUT** – output gain
12. **QUALITY** – oversampling quality

## Product requirements

- Stereo processing with a stable, centred dry signal.
- Tube and tape-inspired nonlinear saturation stages.
- Frequency-conscious tone shaping to manage low-frequency buildup.
- Oversampling options to reduce aliasing.
- Smooth parameter changes and host automation.
- Complete state save/restore.
- Factory preset bank.
- Custom black, charcoal and champagne-gold interface.
- VST3 build for Windows x64 through GitHub Actions.

## Build

The project uses JUCE and CMake. Windows builds are performed by GitHub Actions so a local JUCE toolchain is not required.

The plugin is not considered release-ready until the full feature set is implemented, the workflow builds successfully, and it has been tested in a VST3 host.
