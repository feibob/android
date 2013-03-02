/***************************************************************************
 *      Mechanized Assault and Exploration Reloaded Projectfile            *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.             *
 ***************************************************************************/
#ifndef definesH
#define definesH
#include "settings.h"

#define GRID_COLOR			0x305C04	// color of the grid
#define SCAN_COLOR			0xE3E300	// color of scan circles
#define RANGE_GROUND_COLOR	0xE20000	// color of range circles for ground attack
#define RANGE_AIR_COLOR		0xFCA800	// color of range circles for air attack
#define PFEIL_COLOR			0x00FF00	// color of a waypointarrow
#define PFEILS_COLOR		0x0000FF	// color of a special waypointarrow
#define MOVE_SPEED			7			// speed of vehcilemovements
#define MSG_TICKS			30000		// number of ticks for how long a message will be displayed
#define ANIMATION_SPEED		((int)(Client->iTimerTime/(2)))		// this means every 100ms because Client->iTimerTime will increase every 50ms.
#define LANDING_DISTANCE_WARNING	28
#define LANDING_DISTANCE_TOO_CLOSE	10
#define MAX_PLANES_PER_FIELD		5

//minimap configuration
#define MINIMAP_COLOR		0xFC0000 //color of the screen borders on the minimap
#define MINIMAP_POS_X		15		 //the position of the map on the screen
#define MINIMAP_POS_Y		356		 //the position of the map on the screen
#define MINIMAP_SIZE		112		 //the size of the minimap in pixels
#define MINIMAP_ZOOM_FACTOR	2		 //the zoomfactor for minimap zoom switch

// Colors /////////////////////////////////////////////////////////////////////
#define cl_red 0
#define cl_blue 1
#define cl_green 2
#define cl_grey 3
#define cl_orange 4
#define cl_yellow 5
#define cl_purple 6
#define cl_aqua 7

#if HAVE_CONFIG_H
        #include "config.h" //created by autotools on linux holding informations like package_string and versions
#endif
#if HAVE_AUTOVERSION_H
	#include "autoversion.h" //include autoversion created by buildinfo.sh for svn and machine info
#endif

#ifdef __main__
#define EX
#else
#define EX extern
#endif

#define MAXPLAYER_HOTSEAT 8

#ifdef _MSC_VER
	#define CHECK_MEMORY //_ASSERTE( _CrtCheckMemory( ) );
#else
	#define CHECK_MEMORY
#endif

#endif

//some defines for typical menus


#ifndef PATH_DELIMITER
#	ifdef WIN32
#		define PATH_DELIMITER "\\"
#	else
#		define PATH_DELIMITER "/"
#	endif
#endif

#ifndef TEXT_FILE_LF
#	ifdef WIN32
#		define TEXT_FILE_LF "\r\n"
#	else
#		define TEXT_FILE_LF "\n"
#	endif
#endif

