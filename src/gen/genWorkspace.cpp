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

#include "genWorkspace.h"
#include <cstring>
#include <cstdlib>
#include <ctime>

GenWorkspace::GenWorkspace():
  engine(NULL),
  ym2612Available(false),
  auditChannel(0),
  auditNote(72),  // C-1 (middle-ish)
  hasPatch(false),
  currentRole(ROLE_LEAD),
  currentSeed(12345),
  lockSeed(false) {
  memset(patchDesc,0,sizeof(patchDesc));
}

void GenWorkspace::init(DivEngine* e) {
  engine=e;
  detectSystems();
  randomizeSeed();
}

void GenWorkspace::detectSystems() {
  ym2612Available=false;
  if (!engine) return;

  for (int i=0; i<engine->song.systemLen; i++) {
    DivSystem sys=engine->song.system[i];
    if (sys==DIV_SYSTEM_YM2612||
        sys==DIV_SYSTEM_YM2612_EXT||
        sys==DIV_SYSTEM_YM2612_DUALPCM||
        sys==DIV_SYSTEM_YM2612_DUALPCM_EXT||
        sys==DIV_SYSTEM_YM2612_CSM||
        sys==DIV_SYSTEM_GENESIS||
        sys==DIV_SYSTEM_GENESIS_EXT) {
      ym2612Available=true;
      break;
    }
  }
}

void GenWorkspace::generatePatch() {
  if (!lockSeed) {
    patchGen.setSeed(currentSeed);
  }

  const PatchRoleConstraints& constraints=styleEngine.getRoleConstraints(currentRole);
  currentPatch=patchGen.generate(currentRole,constraints);
  hasPatch=true;

  PatchGenerator::describePatch(currentPatch.fm,patchDesc,sizeof(patchDesc));

  if (!lockSeed) {
    currentSeed++;
  }
}

void GenWorkspace::mutatePatch(int mutations) {
  if (!hasPatch) {
    generatePatch();
    return;
  }

  patchGen.setSeed(currentSeed);
  const PatchRoleConstraints& constraints=styleEngine.getRoleConstraints(currentRole);
  currentPatch=patchGen.mutate(currentPatch,currentRole,constraints,mutations);

  PatchGenerator::describePatch(currentPatch.fm,patchDesc,sizeof(patchDesc));

  if (!lockSeed) {
    currentSeed++;
  }
}

void GenWorkspace::auditPatch() {
  if (!engine||!hasPatch) return;

  // load the generated patch as a temporary instrument for preview
  engine->loadTempIns(&currentPatch);
  // play note on the audit channel
  engine->noteOn(auditChannel,-1,auditNote);
}

void GenWorkspace::stopAudit() {
  if (!engine) return;
  engine->noteOff(auditChannel);
}

int GenWorkspace::commitPatch() {
  if (!engine||!hasPatch) return -1;

  // create a new instrument in the song
  DivInstrument* ins=new DivInstrument(currentPatch);
  int idx=engine->addInstrumentPtr(ins);
  return idx;
}

void GenWorkspace::populateParamsFromSong() {
  if (!engine||!engine->curSubSong) return;
  DivSubSong* sub=engine->curSubSong;

  // pull metric grid from song highlights
  if (sub->hilightA>0) patParams.rowsPerBeat=sub->hilightA;
  if (sub->hilightB>0) patParams.rowsPerBar=sub->hilightB;

  // apply style preset defaults (user GUI overrides take priority if set)
  const StylePreset& style=styleEngine.getActivePreset();
  patParams.grooveType=style.defaultGroove;
  patParams.phraseForm=style.defaultPhraseForm;
  patParams.chordToneEmphasis=style.chordToneEmphasis;

  // role-specific motif length from preset (0=auto)
  if (patParams.role>=0&&patParams.role<ROLE_MAX) {
    patParams.motifLengthHint=style.roleMotifLength[patParams.role];
  }
}

void GenWorkspace::generatePattern(int channel, int patIdx) {
  if (!engine) return;

  DivSubSong* sub=engine->curSubSong;
  if (!sub) return;

  // get or create the pattern
  DivPattern* pat=sub->pat[channel].getPattern(patIdx,true);
  if (!pat) return;

  patParams.channel=channel;
  patParams.patternLength=sub->patLen;

  // populate new fields from song metrics and active style
  populateParamsFromSong();

  if (!lockSeed) {
    patternGen.setSeed(currentSeed);
  }

  patternGen.generate(pat,patParams,styleEngine.getActivePreset());

  if (!lockSeed) {
    currentSeed++;
  }
}

void GenWorkspace::generateFill(int channel, int patIdx, int startRow, int endRow) {
  if (!engine) return;

  DivSubSong* sub=engine->curSubSong;
  if (!sub) return;

  DivPattern* pat=sub->pat[channel].getPattern(patIdx,true);
  if (!pat) return;

  patParams.channel=channel;

  // populate new fields from song metrics and active style
  populateParamsFromSong();

  if (!lockSeed) {
    patternGen.setSeed(currentSeed);
  }

  patternGen.generateFill(pat,patParams,styleEngine.getActivePreset(),startRow,endRow);

  if (!lockSeed) {
    currentSeed++;
  }
}

void GenWorkspace::randomizeSeed() {
  currentSeed=(uint32_t)time(NULL);
}
