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

#include "patternGen.h"
#include "../engine/defines.h"
#include "../engine/pattern.h"
#include <cstring>
#include <cmath>
#include <cstdlib>

// ========================================
// BarChord
// ========================================

bool BarChord::isChordTone(int scaleDegree, int scaleLen) const {
  if (scaleLen<=0) scaleLen=7;
  int sd=((scaleDegree%scaleLen)+scaleLen)%scaleLen;
  for (int i=0; i<chordToneCount; i++) {
    int ct=((rootDegree+chordTones[i])%scaleLen+scaleLen)%scaleLen;
    if (sd==ct) return true;
  }
  return false;
}

// ========================================
// Pipeline infrastructure
// ========================================

int PatternGenerator::computeBarCount(int patternLength, int rowsPerBar) {
  if (rowsPerBar<=0) rowsPerBar=16;
  int bars=patternLength/rowsPerBar;
  return genClamp(bars,1,16);
}

void PatternGenerator::generateChordProgression(BarChord* chords, int barCount,
                                                 GenScaleType scaleType, int complexity) {
  static const int minorProgs[][4]={
    {0,3,4,0},   // i-iv-v-i
    {0,5,2,6},   // i-VI-III-VII
    {0,3,6,2},   // i-iv-VII-III
    {0,4,5,4},   // i-v-VI-v
    {0,3,4,3},   // i-iv-v-iv
  };
  static const int majorProgs[][4]={
    {0,3,4,0},   // I-IV-V-I
    {0,5,3,4},   // I-vi-IV-V
    {0,2,5,1},   // I-iii-vi-ii
    {0,3,1,4},   // I-IV-ii-V
    {0,4,3,4},   // I-V-IV-V
  };

  bool isMinor=genIsMinorFamily(scaleType);
  const int (*progs)[4]=isMinor?minorProgs:majorProgs;
  int progCount=5;

  float cf=(float)complexity/100.0f;
  float weights[5];
  for (int i=0; i<progCount; i++) {
    weights[i]=1.0f-(float)i*(1.0f-cf)*0.25f;
    if (weights[i]<0.1f) weights[i]=0.1f;
  }
  int progIdx=rng.weightedPick(weights,progCount);

  for (int b=0; b<barCount; b++) {
    int chordRoot=progs[progIdx][b%4];
    chords[b].rootDegree=chordRoot;
    chords[b].chordToneCount=3;
    chords[b].chordTones[0]=0;
    chords[b].chordTones[1]=2;
    chords[b].chordTones[2]=4;
    chords[b].chordTones[3]=6;
    if (cf>0.6f&&rng.randFloat()<cf*0.4f) {
      chords[b].chordToneCount=4;
    }
  }
}

GrooveTemplate PatternGenerator::buildGrooveTemplate(GenGrooveType type) {
  GrooveTemplate g;
  switch (type) {
    case GEN_GROOVE_STRAIGHT:
      g.velocity[ 0]=0x7F; g.velocity[ 1]=0x50; g.velocity[ 2]=0x58; g.velocity[ 3]=0x48;
      g.velocity[ 4]=0x6C; g.velocity[ 5]=0x50; g.velocity[ 6]=0x58; g.velocity[ 7]=0x48;
      g.velocity[ 8]=0x74; g.velocity[ 9]=0x50; g.velocity[10]=0x58; g.velocity[11]=0x48;
      g.velocity[12]=0x6C; g.velocity[13]=0x50; g.velocity[14]=0x58; g.velocity[15]=0x48;
      break;
    case GEN_GROOVE_SHUFFLE:
      g.velocity[ 0]=0x7F; g.velocity[ 1]=0x40; g.velocity[ 2]=0x68; g.velocity[ 3]=0x38;
      g.velocity[ 4]=0x6C; g.velocity[ 5]=0x40; g.velocity[ 6]=0x68; g.velocity[ 7]=0x38;
      g.velocity[ 8]=0x74; g.velocity[ 9]=0x40; g.velocity[10]=0x68; g.velocity[11]=0x38;
      g.velocity[12]=0x6C; g.velocity[13]=0x40; g.velocity[14]=0x68; g.velocity[15]=0x38;
      break;
    case GEN_GROOVE_FUNK:
      g.velocity[ 0]=0x7F; g.velocity[ 1]=0x30; g.velocity[ 2]=0x58; g.velocity[ 3]=0x30;
      g.velocity[ 4]=0x60; g.velocity[ 5]=0x30; g.velocity[ 6]=0x6A; g.velocity[ 7]=0x30;
      g.velocity[ 8]=0x70; g.velocity[ 9]=0x30; g.velocity[10]=0x58; g.velocity[11]=0x30;
      g.velocity[12]=0x60; g.velocity[13]=0x30; g.velocity[14]=0x6A; g.velocity[15]=0x30;
      break;
    case GEN_GROOVE_DRIVING:
      g.velocity[ 0]=0x7F; g.velocity[ 1]=0x55; g.velocity[ 2]=0x60; g.velocity[ 3]=0x55;
      g.velocity[ 4]=0x70; g.velocity[ 5]=0x55; g.velocity[ 6]=0x60; g.velocity[ 7]=0x55;
      g.velocity[ 8]=0x7A; g.velocity[ 9]=0x55; g.velocity[10]=0x60; g.velocity[11]=0x55;
      g.velocity[12]=0x70; g.velocity[13]=0x55; g.velocity[14]=0x60; g.velocity[15]=0x55;
      break;
    case GEN_GROOVE_HALFTIME:
      g.velocity[ 0]=0x7F; g.velocity[ 1]=0x48; g.velocity[ 2]=0x50; g.velocity[ 3]=0x48;
      g.velocity[ 4]=0x58; g.velocity[ 5]=0x48; g.velocity[ 6]=0x50; g.velocity[ 7]=0x48;
      g.velocity[ 8]=0x78; g.velocity[ 9]=0x48; g.velocity[10]=0x50; g.velocity[11]=0x48;
      g.velocity[12]=0x58; g.velocity[13]=0x48; g.velocity[14]=0x50; g.velocity[15]=0x48;
      break;
    default:
      break;
  }
  return g;
}

