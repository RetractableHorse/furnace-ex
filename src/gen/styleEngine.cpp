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

#include "styleEngine.h"
#include "stylePresets.h"

const char* patchRoleName(PatchRole role) {
  switch (role) {
    case ROLE_LEAD: return "Lead";
    case ROLE_BASS: return "Bass";
    case ROLE_PAD: return "Pad";
    case ROLE_RHYTHM: return "Rhythm";
    case ROLE_SFX: return "SFX";
    case ROLE_SLAP_BASS: return "Slap Bass";
    case ROLE_DIST_GUITAR: return "Dist. Guitar";
    default: return "Unknown";
  }
}

StyleEngine::StyleEngine():
  activePresetIdx(0) {
  loadBuiltinPresets();
}

void StyleEngine::loadBuiltinPresets() {
  presets.clear();
  presets.push_back(genPresetThunderForce());
  presets.push_back(genPresetStreetsOfRage());
  presets.push_back(genPresetSonic());
  presets.push_back(genPresetMUSHA());
  presets.push_back(genPresetCustom());
  activePresetIdx=0;
}

int StyleEngine::getPresetCount() const {
  return (int)presets.size();
}

const StylePreset& StyleEngine::getPreset(int idx) const {
  if (idx<0||idx>=(int)presets.size()) return presets[0];
  return presets[idx];
}

const StylePreset& StyleEngine::getActivePreset() const {
  return getPreset(activePresetIdx);
}

int StyleEngine::getActivePresetIdx() const {
  return activePresetIdx;
}

void StyleEngine::setActivePreset(int idx) {
  if (idx>=0&&idx<(int)presets.size()) {
    activePresetIdx=idx;
  }
}

const PatchRoleConstraints& StyleEngine::getRoleConstraints(PatchRole role) const {
  const StylePreset& p=getActivePreset();
  if (role<0||role>=ROLE_MAX) return p.roles[ROLE_LEAD];
  return p.roles[role];
}

StylePreset& StyleEngine::getCustomPreset() {
  // custom is always the last preset
  return presets.back();
}
