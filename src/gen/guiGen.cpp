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

#include "../gui/gui.h"
#include "genWorkspace.h"
#include "guiGen.h"
#include "imgui.h"

static const char* algoNames[]={
  "0: 1>2>3>4",
  "1: (1+2)>3>4",
  "2: (1+(2>3))>4",
  "3: ((1>2)+3)>4",
  "4: (1>2)+(3>4)",
  "5: 1>(2+3+4)",
  "6: (1>2)+3+4",
  "7: 1+2+3+4"
};

const char* genAlgoName(int algo) {
  if (algo<0||algo>7) return "?";
  return algoNames[algo];
}

static const char* noteNames[]={"C","C#","D","D#","E","F","F#","G","G#","A","A#","B"};

void FurnaceGUI::drawGenWorkspace() {
  if (nextWindow==GUI_WINDOW_GEN_WORKSPACE) {
    genWorkspaceOpen=true;
    ImGui::SetNextWindowFocus();
    nextWindow=GUI_WINDOW_NOTHING;
  }
  if (!genWorkspaceOpen) return;
  if (!genWorkspace) return;

  ImGui::SetNextWindowSizeConstraints(ImVec2(400.0f*dpiScale,300.0f*dpiScale),ImVec2(FLT_MAX,FLT_MAX));
  if (ImGui::Begin("Generative Workspace##GenWorkspace",&genWorkspaceOpen,globalWinFlags,_("Generative Workspace##GenWorkspace"))) {
    // re-detect systems whenever panel is visible
    genWorkspace->detectSystems();

    if (!genWorkspace->ym2612Available) {
      ImGui::TextWrapped("No YM2612/Genesis system detected. Add a YM2612 system to enable generative features.");
      ImGui::End();
      return;
    }

    // === STYLE PRESET ===
    ImGui::SeparatorText("Style");
    {
      int presetIdx=genWorkspace->styleEngine.getActivePresetIdx();
      int presetCount=genWorkspace->styleEngine.getPresetCount();
      if (ImGui::BeginCombo("Preset",genWorkspace->styleEngine.getActivePreset().name.c_str())) {
        for (int i=0; i<presetCount; i++) {
          bool selected=(i==presetIdx);
          if (ImGui::Selectable(genWorkspace->styleEngine.getPreset(i).name.c_str(),selected)) {
            genWorkspace->styleEngine.setActivePreset(i);
          }
          if (selected) ImGui::SetItemDefaultFocus();
        }
        ImGui::EndCombo();
      }

      // scale and key
      int scaleRoot=genWorkspace->patParams.scaleRoot;
      if (ImGui::BeginCombo("Key",noteNames[scaleRoot])) {
        for (int i=0; i<12; i++) {
          bool selected=(i==scaleRoot);
          if (ImGui::Selectable(noteNames[i],selected)) {
            genWorkspace->patParams.scaleRoot=i;
          }
          if (selected) ImGui::SetItemDefaultFocus();
        }
        ImGui::EndCombo();
      }
      ImGui::SameLine();
      int scaleType=(int)genWorkspace->patParams.scaleType;
      if (ImGui::BeginCombo("Scale",genScaleName(genWorkspace->patParams.scaleType))) {
        for (int i=0; i<GEN_SCALE_MAX; i++) {
          bool selected=(i==scaleType);
          if (ImGui::Selectable(genScaleName((GenScaleType)i),selected)) {
            genWorkspace->patParams.scaleType=(GenScaleType)i;
          }
          if (selected) ImGui::SetItemDefaultFocus();
        }
        ImGui::EndCombo();
      }
    }

    // === SEED ===
    ImGui::Separator();
    {
      ImGui::AlignTextToFramePadding();
      ImGui::Text("Seed:");
      ImGui::SameLine();
      int seedInt=(int)genWorkspace->currentSeed;
      ImGui::SetNextItemWidth(120.0f*dpiScale);
      if (ImGui::InputInt("##Seed",&seedInt)) {
        genWorkspace->currentSeed=(uint32_t)seedInt;
      }
      ImGui::SameLine();
      if (ImGui::Checkbox("Lock",&genWorkspace->lockSeed)) {}
      ImGui::SameLine();
      if (ImGui::Button("Randomize##SeedRand")) {
        genWorkspace->randomizeSeed();
      }
    }

    // === PATCH GENERATOR ===
    ImGui::SeparatorText("Patch Generator");
    {
      // role selector
      int role=(int)genWorkspace->currentRole;
      if (ImGui::BeginCombo("Role",patchRoleName(genWorkspace->currentRole))) {
        for (int i=0; i<ROLE_MAX; i++) {
          bool selected=(i==role);
          if (ImGui::Selectable(patchRoleName((PatchRole)i),selected)) {
            genWorkspace->currentRole=(PatchRole)i;
          }
          if (selected) ImGui::SetItemDefaultFocus();
        }
        ImGui::EndCombo();
      }

      // buttons
      if (ImGui::Button("Generate")) {
        genWorkspace->generatePatch();
      }
      ImGui::SameLine();
      if (ImGui::Button("Mutate")) {
        genWorkspace->mutatePatch(3);
      }
      ImGui::SameLine();
      if (ImGui::Button("Audition")) {
        genWorkspace->auditPatch();
      }
      ImGui::SameLine();
      if (ImGui::Button("Stop")) {
        genWorkspace->stopAudit();
      }
      ImGui::SameLine();
      if (ImGui::Button("Commit")) {
        int idx=genWorkspace->commitPatch();
        if (idx>=0) {
          curIns=idx;
        }
      }

      // audition settings
      ImGui::SetNextItemWidth(100.0f*dpiScale);
      ImGui::InputInt("Audit Channel",&genWorkspace->auditChannel);
      genWorkspace->auditChannel=genClamp(genWorkspace->auditChannel,0,e->getTotalChannelCount()-1);
      ImGui::SameLine();
      ImGui::SetNextItemWidth(100.0f*dpiScale);
      ImGui::InputInt("Audit Note",&genWorkspace->auditNote);
      genWorkspace->auditNote=genClamp(genWorkspace->auditNote,0,179);

      // patch preview
      if (genWorkspace->hasPatch) {
        ImGui::Text("Preview: %s",genWorkspace->patchDesc);
        ImGui::Text("  Algorithm: %s",genAlgoName(genWorkspace->currentPatch.fm.alg));
      }
    }

    // === PATTERN GENERATOR ===
    ImGui::SeparatorText("Pattern Generator");
    {
      int totalCh=e->getTotalChannelCount();

      ImGui::SetNextItemWidth(150.0f*dpiScale);
      if (ImGui::BeginCombo("Target Channel",e->getChannelName(genWorkspace->patParams.channel))) {
        for (int i=0; i<totalCh; i++) {
          bool selected=(i==genWorkspace->patParams.channel);
          if (ImGui::Selectable(e->getChannelName(i),selected)) {
            genWorkspace->patParams.channel=i;
          }
          if (selected) ImGui::SetItemDefaultFocus();
        }
        ImGui::EndCombo();
      }

      ImGui::SetNextItemWidth(100.0f*dpiScale);
      ImGui::InputInt("Instrument##PatIns",&genWorkspace->patParams.insIndex);
      if (e->song.insLen>0) {
        genWorkspace->patParams.insIndex=genClamp(genWorkspace->patParams.insIndex,0,e->song.insLen-1);
      } else {
        genWorkspace->patParams.insIndex=0;
      }

      // role for pattern
      int patRole=(int)genWorkspace->patParams.role;
      if (ImGui::BeginCombo("Pattern Role",patchRoleName(genWorkspace->patParams.role))) {
        for (int i=0; i<ROLE_MAX; i++) {
          bool selected=(i==patRole);
          if (ImGui::Selectable(patchRoleName((PatchRole)i),selected)) {
            genWorkspace->patParams.role=(PatchRole)i;
          }
          if (selected) ImGui::SetItemDefaultFocus();
        }
        ImGui::EndCombo();
      }

      ImGui::SliderInt("Density",&genWorkspace->patParams.density,0,100,"%d%%");
      ImGui::SliderInt("Complexity",&genWorkspace->patParams.complexity,0,100,"%d%%");

      ImGui::SetNextItemWidth(80.0f*dpiScale);
      ImGui::InputInt("Oct Min",&genWorkspace->patParams.octaveMin);
      genWorkspace->patParams.octaveMin=genClamp(genWorkspace->patParams.octaveMin,0,9);
      ImGui::SameLine();
      ImGui::SetNextItemWidth(80.0f*dpiScale);
      ImGui::InputInt("Oct Max",&genWorkspace->patParams.octaveMax);
      genWorkspace->patParams.octaveMax=genClamp(genWorkspace->patParams.octaveMax,genWorkspace->patParams.octaveMin,9);

      ImGui::Checkbox("Allow Effects",&genWorkspace->patParams.allowEffects);

      // groove type
      {
        static const char* grooveNames[]={"Straight","Shuffle","Funk","Driving","Half-time"};
        int groove=(int)genWorkspace->patParams.grooveType;
        ImGui::SetNextItemWidth(150.0f*dpiScale);
        if (ImGui::BeginCombo("Groove",grooveNames[groove])) {
          for (int i=0; i<GEN_GROOVE_MAX; i++) {
            bool selected=(i==groove);
            if (ImGui::Selectable(grooveNames[i],selected)) {
              genWorkspace->patParams.grooveType=(GenGrooveType)i;
            }
            if (selected) ImGui::SetItemDefaultFocus();
          }
          ImGui::EndCombo();
        }
      }

      // phrase form
      {
        static const char* phraseNames[]={"AABA","ABAB","AAAB","ABAC","Random"};
        int pf=(int)genWorkspace->patParams.phraseForm;
        ImGui::SetNextItemWidth(150.0f*dpiScale);
        if (ImGui::BeginCombo("Phrase Form",phraseNames[pf])) {
          for (int i=0; i<GEN_PHRASE_MAX; i++) {
            bool selected=(i==pf);
            if (ImGui::Selectable(phraseNames[i],selected)) {
              genWorkspace->patParams.phraseForm=(GenPhraseForm)i;
            }
            if (selected) ImGui::SetItemDefaultFocus();
          }
          ImGui::EndCombo();
        }
      }

      // contour hint
      {
        static const char* contourNames[]={"Arch","Valley","Ascending","Descending","Flat","Random"};
        int ct=(int)genWorkspace->patParams.contourHint;
        ImGui::SetNextItemWidth(150.0f*dpiScale);
        if (ImGui::BeginCombo("Contour",contourNames[ct])) {
          for (int i=0; i<GEN_CONTOUR_MAX; i++) {
            bool selected=(i==ct);
            if (ImGui::Selectable(contourNames[i],selected)) {
              genWorkspace->patParams.contourHint=(GenContour)i;
            }
            if (selected) ImGui::SetItemDefaultFocus();
          }
          ImGui::EndCombo();
        }
      }

      // chord tone emphasis
      ImGui::SliderFloat("Chord Tone Emphasis",&genWorkspace->patParams.chordToneEmphasis,0.0f,1.0f,"%.2f");

      // articulation gap
      ImGui::SliderInt("Articulation Gap",&genWorkspace->patParams.articulationGap,0,4,
        genWorkspace->patParams.articulationGap==0?"Legato":"%d rows");

      // generate buttons
      if (ImGui::Button("Generate Pattern")) {
        // use current order's pattern index for this channel
        if (e->curSubSong) {
          unsigned char order=e->getOrder();
          int patIdx=e->curSubSong->orders.ord[genWorkspace->patParams.channel][order];
          genWorkspace->generatePattern(genWorkspace->patParams.channel,patIdx);
        }
      }
      ImGui::SameLine();
      if (ImGui::Button("Generate Fill")) {
        if (e->curSubSong) {
          unsigned char order=e->getOrder();
          int patIdx=e->curSubSong->orders.ord[genWorkspace->patParams.channel][order];
          int selStart=0;
          int selEnd=e->curSubSong->patLen;
          genWorkspace->generateFill(genWorkspace->patParams.channel,patIdx,selStart,selEnd);
        }
      }
    }
  }
  if (ImGui::IsWindowFocused(ImGuiFocusedFlags_ChildWindows)) curWindow=GUI_WINDOW_GEN_WORKSPACE;
  ImGui::End();
}
