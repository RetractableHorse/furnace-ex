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

#include "patchGen.h"
#include <cstdio>
#include <cstring>

void PatchGenerator::applyOperatorConstraints(DivInstrumentFM::Operator& op, const OperatorConstraints& c) {
  op.tl=(unsigned char)rng.randInt(c.tlMin,c.tlMax);
  op.ar=(unsigned char)rng.randInt(c.arMin,c.arMax);
  op.dr=(unsigned char)rng.randInt(c.drMin,c.drMax);
  op.sl=(unsigned char)rng.randInt(c.slMin,c.slMax);
  op.rr=(unsigned char)rng.randInt(c.rrMin,c.rrMax);
  op.mult=(unsigned char)rng.randInt(c.multMin,c.multMax);
  op.dt=(unsigned char)rng.randInt(c.dtMin,c.dtMax);
  op.d2r=(unsigned char)rng.randInt(c.d2rMin,c.d2rMax);
  op.rs=(unsigned char)rng.randInt(c.rsMin,c.rsMax);
  op.am=(unsigned char)rng.randInt(c.amMin,c.amMax);
  op.enable=true;
}

DivInstrument PatchGenerator::generate(PatchRole role, const PatchRoleConstraints& constraints) {
  DivInstrument ins;
  ins.type=DIV_INS_FM;
  ins.fm.ops=4; // YM2612 always uses 4 operators

  // pick algorithm
  if (!constraints.algorithms.empty()) {
    ins.fm.alg=(unsigned char)rng.pick(constraints.algorithms);
  } else {
    ins.fm.alg=(unsigned char)rng.randInt(0,7);
  }

  // pick feedback
  ins.fm.fb=(unsigned char)rng.randInt(constraints.feedbackMin,constraints.feedbackMax);

  // generate each operator
  for (int i=0; i<4; i++) {
    applyOperatorConstraints(ins.fm.op[i],constraints.ops[i]);
  }

  // set name based on role
  char nameBuf[64];
  snprintf(nameBuf,sizeof(nameBuf),"Gen %s",patchRoleName(role));
  ins.name=nameBuf;

  return ins;
}

DivInstrument PatchGenerator::mutate(const DivInstrument& source, PatchRole role, const PatchRoleConstraints& constraints, int mutations) {
  DivInstrument ins=source;

  for (int m=0; m<mutations; m++) {
    // pick a random parameter to mutate
    int target=rng.randInt(0,5);
    switch (target) {
      case 0: // algorithm
        if (!constraints.algorithms.empty()) {
          ins.fm.alg=(unsigned char)rng.pick(constraints.algorithms);
        }
        break;
      case 1: // feedback
        ins.fm.fb=(unsigned char)rng.randInt(constraints.feedbackMin,constraints.feedbackMax);
        break;
      default: {
        // mutate a random operator parameter
        int opIdx=rng.randInt(0,3);
        const OperatorConstraints& c=constraints.ops[opIdx];
        DivInstrumentFM::Operator& op=ins.fm.op[opIdx];
        int param=rng.randInt(0,7);
        switch (param) {
          case 0: op.tl=(unsigned char)rng.randInt(c.tlMin,c.tlMax); break;
          case 1: op.ar=(unsigned char)rng.randInt(c.arMin,c.arMax); break;
          case 2: op.dr=(unsigned char)rng.randInt(c.drMin,c.drMax); break;
          case 3: op.sl=(unsigned char)rng.randInt(c.slMin,c.slMax); break;
          case 4: op.rr=(unsigned char)rng.randInt(c.rrMin,c.rrMax); break;
          case 5: op.mult=(unsigned char)rng.randInt(c.multMin,c.multMax); break;
          case 6: op.dt=(unsigned char)rng.randInt(c.dtMin,c.dtMax); break;
          case 7: op.d2r=(unsigned char)rng.randInt(c.d2rMin,c.d2rMax); break;
        }
        break;
      }
    }
  }

  return ins;
}

void PatchGenerator::setSeed(uint32_t seed) {
  rng.seed(seed);
}

void PatchGenerator::describePatch(const DivInstrumentFM& fm, char* buf, int bufLen) {
  snprintf(buf,bufLen,"Algo %d | FB %d | MUL %d,%d,%d,%d | TL %d,%d,%d,%d",
    fm.alg,fm.fb,
    fm.op[0].mult,fm.op[1].mult,fm.op[2].mult,fm.op[3].mult,
    fm.op[0].tl,fm.op[1].tl,fm.op[2].tl,fm.op[3].tl);
}
