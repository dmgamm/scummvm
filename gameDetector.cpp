/* ScummVM - Scumm Interpreter
 * Copyright (C) 2001  Ludvig Strigeus
 * Copyright (C) 2001/2002 The ScummVM project
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
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 *
 * $Header$
 *
 */


#include "stdafx.h"
#include "scumm.h"
#include "gameDetector.h"



static const char USAGE_STRING[] = 
	"ScummVM - Scumm Interpreter\n"
	"Syntax:\n"
  "\tscummvm [-v] [-d] [-n] [-b<num>] [-t<num>] [-s<num>] [-p<path>] [-m<num>] [-f] game\n"
	"Flags:\n"
	"\tv       - show version info and exit\n"
	"\tc<num>  - use cdrom <num> for cd audio\n"
	"\td       - enable debug output\n"
	"\tn       - no subtitles for speech\n"
	"\tb<num>  - start in room <num>\n"
	"\tt<num>  - set music tempo. Suggested: 1F0000\n"
	"\ts<num>  - set scale factor to <num> (1, 2, or 3 - 2 by default)\n"
	"\tp<path> - look for game in <path>\n"
	"\tm<num>  - set music volume to <num> (0-100)\n"
	"\te<num>  - set music engine. see readme.txt for details\n"
	"\tr       - emulate roland mt32 instruments\n"
	"\tf       - fullscreen mode\n"
	"\tg       - graphics mode. 1 for 2xSai anti-aliasing\n"
	"\ta       - load autosave game (for recovering from crashes)\n"
;

void GameDetector::parseCommandLine(int argc, char **argv)
{
#if !defined(__APPLE__CW)
	int i;
	char *s;

	// check for arguments
	if (argc < 2) {
		printf(USAGE_STRING);
		//exit(1);
	}

	/* Parse the arguments */
	for (i = 1; i < argc; i++) {
		s = argv[i];

		if (s && s[0] == '-') {
			s++;
			while (*s) {
				switch (tolower(*s)) {
				case 'a':
					_restore = true;
					break;
				case 'b':
					if (*(s + 1) == '\0')
						goto ShowHelpAndExit;
					_bootParam = atoi(s + 1);
					goto NextArg;
				case 'f':
					_fullScreen = true;
					break;
				case 'd':
					_debugMode = true;
					break;
				case 'n':
					_noSubtitles = true;
					break;
				case 's':
					if (*(s + 1) == '\0')
						goto ShowHelpAndExit;
					_scale = atoi(s + 1);
					if (_scale == 0 || _scale > 3) {
						// bad scale - only 1, 2, 3 work for now
						printf("Invalid scale '%s' - valid values are 1, 2, 3\n", s + 1);
						exit(1);
					}
					goto NextArg;
				case 'v':
					printf("ScummVM " SCUMMVM_VERSION "\nBuilt on " __DATE__ " "
								 __TIME__ "\n");
#ifdef SCUMMVM_PLATFORM_VERSION
					printf("    " SCUMMVM_PLATFORM_VERSION "\n");
#endif
					exit(1);
				case 'p':
					if (*(s + 1) == '\0')
						goto ShowHelpAndExit;
					_gameDataPath = s + 1;
					goto NextArg;
				case 't':
					if (*(s + 1) == '\0')
						goto ShowHelpAndExit;
					_gameTempo = atoi(s + 1);
					goto NextArg;
				case 'm':{
						if (*(s + 1) == '\0')
							goto ShowHelpAndExit;
						SoundEngine *se = (SoundEngine *)_soundEngine;

						if (se)
							se->set_music_volume(atoi(s + 1));
						goto NextArg;
					}
				case 'r':{
						SoundEngine *se = (SoundEngine *)_soundEngine;

						if (se)
							se->_mt32emulate = true;
						break;
					}
				case 'e':
					if (*(s + 1) == '\0')
						goto ShowHelpAndExit;
					_midi_driver = atoi(s + 1);
					goto NextArg;
				case 'g':
					if (*(s + 1) == '\0')
						goto ShowHelpAndExit;
					_videoMode = atoi(s + 1);
					goto NextArg;

				case 'c':
					if (*(s + 1) == '\0')
						goto ShowHelpAndExit;
					_cdrom = atoi(s + 1);
					goto NextArg;

				default:
				ShowHelpAndExit:;
					printf(USAGE_STRING);
					exit(1);
				}
				s++;
			}
		NextArg:;
		} else {
			if (_exe_name)
				goto ShowHelpAndExit;
			_exe_name = s;
		}
	}

#else
	_midi_driver = 4;
	_exe_name = *argv;
	_gameDataPath = (char *)malloc(strlen(_exe_name) + 3);
	sprintf(_gameDataPath, ":%s:", _exe_name);
#endif

}

