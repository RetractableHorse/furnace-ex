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

// This header is intentionally minimal.
// The generative workspace GUI is implemented as FurnaceGUI::drawGenWorkspace()
// which is declared in gui.h. This file exists for the build system and
// any future GUI-specific helpers.

#ifndef _GUI_GEN_H
#define _GUI_GEN_H

// algorithm topology diagrams for visual display
const char* genAlgoName(int algo);

#endif
