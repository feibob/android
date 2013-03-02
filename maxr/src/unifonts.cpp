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
#include "autosurface.h"
#include "unifonts.h"
#include "pcx.h"
#include "files.h"
#include "log.h"
#include "settings.h"
#include "main.h"

using namespace std;

cUnicodeFont::cUnicodeFont()
{
	// load all existing fonts. If there will be added some, they have also to be added here!
	loadChars ( CHARSET_ISO8559_ALL, FONT_LATIN_NORMAL );
	loadChars ( CHARSET_ISO8559_1, FONT_LATIN_NORMAL );
	loadChars ( CHARSET_ISO8559_2, FONT_LATIN_NORMAL );
	loadChars ( CHARSET_ISO8559_5, FONT_LATIN_NORMAL );

	loadChars ( CHARSET_ISO8559_ALL, FONT_LATIN_BIG );
	loadChars ( CHARSET_ISO8559_1, FONT_LATIN_BIG );
	loadChars ( CHARSET_ISO8559_2, FONT_LATIN_BIG );
	loadChars ( CHARSET_ISO8559_5, FONT_LATIN_BIG );

	loadChars ( CHARSET_ISO8559_ALL, FONT_LATIN_BIG_GOLD );
	loadChars ( CHARSET_ISO8559_1, FONT_LATIN_BIG_GOLD );
	loadChars ( CHARSET_ISO8559_2, FONT_LATIN_BIG_GOLD );

	loadChars ( CHARSET_ISO8559_ALL, FONT_LATIN_SMALL_WHITE );
	loadChars ( CHARSET_ISO8559_1, FONT_LATIN_SMALL_WHITE );
	loadChars ( CHARSET_ISO8559_2, FONT_LATIN_SMALL_WHITE );
	loadChars ( CHARSET_ISO8559_5, FONT_LATIN_SMALL_WHITE );

	loadChars ( CHARSET_ISO8559_ALL, FONT_LATIN_SMALL_RED );
	loadChars ( CHARSET_ISO8559_1, FONT_LATIN_SMALL_RED );
	loadChars ( CHARSET_ISO8559_2, FONT_LATIN_SMALL_RED );
	loadChars ( CHARSET_ISO8559_5, FONT_LATIN_SMALL_RED );

	loadChars ( CHARSET_ISO8559_ALL, FONT_LATIN_SMALL_GREEN );
	loadChars ( CHARSET_ISO8559_1, FONT_LATIN_SMALL_GREEN );
	loadChars ( CHARSET_ISO8559_2, FONT_LATIN_SMALL_GREEN );
	loadChars ( CHARSET_ISO8559_5, FONT_LATIN_SMALL_GREEN );

	loadChars ( CHARSET_ISO8559_ALL, FONT_LATIN_SMALL_YELLOW );
	loadChars ( CHARSET_ISO8559_1, FONT_LATIN_SMALL_YELLOW );
	loadChars ( CHARSET_ISO8559_2, FONT_LATIN_SMALL_YELLOW );
	loadChars ( CHARSET_ISO8559_5, FONT_LATIN_SMALL_YELLOW );
}

