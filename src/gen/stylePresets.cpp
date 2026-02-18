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

#include "stylePresets.h"

// helper: set operator constraints for a "carrier" operator (audible output)
static void setCarrierDefaults(OperatorConstraints& op) {
  op.tlMin=0; op.tlMax=20;    // carriers should be loud
  op.arMin=25; op.arMax=31;   // fast attack
  op.drMin=4; op.drMax=15;
  op.slMin=2; op.slMax=12;
  op.rrMin=3; op.rrMax=10;
  op.multMin=0; op.multMax=4;
  op.dtMin=0; op.dtMax=3;
  op.d2rMin=0; op.d2rMax=8;
  op.rsMin=0; op.rsMax=2;
}

// helper: set operator constraints for a "modulator" operator
static void setModulatorDefaults(OperatorConstraints& op) {
  op.tlMin=15; op.tlMax=90;   // modulators need headroom
  op.arMin=20; op.arMax=31;
  op.drMin=3; op.drMax=20;
  op.slMin=0; op.slMax=15;
  op.rrMin=1; op.rrMax=12;
  op.multMin=1; op.multMax=10;
  op.dtMin=0; op.dtMax=7;
  op.d2rMin=0; op.d2rMax=15;
  op.rsMin=0; op.rsMax=3;
}

