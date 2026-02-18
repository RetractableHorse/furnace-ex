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

#ifndef _GEN_WORKSPACE_H
#define _GEN_WORKSPACE_H

#include "patchGen.h"
#include "patternGen.h"
#include "styleEngine.h"
#include "../engine/engine.h"

class GenWorkspace {
  DivEngine* engine;

public:
  PatchGenerator patchGen;
  PatternGenerator patternGen;
  StyleEngine styleEngine;

  // state
  bool ym2612Available;
  int auditChannel;        // channel to use for patch audition
  int auditNote;           // note to play for audition (Furnace note value)

  // current generated patch (before commit)
  DivInstrument currentPatch;
  bool hasPatch;
  char patchDesc[256];

  // generation params
  PatchRole currentRole;
  GenPatternParams patParams;
  uint32_t currentSeed;
  bool lockSeed;

  void init(DivEngine* e);
  void detectSystems();
  void populateParamsFromSong();

  // patch generation
  void generatePatch();
  void mutatePatch(int mutations);
  void auditPatch();
  void stopAudit();
  int commitPatch();  // returns new instrument index

  // pattern generation
  void generatePattern(int channel, int patIdx);
  void generateFill(int channel, int patIdx, int startRow, int endRow);

  // seed management
  void randomizeSeed();

  GenWorkspace();
};

#endif
