/**
 * Furnace Tracker - multi-system chiptune tracker
 * Copyright (C) 2021-2026 tildearrow and contributors
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, write to the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 */

#ifndef _PATTERN_GEN_H
#define _PATTERN_GEN_H

#include "genUtil.h"
#include "styleEngine.h"

struct DivPattern;

// --- Motif system ---

// a single note within a motif, stored relative to a reference degree
struct MotifNote {
  int relativeDegree;   // scale degrees relative to motif root
  int rowOffset;        // position in rows from motif start
  int duration;         // note duration in rows (0=sustain until next note)
  int velOffset;        // velocity offset from groove template (-20 to +20)
  bool isRest;          // true = rhythmic rest

  MotifNote():
    relativeDegree(0),rowOffset(0),duration(0),velOffset(0),isRest(false) {}
};

// a motif: a short musical idea (2-8 notes) spanning up to 1 bar
struct Motif {
  MotifNote notes[8];
  int noteCount;
  int lengthInRows;     // total rows this motif occupies (typically 16)

  Motif(): noteCount(0),lengthInRows(16) {}
};

// --- Groove system ---

// velocity template for one bar (16 rows at standard grid)
struct GrooveTemplate {
  int velocity[16];

  GrooveTemplate() {
    for (int i=0; i<16; i++) velocity[i]=0x68;
    velocity[0]=0x7F;
    velocity[4]=0x70;
    velocity[8]=0x74;
    velocity[12]=0x70;
  }
};

// --- Harmonic system ---

// chord information for one bar
struct BarChord {
  int rootDegree;         // root as scale degree (0=tonic, 3=IV, 4=V, etc.)
  int chordTones[4];      // scale degrees relative to chord root
  int chordToneCount;

  BarChord(): rootDegree(0),chordToneCount(3) {
    chordTones[0]=0;  // root
    chordTones[1]=2;  // third
    chordTones[2]=4;  // fifth
    chordTones[3]=6;  // seventh
  }

  bool isChordTone(int scaleDegree, int scaleLen) const;
};

// --- Phrase system ---

// how to place one motif instance within a phrase
struct MotifPlacement {
  int motifIndex;          // which motif (0=A, 1=B)
  int barIndex;            // which bar
  int transposeDegrees;    // transpose by this many scale degrees
  bool invertContour;

  MotifPlacement():
    motifIndex(0),barIndex(0),transposeDegrees(0),invertContour(false) {}
};

// the full plan for one pattern
struct Phrase {
  MotifPlacement placements[16];
  int placementCount;
  int totalBars;

  Phrase(): placementCount(0),totalBars(4) {}
};

// --- Pattern parameters ---

struct GenPatternParams {
  // existing fields (unchanged)
  int channel;
  int insIndex;
  PatchRole role;
  int scaleRoot;            // 0-11 (C=0 ... B=11)
  GenScaleType scaleType;
  int density;              // 0-100
  int complexity;           // 0-100
  int octaveMin;
  int octaveMax;
  int patternLength;
  bool allowEffects;

  // new fields (backward-compatible additions)
  int rowsPerBeat;          // metric grid (default 4, from hilightA)
  int rowsPerBar;           // metric grid (default 16, from hilightB)
  GenGrooveType grooveType;
  GenPhraseForm phraseForm;
  GenContour contourHint;
  int motifLengthHint;      // 0=auto
  int articulationGap;      // 0=legato, 1-4=staccato rows
  float chordToneEmphasis;  // 0.0-1.0

  GenPatternParams():
    channel(0),
    insIndex(0),
    role(ROLE_LEAD),
    scaleRoot(9),
    scaleType(GEN_SCALE_MINOR),
    density(60),
    complexity(50),
    octaveMin(3),
    octaveMax(5),
    patternLength(64),
    allowEffects(true),
    rowsPerBeat(4),
    rowsPerBar(16),
    grooveType(GEN_GROOVE_STRAIGHT),
    phraseForm(GEN_PHRASE_RANDOM),
    contourHint(GEN_CONTOUR_RANDOM),
    motifLengthHint(0),
    articulationGap(0),
    chordToneEmphasis(0.7f) {}
};

// --- Pattern Generator ---

class PatternGenerator {
  GenRNG rng;

  // pipeline steps
  int computeBarCount(int patternLength, int rowsPerBar);
  void generateChordProgression(BarChord* chords, int barCount, GenScaleType scaleType, int complexity);
  GrooveTemplate buildGrooveTemplate(GenGrooveType type);
  Phrase buildPhrase(GenPhraseForm form, int barCount, int motifCount);
  void applyMelodicContour(Motif& motif, GenContour contour, int complexity, int scaleLen);
  void applyChordToneGravity(Motif& motif, const BarChord& chord, int scaleLen, int rowsPerBeat, float emphasis);
  void writeMotifToPattern(DivPattern* pat, const Motif& motif, const MotifPlacement& placement,
                            const BarChord& chord, const GrooveTemplate& groove,
                            const GenPatternParams& params, int scaleLen, const int* intervals,
                            int barStartRow, int degreeRangeBase);
  void applyEffects(DivPattern* pat, const GenPatternParams& params, const StylePreset& style,
                     int startRow, int endRow);
  void applyNoteOffs(DivPattern* pat, int startRow, int endRow, int articulationGap, PatchRole role);
  void applyChromaticPassing(DivPattern* pat, const GenPatternParams& params,
                              float chromaticism, int startRow, int endRow);

  // role-specific motif generators
  Motif generateRoleMotif(PatchRole role, int density, int complexity,
                           float syncopation, int rowsPerBar, int motifLengthHint, int scaleLen);
  Motif generateBassMotif(int density, int complexity, float syncopation, int rowsPerBar, int scaleLen);
  Motif generateLeadMotif(int density, int complexity, float syncopation, int rowsPerBar, int scaleLen);
  Motif generatePadMotif(int density, int complexity, float syncopation, int rowsPerBar, int scaleLen);
  Motif generateRhythmMotif(int density, int complexity, float syncopation, int rowsPerBar, int scaleLen);
  Motif generateSfxMotif(int density, int complexity, float syncopation, int rowsPerBar, int scaleLen);
  Motif generateSlapBassMotif(int density, int complexity, float syncopation, int rowsPerBar, int scaleLen);
  Motif generateDistGuitarMotif(int density, int complexity, float syncopation, int rowsPerBar, int scaleLen);

public:
  // public API (signatures unchanged)
  void generate(DivPattern* pat, const GenPatternParams& params, const StylePreset& style);
  void generateFill(DivPattern* pat, const GenPatternParams& params, const StylePreset& style, int startRow, int endRow);
  void setSeed(uint32_t seed);
};

#endif
