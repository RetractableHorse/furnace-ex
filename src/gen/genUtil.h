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

#ifndef _GEN_UTIL_H
#define _GEN_UTIL_H

#include <cstdint>
#include <vector>

// seedable PRNG (xoshiro128**)
class GenRNG {
  uint32_t state[4];
  uint32_t rotl(uint32_t x, int k);
public:
  void seed(uint32_t s);
  uint32_t next();
  // random int in [min, max] inclusive
  int randInt(int min, int max);
  // random float in [0.0, 1.0)
  float randFloat();
  // pick random element index from a weighted distribution
  int weightedPick(const float* weights, int count);
  // pick random element from a vector
  int pick(const std::vector<int>& v);

  GenRNG();
};

// scale definitions
enum GenScaleType {
  GEN_SCALE_MINOR=0,
  GEN_SCALE_HARMONIC_MINOR,
  GEN_SCALE_MELODIC_MINOR,
  GEN_SCALE_PHRYGIAN,
  GEN_SCALE_PHRYGIAN_DOMINANT,
  GEN_SCALE_DORIAN,
  GEN_SCALE_MIXOLYDIAN,
  GEN_SCALE_MAJOR,
  GEN_SCALE_PENTATONIC_MINOR,
  GEN_SCALE_PENTATONIC_MAJOR,
  GEN_SCALE_CHROMATIC,
  GEN_SCALE_LOCRIAN,
  GEN_SCALE_BLUES,
  GEN_SCALE_MAX
};

const char* genScaleName(GenScaleType scale);

// returns the intervals for a given scale (array of semitone offsets from root, length stored in outLen)
const int* genScaleIntervals(GenScaleType scale, int& outLen);

// map a scale degree + octave to a Furnace note value (C-0 = 0, C#0 = 1, ..., B-9 = 119)
// root is 0-11 (C=0, C#=1, ..., B=11)
int genScaleNoteToFurnace(int root, GenScaleType scale, int degree, int octave);

// clamp a value to [min, max]
int genClamp(int val, int min, int max);

// convert BPM to Furnace hz (approximate)
float genBpmToHz(int bpm, int speed);

// melodic contour shapes
enum GenContour {
  GEN_CONTOUR_ARCH=0,
  GEN_CONTOUR_INV_ARCH,
  GEN_CONTOUR_ASCENDING,
  GEN_CONTOUR_DESCENDING,
  GEN_CONTOUR_FLAT,
  GEN_CONTOUR_RANDOM,
  GEN_CONTOUR_MAX
};

// phrase structure forms
enum GenPhraseForm {
  GEN_PHRASE_AABA=0,
  GEN_PHRASE_ABAB,
  GEN_PHRASE_AAAB,
  GEN_PHRASE_ABAC,
  GEN_PHRASE_RANDOM,
  GEN_PHRASE_MAX
};

// groove feel types
enum GenGrooveType {
  GEN_GROOVE_STRAIGHT=0,
  GEN_GROOVE_SHUFFLE,
  GEN_GROOVE_FUNK,
  GEN_GROOVE_DRIVING,
  GEN_GROOVE_HALFTIME,
  GEN_GROOVE_MAX
};

// determine if a scale type belongs to the minor family
bool genIsMinorFamily(GenScaleType scale);

// given a scale degree and scale type, return the semitone offset from the root
// (wrapping around for degrees > scaleLen, accounting for octave shifts)
int genDegreesToSemitones(int degree, GenScaleType scale);

// Fisher-Yates shuffle for an int array
void genShuffle(int* arr, int count, GenRNG& rng);

#endif