Phrase PatternGenerator::buildPhrase(GenPhraseForm form, int barCount, int motifCount) {
  Phrase ph;
  ph.totalBars=barCount;

  if (form==GEN_PHRASE_RANDOM) {
    form=(GenPhraseForm)rng.randInt(0,(int)GEN_PHRASE_ABAC);
  }

  static const int formMap[4][4]={
    {0,0,1,0},   // AABA
    {0,1,0,1},   // ABAB
    {0,0,0,1},   // AAAB
    {0,1,0,-1},  // ABAC (-1 = variation of A)
  };

  int formIdx=(int)form;
  if (formIdx<0||formIdx>3) formIdx=0;

  ph.placementCount=0;
  for (int b=0; b<barCount&&ph.placementCount<16; b++) {
    MotifPlacement mp;
    mp.barIndex=b;

    int mapVal=formMap[formIdx][b%4];
    if (mapVal==-1) {
      mp.motifIndex=0;
      mp.transposeDegrees=2;
    } else {
      mp.motifIndex=mapVal%motifCount;
      mp.transposeDegrees=0;
    }

    if (b>=4) {
      int cycleNum=b/4;
      mp.transposeDegrees+=cycleNum*rng.randInt(-1,2);
    }

    mp.invertContour=false;
    ph.placements[ph.placementCount++]=mp;
  }

  return ph;
}

void PatternGenerator::applyMelodicContour(Motif& motif, GenContour contour,
                                            int complexity, int scaleLen) {
  if (motif.noteCount<2) return;
  float cf=(float)complexity/100.0f;
  int amplitude=2+(int)(cf*5.0f);

  for (int i=0; i<motif.noteCount; i++) {
    if (motif.notes[i].isRest) continue;

    float position=(float)i/(float)(motif.noteCount-1);
    int contourOffset=0;

    switch (contour) {
      case GEN_CONTOUR_ARCH:
        contourOffset=(int)(sinf(3.14159f*position)*(float)amplitude);
        break;
      case GEN_CONTOUR_INV_ARCH:
        contourOffset=-(int)(sinf(3.14159f*position)*(float)amplitude);
        break;
      case GEN_CONTOUR_ASCENDING:
        contourOffset=(int)(position*(float)amplitude);
        break;
      case GEN_CONTOUR_DESCENDING:
        contourOffset=(int)((1.0f-position)*(float)amplitude);
        break;
      case GEN_CONTOUR_FLAT:
        contourOffset=rng.randInt(-1,1);
        break;
      default:
        break;
    }

    motif.notes[i].relativeDegree+=contourOffset;
  }
}

