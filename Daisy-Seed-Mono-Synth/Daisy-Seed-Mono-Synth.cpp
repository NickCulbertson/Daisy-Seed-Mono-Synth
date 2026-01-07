#include "daisy_seed.h"
#include "daisysp.h"

using namespace daisy;
using namespace daisysp;

// MIDI CC Assignments (Arturia Keylab knobs)
#define CC_AMP_ATTACK     73
#define CC_AMP_DECAY      75
#define CC_AMP_SUSTAIN    79
#define CC_AMP_RELEASE    72
#define CC_FILT_ATTACK    80
#define CC_FILT_DECAY     81
#define CC_FILT_SUSTAIN   82
#define CC_FILT_RELEASE   83
#define CC_CUTOFF         74
#define CC_RESONANCE      71
#define CC_OSC_MIX        76  // Oscillator mix
#define CC_SUB_LEVEL      77  // Sub oscillator level
#define CC_FILT_ENV_AMT   93  // Filter envelope amount
#define CC_DETUNE         18  // Oscillator detune amount
#define CC_OSC1_WAVE      19  // Oscillator 1 waveform
#define CC_OSC2_WAVE      16  // Oscillator 2 waveform
#define CC_DRIVE          17  // Drive/distortion amount
#define CC_MASTER_VOL     85  // Master volume
// Note: Mod wheel (CC 1) controls pulse width, handled separately


// Features:
//   - USB MIDI input (Note On/Off, CC, Pitch Bend, Program Change)
//   - 8 built-in presets via MIDI Program Change (0-7)
//   - Dual oscillators with detune and waveform selection
//   - Sub oscillator (octave down)
//   - Dual ADSR envelopes (amp + filter)
//   - State variable filter with resonance
//   - Drive/distortion effect
//
// MIDI Program Change Presets:
//   0: Init - Your current default settings (start here!)
//   1: Classic Analog Lead - Sharp saw lead with moderate filter
//   2: Warm Pad - Slow attack, lush detuned oscillators
//   3: Plucky Bass - Quick envelope, heavy sub, punchy filter
//   4: Ethereal Lead - Heavy detune, sine/tri waves, dreamy
//   5: Aggressive Sync - Heavy filter sweep, pulse waves
//   6: Deep Sub Bass - Maximum sub level, minimal highs
//   7: Pulse Wave Lead - Narrow pulse, bright filter
//
// Future enhancements
//   - LFO for modulation
//   - Portamento/glide
//   - Multiple filter types
//   - Delay/reverb effects

// Hardware and MIDI
DaisySeed hw;
MidiUsbHandler midi;

// Oscillators: 2 detuned saws + 1 sub square
Oscillator saw1, saw2, subOsc;

// Envelopes
Adsr ampEnv, filtEnv;

// Filter
Svf filt;

// Note stack for proper mono behavior (LIFO - last in, first out)
int note_stack[128];  // Stack of pressed notes
int stack_size = 0;   // Current stack depth
bool held[128] = {false};  // Track which notes are held
int current_note = -1;     // Currently playing note
bool gate_state = false;   // Gate state for envelopes

// Synth parameters
float sampleRate = 48000.0f;
float baseCutoff = 200.0f;     // Base filter cutoff (Hz)
float filtEnvAmt = 8000.0f;     // Filter envelope amount (Hz)
float resonance = 0.1f;         // Filter resonance (0-1)
float oscMix = 0.5f;            // Saw1/Saw2 balance (0=saw1, 1=saw2)
float subLevel = 0.8f;          // Sub oscillator level (0-1)
float masterVol = 1.0f;         // Master volume (0-1)
float detune = 0.002f;          // Oscillator detune amount (0-0.05)
float pulseWidth = 0.5f;        // Pulse width for square waves (0.1-0.9)
int osc1Wave = 0;               // Osc1 waveform (0=saw, 1=square, 2=tri, 3=sine)
int osc2Wave = 0;               // Osc2 waveform (0=saw, 1=square, 2=tri, 3=sine)
float pitchBend = 0.0f;         // Pitch bend amount (-2 to +2 semitones)
float drive = 0.0f;             // Drive/distortion amount (0-1)

// Preset
struct SynthPreset {
    // Oscillator settings
    float detune;
    float oscMix;
    float subLevel;
    int osc1Wave;
    int osc2Wave;
    float pulseWidth;
    
