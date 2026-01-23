# Daisy Seed Mono Synthesizer

A comprehensive USB MIDI controlled monosynth for the Electrosmith Daisy Seed platform.

[![DIY Daisy Seed Mono Synth | Simple Build, Big Sound](https://img.youtube.com/vi/hb_fzLtgpfs/0.jpg)](https://www.youtube.com/watch?v=hb_fzLtgpfs "DIY Daisy Seed Mono Synth | Simple Build, Big Sound")

Demo Video: ["DIY Daisy Seed Mono Synth | Simple Build, Big Sound"](https://youtu.be/hb_fzLtgpfs)

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
  - **1**: G-Thang - Filter sweep with balanced oscillators
  - **2**: Take Me On - Detuned saws with sub, moderate envelopes
  - **3**: Bass Drive - Square waves with sub and drive for aggressive bass
  - **4**: 8bit - Pure square wave with narrow pulse, retro video game sound
  - **5**: Sines - Smooth sine waves with slight detune and drive warmth
  - **6**: Deep Sub Bass - Maximum sub level, minimal highs
  - **7**: Pulse Wave Lead - Narrow pulse, bright filter

## Build Instructions
The project now follows the same build steps as the Daisy Examples repo. Refer to that page for steps on getting it all setup: https://github.com/electro-smith/DaisyExamples

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

- Monophonic note management with proper note priority (LIFO)
- Real-time MIDI CC parameter control  
- High-quality audio synthesis at 48kHz
- Professional synthesizer architecture
- 8 presets covering various musical styles

## Troubleshooting

### Build Issues
- Ensure ARM toolchain is properly installed  
- Check that libDaisy and DaisySP built successfully before building main project
- Use VS Code tasks for easier building: `Ctrl/Cmd + Shift + P` → "Tasks: Run Task" → "build_and_program_dfu"

### Programming Issues
- Put Daisy Seed in DFU mode (hold BOOT button, press RESET)
- Use `make program-dfu` to program via USB
- Ensure USB cable supports data transfer

---
*This project was developed with assistance from [Claude Code](https://code.claude.com/docs/en/overview).*