// GFX On Demand /////////////////////////////////////////////////////////////
#define GFXOD_MAIN				(cSettings::getInstance().getGfxPath() + PATH_DELIMITER "main.pcx").c_str()
#define GFXOD_HELP				(cSettings::getInstance().getGfxPath() + PATH_DELIMITER "help_screen.pcx").c_str()
#define GFXOD_OPTIONS			(cSettings::getInstance().getGfxPath() + PATH_DELIMITER "options.pcx").c_str()
#define GFXOD_SAVELOAD			(cSettings::getInstance().getGfxPath() + PATH_DELIMITER "load_save_menu.pcx").c_str()
#define GFXOD_PLANET_SELECT		(cSettings::getInstance().getGfxPath() + PATH_DELIMITER "planet_select.pcx").c_str()
#define GFXOD_CLAN_SELECT		(cSettings::getInstance().getGfxPath() + PATH_DELIMITER "clanselection.pcx").c_str()
#define GFXOD_PLAYER_SELECT		(cSettings::getInstance().getGfxPath() + PATH_DELIMITER "customgame_menu.pcx").c_str()
#define GFXOD_PLAYERHS_SELECT	(cSettings::getInstance().getGfxPath() + PATH_DELIMITER "hotseatplayers.pcx").c_str()
#define GFXOD_HANGAR			(cSettings::getInstance().getGfxPath() + PATH_DELIMITER "hangar.pcx").c_str()
#define GFXOD_BUILD_SCREEN		(cSettings::getInstance().getGfxPath() + PATH_DELIMITER "build_screen.pcx").c_str()
#define GFXOD_FAC_BUILD_SCREEN	(cSettings::getInstance().getGfxPath() + PATH_DELIMITER "fac_build_screen.pcx").c_str()
#define GFXOD_MULT				(cSettings::getInstance().getGfxPath() + PATH_DELIMITER "multi.pcx").c_str()
#define GFXOD_UPGRADE			(cSettings::getInstance().getGfxPath() + PATH_DELIMITER "upgrade.pcx").c_str()
#define GFXOD_STORAGE			(cSettings::getInstance().getGfxPath() + PATH_DELIMITER "storage.pcx").c_str()
#define GFXOD_STORAGE_GROUND	(cSettings::getInstance().getGfxPath() + PATH_DELIMITER "storage_ground.pcx").c_str()
#define GFXOD_MULT				(cSettings::getInstance().getGfxPath() + PATH_DELIMITER "multi.pcx").c_str()
#define GFXOD_MINEMANAGER		(cSettings::getInstance().getGfxPath() + PATH_DELIMITER "mine_manager.pcx").c_str()
#define GFXOD_REPORTS			(cSettings::getInstance().getGfxPath() + PATH_DELIMITER "reports.pcx").c_str()
#define GFXOD_DIALOG2			(cSettings::getInstance().getGfxPath() + PATH_DELIMITER "dialog2.pcx").c_str()
#define GFXOD_DIALOG4			(cSettings::getInstance().getGfxPath() + PATH_DELIMITER "dialog4.pcx").c_str()
#define GFXOD_DIALOG5			(cSettings::getInstance().getGfxPath() + PATH_DELIMITER "dialog5.pcx").c_str()
#define GFXOD_DIALOG6			(cSettings::getInstance().getGfxPath() + PATH_DELIMITER "dialog6.pcx").c_str()
#define GFXOD_DIALOG_TRANSFER	(cSettings::getInstance().getGfxPath() + PATH_DELIMITER "transfer.pcx").c_str()
#define GFXOD_DIALOG_RESEARCH	(cSettings::getInstance().getGfxPath() + PATH_DELIMITER "research.pcx").c_str()
#define GFXOD_DESTRUCTION		(cSettings::getInstance().getGfxPath() + PATH_DELIMITER "destruction.pcx").c_str()

// Other Resources /////////////////////////////////////////////////////////////
#define PLAYERCOLORS		8
//^-- make sure that given amount of colors is loaded too

#define DEFAULTPORT		58600
#define MAX_XML			"max.xml"
#define MAX_LOG			"maxr.log"
#define MAX_NET_LOG		"net.log"
#define CLANS_XML		(cSettings::getInstance().getDataDir() + "clans.xml").c_str()
#define KEYS_XML		(cSettings::getInstance().getDataDir() + "keys.xml").c_str()
#define SPLASH_BACKGROUND	(cSettings::getInstance().getDataDir() + "init.pcx").c_str()
#ifdef MAC
	#define MAXR_ICON             (cSettings::getInstance().getDataDir() + "maxr_mac.bmp").c_str()
#else
	#define MAXR_ICON             (cSettings::getInstance().getDataDir() + "maxr.bmp").c_str()
#endif


#if HAVE_AUTOVERSION_H
	//define nothing on linux - comes all from autoversion.h generated by buildinfo.sh
#else // We have no autoversion => take care of these manually!
	//default path to data dir only used on linux/other
	#define BUILD_DATADIR "/usr/share/maxr"
	// Builddate: Mmm DD YYYY HH:MM:SS
	#define MAX_BUILD_DATE		(std::string)__DATE__ + " " + __TIME__
	#ifdef RELEASE
		#define PACKAGE_REV "Releaseversion"
	#else
		#define PACKAGE_REV "SVN Rev 2937"
	#endif
#endif

#if HAVE_CONFIG_H
	//define nothing on linux - comes all from config.h generated by autotools
#else	//We have no config.h => take care of these manually
	#define PACKAGE_VERSION     "0.2.8"
	#define PACKAGE_NAME  "M.A.X.R."

#endif

#if DEDICATED_SERVER_APPLICATION
	#define DEDICATED_SERVER true
#else
	#define DEDICATED_SERVER false
#endif
