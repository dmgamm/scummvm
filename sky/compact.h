/* ScummVM - Scumm Interpreter
 * Copyright (C) 2003-2004 The ScummVM project
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	 See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 *
 * $Header$
 *
 */

#ifndef SKYCOMPACT_H
#define SKYCOMPACT_H

#include "sky/sky.h"
#include "sky/struc.h"
#include "sky/skydefs.h"

class File;

enum CptIds {
	CPT_JOEY = 1,
	CPT_FOSTER = 3,
	CPT_REICH_DOOR_20 = 0x30AB,
	CPT_MOVE_LIST = 0xBD //0x8000
};

enum CptTypeIds {
	CPT_NULL = 0,
	COMPACT,
	TURNTAB,
	ANIMSEQ,
	MISCBIN,
	GETTOTAB,
	ROUTEBUF,
	MAINLIST
};

#define TALKTABLE_LIST_ID 0x7000

namespace Sky {

class SkyCompact {
public:
	SkyCompact(void);
	~SkyCompact(void);
	Compact *fetchCpt(uint16 cptId);
	Compact *fetchCptInfo(uint16 cptId, uint16 *elems = NULL, uint16 *type = NULL, char *name = NULL);
	static uint16 *getSub(Compact *cpt, uint16 mode);
	static MegaSet *getMegaSet(Compact *cpt);
	uint16 *getGrafixPtr(Compact *cpt);
	uint16 *getTurnTable(Compact *cpt, uint16 dir);
	void *getCompactElem(Compact *cpt, uint16 off);
	bool cptIsId(Compact *cpt, uint16 id);
	uint8	*createResetData(uint16 gameVersion);
	uint16	_numSaveIds;
	uint16	*_saveIds;
private:
	uint16  _numDataLists;
	uint16  *_dataListLen;
	uint16  *_rawBuf;
	char	*_asciiBuf;
	Compact ***_compacts;
	char    ***_cptNames;
	uint16	**_cptSizes;
	uint16  **_cptTypes;
	File	*_cptFile;
	uint32	_resetDataPos;
};

} // End of namespace Sky

#endif