void PatternGenerator::applyChordToneGravity(Motif& motif, const BarChord& chord,
                                              int scaleLen, int rowsPerBeat, float emphasis) {
  if (scaleLen<=0) scaleLen=7;
  for (int i=0; i<motif.noteCount; i++) {
    if (motif.notes[i].isRest) continue;

    bool isStrongBeat=(motif.notes[i].rowOffset%rowsPerBeat==0);
    if (isStrongBeat&&rng.randFloat()<emphasis) {
      int degree=motif.notes[i].relativeDegree;
      if (chord.isChordTone(degree,scaleLen)) continue;

      int bestDist=999;
      int bestDegree=degree;
      for (int ct=0; ct<chord.chordToneCount; ct++) {
        int chordDegree=chord.rootDegree+chord.chordTones[ct];
        for (int oct=-1; oct<=1; oct++) {
          int candidate=chordDegree+oct*scaleLen;
          int dist=abs(candidate-degree);
          if (dist<bestDist) {
            bestDist=dist;
            bestDegree=candidate;
          }
        }
      }
      motif.notes[i].relativeDegree=bestDegree;
    }
  }
}

// ========================================
// Role-specific motif generators
// ========================================

Motif PatternGenerator::generateRoleMotif(PatchRole role, int density, int complexity,
                                           float syncopation, int rowsPerBar, int motifLengthHint,
                                           int scaleLen) {
  switch (role) {
    case ROLE_BASS:        return generateBassMotif(density,complexity,syncopation,rowsPerBar,scaleLen);
    case ROLE_LEAD:        return generateLeadMotif(density,complexity,syncopation,rowsPerBar,scaleLen);
    case ROLE_PAD:         return generatePadMotif(density,complexity,syncopation,rowsPerBar,scaleLen);
    case ROLE_RHYTHM:      return generateRhythmMotif(density,complexity,syncopation,rowsPerBar,scaleLen);
    case ROLE_SFX:         return generateSfxMotif(density,complexity,syncopation,rowsPerBar,scaleLen);
    case ROLE_SLAP_BASS:   return generateSlapBassMotif(density,complexity,syncopation,rowsPerBar,scaleLen);
    case ROLE_DIST_GUITAR: return generateDistGuitarMotif(density,complexity,syncopation,rowsPerBar,scaleLen);
    default:               return generateLeadMotif(density,complexity,syncopation,rowsPerBar,scaleLen);
  }
}

Motif PatternGenerator::generateBassMotif(int density, int complexity, float syncopation,
                                           int rowsPerBar, int scaleLen) {
  Motif m;
  m.lengthInRows=rowsPerBar;
  float cf=(float)complexity/100.0f;

  if (cf<0.34f) {
    // ROOT-FIFTH OSTINATO
    m.noteCount=2;
    m.notes[0].rowOffset=0;
    m.notes[0].relativeDegree=0;
    m.notes[0].velOffset=10;

    int fifthPos=rowsPerBar/2;
    if (syncopation>0.3f&&rng.randFloat()<syncopation) {
      fifthPos+=(rng.randFloat()<0.5f)?-1:1;
    }
    m.notes[1].rowOffset=fifthPos;
    m.notes[1].relativeDegree=4;
    m.notes[1].velOffset=0;

    if (syncopation>0.5f&&rng.randFloat()<0.5f) {
      m.notes[2].rowOffset=rowsPerBar*3/4;
      m.notes[2].relativeDegree=scaleLen;
      m.notes[2].velOffset=-5;
      m.noteCount=3;
    }
  } else if (cf<0.67f) {
    // WALKING BASS
    m.noteCount=4;
    int beatStep=rowsPerBar/4;
    m.notes[0].rowOffset=0;          m.notes[0].relativeDegree=0;  m.notes[0].velOffset=8;
    m.notes[1].rowOffset=beatStep;   m.notes[1].relativeDegree=2;  m.notes[1].velOffset=0;
    m.notes[2].rowOffset=beatStep*2; m.notes[2].relativeDegree=4;  m.notes[2].velOffset=0;
    int approach=(rng.randFloat()<0.5f)?-1:(scaleLen-1);
    m.notes[3].rowOffset=beatStep*3; m.notes[3].relativeDegree=approach; m.notes[3].velOffset=-3;
  } else {
    // SYNCOPATED FUNK
    int candidates[]={0,3,6,8,11,14};
    int candCount=6;
    for (int c=0; c<candCount; c++) {
      candidates[c]=candidates[c]*rowsPerBar/16;
    }
    m.noteCount=0;
    float df=(float)density/100.0f;
    for (int c=0; c<candCount&&m.noteCount<8; c++) {
      if (rng.randFloat()<df) {
        MotifNote& mn=m.notes[m.noteCount];
        mn.rowOffset=candidates[c];
        mn.relativeDegree=(c%2==0)?0:4;
        if (rng.randFloat()<0.2f) mn.relativeDegree-=scaleLen;
        mn.velOffset=(candidates[c]%(rowsPerBar/4)!=0)?-15:5;
        m.noteCount++;
      }
    }
    if (m.noteCount<2) {
      m.notes[0].rowOffset=0;              m.notes[0].relativeDegree=0; m.notes[0].velOffset=8;
      m.notes[1].rowOffset=rowsPerBar/2;   m.notes[1].relativeDegree=4; m.notes[1].velOffset=0;
      m.noteCount=2;
    }
  }
  return m;
}