// ========================================
// Thunder Force III/IV style
// Aggressive leads, driving bass, 150+ BPM, minor/harmonic minor
// ========================================
StylePreset genPresetThunderForce() {
  StylePreset p;
  p.name="Thunder Force";
  p.tempoMin=148;
  p.tempoMax=180;
  p.preferredScales={GEN_SCALE_MINOR, GEN_SCALE_HARMONIC_MINOR, GEN_SCALE_PHRYGIAN_DOMINANT};
  p.rhythmDensity=0.8f;
  p.syncopation=0.4f;
  p.chromaticism=0.35f;
  p.preferFastArpeggios=true;
  p.use16thSubdivisions=true;

  // pattern generation style
  p.defaultGroove=GEN_GROOVE_DRIVING;
  p.defaultPhraseForm=GEN_PHRASE_ABAB;
  p.chordToneEmphasis=0.6f;
  p.roleMotifLength[ROLE_LEAD]=16;
  p.roleMotifLength[ROLE_BASS]=16;

  // --- Lead ---
  {
    PatchRoleConstraints& r=p.roles[ROLE_LEAD];
    r.algorithms={0,1,2};
    r.feedbackMin=4; r.feedbackMax=7;
    // OP1: main modulator (high feedback, creates grit)
    r.ops[0].tlMin=30; r.ops[0].tlMax=60;
    r.ops[0].arMin=28; r.ops[0].arMax=31;
    r.ops[0].drMin=5; r.ops[0].drMax=12;
    r.ops[0].slMin=3; r.ops[0].slMax=10;
    r.ops[0].rrMin=3; r.ops[0].rrMax=8;
    r.ops[0].multMin=1; r.ops[0].multMax=3;
    r.ops[0].dtMin=3; r.ops[0].dtMax=6;
    // OP2: secondary modulator
    r.ops[1].tlMin=35; r.ops[1].tlMax=70;
    r.ops[1].arMin=25; r.ops[1].arMax=31;
    r.ops[1].drMin=5; r.ops[1].drMax=15;
    r.ops[1].slMin=2; r.ops[1].slMax=12;
    r.ops[1].rrMin=2; r.ops[1].rrMax=8;
    r.ops[1].multMin=2; r.ops[1].multMax=7;
    r.ops[1].dtMin=0; r.ops[1].dtMax=5;
    // OP3: modulator/carrier depending on algo
    setModulatorDefaults(r.ops[2]);
    r.ops[2].multMin=1; r.ops[2].multMax=4;
    // OP4: main carrier
    setCarrierDefaults(r.ops[3]);
  }

  // --- Bass ---
  {
    PatchRoleConstraints& r=p.roles[ROLE_BASS];
    r.algorithms={0,4};
    r.feedbackMin=3; r.feedbackMax=6;
    // low MUL ratios across the board
    for (int i=0; i<4; i++) {
      r.ops[i].multMin=0; r.ops[i].multMax=3;
    }
    r.ops[0].tlMin=25; r.ops[0].tlMax=55;
    r.ops[0].arMin=28; r.ops[0].arMax=31;
    r.ops[0].drMin=8; r.ops[0].drMax=18;
    r.ops[0].slMin=2; r.ops[0].slMax=8;
    r.ops[0].rrMin=5; r.ops[0].rrMax=10;
    // carrier: punchy
    setCarrierDefaults(r.ops[3]);
    r.ops[3].arMin=28; r.ops[3].arMax=31;
    r.ops[3].drMin=6; r.ops[3].drMax=14;
  }

  // --- Pad ---
  {
    PatchRoleConstraints& r=p.roles[ROLE_PAD];
    r.algorithms={2,4,5};
    r.feedbackMin=0; r.feedbackMax=3;
    for (int i=0; i<4; i++) {
      r.ops[i].arMin=8; r.ops[i].arMax=22;
      r.ops[i].drMin=2; r.ops[i].drMax=10;
      r.ops[i].slMin=5; r.ops[i].slMax=14;
      r.ops[i].rrMin=4; r.ops[i].rrMax=12;
      r.ops[i].multMin=0; r.ops[i].multMax=4;
    }
    setCarrierDefaults(r.ops[3]);
    r.ops[3].arMin=10; r.ops[3].arMax=20;
  }

  // --- Rhythm ---
  {
    PatchRoleConstraints& r=p.roles[ROLE_RHYTHM];
    r.algorithms={5,6,7};
    r.feedbackMin=2; r.feedbackMax=6;
    for (int i=0; i<4; i++) {
      r.ops[i].arMin=28; r.ops[i].arMax=31;
      r.ops[i].drMin=12; r.ops[i].drMax=25;
      r.ops[i].slMin=0; r.ops[i].slMax=5;
      r.ops[i].rrMin=8; r.ops[i].rrMax=15;
      r.ops[i].multMin=1; r.ops[i].multMax=14;
      r.ops[i].dtMin=0; r.ops[i].dtMax=7;
    }
  }

  // --- SFX ---
  {
    PatchRoleConstraints& r=p.roles[ROLE_SFX];
    r.algorithms={0,1,2,3,4,5,6,7};
    r.feedbackMin=3; r.feedbackMax=7;
    for (int i=0; i<4; i++) {
      r.ops[i].tlMin=0; r.ops[i].tlMax=127;
      r.ops[i].arMin=20; r.ops[i].arMax=31;
      r.ops[i].drMin=5; r.ops[i].drMax=31;
      r.ops[i].multMin=0; r.ops[i].multMax=15;
      r.ops[i].dtMin=0; r.ops[i].dtMax=7;
    }
  }

  // --- Slap Bass ---
  {
    PatchRoleConstraints& r=p.roles[ROLE_SLAP_BASS];
    r.algorithms={0,4};
    r.feedbackMin=4; r.feedbackMax=7;
    // OP1: high-mul modulator with fast decay = the "pop"
    r.ops[0].tlMin=20; r.ops[0].tlMax=50;
    r.ops[0].arMin=30; r.ops[0].arMax=31;
    r.ops[0].drMin=15; r.ops[0].drMax=25;
    r.ops[0].slMin=0; r.ops[0].slMax=3;
    r.ops[0].rrMin=8; r.ops[0].rrMax=15;
    r.ops[0].multMin=4; r.ops[0].multMax=8;
    // OP2: supporting mod
    setModulatorDefaults(r.ops[1]);
    r.ops[1].multMin=1; r.ops[1].multMax=3;
    // OP3
    setModulatorDefaults(r.ops[2]);
    r.ops[2].multMin=0; r.ops[2].multMax=2;
    // OP4: carrier, punchy
    setCarrierDefaults(r.ops[3]);
    r.ops[3].arMin=30; r.ops[3].arMax=31;
    r.ops[3].drMin=10; r.ops[3].drMax=18;
    r.ops[3].multMin=0; r.ops[3].multMax=2;
  }

  // --- Distorted Guitar ---
  {
    PatchRoleConstraints& r=p.roles[ROLE_DIST_GUITAR];
    r.algorithms={0,1};
    r.feedbackMin=5; r.feedbackMax=7;
    r.ops[0].tlMin=25; r.ops[0].tlMax=50;
    r.ops[0].arMin=28; r.ops[0].arMax=31;
    r.ops[0].drMin=6; r.ops[0].drMax=12;
    r.ops[0].slMin=4; r.ops[0].slMax=10;
    r.ops[0].rrMin=3; r.ops[0].rrMax=8;
    r.ops[0].multMin=1; r.ops[0].multMax=2;
    r.ops[0].dtMin=3; r.ops[0].dtMax=6;
    r.ops[1].tlMin=30; r.ops[1].tlMax=65;
    r.ops[1].arMin=26; r.ops[1].arMax=31;
    r.ops[1].drMin=5; r.ops[1].drMax=14;
    r.ops[1].multMin=1; r.ops[1].multMax=5;
    setModulatorDefaults(r.ops[2]);
    setCarrierDefaults(r.ops[3]);
  }

  return p;
}