void cUnicodeFont::loadChars( eUnicodeFontCharset charset, eUnicodeFontType fonttype )
{
	const unsigned short *iso8859_to_uni;
	int currentChar, highcount;
	int cellW, cellH;
	int pX, pY;

	AutoSurface surface(loadCharsetSurface(charset, fonttype));
	if ( !surface )
	{
		// LOG: error while loading font
		return;
	}
	AutoSurface* const chars = getFontTypeSurfaces(fonttype);
	if ( !chars )
	{
		// LOG: error while loading font
		return;
	}
	iso8859_to_uni = getIsoPage ( charset );

	if ( charset == CHARSET_ISO8559_ALL ) highcount = 16;
	else highcount = 6;

	cellW = surface->w / 16;
	cellH = surface->h / highcount;
	currentChar = 0;
	pX = 0;
	pY = 0;

	for( int rows = 0; rows < highcount; rows ++)
	{
		//go through the cols
		for( int cols = 0; cols < 16; cols ++)
		{
			SDL_Rect Rect;
			Rect.x = cellW * cols; //write each cell position and size into array
			Rect.y = cellH * rows;
			Rect.h = cellH;
			Rect.w = cellW;

			//go through pixels to find offset x
			for( int pCol = 0; pCol < cellH; pCol++)
			{
				for( int pRow = 0; pRow < cellH; pRow++)
				{
					pX = (cellW * cols) + pCol;
					pY = (cellH * rows) + pRow;

					if( getPixel32(pX, pY, surface) != SDL_MapRGB(surface->format, 0xFF, 0, 0xFF) )
					{
						//offset
						Rect.x = pX;
						pCol = cellW; //break loop
						pRow = cellH;
					}
				}
			}
			//go through pixel to find offset w
			for( int pCol_w = cellW - 1; pCol_w >= 0; pCol_w--)
			{
				for ( int pRow_w = 0; pRow_w < cellH; pRow_w++)
				{
					pX = (cellW * cols ) + pCol_w;
					pY = (cellH * rows ) + pRow_w;

					if( getPixel32(pX, pY, surface) != SDL_MapRGB(surface->format, 0xFF, 0, 0xFF) )
					{
						Rect.w = (pX - Rect.x) +1;
						pCol_w = -1; //break loop
						pRow_w = cellH;
					}

				}
			}

			// get the unicode place of the character
			int unicodeplace = 0;
			if ( iso8859_to_uni == NULL )
			{
				if ( charset == CHARSET_ISO8559_ALL ) unicodeplace = currentChar;
				else if ( charset == CHARSET_ISO8559_1 ) unicodeplace = currentChar + 128 + 2*16;
			}
			else unicodeplace = iso8859_to_uni[currentChar];
			chars[unicodeplace] = SDL_CreateRGBSurface ( Video.getSurfaceType()|SDL_SRCCOLORKEY,Rect.w,Rect.h,32,0,0,0,0 );

			// change color of smal fonts
			switch ( fonttype )
			{
			case FONT_LATIN_SMALL_RED:
				SDL_SetColorKey ( surface, SDL_SRCCOLORKEY, 0xf0d8b8 );
				SDL_FillRect ( chars[unicodeplace], NULL, 0xe60000 );
				break;
			case FONT_LATIN_SMALL_GREEN:
				SDL_SetColorKey ( surface, SDL_SRCCOLORKEY, 0xf0d8b8 );
				SDL_FillRect ( chars[unicodeplace], NULL, 0x04ae04 );
				break;
			case FONT_LATIN_SMALL_YELLOW:
				SDL_SetColorKey ( surface, SDL_SRCCOLORKEY, 0xf0d8b8 );
				SDL_FillRect ( chars[unicodeplace], NULL, 0xdbde00 );
				break;
			default:
				SDL_FillRect ( chars[unicodeplace], NULL, 0xFF00FF );
				break;
			}
			SDL_BlitSurface ( surface, &Rect, chars[unicodeplace], NULL );
			SDL_SetColorKey ( chars[unicodeplace], SDL_SRCCOLORKEY, 0xFF00FF );

			//goto next character
			currentChar++;
		}
	}
}

Uint32 cUnicodeFont::getPixel32(int x, int y, SDL_Surface *surface)
{
	//converts the Pixel to 32 bit
	Uint32 *pixels = (Uint32 *) surface->pixels;

	//get the requested pixels
	return pixels[(y*surface->w)+x];
}

AutoSurface* cUnicodeFont::getFontTypeSurfaces(eUnicodeFontType const fonttype)
{
	switch ( fonttype )
	{
	case FONT_LATIN_NORMAL:
		return charsNormal;
	case FONT_LATIN_BIG:
		return charsBig;
	case FONT_LATIN_BIG_GOLD:
		return charsBigGold;
	case FONT_LATIN_SMALL_WHITE:
		return charsSmallWhite;
	case FONT_LATIN_SMALL_RED:
		return charsSmallRed;
	case FONT_LATIN_SMALL_GREEN:
		return charsSmallGreen;
	case FONT_LATIN_SMALL_YELLOW:
		return charsSmallYellow;
	}
	return NULL;
}