    // Filter settings
    float baseCutoff;
    float resonance;
    float filtEnvAmt;
    
    // Amp envelope settings
    float ampAttack, ampDecay, ampSustain, ampRelease;
    
    // Filter envelope settings
    float filtAttack, filtDecay, filtSustain, filtRelease;
    
    // Effects
    float drive;
    float masterVol;
};

// Predefined presets
SynthPreset presets[8] = {
    // Preset 0: Init - Your current default settings
    {0.002f, 0.5f, 0.8f, 0, 0, 0.5f, 200.0f, 0.1f, 8000.0f,
     0.001f, 0.1f, 1.0f, 0.1f, 0.001f, 0.05f, 0.0f, 0.05f, 0.0f, 1.0f},
    
    // Preset 1: Classic Analog Lead
    {0.005f, 0.3f, 0.2f, 0, 0, 0.5f, 1200.0f, 0.7f, 4000.0f, 
     0.001f, 0.2f, 0.8f, 0.3f, 0.001f, 0.1f, 0.0f, 0.2f, 0.1f, 0.8f},
    
    // Preset 2: Warm Pad
    {0.008f, 0.5f, 0.6f, 0, 0, 0.5f, 800.0f, 0.3f, 2000.0f,
     0.8f, 1.2f, 0.9f, 1.5f, 0.5f, 0.8f, 0.4f, 1.0f, 0.0f, 0.7f},
    
    // Preset 3: Plucky Bass
    {0.002f, 0.1f, 0.9f, 1, 1, 0.3f, 400.0f, 0.8f, 6000.0f,
     0.001f, 0.05f, 0.6f, 0.1f, 0.001f, 0.03f, 0.0f, 0.05f, 0.2f, 0.9f},
    
    // Preset 4: Ethereal Lead
    {0.015f, 0.7f, 0.3f, 3, 2, 0.5f, 2000.0f, 0.2f, 3000.0f,
     0.3f, 0.8f, 0.7f, 2.0f, 0.2f, 0.6f, 0.3f, 1.5f, 0.0f, 0.6f},
    
    // Preset 5: Aggressive Sync Lead  
    {0.025f, 0.8f, 0.4f, 1, 0, 0.7f, 1800.0f, 0.9f, 7000.0f,
     0.001f, 0.1f, 0.7f, 0.2f, 0.001f, 0.05f, 0.0f, 0.1f, 0.4f, 0.8f},
    
    // Preset 6: Deep Sub Bass
    {0.001f, 0.2f, 1.0f, 0, 0, 0.5f, 300.0f, 0.5f, 1000.0f,
     0.001f, 0.3f, 0.9f, 0.8f, 0.1f, 0.2f, 0.1f, 0.3f, 0.1f, 1.0f},
    
    // Preset 7: Pulse Wave Lead
    {0.003f, 0.6f, 0.1f, 1, 1, 0.2f, 1500.0f, 0.6f, 5000.0f,
     0.01f, 0.15f, 0.8f, 0.4f, 0.01f, 0.08f, 0.0f, 0.15f, 0.0f, 0.7f}
};

int currentPreset = 0;

// MIDI CC value mapping functions
float MapTime(uint8_t val, float minT, float maxT) {
    float norm = static_cast<float>(val) / 127.0f;
    // Logarithmic curve for more musical ADSR response
    norm = norm * norm;  // Square for exponential feel
    return minT + norm * (maxT - minT);
}

float MapSustain(uint8_t val) {
    return static_cast<float>(val) / 127.0f;
}

float MapCutoff(uint8_t val) {
    // Exponential mapping: 80Hz to 8kHz
    float norm = static_cast<float>(val) / 127.0f;
    return 80.0f * powf(100.0f, norm);  // 80 * 100^norm = 80-8000Hz range
}

float MapResonance(uint8_t val) {
    float norm = static_cast<float>(val) / 127.0f;
    return 0.1f + norm * 0.85f;  // 0.1 to 0.95 range
}

// Forward declarations
void SetOscillatorWaveforms();
void UpdateOscFreqs();
void PrintCurrentPreset();