// ========================================
// Streets of Rage style
// Funky bass, groovier rhythms, soul/jazz scales, moderate tempo
// ========================================
StylePreset genPresetStreetsOfRage() {
  StylePreset p;
  p.name="Streets of Rage";
  p.tempoMin=100;
  p.tempoMax=130;
  p.preferredScales={GEN_SCALE_DORIAN, GEN_SCALE_MINOR, GEN_SCALE_BLUES, GEN_SCALE_PENTATONIC_MINOR};
  p.rhythmDensity=0.6f;
  p.syncopation=0.65f;
  p.chromaticism=0.2f;
  p.preferFastArpeggios=false;
  p.use16thSubdivisions=true;

  // pattern generation style
  p.defaultGroove=GEN_GROOVE_FUNK;
  p.defaultPhraseForm=GEN_PHRASE_AABA;
  p.chordToneEmphasis=0.75f;
  p.roleMotifLength[ROLE_BASS]=16;
  p.roleMotifLength[ROLE_SLAP_BASS]=16;

  // Lead: smoother, jazzy FM
  {
    PatchRoleConstraints& r=p.roles[ROLE_LEAD];
    r.algorithms={2,4,5};
    r.feedbackMin=2; r.feedbackMax=5;
    setModulatorDefaults(r.ops[0]);
    r.ops[0].multMin=1; r.ops[0].multMax=4;
    setModulatorDefaults(r.ops[1]);
    setModulatorDefaults(r.ops[2]);
    setCarrierDefaults(r.ops[3]);
    r.ops[3].arMin=20; r.ops[3].arMax=28;
  }

  // Bass: funky, round
  {
    PatchRoleConstraints& r=p.roles[ROLE_BASS];
    r.algorithms={0,4};
    r.feedbackMin=2; r.feedbackMax=5;
    for (int i=0; i<4; i++) {
      r.ops[i].multMin=0; r.ops[i].multMax=3;
    }
    setModulatorDefaults(r.ops[0]);
    setCarrierDefaults(r.ops[3]);
    r.ops[3].arMin=26; r.ops[3].arMax=31;
    r.ops[3].drMin=8; r.ops[3].drMax=16;
  }

  // Pad
  {
    PatchRoleConstraints& r=p.roles[ROLE_PAD];
    r.algorithms={4,5,7};
    r.feedbackMin=0; r.feedbackMax=2;
    for (int i=0; i<4; i++) {
      r.ops[i].arMin=10; r.ops[i].arMax=20;
      r.ops[i].slMin=6; r.ops[i].slMax=14;
    }
    setCarrierDefaults(r.ops[3]);
    r.ops[3].arMin=10; r.ops[3].arMax=18;
  }

  // Rhythm
  {
    PatchRoleConstraints& r=p.roles[ROLE_RHYTHM];
    r.algorithms={5,6,7};
    r.feedbackMin=1; r.feedbackMax=4;
    for (int i=0; i<4; i++) {
      r.ops[i].arMin=28; r.ops[i].arMax=31;
      r.ops[i].drMin=10; r.ops[i].drMax=22;
      r.ops[i].slMin=0; r.ops[i].slMax=4;
      r.ops[i].rrMin=7; r.ops[i].rrMax=14;
    }
  }

  // SFX, Slap Bass, Dist Guitar: use defaults
  p.roles[ROLE_SFX]=PatchRoleConstraints();
  p.roles[ROLE_SLAP_BASS]=PatchRoleConstraints();
  p.roles[ROLE_DIST_GUITAR]=PatchRoleConstraints();

  return p;
}