Motif PatternGenerator::generateLeadMotif(int density, int complexity, float syncopation,
                                           int rowsPerBar, int scaleLen) {
  Motif m;
  m.lengthInRows=rowsPerBar;
  float cf=(float)complexity/100.0f;

  if (cf<0.34f) {
    // STEPWISE MELODY
    int beatStep=rowsPerBar/4;
    if (beatStep<1) beatStep=1;
    m.noteCount=3+rng.randInt(0,2);
    int beatPositions[]={0,beatStep,beatStep*2,beatStep*3,beatStep*3+beatStep/2};
    int deg=0;
    for (int i=0; i<m.noteCount; i++) {
      m.notes[i].rowOffset=beatPositions[i%5];
      if (syncopation>0.2f&&rng.randFloat()<syncopation&&i>0) {
        m.notes[i].rowOffset+=(rng.randFloat()<0.5f)?-1:1;
        m.notes[i].rowOffset=genClamp(m.notes[i].rowOffset,0,rowsPerBar-1);
      }
      if (i==m.noteCount/2) {
        deg+=rng.randInt(1,2)*(rng.randFloat()<0.5f?1:-1);
      } else {
        deg+=(rng.randFloat()<0.5f)?1:-1;
      }
      m.notes[i].relativeDegree=deg;
      m.notes[i].velOffset=0;
    }
  } else if (cf<0.67f) {
    // SEQUENTIAL MOTIF
    int kernel[3]={0,rng.randInt(1,2),rng.randInt(-1,1)};
    int step=rowsPerBar/8;
    if (step<1) step=1;
    m.noteCount=6;
    for (int i=0; i<3; i++) {
      m.notes[i].rowOffset=i*step;
      m.notes[i].relativeDegree=kernel[i];
      m.notes[i].velOffset=(i==0)?5:0;
    }
    int halfBar=rowsPerBar/2;
    for (int i=0; i<3; i++) {
      m.notes[3+i].rowOffset=halfBar+i*step;
      m.notes[3+i].relativeDegree=kernel[i]+2;
      m.notes[3+i].velOffset=(i==0)?5:0;
    }
  } else {
    // ARPEGGIATED RUN
    m.noteCount=5+rng.randInt(0,3);
    if (m.noteCount>8) m.noteCount=8;
    int startPos=(rng.randFloat()<0.5f)?0:rowsPerBar/2;
    for (int i=0; i<m.noteCount; i++) {
      m.notes[i].rowOffset=startPos+i;
      if (m.notes[i].rowOffset>=rowsPerBar) m.notes[i].rowOffset=rowsPerBar-1;
      m.notes[i].relativeDegree=i;
      m.notes[i].velOffset=-5+(i==0?10:0);
    }
  }
  return m;
}

Motif PatternGenerator::generatePadMotif(int density, int complexity, float syncopation,
                                          int rowsPerBar, int scaleLen) {
  Motif m;
  m.lengthInRows=rowsPerBar;
  float cf=(float)complexity/100.0f;

  m.noteCount=1;
  m.notes[0].rowOffset=0;
  m.notes[0].relativeDegree=0;
  m.notes[0].duration=0;
  m.notes[0].velOffset=5;

  if (cf>0.3f&&rng.randFloat()<cf) {
    m.notes[1].rowOffset=rowsPerBar/2;
    m.notes[1].relativeDegree=2;
    m.notes[1].duration=0;
    m.notes[1].velOffset=0;
    m.noteCount=2;
  }

  if (cf>0.6f&&rng.randFloat()<cf*0.5f&&m.noteCount<8) {
    m.notes[m.noteCount].rowOffset=rowsPerBar/4;
    m.notes[m.noteCount].relativeDegree=4;
    m.notes[m.noteCount].duration=0;
    m.notes[m.noteCount].velOffset=-5;
    m.noteCount++;
  }
  return m;
}

