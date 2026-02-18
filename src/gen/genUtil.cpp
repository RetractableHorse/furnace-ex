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

#include "genUtil.h"
#include <cstring>

// --- GenRNG (xoshiro128**) ---

uint32_t GenRNG::rotl(uint32_t x, int k) {
  return (x<<k)|(x>>(32-k));
}

void GenRNG::seed(uint32_t s) {
  // splitmix32 to initialize state from a single seed
  for (int i=0; i<4; i++) {
    s+=0x9e3779b9;
    uint32_t z=s;
    z=(z^(z>>16))*0x85ebca6b;
    z=(z^(z>>13))*0xc2b2ae35;
    z=z^(z>>16);
    state[i]=z;
  }
}

uint32_t GenRNG::next() {
  uint32_t result=rotl(state[1]*5,7)*9;
  uint32_t t=state[1]<<9;
  state[2]^=state[0];
  state[3]^=state[1];
  state[1]^=state[2];
  state[0]^=state[3];
  state[2]^=t;
  state[3]=rotl(state[3],11);
  return result;
}

int GenRNG::randInt(int min, int max) {
  if (min>=max) return min;
  uint32_t range=(uint32_t)(max-min+1);
  return min+(int)(next()%range);
}

float GenRNG::randFloat() {
  return (float)(next()>>8)/(float)(1<<24);
}

int GenRNG::weightedPick(const float* weights, int count) {
  float total=0.0f;
  for (int i=0; i<count; i++) total+=weights[i];
  if (total<=0.0f) return 0;
  float r=randFloat()*total;
  float accum=0.0f;
  for (int i=0; i<count; i++) {
    accum+=weights[i];
    if (r<accum) return i;
  }
  return count-1;
}

int GenRNG::pick(const std::vector<int>& v) {
  if (v.empty()) return 0;
  return v[randInt(0,(int)v.size()-1)];
}

GenRNG::GenRNG() {
  memset(state,0,sizeof(state));
  seed(12345);
}

// --- Scale system ---

static const char* scaleNames[GEN_SCALE_MAX]={
  "Minor (Natural)",
  "Harmonic Minor",
  "Melodic Minor",
  "Phrygian",
  "Phrygian Dominant",
  "Dorian",
  "Mixolydian",
  "Major",
  "Pentatonic Minor",
  "Pentatonic Major",
  "Chromatic",
  "Locrian",
  "Blues"
};

const char* genScaleName(GenScaleType scale) {
  if (scale<0||scale>=GEN_SCALE_MAX) return "Unknown";
  return scaleNames[scale];
}

// scale intervals (semitones from root)
static const int scaleMinor[]={0,2,3,5,7,8,10};
static const int scaleHarmonicMinor[]={0,2,3,5,7,8,11};
static const int scaleMelodicMinor[]={0,2,3,5,7,9,11};
static const int scalePhrygian[]={0,1,3,5,7,8,10};
static const int scalePhrygianDom[]={0,1,4,5,7,8,10};
static const int scaleDorian[]={0,2,3,5,7,9,10};
static const int scaleMixolydian[]={0,2,4,5,7,9,10};
static const int scaleMajor[]={0,2,4,5,7,9,11};
static const int scalePentMinor[]={0,3,5,7,10};
static const int scalePentMajor[]={0,2,4,7,9};
static const int scaleChromatic[]={0,1,2,3,4,5,6,7,8,9,10,11};
static const int scaleLocrian[]={0,1,3,5,6,8,10};
static const int scaleBlues[]={0,3,5,6,7,10};

struct ScaleData {
  const int* intervals;
  int len;
};

static const ScaleData scaleTable[GEN_SCALE_MAX]={
  {scaleMinor, 7},
  {scaleHarmonicMinor, 7},
  {scaleMelodicMinor, 7},
  {scalePhrygian, 7},
  {scalePhrygianDom, 7},
  {scaleDorian, 7},
  {scaleMixolydian, 7},
  {scaleMajor, 7},
  {scalePentMinor, 5},
  {scalePentMajor, 5},
  {scaleChromatic, 12},
  {scaleLocrian, 7},
  {scaleBlues, 6},
};

const int* genScaleIntervals(GenScaleType scale, int& outLen) {
  if (scale<0||scale>=GEN_SCALE_MAX) {
    outLen=7;
    return scaleMinor;
  }
  outLen=scaleTable[scale].len;
  return scaleTable[scale].intervals;
}

int genScaleNoteToFurnace(int root, GenScaleType scale, int degree, int octave) {
  int scaleLen;
  const int* intervals=genScaleIntervals(scale,scaleLen);

  // wrap degree into scale
  int octaveOffset=0;
  while (degree<0) {
    degree+=scaleLen;
    octaveOffset--;
  }
  octaveOffset+=degree/scaleLen;
  degree=degree%scaleLen;

  // Furnace note: 0=C-(-5), 60=C-0, 179=B-9
  // note = (octave+octaveOffset+5)*12 + root + intervals[degree]
  int note=root+intervals[degree]+((octave+octaveOffset+5)*12);
  return genClamp(note,0,179);
}

int genClamp(int val, int min, int max) {
  if (val<min) return min;
  if (val>max) return max;
  return val;
}

float genBpmToHz(int bpm, int speed) {
  if (speed<=0) speed=6;
  // Furnace tick rate: hz = bpm * speed / 2.5 (for 4/4 time with highlight=4)
  // More precisely: ticks_per_beat = speed, beats_per_min = bpm
  // hz = (bpm * speed) / 60.0 * rows_per_beat
  // For typical tracker: hz ~= bpm / 2.5 when speed=6
  return (float)(bpm*speed)/150.0f;
}

bool genIsMinorFamily(GenScaleType scale) {
  return (scale==GEN_SCALE_MINOR||scale==GEN_SCALE_HARMONIC_MINOR||
          scale==GEN_SCALE_MELODIC_MINOR||scale==GEN_SCALE_PHRYGIAN||
          scale==GEN_SCALE_PHRYGIAN_DOMINANT||scale==GEN_SCALE_DORIAN||
          scale==GEN_SCALE_LOCRIAN||scale==GEN_SCALE_BLUES||
          scale==GEN_SCALE_PENTATONIC_MINOR);
}

int genDegreesToSemitones(int degree, GenScaleType scale) {
  int scaleLen;
  const int* intervals=genScaleIntervals(scale,scaleLen);
  int octOffset=0;
  while (degree<0) {
    degree+=scaleLen;
    octOffset--;
  }
  octOffset+=degree/scaleLen;
  degree=degree%scaleLen;
  return intervals[degree]+octOffset*12;
}

void genShuffle(int* arr, int count, GenRNG& rng) {
  for (int i=count-1; i>0; i--) {
    int j=rng.randInt(0,i);
    int tmp=arr[i];
    arr[i]=arr[j];
    arr[j]=tmp;
  }
}
