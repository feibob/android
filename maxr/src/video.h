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

#ifndef videoH
#define videoH

#include <SDL.h>
#include <string>


// Screenbuffers //////////////////////////////////////////////////////////////
EX SDL_Surface* screen; // Der Bildschirm
EX SDL_Surface* buffer; // Der Bildschirm-Buffer


struct sVidMode {
        unsigned int width;
	unsigned int height;
	unsigned int mode;
};

struct sVidData {
	unsigned int width;
	unsigned int height;
	unsigned int mode;
	Uint32 iSurfaceType;
	int iColDepth;
	bool bWindowMode;
};

/**
 * cVideo class. Stores videosettings (and can hopefully operate 'em too one day'):-)
 *
 * @author Bernd "beko" Kosmahl
 */
class cVideo
{
  public:
  /**
  *Sets whether app should appear windowed or in fullscreen mode
  *@param bWindowMode pass true if app should work in windowed mode<br>pass false it app should start in fullscreen
  *@param bApply set to true if app should apply new windowMode too
  *@return 0 on success
  */
  int setWindowMode(bool bWindowMode, bool bApply=false);

  /**
  *Get whether app should appear windowed or in fullscreen mode
  *@return  true if app should work in windowed mode<br>false it app should start in fullscreen
  */
  bool getWindowMode() const;

  /**
  *Set resolution/dimension of app window.
  *@param iWidth desired window width
  *@param iHeight desired window height
  *@param bApply  set to true if app should apply new resolution too
  *@return 0 on success
  */
  int setResolution(int iWidth, int iHeight, bool bApply=false);

  /**
  *@deprecated for compat only - will be removed!
  *@return stored window width
  */
  int getResolutionX() const;

  /**
  *@deprecated for compat only - will be removed!
  *@return stored window height
  */
  int getResolutionY() const;

  /**
  *Sets colordepth
  *@param iDepth colordepth to set. e.g. 32 (bpp)
  *@return 0 on success
  */
  int setColDepth(unsigned int iDepth);

  /**
  *Gets colordepth
  @return colordepth
  */
  int getColDepth() const;

  /**
  *Sets SurfaceType
  *@param iSurfaceType surfacetype to set. e.g. SDL_HWSURFACE
  */
  void setSurfaceType(Uint32 iSurfaceType);

  /**
  *Gets SurfaceType
  @return surfacetype e.g. SDL_HWSURFACE
  */
  Uint32 getSurfaceType() const;

  /**
  *@return Detected videomodes
  */
  int getVideoSize () const;

  /**
  *@param iMode video mode num from video mode array
  *@return Videomode as string widthxheight. If iMode is unknown minimal needed videomode will be returned.
  */
  std::string getVideoMode(unsigned int iMode) const;

  /**
  *Try to autodetect avail video modes from SDL. Might fail.
  *@return true on success
  */
  bool doDetection();

  /**
  *Check whether the provided mode is known to our video mode list
  *@param width Screenwidth to look for
  *@param height Screenheight to look for
  *@return iMode or -1 on unknown mode
  */
  int validateMode(unsigned int iWidth, unsigned int iHeight) const;

  /**
  *@return Splash width
  */
  int getSplashW() const;

  /**
  *@return Splash height
  */
  int getSplashH() const;

  /**
  *@return Minimal needed screen resolution width
  */
  int getMinW() const;

  /**
  *@return Minimal needed screen resolution height
  */
  int getMinH() const;

  /**
  *Inits our buffers and draws the splashscreen
  */
  void initSplash();

  /**
  *clears buffer (black)
  */
  void clearBuffer(void);

  /**
  *Blits or updates buffer to screen depending on windowmode
  */
  void draw(void);

  private:

  /** Slashscreen width  */
  #define SPLASHWIDTH 500
  /** Slashscreen height  */
  #define SPLASHHEIGHT 420
  #define COLOURDEPTH 32
  /**Minimum video mode resultion we need */
  #define MINWIDTH 640
  #define MINHEIGHT 480
  /**
  *@return Length of videomodes array
  */
  int getVideoNum (void) const;

  /**
  * Just for readable log entries
  *@param iSurfaceType surface type SDL_HWSURFACE or SDL_SWSURFACE
  *@return Surface type as string e.g."SDL_HWSURFACE"
  */
  std::string getSurfaceName (Uint32 iSurfaceType);

  /**
  *Switch SDL_HWSURFACE with SDL_SWSURFACE and vice versa
  */
  void switchSurface (void);

  /**
  *Checks whether our minimal needed videomode has been autodetected
  *@return true if mininal video mode looks valid
  */
  bool bHaveMinMode(void);

  /**
  *Applys current video settings
  *@param bNoFrame set to true to start without window frame (e.g. for splashscreen)
  *@return 0 on success
  */
  int applySettings(void);
};

extern cVideo Video;

#endif
