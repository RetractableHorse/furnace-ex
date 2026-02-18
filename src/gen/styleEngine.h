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

#ifndef _STYLE_ENGINE_H
#define _STYLE_ENGINE_H

#include "genUtil.h"
#include <vector>
#include <string>

enum PatchRole {
  ROLE_LEAD=0,
  ROLE_BASS,
  ROLE_PAD,
  ROLE_RHYTHM,
  ROLE_SFX,
  ROLE_SLAP_BASS,
  ROLE_DIST_GUITAR,
  ROLE_MAX
};

const char* patchRoleName(PatchRole role);

struct OperatorConstraints {
  int tlMin, tlMax;
  int arMin, arMax;
  int drMin, drMax;
  int slMin, slMax;
  int rrMin, rrMax;
  int multMin, multMax;
  int dtMin, dtMax;
  int d2rMin, d2rMax;
  int rsMin, rsMax;
  int amMin, amMax;

  OperatorConstraints():
    tlMin(0), tlMax(127),
    arMin(0), arMax(31),
    drMin(0), drMax(31),
    slMin(0), slMax(15),
    rrMin(0), rrMax(15),
    multMin(0), multMax(15),
    dtMin(0), dtMax(7),
    d2rMin(0), d2rMax(31),
    rsMin(0), rsMax(3),
    amMin(0), amMax(1) {}
};

struct PatchRoleConstraints {
  std::vector<int> algorithms;   // allowed algorithm indices
  int feedbackMin, feedbackMax;
  OperatorConstraints ops[4];

  PatchRoleConstraints():
    feedbackMin(0), feedbackMax(7) {}
};

struct StylePreset {
  std::string name;

  // global
  int tempoMin, tempoMax;
  std::vector<GenScaleType> preferredScales;

  // patch constraints per role
  PatchRoleConstraints roles[ROLE_MAX];

  // pattern style
  float rhythmDensity;      // 0.0-1.0
  float syncopation;        // 0.0-1.0
  float chromaticism;       // 0.0-1.0
  bool preferFastArpeggios;
  bool use16thSubdivisions;

  // pattern generation style hints
  GenGrooveType defaultGroove;
  GenPhraseForm defaultPhraseForm;
  float chordToneEmphasis;    // 0.0-1.0
  int roleMotifLength[ROLE_MAX]; // 0=auto, 8=half bar, 16=full bar

  StylePreset():
    name("Custom"),
    tempoMin(100), tempoMax(200),
    rhythmDensity(0.5f),
    syncopation(0.3f),
    chromaticism(0.2f),
    preferFastArpeggios(false),
    use16thSubdivisions(false),
    defaultGroove(GEN_GROOVE_STRAIGHT),
    defaultPhraseForm(GEN_PHRASE_RANDOM),
    chordToneEmphasis(0.7f) {
    for (int i=0; i<ROLE_MAX; i++) roleMotifLength[i]=0;
  }
};

class StyleEngine {
  std::vector<StylePreset> presets;
  int activePresetIdx;

public:
  StyleEngine();

  void loadBuiltinPresets();

  int getPresetCount() const;
  const StylePreset& getPreset(int idx) const;
  const StylePreset& getActivePreset() const;
  int getActivePresetIdx() const;
  void setActivePreset(int idx);

  const PatchRoleConstraints& getRoleConstraints(PatchRole role) const;

  // get a mutable reference to the custom preset (always last)
  StylePreset& getCustomPreset();
};

#endif