struct VersionSettings {
	const char *filename;
	const char *gamename;
	byte id, major, middle, minor;
	uint32 features;
};

/*
        This is a list of all known SCUMM games. Commented games are not
        supported at this time */

static const VersionSettings version_settings[] = {
	/* Scumm Version 1 */
//      {"maniac",      "Maniac Mansion (C64)",                         GID_MANIAC64, 1, 0, 0,},
//      {"zak",         "Zak McKracken and the Alien Mindbenders (C64)", GID_ZAK64, 1, 0, 0,},

	/* Scumm Version 2 */
//      {"maniac",      "Maniac Mansion", GID_MANIAC, 2, 0, 0,},
//      {"zak",         "Zak McKracken and the Alien Mindbenders",      GID_ZAK,     2, 0, 0,},
//      {"indy3",       "Indiana Jones and the Last Crusade",           GID_INDY3,   2, 0, 0,},

	/* Scumm Version 3 */
	{"indy3", "Indiana Jones and the Last Crusade (256)", GID_INDY3_256, 3, 0,
	 22,
	 GF_SMALL_HEADER | GF_USE_KEY | GF_SMALL_NAMES | GF_OLD256 |
	 GF_NO_SCALLING},
	{"zak256", "Zak McKracken and the Alien Mindbenders (256)", GID_ZAK256, 3,
	 0, 0,
	 GF_SMALL_HEADER | GF_USE_KEY | GF_SMALL_NAMES | GF_OLD256 | GF_AUDIOTRACKS
	 | GF_NO_SCALLING},
	{"loom", "Loom", GID_LOOM, 3, 5, 40,
	 GF_SMALL_HEADER | GF_USE_KEY | GF_SMALL_NAMES | GF_OLD_BUNDLE | GF_16COLOR
	 | GF_NO_SCALLING},

	/* Scumm Version 4 */
	{"monkeyEGA", "Monkey Island 1 (EGA)", GID_MONKEY_EGA, 4, 0, 67, GF_SMALL_HEADER | GF_USE_KEY | GF_16COLOR},	// EGA version

	/* Scumm version 5 */
	{"loomcd", "Loom (256 color CD version)", GID_LOOM256, 5, 1, 42,
	 GF_SMALL_HEADER | GF_USE_KEY | GF_AUDIOTRACKS},
	{"monkey", "Monkey Island 1", GID_MONKEY, 5, 2, 2,
	 GF_USE_KEY | GF_AUDIOTRACKS},
	{"monkey1", "Monkey Island 1 (alt)", GID_MONKEY, 5, 2, 2,
	 GF_USE_KEY | GF_AUDIOTRACKS},
	{"monkey2", "Monkey Island 2: LeChuck's revenge", GID_MONKEY2, 5, 2, 2,
	 GF_USE_KEY},
	{"atlantis", "Indiana Jones 4 and the Fate of Atlantis", GID_INDY4, 5, 5, 0,
	 GF_USE_KEY},
	{"playfate", "Indiana Jones 4 and the Fate of Atlantis (Demo)", GID_INDY4,
	 5, 5, 0, GF_USE_KEY},

	/* Scumm Version 6 */
	{"tentacle", "Day Of The Tentacle", GID_TENTACLE, 6, 4, 2,
	 GF_NEW_OPCODES | GF_AFTER_V6 | GF_USE_KEY},
	{"dottdemo", "Day Of The Tentacle (Demo)", GID_TENTACLE, 6, 3, 2,
	 GF_NEW_OPCODES | GF_AFTER_V6 | GF_USE_KEY},
	{"samnmax", "Sam & Max", GID_SAMNMAX, 6, 4, 2,
	 GF_NEW_OPCODES | GF_AFTER_V6 | GF_USE_KEY | GF_DRAWOBJ_OTHER_ORDER},
	{"snmdemo", "Sam & Max (Demo)", GID_SAMNMAX, 6, 3, 0,
	 GF_NEW_OPCODES | GF_AFTER_V6 | GF_USE_KEY},

	/* Scumm Version 7 */
	{"ft", "Full Throttle", GID_FT, 7, 3, 0,
	 GF_NEW_OPCODES | GF_AFTER_V6 | GF_AFTER_V7},
	{"dig", "The Dig", GID_DIG, 7, 5, 0,
	 GF_NEW_OPCODES | GF_AFTER_V6 | GF_AFTER_V7},

	/* Simon the Sorcerer 1 & 2 (not SCUMM games) */
	{"simon1dos", "Simon the Sorcerer 1 for DOS", GID_SIMON_FIRST+1, 99, 99, 99, 0},
	{"simon1win", "Simon the Sorcerer 1 for Windows", GID_SIMON_FIRST+2, 99, 99, 99, 0},
	{"simon2win", "Simon the Sorcerer 2 for Windows", GID_SIMON_FIRST+3, 99, 99, 99, 0},
	
	/* Scumm Version 8 */
//    {"curse",       "The Curse of Monkey Island",                   GID_CMI,      8, 1, 0,},
	{NULL, NULL}
};