SDL_Surface *cUnicodeFont::loadCharsetSurface( eUnicodeFontCharset charset, eUnicodeFontType fonttype )
{
	// build the filename from the information
	string filename = cSettings::getInstance().getFontPath() + PATH_DELIMITER + "latin_";
	switch ( fonttype )
	{
	case FONT_LATIN_NORMAL:
		filename += "normal";
		break;
	case FONT_LATIN_BIG:
		filename += "big";
		break;
	case FONT_LATIN_BIG_GOLD:
		filename += "big_gold";
		break;
	case FONT_LATIN_SMALL_WHITE:
	case FONT_LATIN_SMALL_RED:
	case FONT_LATIN_SMALL_GREEN:
	case FONT_LATIN_SMALL_YELLOW:
		filename += "small";
		break;
	}
	if ( charset != CHARSET_ISO8559_ALL )
	{
		filename += "_iso-8559-";
		// it's important that the enum-numbers are the same as theire iso-numbers!
		filename += iToStr ( charset );
	}
	filename += ".pcx";

	// load the bitmap
	if ( FileExists( filename.c_str() ) ) return LoadPCX ( filename.c_str() );
	else return NULL;
}

const unsigned short *cUnicodeFont::getIsoPage ( eUnicodeFontCharset charset )
{
	switch ( charset )
	{
	case CHARSET_ISO8559_ALL:
		return NULL;
	case CHARSET_ISO8559_1:
		return NULL;
	case CHARSET_ISO8559_2:
		return iso8859_2_2uni;
	case CHARSET_ISO8559_3:
		return iso8859_3_2uni;
	case CHARSET_ISO8559_4:
		return iso8859_4_2uni;
	case CHARSET_ISO8559_5:
		return iso8859_5_2uni;
	case CHARSET_ISO8559_6:
		return iso8859_6_2uni;
	case CHARSET_ISO8559_7:
		return iso8859_7_2uni;
	case CHARSET_ISO8559_8:
		return iso8859_8_2uni;
	case CHARSET_ISO8559_9:
		return iso8859_9_2uni;
	case CHARSET_ISO8559_10:
		return iso8859_10_2uni;
	case CHARSET_ISO8559_11:
		return NULL;
	case CHARSET_ISO8559_13:
		return iso8859_13_2uni;
	case CHARSET_ISO8559_14:
		return iso8859_14_2uni;
	case CHARSET_ISO8559_15:
		return iso8859_15_2uni;
	case CHARSET_ISO8559_16:
		return iso8859_16_2uni;
	default:
		//LOG: unknown iso format
		break;
	}
	return NULL;
}

void cUnicodeFont::showText( SDL_Rect rDest, string sText, eUnicodeFontType fonttype, SDL_Surface *surface, bool encode )
{
	showText ( rDest.x, rDest.y, sText, fonttype, surface, encode );
}

void cUnicodeFont::showText( int x, int y, string sText, eUnicodeFontType fonttype, SDL_Surface *surface, bool encode )
{
	int offX = x;
	int offY = y;
	int iSpace = 0;
	AutoSurface* const chars = getFontTypeSurfaces(fonttype);

	//make sure only upper characters are read for the small fonts
	// since we don't support lower chars on the small fonts
	switch( fonttype )
	{
		case FONT_LATIN_SMALL_GREEN:
		case FONT_LATIN_SMALL_RED:
		case FONT_LATIN_SMALL_WHITE:
		case FONT_LATIN_SMALL_YELLOW:
			for( size_t i=0; i < sText.size(); i++ ) sText[i] = toupper(sText[i]);
			iSpace = 1;
			break;
		case FONT_LATIN_NORMAL:
		case FONT_LATIN_BIG:
		case FONT_LATIN_BIG_GOLD:
			break;
	}

	// decode the UTF-8 String:
	const char *p = sText.c_str();
	const char *now = &p[sText.length()];
	while ( p < now )
	{
		//is space?
		if( *p == ' ' )
		{
			if ( chars[97] ) offX += chars[97]->w;
			p++;
		} //is new line?
		else if( *p == '\n' )
		{
			offY += getFontHeight ( fonttype );
			offX = x;
			p++;
		}
		else if( 13 == *(const unsigned char *)p )
		{
			p++;
			//ignore - is breakline in file
		}
		else
		{
			Uint16 uni;
			if ( encode )
			{
				int increase;
				uni = encodeUTF8Char ( (unsigned char *)p, &increase );
				p += increase;
			}
			else
			{
				uni = *(unsigned char *)p;
				p++;
			}
			if ( chars[uni] != NULL )
			{
				SDL_Rect rTmp = {offX, offY, 16, 16};
				SDL_BlitSurface( chars[uni], NULL, surface, &rTmp);

				//move one px forward for space between signs
				offX += chars[uni]->w + iSpace;
			}
		}
	}
}

