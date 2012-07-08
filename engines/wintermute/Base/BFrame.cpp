/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

/*
 * This file is based on WME Lite.
 * http://dead-code.org/redir.php?target=wmelite
 * Copyright (c) 2011 Jan Nedoma
 */

#include "engines/wintermute/dcgf.h"
#include "engines/wintermute/Base/BParser.h"
#include "engines/wintermute/Base/BFrame.h"
#include "engines/wintermute/Base/BGame.h"
#include "engines/wintermute/Base/BDynBuffer.h"
#include "engines/wintermute/Base/BSoundMgr.h"
#include "engines/wintermute/Base/BSound.h"
#include "engines/wintermute/Base/BSubFrame.h"
#include "engines/wintermute/PlatformSDL.h"
#include "engines/wintermute/Base/scriptables/ScValue.h"
#include "engines/wintermute/Base/scriptables/ScScript.h"
#include "engines/wintermute/Base/scriptables/ScStack.h"
#include "common/str.h"

namespace WinterMute {

IMPLEMENT_PERSISTENT(CBFrame, false)

//////////////////////////////////////////////////////////////////////
CBFrame::CBFrame(CBGame *inGame): CBScriptable(inGame, true) {
	_delay = 0;
	_moveX = _moveY = 0;

	_sound = NULL;
	_killSound = false;

	_editorExpanded = false;
	_keyframe = false;
}


//////////////////////////////////////////////////////////////////////
CBFrame::~CBFrame() {
	delete _sound;
	_sound = NULL;

	for (int i = 0; i < _subframes.GetSize(); i++) 
		delete _subframes[i];
	_subframes.RemoveAll();

	for (int i = 0; i < _applyEvent.GetSize(); i++) {
		delete[] _applyEvent[i];
		_applyEvent[i] = NULL;
	}
	_applyEvent.RemoveAll();
}


//////////////////////////////////////////////////////////////////////
HRESULT CBFrame::draw(int x, int y, CBObject *registerOwner, float zoomX, float zoomY, bool precise, uint32 alpha, bool allFrames, float rotate, TSpriteBlendMode blendMode) {
	HRESULT res;

	for (int i = 0; i < _subframes.GetSize(); i++) {
		res = _subframes[i]->draw(x, y, registerOwner, zoomX, zoomY, precise, alpha, rotate, blendMode);
		if (FAILED(res)) return res;
	}
	return S_OK;
}


//////////////////////////////////////////////////////////////////////////
HRESULT CBFrame::oneTimeDisplay(CBObject *owner, bool muted) {
	if (_sound && !muted) {
		if (owner) owner->updateOneSound(_sound);
		_sound->play();
		/*
		if (Game->_state == GAME_FROZEN) {
		    _sound->Pause(true);
		}
		*/
	}
	if (owner) {
		for (int i = 0; i < _applyEvent.GetSize(); i++) {
			owner->applyEvent(_applyEvent[i]);
		}
	}
	return S_OK;
}



TOKEN_DEF_START
TOKEN_DEF(DELAY)
TOKEN_DEF(IMAGE)
TOKEN_DEF(TRANSPARENT)
TOKEN_DEF(RECT)
TOKEN_DEF(HOTSPOT)
TOKEN_DEF(2D_ONLY)
TOKEN_DEF(3D_ONLY)
TOKEN_DEF(MIRROR_X)
TOKEN_DEF(MIRROR_Y)
TOKEN_DEF(MOVE)
TOKEN_DEF(ALPHA_COLOR)
TOKEN_DEF(ALPHA)
TOKEN_DEF(SUBFRAME)
TOKEN_DEF(SOUND)
TOKEN_DEF(KEYFRAME)
TOKEN_DEF(DECORATION)
TOKEN_DEF(APPLY_EVENT)
TOKEN_DEF(EDITOR_SELECTED)
TOKEN_DEF(EDITOR_EXPANDED)
TOKEN_DEF(EDITOR_PROPERTY)
TOKEN_DEF(KILL_SOUND)
TOKEN_DEF_END
//////////////////////////////////////////////////////////////////////
HRESULT CBFrame::loadBuffer(byte *buffer, int lifeTime, bool keepLoaded) {
	TOKEN_TABLE_START(commands)
	TOKEN_TABLE(DELAY)
	TOKEN_TABLE(IMAGE)
	TOKEN_TABLE(TRANSPARENT)
	TOKEN_TABLE(RECT)
	TOKEN_TABLE(HOTSPOT)
	TOKEN_TABLE(2D_ONLY)
	TOKEN_TABLE(3D_ONLY)
	TOKEN_TABLE(MIRROR_X)
	TOKEN_TABLE(MIRROR_Y)
	TOKEN_TABLE(MOVE)
	TOKEN_TABLE(ALPHA_COLOR)
	TOKEN_TABLE(ALPHA)
	TOKEN_TABLE(SUBFRAME)
	TOKEN_TABLE(SOUND)
	TOKEN_TABLE(KEYFRAME)
	TOKEN_TABLE(DECORATION)
	TOKEN_TABLE(APPLY_EVENT)
	TOKEN_TABLE(EDITOR_SELECTED)
	TOKEN_TABLE(EDITOR_EXPANDED)
	TOKEN_TABLE(EDITOR_PROPERTY)
	TOKEN_TABLE(KILL_SOUND)
	TOKEN_TABLE_END

	char *params;
	int cmd;
	CBParser parser(Game);
	RECT rect;
	int r = 255, g = 255, b = 255;
	int ar = 255, ag = 255, ab = 255, alpha = 255;
	int hotspotX = 0, hotspotY = 0;
	bool custoTrans = false;
	bool editorSelected = false;
	bool is2DOnly = false;
	bool is3DOnly = false;
	bool decoration = false;
	bool mirrorX = false;
	bool mirrorY = false;
	CBPlatform::setRectEmpty(&rect);
	char *surface_file = NULL;

	while ((cmd = parser.getCommand((char **)&buffer, commands, &params)) > 0) {
		switch (cmd) {
		case TOKEN_DELAY:
			parser.scanStr(params, "%d", &_delay);
			break;

		case TOKEN_IMAGE:
			surface_file = params;
			break;

		case TOKEN_TRANSPARENT:
			parser.scanStr(params, "%d,%d,%d", &r, &g, &b);
			custoTrans = true;
			break;

		case TOKEN_RECT:
			parser.scanStr(params, "%d,%d,%d,%d", &rect.left, &rect.top, &rect.right, &rect.bottom);
			break;

		case TOKEN_HOTSPOT:
			parser.scanStr(params, "%d,%d", &hotspotX, &hotspotY);
			break;

		case TOKEN_MOVE:
			parser.scanStr(params, "%d,%d", &_moveX, &_moveY);
			break;

		case TOKEN_2D_ONLY:
			parser.scanStr(params, "%b", &is2DOnly);
			break;

		case TOKEN_3D_ONLY:
			parser.scanStr(params, "%b", &is3DOnly);
			break;

		case TOKEN_MIRROR_X:
			parser.scanStr(params, "%b", &mirrorX);
			break;

		case TOKEN_MIRROR_Y:
			parser.scanStr(params, "%b", &mirrorY);
			break;

		case TOKEN_ALPHA_COLOR:
			parser.scanStr(params, "%d,%d,%d", &ar, &ag, &ab);
			break;

		case TOKEN_ALPHA:
			parser.scanStr(params, "%d", &alpha);
			break;

		case TOKEN_EDITOR_SELECTED:
			parser.scanStr(params, "%b", &editorSelected);
			break;

		case TOKEN_EDITOR_EXPANDED:
			parser.scanStr(params, "%b", &_editorExpanded);
			break;

		case TOKEN_KILL_SOUND:
			parser.scanStr(params, "%b", &_killSound);
			break;

		case TOKEN_SUBFRAME: {
			CBSubFrame *subframe = new CBSubFrame(Game);
			if (!subframe || FAILED(subframe->loadBuffer((byte *)params, lifeTime, keepLoaded))) {
				delete subframe;
				cmd = PARSERR_GENERIC;
			} else _subframes.Add(subframe);
		}
		break;

		case TOKEN_SOUND: {
			if (_sound) {
				delete _sound;
				_sound = NULL;
			}
			_sound = new CBSound(Game);
			if (!_sound || FAILED(_sound->setSound(params, SOUND_SFX, false))) {
				if (Game->_soundMgr->_soundAvailable) Game->LOG(0, "Error loading sound '%s'.", params);
				delete _sound;
				_sound = NULL;
			}
		}
		break;

		case TOKEN_APPLY_EVENT: {
			char *Event = new char[strlen(params) + 1];
			strcpy(Event, params);
			_applyEvent.Add(Event);
		}
		break;

		case TOKEN_KEYFRAME:
			parser.scanStr(params, "%b", &_keyframe);
			break;

		case TOKEN_DECORATION:
			parser.scanStr(params, "%b", &decoration);
			break;

		case TOKEN_EDITOR_PROPERTY:
			parseEditorProperty((byte *)params, false);
			break;
		}
	}
	if (cmd == PARSERR_TOKENNOTFOUND) {
		Game->LOG(0, "Syntax error in FRAME definition");
		return E_FAIL;
	}

	if (cmd == PARSERR_GENERIC) {
		Game->LOG(0, "Error loading FRAME definition");
		return E_FAIL;
	}


	CBSubFrame *sub = new CBSubFrame(Game);
	if (surface_file != NULL) {
		if (custoTrans) sub->setSurface(surface_file, false, r, g, b, lifeTime, keepLoaded);
		else sub->setSurface(surface_file, true, 0, 0, 0, lifeTime, keepLoaded);

		if (!sub->_surface) {
			delete sub;
			Game->LOG(0, "Error loading SUBFRAME");
			return E_FAIL;
		}

		sub->_alpha = DRGBA(ar, ag, ab, alpha);
		if (custoTrans) sub->_transparent = DRGBA(r, g, b, 0xFF);
	}

	if (CBPlatform::isRectEmpty(&rect)) sub->setDefaultRect();
	else sub->_rect = rect;

	sub->_hotspotX = hotspotX;
	sub->_hotspotY = hotspotY;
	sub->_2DOnly = is2DOnly;
	sub->_3DOnly = is3DOnly;
	sub->_decoration = decoration;
	sub->_mirrorX = mirrorX;
	sub->_mirrorY = mirrorY;


	sub->_editorSelected = editorSelected;
	_subframes.InsertAt(0, sub);

	return S_OK;
}


//////////////////////////////////////////////////////////////////////////
bool CBFrame::getBoundingRect(LPRECT rect, int x, int y, float scaleX, float scaleY) {
	if (!rect) return false;
	CBPlatform::setRectEmpty(rect);

	RECT subRect;

	for (int i = 0; i < _subframes.GetSize(); i++) {
		_subframes[i]->getBoundingRect(&subRect, x, y, scaleX, scaleY);
		CBPlatform::unionRect(rect, rect, &subRect);
	}
	return true;
}



//////////////////////////////////////////////////////////////////////////
HRESULT CBFrame::saveAsText(CBDynBuffer *buffer, int indent) {
	buffer->putTextIndent(indent, "FRAME {\n");
	buffer->putTextIndent(indent + 2, "DELAY = %d\n", _delay);

	if (_moveX != 0 || _moveY != 0)
		buffer->putTextIndent(indent + 2, "MOVE {%d, %d}\n", _moveX, _moveY);

	if (_sound && _sound->_soundFilename)
		buffer->putTextIndent(indent + 2, "SOUND=\"%s\"\n", _sound->_soundFilename);

	buffer->putTextIndent(indent + 2, "KEYFRAME=%s\n", _keyframe ? "TRUE" : "FALSE");

	if (_killSound)
		buffer->putTextIndent(indent + 2, "KILL_SOUND=%s\n", _killSound ? "TRUE" : "FALSE");

	if (_editorExpanded)
		buffer->putTextIndent(indent + 2, "EDITOR_EXPANDED=%s\n", _editorExpanded ? "TRUE" : "FALSE");

	if (_subframes.GetSize() > 0) _subframes[0]->saveAsText(buffer, indent, false);

	for (int i = 1; i < _subframes.GetSize(); i++) {
		_subframes[i]->saveAsText(buffer, indent + 2);
	}

	for (int i = 0; i < _applyEvent.GetSize(); i++) {
		buffer->putTextIndent(indent + 2, "APPLY_EVENT=\"%s\"\n", _applyEvent[i]);
	}

	CBBase::saveAsText(buffer, indent + 2);


	buffer->putTextIndent(indent, "}\n\n");

	return S_OK;
}


//////////////////////////////////////////////////////////////////////////
HRESULT CBFrame::persist(CBPersistMgr *persistMgr) {
	CBScriptable::persist(persistMgr);

	_applyEvent.persist(persistMgr);
	persistMgr->transfer(TMEMBER(_delay));
	persistMgr->transfer(TMEMBER(_editorExpanded));
	persistMgr->transfer(TMEMBER(_keyframe));
	persistMgr->transfer(TMEMBER(_killSound));
	persistMgr->transfer(TMEMBER(_moveX));
	persistMgr->transfer(TMEMBER(_moveY));
	persistMgr->transfer(TMEMBER(_sound));
	_subframes.persist(persistMgr);

	return S_OK;
}


//////////////////////////////////////////////////////////////////////////
// high level scripting interface
//////////////////////////////////////////////////////////////////////////
HRESULT CBFrame::scCallMethod(CScScript *script, CScStack *stack, CScStack *thisStack, const char *name) {

	//////////////////////////////////////////////////////////////////////////
	// GetSound
	//////////////////////////////////////////////////////////////////////////
	if (strcmp(name, "GetSound") == 0) {
		stack->correctParams(0);

		if (_sound && _sound->_soundFilename) stack->pushString(_sound->_soundFilename);
		else stack->pushNULL();
		return S_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// SetSound
	//////////////////////////////////////////////////////////////////////////
	if (strcmp(name, "SetSound") == 0) {
		stack->correctParams(1);
		CScValue *val = stack->pop();
		delete _sound;
		_sound = NULL;

		if (!val->isNULL()) {
			_sound = new CBSound(Game);
			if (!_sound || FAILED(_sound->setSound(val->getString(), SOUND_SFX, false))) {
				stack->pushBool(false);
				delete _sound;
				_sound = NULL;
			} else stack->pushBool(true);
		} else stack->pushBool(true);
		return S_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// GetSubframe
	//////////////////////////////////////////////////////////////////////////
	if (strcmp(name, "GetSubframe") == 0) {
		stack->correctParams(1);
		int index = stack->pop()->getInt(-1);
		if (index < 0 || index >= _subframes.GetSize()) {
			script->runtimeError("Frame.GetSubframe: Subframe index %d is out of range.", index);
			stack->pushNULL();
		} else stack->pushNative(_subframes[index], true);

		return S_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// DeleteSubframe
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "DeleteSubframe") == 0) {
		stack->correctParams(1);
		CScValue *val = stack->pop();
		if (val->isInt()) {
			int index = val->getInt(-1);
			if (index < 0 || index >= _subframes.GetSize()) {
				script->runtimeError("Frame.DeleteSubframe: Subframe index %d is out of range.", index);
			}
		} else {
			CBSubFrame *sub = (CBSubFrame *)val->getNative();
			for (int i = 0; i < _subframes.GetSize(); i++) {
				if (_subframes[i] == sub) {
					delete _subframes[i];
					_subframes.RemoveAt(i);
					break;
				}
			}
		}
		stack->pushNULL();
		return S_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// AddSubframe
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "AddSubframe") == 0) {
		stack->correctParams(1);
		CScValue *val = stack->pop();
		const char *filename = NULL;
		if (!val->isNULL()) filename = val->getString();

		CBSubFrame *sub = new CBSubFrame(Game);
		if (filename != NULL) {
			sub->setSurface(filename);
			sub->setDefaultRect();
		}
		_subframes.Add(sub);

		stack->pushNative(sub, true);
		return S_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// InsertSubframe
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "InsertSubframe") == 0) {
		stack->correctParams(2);
		int index = stack->pop()->getInt();
		if (index < 0) index = 0;

		CScValue *val = stack->pop();
		const char *filename = NULL;
		if (!val->isNULL()) filename = val->getString();

		CBSubFrame *sub = new CBSubFrame(Game);
		if (filename != NULL) {
			sub->setSurface(filename);
		}

		if (index >= _subframes.GetSize()) _subframes.Add(sub);
		else _subframes.InsertAt(index, sub);

		stack->pushNative(sub, true);
		return S_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// GetEvent
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "GetSubframe") == 0) {
		stack->correctParams(1);
		int index = stack->pop()->getInt(-1);
		if (index < 0 || index >= _applyEvent.GetSize()) {
			script->runtimeError("Frame.GetEvent: Event index %d is out of range.", index);
			stack->pushNULL();
		} else stack->pushString(_applyEvent[index]);
		return S_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// AddEvent
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "AddEvent") == 0) {
		stack->correctParams(1);
		const char *event = stack->pop()->getString();
		for (int i = 0; i < _applyEvent.GetSize(); i++) {
			if (scumm_stricmp(_applyEvent[i], event) == 0) {
				stack->pushNULL();
				return S_OK;
			}
		}
		_applyEvent.Add(event);
		stack->pushNULL();
		return S_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// DeleteEvent
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "DeleteEvent") == 0) {
		stack->correctParams(1);
		const char *event = stack->pop()->getString();
		for (int i = 0; i < _applyEvent.GetSize(); i++) {
			if (scumm_stricmp(_applyEvent[i], event) == 0) {
				delete [] _applyEvent[i];
				_applyEvent.RemoveAt(i);
				break;
			}
		}
		stack->pushNULL();
		return S_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	else {
		if (_subframes.GetSize() == 1) return _subframes[0]->scCallMethod(script, stack, thisStack, name);
		else return CBScriptable::scCallMethod(script, stack, thisStack, name);
	}
}