bool GameDetector::detectGame()
{
	const VersionSettings *gnl = version_settings;

	_gameId = 0;
	_gameText = NULL;
	do {
		if (!scumm_stricmp(_exe_name, gnl->filename)) {
			_gameId = gnl->id;
			_scummVersion = gnl->major;

			_features = gnl->features;
			_gameText = gnl->gamename;
			debug(1, "Detected game '%s', version %d.%d.%d",
						gnl->gamename, gnl->major, gnl->middle, gnl->minor);
			return true;
		}
	} while ((++gnl)->filename);

	debug(1, "Failed game detection");

	return true;
}

char *GameDetector::getGameName()
{
	if (_gameText == NULL) {
		char buf[256];
		sprintf(buf, "Unknown game: \"%s\"", _exe_name);
		return strdup(buf);
	}
	return strdup(_gameText);
}

int GameDetector::detectMain(int argc, char **argv)
{
	_debugMode = 0;								// off by default...

	_noSubtitles = 0;							// use by default - should this depend on soundtrack?        
	_scale = 2;										// double size by default

	_gameDataPath = NULL;
	_gameTempo = 0;
	_videoMode = 0;
	_soundCardType = 3;

#ifdef WIN32
	_midi_driver = MIDI_WINDOWS;
#else
	_midi_driver = MIDI_NULL;
#endif

	parseCommandLine(argc, argv);

	if (_exe_name == NULL) {
		//launcherLoop();
		//setWindowName(this);
		warning("No game was specified...");
		return (-1);
	}


	if (!detectGame()) {
		warning("Game detection failed. Using default settings");
		_features = GF_DEFAULT;
		_gameText = "Please choose a game";
	}

	if (!_gameDataPath) {
		warning
			("No path was provided. Assuming that data file are in the current directory");
		_gameDataPath = (char *)malloc(sizeof(char) * 2);
		strcpy(_gameDataPath, "");
	}

	return (0);
}
