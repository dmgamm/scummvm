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
#include "engines/wintermute/Base/BFader.h"
#include "engines/wintermute/Base/BGame.h"
#include "engines/wintermute/PlatformSDL.h"
#include "common/util.h"

namespace WinterMute {

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

IMPLEMENT_PERSISTENT(CBFader, false)

//////////////////////////////////////////////////////////////////////////
CBFader::CBFader(CBGame *inGame): CBObject(inGame) {
	_active = false;
	_red = _green = _blue = 0;
	_currentAlpha = 0x00;
	_sourceAlpha = 0;
	_targetAlpha = 0;
	_duration = 1000;
	_startTime = 0;
	_system = false;
}


//////////////////////////////////////////////////////////////////////////
CBFader::~CBFader() {

}


//////////////////////////////////////////////////////////////////////////
HRESULT CBFader::update() {
	if (!_active) return S_OK;

	int alphaDelta = _targetAlpha - _sourceAlpha;

	uint32 time;

	if (_system) time = CBPlatform::getTime() - _startTime;
	else time = Game->_timer - _startTime;

	if (time >= _duration) _currentAlpha = _targetAlpha;
	else {
		_currentAlpha = (byte)(_sourceAlpha + (float)time / (float)_duration * alphaDelta);
	}
	_currentAlpha = MIN((unsigned char)255, MAX(_currentAlpha, (byte)0));  // TODO: clean

	_ready = time >= _duration;
	if (_ready && _currentAlpha == 0x00) _active = false;

	return S_OK;
}


//////////////////////////////////////////////////////////////////////////
HRESULT CBFader::display() {
	if (!_active) return S_OK;

	if (_currentAlpha > 0x00) return Game->_renderer->fadeToColor(DRGBA(_red, _green, _blue, _currentAlpha));
	else return S_OK;
}


//////////////////////////////////////////////////////////////////////////
HRESULT CBFader::deactivate() {
	_active = false;
	_ready = true;
	return S_OK;
}


//////////////////////////////////////////////////////////////////////////
HRESULT CBFader::fadeIn(uint32 sourceColor, uint32 duration, bool system) {
	_ready = false;
	_active = true;

	_red   = D3DCOLGetR(sourceColor);
	_green = D3DCOLGetG(sourceColor);
	_blue  = D3DCOLGetB(sourceColor);

	_sourceAlpha = D3DCOLGetA(sourceColor);
	_targetAlpha = 0;

	_duration = duration;
	_system = system;

	if (_system) _startTime = CBPlatform::getTime();
	else _startTime = Game->_timer;

	return S_OK;
}


//////////////////////////////////////////////////////////////////////////
HRESULT CBFader::fadeOut(uint32 targetColor, uint32 duration, bool system) {
	_ready = false;
	_active = true;

	_red   = D3DCOLGetR(targetColor);
	_green = D3DCOLGetG(targetColor);
	_blue  = D3DCOLGetB(targetColor);

	//_sourceAlpha = 0;
	_sourceAlpha = _currentAlpha;
	_targetAlpha = D3DCOLGetA(targetColor);

	_duration = duration;
	_system = system;

	if (_system) _startTime = CBPlatform::getTime();
	else _startTime = Game->_timer;


	return S_OK;
}


//////////////////////////////////////////////////////////////////////////
uint32 CBFader::getCurrentColor() {
	return DRGBA(_red, _green, _blue, _currentAlpha);
}



//////////////////////////////////////////////////////////////////////////
HRESULT CBFader::persist(CBPersistMgr *persistMgr) {
	CBObject::persist(persistMgr);

	persistMgr->transfer(TMEMBER(_active));
	persistMgr->transfer(TMEMBER(_blue));
	persistMgr->transfer(TMEMBER(_currentAlpha));
	persistMgr->transfer(TMEMBER(_duration));
	persistMgr->transfer(TMEMBER(_green));
	persistMgr->transfer(TMEMBER(_red));
	persistMgr->transfer(TMEMBER(_sourceAlpha));
	persistMgr->transfer(TMEMBER(_startTime));
	persistMgr->transfer(TMEMBER(_targetAlpha));
	persistMgr->transfer(TMEMBER(_system));

	if (_system && !persistMgr->_saving) _startTime = 0;

	return S_OK;
}

} // end of namespace WinterMute