//////////////////////////////////////////////////////////////////////////
CScValue *CBFrame::scGetProperty(const char *name) {
	if (!_scValue) _scValue = new CScValue(Game);
	_scValue->setNULL();

	//////////////////////////////////////////////////////////////////////////
	// Type (RO)
	//////////////////////////////////////////////////////////////////////////
	if (strcmp(name, "Type") == 0) {
		_scValue->setString("frame");
		return _scValue;
	}

	//////////////////////////////////////////////////////////////////////////
	// Delay
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "Delay") == 0) {
		_scValue->setInt(_delay);
		return _scValue;
	}

	//////////////////////////////////////////////////////////////////////////
	// Keyframe
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "Keyframe") == 0) {
		_scValue->setBool(_keyframe);
		return _scValue;
	}

	//////////////////////////////////////////////////////////////////////////
	// KillSounds
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "KillSounds") == 0) {
		_scValue->setBool(_killSound);
		return _scValue;
	}

	//////////////////////////////////////////////////////////////////////////
	// MoveX
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "MoveX") == 0) {
		_scValue->setInt(_moveX);
		return _scValue;
	}

	//////////////////////////////////////////////////////////////////////////
	// MoveY
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "MoveY") == 0) {
		_scValue->setInt(_moveY);
		return _scValue;
	}

	//////////////////////////////////////////////////////////////////////////
	// NumSubframes (RO)
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "NumSubframes") == 0) {
		_scValue->setInt(_subframes.GetSize());
		return _scValue;
	}

	//////////////////////////////////////////////////////////////////////////
	// NumEvents (RO)
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "NumEvents") == 0) {
		_scValue->setInt(_applyEvent.GetSize());
		return _scValue;
	}

	//////////////////////////////////////////////////////////////////////////
	else {
		if (_subframes.GetSize() == 1) return _subframes[0]->scGetProperty(name);
		else return CBScriptable::scGetProperty(name);
	}
}


//////////////////////////////////////////////////////////////////////////
HRESULT CBFrame::scSetProperty(const char *name, CScValue *value) {
	//////////////////////////////////////////////////////////////////////////
	// Delay
	//////////////////////////////////////////////////////////////////////////
	if (strcmp(name, "Delay") == 0) {
		_delay = MAX(0, value->getInt());
		return S_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// Keyframe
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "Keyframe") == 0) {
		_keyframe = value->getBool();
		return S_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// KillSounds
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "KillSounds") == 0) {
		_killSound = value->getBool();
		return S_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// MoveX
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "MoveX") == 0) {
		_moveX = value->getInt();
		return S_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// MoveY
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "MoveY") == 0) {
		_moveY = value->getInt();
		return S_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	else {
		if (_subframes.GetSize() == 1) return _subframes[0]->scSetProperty(name, value);
		else return CBScriptable::scSetProperty(name, value);
	}
}


//////////////////////////////////////////////////////////////////////////
const char *CBFrame::scToString() {
	return "[frame]";
}

} // end of namespace WinterMute