Motif PatternGenerator::generateRhythmMotif(int density, int complexity, float syncopation,
                                             int rowsPerBar, int scaleLen) {
  Motif m;
  m.lengthInRows=rowsPerBar;
  float df=(float)density/100.0f;

  int subdivision;
  if (df<0.33f) subdivision=4;
  else if (df<0.66f) subdivision=8;
  else subdivision=16;

  int step=rowsPerBar/subdivision;
  if (step<1) step=1;

  m.noteCount=0;
  for (int s=0; s<subdivision&&m.noteCount<8; s++) {
    int pos=s*step;
    if (syncopation>0.3f&&rng.randFloat()<syncopation*0.3f) {
      if (pos%(rowsPerBar/4)==0&&pos!=0) continue;
    }
    m.notes[m.noteCount].rowOffset=pos;
    m.notes[m.noteCount].relativeDegree=0;
    m.notes[m.noteCount].duration=(step>1)?step-1:1;

    if (pos==0) m.notes[m.noteCount].velOffset=15;
    else if (pos==rowsPerBar/2) m.notes[m.noteCount].velOffset=8;
    else if (pos%(rowsPerBar/4)==0) m.notes[m.noteCount].velOffset=3;
    else m.notes[m.noteCount].velOffset=-10;

    m.noteCount++;
  }
  return m;
}

Motif PatternGenerator::generateSfxMotif(int density, int complexity, float syncopation,
                                          int rowsPerBar, int scaleLen) {
  Motif m;
  m.lengthInRows=rowsPerBar;

  int burstStart=(rng.randFloat()<0.5f)?0:rowsPerBar-4;
  if (burstStart<0) burstStart=0;
  m.noteCount=rng.randInt(2,4);
  if (m.noteCount>8) m.noteCount=8;
  for (int i=0; i<m.noteCount; i++) {
    m.notes[i].rowOffset=burstStart+i;
    if (m.notes[i].rowOffset>=rowsPerBar) m.notes[i].rowOffset=rowsPerBar-1;
    m.notes[i].relativeDegree=rng.randInt(-3,3);
    m.notes[i].duration=1;
    m.notes[i].velOffset=10-i*5;
  }
  return m;
}

Motif PatternGenerator::generateSlapBassMotif(int density, int complexity, float syncopation,
                                               int rowsPerBar, int scaleLen) {
  Motif m;
  m.lengthInRows=rowsPerBar;
  float df=(float)density/100.0f;

  m.noteCount=0;
  m.notes[m.noteCount].rowOffset=0;
  m.notes[m.noteCount].relativeDegree=0;
  m.notes[m.noteCount].velOffset=15;
  m.notes[m.noteCount].duration=2;
  m.noteCount++;

  int ghostPositions[]={3,5,7,11,13,15};
  for (int g=0; g<6&&m.noteCount<7; g++) {
    if (rng.randFloat()<df*0.7f) {
      int pos=ghostPositions[g]*rowsPerBar/16;
      if (syncopation>0.3f) pos+=(rng.randFloat()<0.5f?-1:0);
      pos=genClamp(pos,1,rowsPerBar-1);
      m.notes[m.noteCount].rowOffset=pos;
      m.notes[m.noteCount].relativeDegree=0;
      m.notes[m.noteCount].velOffset=-20;
      m.notes[m.noteCount].duration=1;
      m.noteCount++;
    }
  }

  if (rng.randFloat()<0.7f&&m.noteCount<8) {
    int popPos=(rng.randFloat()<0.5f)?rowsPerBar*3/8:rowsPerBar*5/8;
    m.notes[m.noteCount].rowOffset=popPos;
    m.notes[m.noteCount].relativeDegree=scaleLen;
    m.notes[m.noteCount].velOffset=5;
    m.notes[m.noteCount].duration=2;
    m.noteCount++;
  }
  return m;
}