// ========================================
// Sonic style
// Bright FM, major/mixolydian, bouncy rhythms
// ========================================
StylePreset genPresetSonic() {
  StylePreset p;
  p.name="Sonic";
  p.tempoMin=120;
  p.tempoMax=160;
  p.preferredScales={GEN_SCALE_MAJOR, GEN_SCALE_MIXOLYDIAN, GEN_SCALE_PENTATONIC_MAJOR};
  p.rhythmDensity=0.65f;
  p.syncopation=0.5f;
  p.chromaticism=0.15f;
  p.preferFastArpeggios=true;
  p.use16thSubdivisions=true;

  // pattern generation style
  p.defaultGroove=GEN_GROOVE_STRAIGHT;
  p.defaultPhraseForm=GEN_PHRASE_ABAB;
  p.chordToneEmphasis=0.8f;
  p.roleMotifLength[ROLE_LEAD]=16;

  // Lead: bright, bouncy
  {
    PatchRoleConstraints& r=p.roles[ROLE_LEAD];
    r.algorithms={2,3,4};
    r.feedbackMin=2; r.feedbackMax=5;
    setModulatorDefaults(r.ops[0]);
    r.ops[0].multMin=1; r.ops[0].multMax=5;
    setModulatorDefaults(r.ops[1]);
    r.ops[1].multMin=1; r.ops[1].multMax=4;
    setModulatorDefaults(r.ops[2]);
    setCarrierDefaults(r.ops[3]);
    r.ops[3].arMin=26; r.ops[3].arMax=31;
  }

  // Bass: clean, round
  {
    PatchRoleConstraints& r=p.roles[ROLE_BASS];
    r.algorithms={0,4};
    r.feedbackMin=1; r.feedbackMax=4;
    for (int i=0; i<4; i++) {
      r.ops[i].multMin=0; r.ops[i].multMax=3;
    }
    setCarrierDefaults(r.ops[3]);
  }

  // Others: use defaults
  p.roles[ROLE_PAD]=PatchRoleConstraints();
  p.roles[ROLE_RHYTHM]=PatchRoleConstraints();
  p.roles[ROLE_SFX]=PatchRoleConstraints();
  p.roles[ROLE_SLAP_BASS]=PatchRoleConstraints();
  p.roles[ROLE_DIST_GUITAR]=PatchRoleConstraints();

  return p;
}

// ========================================
// M.U.S.H.A. style
// Dark, atmospheric, phrygian/locrian, complex operator routing
// ========================================
StylePreset genPresetMUSHA() {
  StylePreset p;
  p.name="M.U.S.H.A.";
  p.tempoMin=130;
  p.tempoMax=165;
  p.preferredScales={GEN_SCALE_PHRYGIAN, GEN_SCALE_LOCRIAN, GEN_SCALE_HARMONIC_MINOR, GEN_SCALE_PHRYGIAN_DOMINANT};
  p.rhythmDensity=0.7f;
  p.syncopation=0.35f;
  p.chromaticism=0.45f;
  p.preferFastArpeggios=true;
  p.use16thSubdivisions=true;

  // pattern generation style
  p.defaultGroove=GEN_GROOVE_DRIVING;
  p.defaultPhraseForm=GEN_PHRASE_AAAB;
  p.chordToneEmphasis=0.5f;
  p.roleMotifLength[ROLE_LEAD]=16;

  // Lead: dark, complex
  {
    PatchRoleConstraints& r=p.roles[ROLE_LEAD];
    r.algorithms={0,1,3};
    r.feedbackMin=3; r.feedbackMax=7;
    setModulatorDefaults(r.ops[0]);
    r.ops[0].dtMin=3; r.ops[0].dtMax=7;
    r.ops[0].multMin=1; r.ops[0].multMax=6;
    setModulatorDefaults(r.ops[1]);
    r.ops[1].multMin=2; r.ops[1].multMax=8;
    setModulatorDefaults(r.ops[2]);
    setCarrierDefaults(r.ops[3]);
  }

  // Bass: heavy
  {
    PatchRoleConstraints& r=p.roles[ROLE_BASS];
    r.algorithms={0,1};
    r.feedbackMin=4; r.feedbackMax=7;
    setModulatorDefaults(r.ops[0]);
    r.ops[0].multMin=1; r.ops[0].multMax=3;
    setModulatorDefaults(r.ops[1]);
    setModulatorDefaults(r.ops[2]);
    setCarrierDefaults(r.ops[3]);
    r.ops[3].multMin=0; r.ops[3].multMax=2;
  }

  // Others: defaults
  p.roles[ROLE_PAD]=PatchRoleConstraints();
  p.roles[ROLE_RHYTHM]=PatchRoleConstraints();
  p.roles[ROLE_SFX]=PatchRoleConstraints();
  p.roles[ROLE_SLAP_BASS]=PatchRoleConstraints();
  p.roles[ROLE_DIST_GUITAR]=PatchRoleConstraints();

  return p;
}

// ========================================
// Custom preset — wide-open defaults
// ========================================
StylePreset genPresetCustom() {
  StylePreset p;
  p.name="Custom";
  p.tempoMin=80;
  p.tempoMax=220;
  p.preferredScales={GEN_SCALE_MINOR, GEN_SCALE_MAJOR, GEN_SCALE_CHROMATIC};
  p.rhythmDensity=0.5f;
  p.syncopation=0.3f;
  p.chromaticism=0.2f;
  p.preferFastArpeggios=false;
  p.use16thSubdivisions=false;

  for (int role=0; role<ROLE_MAX; role++) {
    PatchRoleConstraints& r=p.roles[role];
    r.algorithms={0,1,2,3,4,5,6,7};
    r.feedbackMin=0;
    r.feedbackMax=7;
    // operators use default OperatorConstraints (full range)
  }

  return p;
}