// Preset management
void LoadPreset(int presetNum) {
    if (presetNum < 0 || presetNum >= 8) return;
    
    currentPreset = presetNum;
    SynthPreset& p = presets[presetNum];
    
    // Load oscillator settings
    detune = p.detune;
    oscMix = p.oscMix;
    subLevel = p.subLevel;
    osc1Wave = p.osc1Wave;
    osc2Wave = p.osc2Wave;
    pulseWidth = p.pulseWidth;
    
    // Load filter settings
    baseCutoff = p.baseCutoff;
    resonance = p.resonance;
    filtEnvAmt = p.filtEnvAmt;
    
    // Load amp envelope settings
    ampEnv.SetTime(ADSR_SEG_ATTACK, p.ampAttack);
    ampEnv.SetTime(ADSR_SEG_DECAY, p.ampDecay);
    ampEnv.SetSustainLevel(p.ampSustain);
    ampEnv.SetTime(ADSR_SEG_RELEASE, p.ampRelease);
    
    // Load filter envelope settings
    filtEnv.SetTime(ADSR_SEG_ATTACK, p.filtAttack);
    filtEnv.SetTime(ADSR_SEG_DECAY, p.filtDecay);
    filtEnv.SetSustainLevel(p.filtSustain);
    filtEnv.SetTime(ADSR_SEG_RELEASE, p.filtRelease);
    
    // Load effects
    drive = p.drive;
    masterVol = p.masterVol;
    
    // Update hardware
    SetOscillatorWaveforms();
    UpdateOscFreqs();
}

// Debug function to print current parameter values
void PrintCurrentPreset() {
    hw.PrintLine("=== Current Preset Values ===");
    hw.PrintLine("// Copy this into presets array:");
    hw.PrintLine("{%.3ff, %.3ff, %.3ff, %d, %d, %.3ff, %.1ff, %.3ff, %.1ff,", 
                 detune, oscMix, subLevel, osc1Wave, osc2Wave, pulseWidth, baseCutoff, resonance, filtEnvAmt);
    
    // Get current envelope values (approximate)
    hw.PrintLine(" %.3ff, %.3ff, %.3ff, %.3ff, %.3ff, %.3ff, %.3ff, %.3ff, %.3ff, %.3ff},",
                 0.001f, 0.1f, 1.0f, 0.1f,  // amp envelope (defaults, would need proper getter)
                 0.001f, 0.05f, 0.0f, 0.05f, // filt envelope (defaults)
                 drive, masterVol);
    hw.PrintLine("========================");
}

// Waveform management
void SetOscillatorWaveforms() {
    uint8_t waveforms[] = {
        Oscillator::WAVE_POLYBLEP_SAW,
        Oscillator::WAVE_POLYBLEP_SQUARE,
        Oscillator::WAVE_POLYBLEP_TRI,
        Oscillator::WAVE_SIN
    };
    
    saw1.SetWaveform(waveforms[osc1Wave]);
    saw2.SetWaveform(waveforms[osc2Wave]);
    
    // Update pulse width for square waves
    if (osc1Wave == 1) saw1.SetPw(pulseWidth);
    if (osc2Wave == 1) saw2.SetPw(pulseWidth);
}

// Oscillator frequency management
void UpdateOscFreqs() {
    if (current_note < 0) return;
    
    // Calculate base frequency with pitch bend
    float noteWithBend = current_note + pitchBend;
    float base = mtof(noteWithBend);
    
    saw1.SetFreq(base * (1.0f - detune));   // Variable detune down
    saw2.SetFreq(base * (1.0f + detune));   // Variable detune up
    subOsc.SetFreq(base * 0.5f);            // Sub oscillator (octave down)
}

void PushNote(int note)
{
    // Add note to stack if not already there
    if (!held[note] && stack_size < 128) {
        note_stack[stack_size++] = note;
        held[note] = true;
    }
    current_note = note;
    UpdateOscFreqs();
}

void PopNote(int note)
{
    if (!held[note]) return;
    
    held[note] = false;
    int previous_note = current_note;
    
    // Find and remove note from stack
    for (int i = 0; i < stack_size; i++) {
        if (note_stack[i] == note) {
            // Shift remaining notes down
            for (int j = i; j < stack_size - 1; j++) {
                note_stack[j] = note_stack[j + 1];
            }
            stack_size--;
            break;
        }
    }
    
    // Set current note to most recent (top of stack)
    if (stack_size > 0) {
        current_note = note_stack[stack_size - 1];
        UpdateOscFreqs();
        // Only retrigger envelopes if switching to a different note
        if (current_note != previous_note) {
            ampEnv.Retrigger(false);
            filtEnv.Retrigger(false);
        }
    } else {
        current_note = -1;
    }
}