Motif PatternGenerator::generateDistGuitarMotif(int density, int complexity, float syncopation,
                                                 int rowsPerBar, int scaleLen) {
  Motif m;
  m.lengthInRows=rowsPerBar;
  float cf=(float)complexity/100.0f;

  if (cf<0.5f) {
    int step=rowsPerBar/8;
    if (step<1) step=1;
    m.noteCount=0;
    for (int r=0; r<rowsPerBar&&m.noteCount<8; r+=step) {
      m.notes[m.noteCount].rowOffset=r;
      m.notes[m.noteCount].relativeDegree=0;
      m.notes[m.noteCount].duration=1;
      m.notes[m.noteCount].velOffset=(r%(rowsPerBar/2)==0)?10:-5;
      m.noteCount++;
    }
  } else {
    int pattern[]={0,3,6,8,11};
    int degrees[]={0,4,0,4,0};
    m.noteCount=0;
    for (int i=0; i<5&&m.noteCount<8; i++) {
      if (rng.randFloat()<0.7f) {
        m.notes[m.noteCount].rowOffset=pattern[i]*rowsPerBar/16;
        m.notes[m.noteCount].relativeDegree=degrees[i];
        m.notes[m.noteCount].duration=1;
        m.notes[m.noteCount].velOffset=(pattern[i]%4!=0)?5:-3;
        m.noteCount++;
      }
    }
    if (m.noteCount<2) {
      m.notes[0].rowOffset=0;              m.notes[0].relativeDegree=0; m.notes[0].velOffset=10;
      m.notes[1].rowOffset=rowsPerBar/2;   m.notes[1].relativeDegree=0; m.notes[1].velOffset=5;
      m.noteCount=2;
    }
  }
  return m;
}

// ========================================
// Pattern writing
// ========================================

void PatternGenerator::writeMotifToPattern(DivPattern* pat, const Motif& motif,
                                            const MotifPlacement& placement,
                                            const BarChord& chord, const GrooveTemplate& groove,
                                            const GenPatternParams& params,
                                            int scaleLen, const int* intervals,
                                            int barStartRow, int degreeRangeBase) {
  int octMin=genClamp(params.octaveMin,0,9);
  int octMax=genClamp(params.octaveMax,0,9);
  int degreeRange=(octMax-octMin+1)*scaleLen;
  if (degreeRange<=0) degreeRange=scaleLen;

  for (int i=0; i<motif.noteCount; i++) {
    const MotifNote& mn=motif.notes[i];
    if (mn.isRest) continue;

    int noteRow=barStartRow+mn.rowOffset;
    if (noteRow<0||noteRow>=DIV_MAX_ROWS) continue;

    int degree=degreeRangeBase+mn.relativeDegree+placement.transposeDegrees;

    if (placement.invertContour) {
      degree=degreeRangeBase-(mn.relativeDegree+placement.transposeDegrees)+degreeRangeBase;
    }

    degree+=chord.rootDegree;

    while (degree<0) degree+=scaleLen;
    while (degree>=degreeRange) degree-=scaleLen;

    int octave=octMin+(degree/scaleLen);
    int degInScale=((degree%scaleLen)+scaleLen)%scaleLen;
    int semitone=params.scaleRoot+intervals[degInScale];

    while (semitone>=12) { semitone-=12; octave++; }
    while (semitone<0) { semitone+=12; octave--; }
    octave=genClamp(octave,0,9);

    int furnaceNote=(octave+5)*12+semitone;
    furnaceNote=genClamp(furnaceNote,0,179);

    pat->newData[noteRow][DIV_PAT_NOTE]=(short)furnaceNote;
    pat->newData[noteRow][DIV_PAT_INS]=(short)params.insIndex;

    int grooveRow=mn.rowOffset%16;
    int vel=groove.velocity[grooveRow]+mn.velOffset;
    vel=genClamp(vel,0x10,0x7F);
    pat->newData[noteRow][DIV_PAT_VOL]=(short)vel;
  }
}

// ========================================
// Post-processing passes
// ========================================

