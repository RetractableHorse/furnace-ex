# Furnace Generative Workspace (`src/gen/`)

A generative music module for Furnace Tracker, targeting **YM2612 (Sega Genesis) FM synthesis**. Generates FM instrument patches and pattern data using style-aware constraints, seedable RNG, and role-based musical logic.

## Overview

The Generative Workspace adds a new panel to Furnace's GUI (accessible via **Window > Generative Workspace**) that lets you:

- **Generate FM patches** constrained by musical role (lead, bass, pad, rhythm, SFX, slap bass, distortion guitar)
- **Mutate existing patches** by randomizing a subset of parameters within style constraints
- **Audition patches** in real-time via Furnace's engine before committing
- **Generate pattern data** with role-aware rhythm grids, scale-quantized melodies, velocity dynamics, and optional effects (portamento, vibrato)
- **Choose from built-in style presets** modeled after classic Sega Genesis soundtracks

All generation is **purely additive** — existing Furnace functionality is untouched.

## Architecture

```
src/gen/
  genUtil.h / .cpp        Core utilities (PRNG, scales, note conversion, clamping)
  styleEngine.h / .cpp    Style preset data structures and management
  stylePresets.h / .cpp    Built-in preset definitions (5 presets)
  patchGen.h / .cpp        FM instrument patch generation and mutation
  patternGen.h / .cpp      Pattern data generation (rhythm, pitch, velocity, effects)
  genWorkspace.h / .cpp    Main coordinator — owns generators, bridges to DivEngine
  guiGen.h / .cpp          ImGui panel (implements FurnaceGUI::drawGenWorkspace())
```

### Dependency Flow

```
genUtil  <--  styleEngine  <--  stylePresets
                  |
                  v
              patchGen  (+ engine/instrument.h)
              patternGen (+ engine/pattern.h in .cpp only)
                  |
                  v
            genWorkspace  (+ engine/engine.h)
                  |
                  v
              guiGen      (+ gui/gui.h, imgui.h)
```

## Components

### GenRNG (`genUtil.h`)

Seedable **xoshiro128\*\*** PRNG providing:
- `randInt(min, max)` — inclusive integer range
- `randFloat()` — [0.0, 1.0)
- `weightedPick(weights, count)` — weighted random selection
- `pick(vector)` — uniform random from vector

Deterministic output for a given seed, enabling reproducible generation.

### Scale System (`genUtil.h`)

13 scale types:
| Scale | Intervals |
|-------|-----------|
| Minor (Natural) | 0 2 3 5 7 8 10 |
| Harmonic Minor | 0 2 3 5 7 8 11 |
| Melodic Minor | 0 2 3 5 7 9 11 |
| Phrygian | 0 1 3 5 7 8 10 |
| Phrygian Dominant | 0 1 4 5 7 8 10 |
| Dorian | 0 2 3 5 7 9 10 |
| Mixolydian | 0 2 4 5 7 9 10 |
| Major | 0 2 4 5 7 9 11 |
| Pentatonic Minor | 0 3 5 7 10 |
| Pentatonic Major | 0 2 4 7 9 |
| Chromatic | 0 1 2 3 4 5 6 7 8 9 10 11 |
| Locrian | 0 1 3 5 6 8 10 |
| Blues | 0 3 5 6 7 10 |

Note values use **Furnace's native format**: `0 = C-(-5)`, `60 = C-0`, `179 = B-9`.

### Style Presets (`stylePresets.cpp`)

Five built-in presets, each defining tempo range, preferred scales, per-role operator constraints, and pattern style parameters:

| Preset | Reference | Tempo | Key Scales | Character |
|--------|-----------|-------|------------|-----------|
| **Thunder Force** | Thunder Force III/IV | 148-180 BPM | Minor, Harmonic Minor | Aggressive leads, wide vibrato, high feedback |
| **Streets of Rage** | Streets of Rage 1-3 | 100-130 BPM | Dorian, Blues | Funky bass, punchy rhythms, groove-focused |
| **Sonic** | Sonic the Hedgehog | 120-160 BPM | Major, Mixolydian | Bright FM, clean operators, upbeat |
| **M.U.S.H.A.** | M.U.S.H.A. / Musha Aleste | 130-165 BPM | Phrygian, Locrian | Dark/atmospheric, dissonant intervals |
| **Custom** | User-defined | 60-240 BPM | All | Wide-open defaults for experimentation |

### Patch Roles

Each role shapes both FM patch generation and pattern rhythm behavior:

| Role | FM Character | Rhythm Tendency |
|------|-------------|-----------------|
| **Lead** | Bright carriers, moderate feedback, algo 0-4 | Syncopated, fills between beats |
| **Bass** | High TL on upper ops, low mult, algo 0-2 | Strong downbeats, sparse offbeats |
| **Pad** | Slow attack/release, low feedback, algo 5-7 | Very sparse, long sustained notes |
| **Rhythm** | Fast AR/RR, short envelopes | 16th-note subdivisions |
| **SFX** | Wide parameter ranges, high detuning | Percussive bursts |
| **Slap Bass** | Punchy attack, quick decay, algo 0-1 | Syncopated funk patterns |
| **Dist. Guitar** | High feedback, algo 3-5, mid-range mult | Power-chord rhythms |