// MIDI event handlers

void HandleNoteOn(uint8_t note, uint8_t velocity)
{
    if (velocity == 0) {
        PopNote(note);  // Treat velocity 0 as note off
        return;
    }

    // Add note to stack and set as current
    PushNote(note);
    
    // Non-legato: always retrigger envelopes on new note
    gate_state = true;
    ampEnv.Retrigger(false);
    filtEnv.Retrigger(false);
}

void HandleNoteOff(uint8_t note)
{
    PopNote(note);
    
    // Release gate when no notes are held
    if (stack_size == 0) {
        gate_state = false;
    }
}


void HandleControlChange(uint8_t control, uint8_t value)
{
    switch(control) {
        // Modulation wheel - controls pulse width
        case 1:
            pulseWidth = MapTime(value, 0.1f, 0.9f);  // 10%-90% pulse width
            SetOscillatorWaveforms();  // Update pulse width immediately
            break;
            
        // Amp envelope controls
        case CC_AMP_ATTACK:
            ampEnv.SetTime(ADSR_SEG_ATTACK, MapTime(value, 0.001f, 3.0f));
            break;
        case CC_AMP_DECAY:
            ampEnv.SetTime(ADSR_SEG_DECAY, MapTime(value, 0.005f, 3.0f));
            break;
        case CC_AMP_SUSTAIN:
            ampEnv.SetSustainLevel(MapSustain(value));
            break;
        case CC_AMP_RELEASE:
            ampEnv.SetTime(ADSR_SEG_RELEASE, MapTime(value, 0.01f, 5.0f));
            break;

        // Filter envelope controls
        case CC_FILT_ATTACK:
            filtEnv.SetTime(ADSR_SEG_ATTACK, MapTime(value, 0.001f, 3.0f));
            break;
        case CC_FILT_DECAY:
            filtEnv.SetTime(ADSR_SEG_DECAY, MapTime(value, 0.005f, 3.0f));
            break;
        case CC_FILT_SUSTAIN:
            filtEnv.SetSustainLevel(MapSustain(value));
            break;
        case CC_FILT_RELEASE:
            filtEnv.SetTime(ADSR_SEG_RELEASE, MapTime(value, 0.01f, 5.0f));
            break;

        // Filter controls
        case CC_CUTOFF:
            baseCutoff = MapCutoff(value);
            break;
        case CC_RESONANCE:
            resonance = MapResonance(value);
            break;
        case CC_FILT_ENV_AMT:
            filtEnvAmt = MapTime(value, 0.0f, 8000.0f);  // 0-8kHz envelope modulation
            break;

        // Oscillator controls
        case CC_OSC_MIX:
            oscMix = MapSustain(value);  // 0-1 balance between saws
            break;
        case CC_SUB_LEVEL:
            subLevel = MapSustain(value);  // 0-1 sub level
            break;
        case CC_DETUNE:
            detune = MapTime(value, 0.0f, 0.05f);  // 0-5% detune
            UpdateOscFreqs();  // Update frequencies immediately
            break;
        case CC_OSC1_WAVE:
            osc1Wave = (value * 3) / 127;  // 0-3 waveform selection
            if (osc1Wave > 3) osc1Wave = 3;
            SetOscillatorWaveforms();
            break;
        case CC_OSC2_WAVE:
            osc2Wave = (value * 3) / 127;  // 0-3 waveform selection
            if (osc2Wave > 3) osc2Wave = 3;
            SetOscillatorWaveforms();
            break;
        case CC_DRIVE:
            drive = MapSustain(value);  // 0-1 drive amount
            break;

        // Master volume
        case CC_MASTER_VOL:
            masterVol = MapSustain(value);  // 0-1 master volume
            break;
            
    }
}

