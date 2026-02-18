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

#ifndef _PATCH_GEN_H
#define _PATCH_GEN_H

#include "genUtil.h"
#include "styleEngine.h"
#include "../engine/instrument.h"

class PatchGenerator {
  GenRNG rng;

  void applyOperatorConstraints(DivInstrumentFM::Operator& op, const OperatorConstraints& c);

public:
  // generate a new FM patch based on role and style constraints
  DivInstrument generate(PatchRole role, const PatchRoleConstraints& constraints);

  // mutate an existing instrument — randomize N parameters within constraints
  DivInstrument mutate(const DivInstrument& source, PatchRole role, const PatchRoleConstraints& constraints, int mutations);

  // set RNG seed
  void setSeed(uint32_t seed);

  // get a descriptive summary of an FM patch
  static void describePatch(const DivInstrumentFM& fm, char* buf, int bufLen);
};

#endif