### Patch Generator (`patchGen.cpp`)

- `generate(role, constraints)` — Creates a `DivInstrument` with `type = DIV_INS_FM`, 4 operators, parameters randomized within the role's `OperatorConstraints`
- `mutate(source, role, constraints, N)` — Copies an instrument and randomly modifies N parameters, keeping them within constraints
- `describePatch(fm, buf, len)` — Generates a human-readable summary string (algorithm, feedback, carrier TL)

### Pattern Generator (`patternGen.cpp`)

Generation pipeline:
1. **Rhythm layer** — Probability grid based on role, density (0-100%), and syncopation
2. **Pitch layer** — Scale-quantized notes via weighted interval selection (step/third/fifth/octave/repeat) controlled by complexity (0-100%)
3. **Velocity layer** — Downbeat accents, beat-level dynamics, ghost notes on offbeats
4. **Effect layer** — Optional portamento (03xx) and vibrato (04xy) at low probability
5. **Chromatic pass** — Post-process inserts chromatic passing tones between adjacent notes, controlled by the style's `chromaticism` parameter

Writes directly into `DivPattern::newData[][]` using `DIV_PAT_NOTE`, `DIV_PAT_INS`, `DIV_PAT_VOL`, `DIV_PAT_FX(0)`, `DIV_PAT_FXVAL(0)`.

### GenWorkspace (`genWorkspace.cpp`)

Main coordinator that:
- Owns `PatchGenerator`, `PatternGenerator`, and `StyleEngine` instances
- Bridges to `DivEngine` for system detection, patch audition (`loadTempIns` + `noteOn`), and pattern access
- Manages seed state (auto-increment after each generation, or locked)
- `detectSystems()` checks for `DIV_SYSTEM_YM2612` and variants (ext, CSM, DualPCM)
- `commitPatch()` adds the generated instrument to the song via `addInstrumentPtr()`

### GUI Panel (`guiGen.cpp`)

Implements `FurnaceGUI::drawGenWorkspace()` as an ImGui window with sections:
- **Style** — Preset selector, key/scale combos
- **Seed** — Numeric input, lock checkbox, randomize button
- **Patch Generator** — Role selector, Generate/Mutate/Audition/Stop/Commit buttons, algorithm preview
- **Pattern Generator** — Channel/instrument/role selection, density/complexity sliders, octave range, effects toggle, Generate Pattern/Fill buttons

## Integration Points

### Modified Existing Files

| File | Changes |
|------|---------|
| `src/gui/gui.h` | Forward decl `GenWorkspace`, member pointer, `genWorkspaceOpen` bool, `GUI_WINDOW_GEN_WORKSPACE` enum, `drawGenWorkspace()` decl |
| `src/gui/gui.cpp` | Include, menu item under Window, `DECLARE_METRIC`/`MEASURE` for perf tracking, config save/load, constructor init, `bindEngine()` creation, `finish()` cleanup |
| `src/gui/doAction.cpp` | Window close handler case |
| `CMakeLists.txt` | `GEN_SOURCES` variable (7 .cpp files), appended to `GUI_SOURCES` |

### Build

All gen sources are compiled as part of `GUI_SOURCES` in the main `furnace` target. No new libraries or dependencies are introduced.

```cmake
set(GEN_SOURCES
  src/gen/genUtil.cpp
  src/gen/styleEngine.cpp
  src/gen/stylePresets.cpp
  src/gen/patchGen.cpp
  src/gen/patternGen.cpp
  src/gen/genWorkspace.cpp
  src/gen/guiGen.cpp
)
list(APPEND GUI_SOURCES ${GEN_SOURCES})
```

## Usage

1. Open Furnace with a song that includes a **YM2612/Genesis system**
2. Go to **Window > Generative Workspace**
3. Select a **style preset** (or use Custom)
4. Choose a **key and scale**
5. **Generate a patch**: pick a role, click Generate, then Audition to hear it
6. **Commit** the patch to add it to your instrument list
7. **Generate a pattern**: select a target channel, set density/complexity, click Generate Pattern
8. The pattern data is written into the current order's pattern for that channel

## Technical Notes

- `pattern.h` in Furnace's engine has **no include guard**. The gen module uses a forward declaration (`struct DivPattern;`) in `patternGen.h` and only includes `pattern.h` in `patternGen.cpp` to avoid double-inclusion.
- The `MEASURE` macro in `gui.cpp` requires a matching `DECLARE_METRIC(genWorkspace)` for performance profiling.
- Furnace note values: `0 = C-(-5)`, `60 = C-0`, `179 = B-9` — NOT the 0-119 range sometimes assumed.
- Pattern data uses `DivPattern::newData[row][col]`, not `data[][]`.
- All generation is client-side and deterministic for a given seed.

## Future Phases

This is **Phase 1** of the Generative Workspace. Planned future work:

- **Phase 2**: Arrangement generation (multi-channel, song structure, transitions)
- **Phase 3**: Style learning from existing Furnace songs
- **Phase 4**: Live performance mode with real-time parameter morphing