// Audio processing callback
void AudioCallback(AudioHandle::InputBuffer in, AudioHandle::OutputBuffer out, size_t size) {
    (void)in;

    for (size_t i = 0; i < size; i++) {
        // Process envelopes
        float ampVal = ampEnv.Process(gate_state);
        float filtEnvVal = filtEnv.Process(gate_state);

        // Calculate filter cutoff (base + envelope modulation)
        float cutoff = baseCutoff + filtEnvVal * filtEnvAmt;
        cutoff = fclamp(cutoff, 40.0f, sampleRate * 0.45f);  // Clamp to safe range
        
        filt.SetFreq(cutoff);
        filt.SetRes(resonance);

        // Mix oscillators with variable balance and sub level
        float saw1_sig = saw1.Process() * (1.0f - oscMix);
        float saw2_sig = saw2.Process() * oscMix;
        float sub_sig = subOsc.Process() * subLevel;
        float sig = saw1_sig + saw2_sig + sub_sig;
        
        // Apply filter (lowpass)
        filt.Process(sig);
        sig = filt.Low();

        // Apply drive/distortion
        if (drive > 0.0f) {
            sig = tanhf(sig * (1.0f + drive * 4.0f));  // Soft saturation
        }

        // Apply amplitude envelope and master volume
        sig *= ampVal * masterVol;
        
        out[0][i] = out[1][i] = sig;  // Mono output
    }
}

// MIDI event dispatcher
void HandleMidiEvent(MidiEvent msg) {
    switch (msg.type) {
        case NoteOn: {
            auto n = msg.AsNoteOn();
            HandleNoteOn(n.note, n.velocity);
            break;
        }
        case NoteOff: {
            auto n = msg.AsNoteOff();
            HandleNoteOff(n.note);
            break;
        }
        case ControlChange: {
            auto cc = msg.AsControlChange();
            HandleControlChange(cc.control_number, cc.value);
            break;
        }
        case PitchBend: {
            auto pb = msg.AsPitchBend();
            // Daisy already centers the value: pb.value = raw_value - 8192
            // So pb.value ranges from -8192 to +8191, with 0 = center
            pitchBend = static_cast<float>(pb.value) / 8192.0f * 2.0f;  // ±2 semitones
            UpdateOscFreqs();
            break;
        }
        case ProgramChange: {
            auto pc = msg.AsProgramChange();
            if (pc.program == 20) {
                PrintCurrentPreset();
                return;
            }
            LoadPreset(pc.program % 8);  // Wrap to 0-7 range
            break;
        }
        default:
            // Ignore unhandled MIDI message types
            break;
    }
}

// Main initialization and loop
int main(void) {
    // Initialize hardware
    hw.Configure();
    hw.Init();
    sampleRate = hw.AudioSampleRate();

    // Initialize MIDI
    MidiUsbHandler::Config midi_cfg;
    midi_cfg.transport_config.periph = MidiUsbTransport::Config::INTERNAL;
    midi.Init(midi_cfg);

    // Initialize oscillators
    saw1.Init(sampleRate);
    saw2.Init(sampleRate);
    subOsc.Init(sampleRate);
    
    SetOscillatorWaveforms();  // Set initial waveforms
    subOsc.SetWaveform(Oscillator::WAVE_POLYBLEP_SQUARE);
    
    saw1.SetAmp(1.0f);
    saw2.SetAmp(1.0f);
    subOsc.SetAmp(0.8f);

    // Initialize envelopes
    ampEnv.Init(sampleRate);
    ampEnv.SetTime(ADSR_SEG_ATTACK, 0.001f);
    ampEnv.SetTime(ADSR_SEG_DECAY, 0.1f);
    ampEnv.SetSustainLevel(1.0f);
    ampEnv.SetTime(ADSR_SEG_RELEASE, 0.1f);

    filtEnv.Init(sampleRate);
    filtEnv.SetTime(ADSR_SEG_ATTACK, 0.001f);
    filtEnv.SetTime(ADSR_SEG_DECAY, 0.05f);
    filtEnv.SetSustainLevel(0.0f);  // Plucky filter envelope
    filtEnv.SetTime(ADSR_SEG_RELEASE, 0.05f);

    // Initialize filter
    filt.Init(sampleRate);
    filt.SetFreq(baseCutoff);
    filt.SetRes(resonance);

    // Load default preset
    LoadPreset(0);  // Start with preset 0 (Classic Analog Lead)

    // Start audio processing
    hw.StartAudio(AudioCallback);

    // Main MIDI processing loop
    while (1) {
        midi.Listen();
        while (midi.HasEvents()) {
            HandleMidiEvent(midi.PopEvent());
        }
    }
}