void PatternGenerator::applyEffects(DivPattern* pat, const GenPatternParams& params,
                                     const StylePreset& style, int startRow, int endRow) {
  float cf=(float)params.complexity/100.0f;

  for (int row=startRow; row<endRow; row++) {
    if (row<0||row>=DIV_MAX_ROWS) continue;
    if (pat->newData[row][DIV_PAT_NOTE]<0||pat->newData[row][DIV_PAT_NOTE]>179) continue;

    int prevRow=-1;
    for (int r=row-1; r>=startRow; r--) {
      if (pat->newData[r][DIV_PAT_NOTE]>=0&&pat->newData[r][DIV_PAT_NOTE]<=179) {
        prevRow=r; break;
      }
    }

    bool largeInterval=false;
    if (prevRow>=0) {
      int diff=abs(pat->newData[row][DIV_PAT_NOTE]-pat->newData[prevRow][DIV_PAT_NOTE]);
      largeInterval=(diff>4);
    }

    bool longNote=true;
    for (int r=row+1; r<row+3&&r<endRow; r++) {
      if (r<DIV_MAX_ROWS&&pat->newData[r][DIV_PAT_NOTE]!=-1) {
        longNote=false; break;
      }
    }

    switch (params.role) {
      case ROLE_LEAD:
        if (largeInterval&&rng.randFloat()<0.4f*cf) {
          pat->newData[row][DIV_PAT_FX(0)]=0x03;
          pat->newData[row][DIV_PAT_FXVAL(0)]=(short)rng.randInt(0x10,0x30);
        } else if (longNote&&rng.randFloat()<0.3f*cf) {
          pat->newData[row][DIV_PAT_FX(0)]=0x04;
          int speed=rng.randInt(3,5);
          int depth=rng.randInt(2,4);
          pat->newData[row][DIV_PAT_FXVAL(0)]=(short)((speed<<4)|depth);
        }
        break;

      case ROLE_BASS:
      case ROLE_SLAP_BASS:
        if ((row-startRow)%params.rowsPerBar==0&&rng.randFloat()<0.2f*cf) {
          pat->newData[row][DIV_PAT_FX(0)]=0x02;
          pat->newData[row][DIV_PAT_FXVAL(0)]=(short)rng.randInt(0x08,0x18);
        }
        break;

      case ROLE_PAD:
        if (rng.randFloat()<0.5f) {
          pat->newData[row][DIV_PAT_FX(0)]=0x04;
          int speed=rng.randInt(2,3);
          int depth=rng.randInt(1,3);
          pat->newData[row][DIV_PAT_FXVAL(0)]=(short)((speed<<4)|depth);
        }
        break;

      case ROLE_DIST_GUITAR:
        if ((row-startRow)%params.rowsPerBeat!=0&&rng.randFloat()<0.3f*cf) {
          pat->newData[row][DIV_PAT_FX(0)]=0x0A;
          pat->newData[row][DIV_PAT_FXVAL(0)]=0x08;
        }
        break;

      default:
        if (largeInterval&&rng.randFloat()<0.15f*cf) {
          pat->newData[row][DIV_PAT_FX(0)]=0x03;
          pat->newData[row][DIV_PAT_FXVAL(0)]=(short)rng.randInt(0x10,0x28);
        }
        break;
    }
  }
}

void PatternGenerator::applyNoteOffs(DivPattern* pat, int startRow, int endRow,
                                      int articulationGap, PatchRole role) {
  int gap=articulationGap;
  if (gap<=0) {
    switch (role) {
      case ROLE_BASS:        gap=1; break;
      case ROLE_SLAP_BASS:   gap=2; break;
      case ROLE_LEAD:        gap=0; break;
      case ROLE_PAD:         gap=0; break;
      case ROLE_RHYTHM:      gap=2; break;
      case ROLE_SFX:         gap=3; break;
      case ROLE_DIST_GUITAR: gap=1; break;
      default:               gap=1; break;
    }
  }
  if (gap<=0) return;

  for (int row=startRow; row<endRow; row++) {
    if (row<0||row>=DIV_MAX_ROWS) continue;
    if (pat->newData[row][DIV_PAT_NOTE]<0||pat->newData[row][DIV_PAT_NOTE]>179) continue;

    int nextNoteRow=endRow;
    for (int r=row+1; r<endRow&&r<DIV_MAX_ROWS; r++) {
      if (pat->newData[r][DIV_PAT_NOTE]!=-1) {
        nextNoteRow=r; break;
      }
    }

    int offRow=nextNoteRow-gap;
    if (offRow<=row) continue;
    if (offRow>=endRow||offRow>=DIV_MAX_ROWS) continue;

    if (pat->newData[offRow][DIV_PAT_NOTE]==-1) {
      pat->newData[offRow][DIV_PAT_NOTE]=253; // DIV_NOTE_OFF
    }
  }
}