int cUnicodeFont::drawWithBreakLines( SDL_Rect rDest, string sText, eUnicodeFontType fonttype, SDL_Surface *surface, bool encode )
{
	string drawString = "";

	while ( getTextWide ( sText, fonttype, encode ) > rDest.w )
	{
		// get the position of the end of as many words from the text as fit in rDest.w
		size_t pos = 0, lastPos = 0;
		do
		{
			lastPos = pos;
			pos = sText.find ( " ", pos+1 );
		}
		while ( getTextWide ( sText.substr ( 0, pos ), fonttype, encode ) < rDest.w && pos != string::npos );

		// get the words. If there was no " " in the text we get the hole text string
		if ( lastPos != 0 ) drawString = sText.substr ( 0, lastPos );
		else drawString = sText;

		// if there is only one word in the string it is possible that this word is to long.
		// we will check this, and cut it if necessary
		while ( getTextWide ( drawString, fonttype, encode ) > rDest.w )
		{
			string stringPart = drawString;

			// delete as many chars as it is needed to fit into the line
			while ( getTextWide ( stringPart, fonttype, encode ) + getTextWide ( "-", fonttype, encode ) > rDest.w ) stringPart.erase ( stringPart.length()-1, 1 );
			stringPart += "-";

			// show the part of the word
			showText ( rDest, stringPart, fonttype, surface, encode );
			rDest.y += getFontHeight ( fonttype );

			// erase the part from the line and from the hole text
			drawString.erase ( 0, stringPart.length()-1 );
			sText.erase ( 0, stringPart.length()-1 );
		}

		// draw the rest of the line
		showText ( rDest, drawString, fonttype, surface, encode );
		rDest.y += getFontHeight ( fonttype );

		sText.erase ( 0, drawString.length() );
		if ( lastPos != 0 ) sText.erase ( 0, 1 );
	}

	// draw the rest of the text
	showText ( rDest, sText, fonttype, surface, encode );
	rDest.y += getFontHeight ( fonttype );

	return rDest.y;
}

int cUnicodeFont::showTextAsBlock ( SDL_Rect rDest, string sText, eUnicodeFontType fonttype, SDL_Surface *surface, bool encode )
{
	string sTmp;

	size_t k;

	do
	{
		//erase all invalid formatted breaklines like we may get them from translation XMLs
		k = sText.find ( "\\n" );

		if ( k != string::npos )
		{
			sText.erase ( k, 2 );
			sText.insert (k, "\n");
		}
	}
	while ( k != string::npos );

	do
	{
		//erase all blanks > 2
		k = sText.find ( "  " ); //IMPORTANT: _two_ blanks! don't change this or this will become an endless loop

		if ( k != string::npos )
		{
			sText.erase ( k, 1 );
		}
	}
	while ( k != string::npos );

	do //support of linebreaks: snip text at linebreaks, do the auto linebreak for first part and proceed with second part
	{
		//search and replace \n since we want a blocktext - no manual breaklines allowed
		k = sText.find ( "\n" );

		if ( k != string::npos )
		{

			sTmp=sText;

			sText.erase ( 0, k+1 ); //delete everything before and including linebreak \n
			sTmp.erase (k, sTmp.size()); //delete everything after \n

			rDest.y = drawWithBreakLines(rDest, sTmp, fonttype, surface, encode); //draw first part of text and proceed searching for breaklines
			// += font->getFontHeight(eBitmapFontType); //add newline for each breakline
		}
	}
	while ( k != string::npos );

	return drawWithBreakLines(rDest, sText, fonttype, surface, encode); //draw rest of text
}

void cUnicodeFont::showTextCentered( SDL_Rect rDest, string sText, eUnicodeFontType fonttype, SDL_Surface *surface, bool encode )
{
	showTextCentered ( rDest.x, rDest.y, sText, fonttype, surface, encode );
}

void cUnicodeFont::showTextCentered( int x, int y, string sText, eUnicodeFontType fonttype, SDL_Surface *surface, bool encode )
{
	SDL_Rect rTmp = getTextSize ( sText, fonttype, encode );
	showText ( x - rTmp.w / 2, y, sText, fonttype, surface, encode );
}

