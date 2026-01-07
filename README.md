# Daisy Seed Mono Synthesizer

A comprehensive USB MIDI controlled monosynth for the Electrosmith Daisy Seed platform.

## Features

- **USB MIDI Input**: Control via any MIDI keyboard or DAW
- **Dual Oscillators**: Two independently tunable saw/square/tri/sine oscillators with detune
- **Sub Oscillator**: Additional sub-bass oscillator (octave down)
- **Dual Envelopes**: Independent ADSR for amplitude and filter
- **State Variable Filter**: Multi-mode filter with envelope modulation
- **Comprehensive MIDI Control**: All parameters mappable via MIDI CC
- **Pitch Bend**: ±2 semitone pitch bend support
- **Multiple Waveforms**: Saw, Square, Triangle, Sine waves
- **Drive/Distortion**: Built-in overdrive circuit
- **8 Built-in Presets**: MIDI Program Change support for instant sound switching

## MIDI Mapping

### Oscillators
- **CC 18**: Detune Amount (0-5%)
- **CC 19**: Oscillator 1 Waveform (0-3)
- **CC 16**: Oscillator 2 Waveform (0-3)  
- **CC 76**: Oscillator Mix (balance between Osc1/Osc2)
- **CC 77**: Sub Oscillator Level
- **CC 1**: Pulse Width (mod wheel)

### Filter
- **CC 74**: Cutoff Frequency
- **CC 71**: Resonance
- **CC 93**: Filter Envelope Amount

### Envelopes
#### Amplitude ADSR
- **CC 73**: Attack
- **CC 75**: Decay
- **CC 79**: Sustain
- **CC 72**: Release

#### Filter ADSR
- **CC 80**: Attack
- **CC 81**: Decay
- **CC 82**: Sustain
- **CC 83**: Release

### Effects
- **CC 17**: Drive/Distortion Amount
- **CC 85**: Master Volume

### Presets
- **Program Change 0-7**: Switch between 8 built-in presets
  - **0**: Init - Default clean settings
  - **1**: Classic Analog Lead - Sharp saw lead with moderate filter
  - **2**: Warm Pad - Slow attack, lush detuned oscillators  
  - **3**: Plucky Bass - Quick envelope, heavy sub, punchy filter
  - **4**: Ethereal Lead - Heavy detune, sine/tri waves, dreamy
  - **5**: Aggressive Sync - Heavy filter sweep, pulse waves
  - **6**: Deep Sub Bass - Maximum sub level, minimal highs
  - **7**: Pulse Wave Lead - Narrow pulse, bright filter

## Build Instructions

1. **Install Dependencies**:
   ```bash
   # Install ARM toolchain (macOS with Homebrew)
   brew install --cask gcc-arm-embedded
   
   # Or use the official ARM installer from:
   # https://developer.arm.com/tools-and-software/open-source-software/developer-tools/gnu-toolchain/gnu-rm
   ```

2. **Clone Repository**:
   ```bash
   git clone https://github.com/yourusername/Daisy-Seed-Mono-Synth.git
   cd Daisy-Seed-Mono-Synth
   ```
   *(Libraries are included - no submodule initialization needed)*

3. **Build Libraries** (from project root):
   ```bash
   make -C libDaisy
   make -C DaisySP  
   ```

4. **Build Project** (from Daisy-Seed-Mono-Synth subfolder):
   ```bash
   cd Daisy-Seed-Mono-Synth
   make
   ```

5. **Program Device**:
   ```bash
   make program-dfu
   ```

## Project Structure

```
├── Daisy-Seed-Mono-Synth/           # Main project folder
│   ├── Daisy-Seed-Mono-Synth.cpp   # Main synthesizer implementation
│   ├── Makefile                     # Build configuration
│   └── .vscode/                     # VS Code configuration
├── libDaisy/                        # Daisy hardware abstraction library
├── DaisySP/                         # Daisy signal processing library
├── README.md                        # This file
└── .gitignore                       # Git ignore patterns
```

## Usage

1. Connect your Daisy Seed to your computer via USB
2. Program the device using DFU mode
3. Connect a MIDI keyboard or use your DAW to send MIDI data
4. Connect headphones/speakers to the Daisy Seed audio output
5. Play and control the synth via MIDI!

## Development

This project is based on the libDaisy and DaisySP libraries from Electrosmith. The synthesizer features:

- Polyphonic note management with proper note priority
- Real-time MIDI CC parameter control  
- High-quality audio synthesis at 48kHz
- Professional synthesizer architecture

## Troubleshooting

### Build Issues
- Ensure ARM toolchain is properly installed  
- Check that libDaisy and DaisySP built successfully before building main project
- Libraries are included in the repo (no submodules needed)
- Use VS Code tasks for easier building: `Ctrl/Cmd + Shift + P` → "Tasks: Run Task" → "build"

### Updating Libraries
- Libraries are included as regular files for convenience
- To update to newer versions: replace `libDaisy/` and `DaisySP/` folders with latest from:
  - [libDaisy releases](https://github.com/electro-smith/libDaisy)
  - [DaisySP releases](https://github.com/electro-smith/DaisySP)
- After updating, rebuild libraries: `make -C libDaisy && make -C DaisySP`

### Programming Issues
- Put Daisy Seed in DFU mode (hold BOOT button, press RESET)
- Use `make program-dfu` to program via USB
- Ensure USB cable supports data transfer

## License

This project follows the same license as the underlying Daisy libraries.