void PatternGenerator::applyChromaticPassing(DivPattern* pat, const GenPatternParams& params,
                                              float chromaticism, int startRow, int endRow) {
  for (int i=1; i<endRow-startRow-1; i++) {
    int row=startRow+i;
    if (row<0||row>=DIV_MAX_ROWS) continue;
    if (pat->newData[row][DIV_PAT_NOTE]!=-1) continue;

    if ((i%4)==0) continue;

    int prevRow=-1;
    int nextRow=-1;
    for (int r=row-1; r>=startRow; r--) {
      if (pat->newData[r][DIV_PAT_NOTE]>=0&&pat->newData[r][DIV_PAT_NOTE]<=179) {
        prevRow=r; break;
      }
    }
    for (int r=row+1; r<endRow&&r<DIV_MAX_ROWS; r++) {
      if (pat->newData[r][DIV_PAT_NOTE]>=0&&pat->newData[r][DIV_PAT_NOTE]<=179) {
        nextRow=r; break;
      }
    }
    if (prevRow<0||nextRow<0) continue;

    if (rng.randFloat()<chromaticism*0.25f) {
      int prevNote=pat->newData[prevRow][DIV_PAT_NOTE];
      int nextNote=pat->newData[nextRow][DIV_PAT_NOTE];
      int diff=nextNote-prevNote;
      if (abs(diff)<2||abs(diff)>6) continue;

      int passing=nextNote+(diff>0?-1:1);
      passing=genClamp(passing,0,179);

      pat->newData[row][DIV_PAT_NOTE]=(short)passing;
      pat->newData[row][DIV_PAT_INS]=(short)params.insIndex;
      pat->newData[row][DIV_PAT_VOL]=(short)rng.randInt(0x30,0x50);
    }
  }
}

// ========================================
// Master pipeline
// ========================================

void PatternGenerator::generate(DivPattern* pat, const GenPatternParams& params,
                                 const StylePreset& style) {
  generateFill(pat,params,style,0,params.patternLength);
}

void PatternGenerator::generateFill(DivPattern* pat, const GenPatternParams& params,
                                     const StylePreset& style, int startRow, int endRow) {
  if (!pat) return;
  int len=endRow-startRow;
  if (len<=0||len>DIV_MAX_ROWS) return;

  int rowsPerBar=params.rowsPerBar;
  int rowsPerBeat=params.rowsPerBeat;
  if (rowsPerBar<=0) rowsPerBar=16;
  if (rowsPerBeat<=0) rowsPerBeat=4;

  int barCount=computeBarCount(len,rowsPerBar);

  int scaleLen;
  const int* intervals=genScaleIntervals(params.scaleType,scaleLen);

  BarChord chords[16];
  generateChordProgression(chords,barCount,params.scaleType,params.complexity);

  GrooveTemplate groove=buildGrooveTemplate(params.grooveType);

  Motif motifA=generateRoleMotif(params.role,params.density,params.complexity,
                                  style.syncopation,rowsPerBar,params.motifLengthHint,scaleLen);
  Motif motifB=generateRoleMotif(params.role,params.density,params.complexity,
                                  style.syncopation,rowsPerBar,params.motifLengthHint,scaleLen);

  GenContour contourA=params.contourHint;
  if (contourA==GEN_CONTOUR_RANDOM) contourA=(GenContour)rng.randInt(0,(int)GEN_CONTOUR_FLAT);
  GenContour contourB=params.contourHint;
  if (contourB==GEN_CONTOUR_RANDOM) contourB=(GenContour)rng.randInt(0,(int)GEN_CONTOUR_FLAT);
  applyMelodicContour(motifA,contourA,params.complexity,scaleLen);
  applyMelodicContour(motifB,contourB,params.complexity,scaleLen);

  Motif motifPool[2]={motifA,motifB};
  Phrase phrase=buildPhrase(params.phraseForm,barCount,2);

  int octMin=genClamp(params.octaveMin,0,9);
  int octMax=genClamp(params.octaveMax,0,9);
  if (octMax<octMin) octMax=octMin;
  int degreeRange=(octMax-octMin+1)*scaleLen;
  if (degreeRange<=0) degreeRange=scaleLen;
  int degreeRangeBase=degreeRange/2;

  for (int p=0; p<phrase.placementCount; p++) {
    MotifPlacement& mp=phrase.placements[p];
    if (mp.barIndex>=barCount) continue;

    int barStartRow=startRow+mp.barIndex*rowsPerBar;
    if (barStartRow>=endRow) continue;

    Motif m=motifPool[mp.motifIndex%2];
    applyChordToneGravity(m,chords[mp.barIndex],scaleLen,rowsPerBeat,params.chordToneEmphasis);

    writeMotifToPattern(pat,m,mp,chords[mp.barIndex],groove,params,
                         scaleLen,intervals,barStartRow,degreeRangeBase);
  }

  if (params.allowEffects) {
    applyEffects(pat,params,style,startRow,endRow);
  }

  applyNoteOffs(pat,startRow,endRow,params.articulationGap,params.role);

  if (style.chromaticism>0.0f) {
    applyChromaticPassing(pat,params,style.chromaticism,startRow,endRow);
  }
}

void PatternGenerator::setSeed(uint32_t seed) {
  rng.seed(seed);
}