int cUnicodeFont::getTextWide( string sText, eUnicodeFontType fonttype, bool encode )
{
	SDL_Rect rTmp = getTextSize( sText, fonttype, encode );
	return rTmp.w;
}

SDL_Rect cUnicodeFont::getTextSize( string sText, eUnicodeFontType fonttype, bool encode )
{
	int iSpace = 0;
	AutoSurface* const chars = getFontTypeSurfaces(fonttype);
	SDL_Rect rTmp = {0, 0, 0, 0};

	//make sure only upper characters are read for the small fonts
	// since we don't support lower chars on the small fonts
	switch( fonttype )
	{
		case FONT_LATIN_SMALL_GREEN:
		case FONT_LATIN_SMALL_RED:
		case FONT_LATIN_SMALL_WHITE:
		case FONT_LATIN_SMALL_YELLOW:
			for( size_t i=0; i < sText.size(); i++ ) sText[i] = toupper(sText[i]);
			iSpace = 1;
			break;
		case FONT_LATIN_NORMAL:
		case FONT_LATIN_BIG:
		case FONT_LATIN_BIG_GOLD:
			break;
	}

	// decode the UTF-8 String:
	const char *p = sText.c_str();
	const char *now = &p[sText.length()];
	while ( p < now )
	{
		//is space?
		if( *p == ' ' )
		{
			// we will use the wight of the 'a' for spaces
			if ( chars[97] ) rTmp.w += chars[97]->w;
			p++;
		} //is new line?
		else if( *p == '\n' )
		{
			rTmp.h += getFontHeight ( fonttype );
			p++;
		}
		else if( 13 == *(const unsigned char *)p )
		{
			p++;
			//ignore - is breakline in file
		}
		else
		{
			Uint16 uni;
			if ( encode )
			{
				int increase;
				uni = encodeUTF8Char ( (unsigned char *)p, &increase );
				p += increase;
			}
			else
			{
				uni = *(unsigned char *)p;
				p++;
			}
			if ( chars[uni] != NULL )
			{
				rTmp.w += chars[uni]->w + iSpace;
				rTmp.h = chars[uni]->h;
			}
		}
	}
	return rTmp;
}

int cUnicodeFont::getFontHeight( eUnicodeFontType fonttype )
{
	const AutoSurface* const chars = getFontTypeSurfaces(fonttype);
	// we will return the height of the first character in the list
	for ( int i = 0; i < 0xFFFF; i++ )
	{
		if ( chars[i] != NULL ) return chars[i]->h;
	}
	return 0;
}

eUnicodeFontSize cUnicodeFont::getFontSize ( eUnicodeFontType fonttype ) const
{
	switch ( fonttype )
	{
	default:
	case FONT_LATIN_NORMAL:
		return FONT_SIZE_NORMAL;
	case FONT_LATIN_BIG:
	case FONT_LATIN_BIG_GOLD:
		return FONT_SIZE_BIG;
	case FONT_LATIN_SMALL_WHITE:
	case FONT_LATIN_SMALL_RED:
	case FONT_LATIN_SMALL_GREEN:
	case FONT_LATIN_SMALL_YELLOW:
		return FONT_SIZE_SMALL;
	}
}


string cUnicodeFont::shortenStringToSize ( string str, int size, eUnicodeFontType fonttype )
{
	if ( font->getTextWide ( str, fonttype ) > size )
	{
		while ( font->getTextWide ( str + "." ) > size )
		{
			str.erase ( str.length()-1, str.length() );
		}
		str += ".";
	}
	return str;
}

Uint16 cUnicodeFont::encodeUTF8Char ( const unsigned char *pch, int *increase ) const
{
	// encode the UTF-8 character to his unicode position
	Uint16 uni = 0;
	unsigned char ch = *pch;
	// we do not need encoding 4 byte long characters because SDL only returns the BMP of the unicode table
	if ( (ch & 0xE0) == 0xE0 )
	{
		uni |= (ch & 0x0F) << 12;
		uni |= (*(pch+1) & 0x3F) << 6;
		uni |= (*(pch+2) & 0x3F);
		*increase = 3;
	}
	else if ( (ch & 0xC0) == 0xC0 )
	{
		uni |= (ch & 0x1F) << 6;
		uni |= (*(pch+1) & 0x3F);
		*increase = 2;
	}
	else
	{
		uni |= (ch & 0x7F);
		*increase = 1;
	}
	return uni;
}
