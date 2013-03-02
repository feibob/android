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

#include <iostream>
#include <time.h>
#include "log.h"
#include "cmutex.h"
#include "settings.h"

#define LOGFILE cSettings::getInstance().getLogPath().c_str()
#define NETLOGFILE cSettings::getInstance().getNetLogPath().c_str()
/** errors */
#define EE "(EE): "
/** warnings */
#define WW "(WW): "
/** informations */
#define II "(II): "
/** debuginformations */
#define DD "(DD): "
/**mem error*/
#define MM "(MM): "

cLog::cLog():
	mutex()
{
	logfile = NULL;
	bNetlogStarted = false;
	bFirstRun = true;
}

bool cLog::open(int TYPE)
{
	//check if netlog file name is initialized
	if ( bFirstRun )
	{
		time_t tTime = time ( NULL );
		tm * tmTime = localtime ( &tTime );
		char timestr[25];
		strftime( timestr, 21, "-%d.%m.%y-%H%M.log", tmTime );
		std::string sTime = timestr;
		std::string netLogPath = cSettings::getInstance().getNetLogPath().erase(cSettings::getInstance().getNetLogPath().size() - 4, cSettings::getInstance().getNetLogPath().size());
		cSettings::getInstance().setNetLogPath((netLogPath + sTime).c_str());
		bFirstRun = false;
	}

	if ( logfile == NULL || ((TYPE == LOG_TYPE_NET_DEBUG || TYPE == LOG_TYPE_NET_WARNING || TYPE == LOG_TYPE_NET_ERROR) && !bNetlogStarted) )
	{
		if(TYPE == LOG_TYPE_NET_DEBUG || TYPE == LOG_TYPE_NET_WARNING || TYPE == LOG_TYPE_NET_ERROR)
		{
			logfile = SDL_RWFromFile ( NETLOGFILE,"w+t" ); //Start new network-log and write at beginning of file
			bNetlogStarted = true;
		}
		else
		{
			logfile = SDL_RWFromFile ( LOGFILE,"w+t" ); //Start new log and write at beginning of file
		}
	}
	else
	{
		if(TYPE == LOG_TYPE_NET_DEBUG || TYPE == LOG_TYPE_NET_WARNING || TYPE == LOG_TYPE_NET_ERROR)
		{
			logfile = SDL_RWFromFile ( NETLOGFILE,"a+t" ); //Reopen network-log and write at end of file
		}
		else
		{
			logfile = SDL_RWFromFile ( LOGFILE,"a+t" ); //Reopen log and write at end of file
		}
	}

	int blocks; //sanity check - is file readable?
	char buf[256];

	if(logfile) //can access logfile
	{
		blocks=SDL_RWread ( logfile,buf,16,256/16 );
	}
	else
	{
		fprintf ( stderr,"(EE): Couldn't open maxr.log!\n Please check file/directory permissions\n" );
		return false;
	}
	if ( blocks<0 )
	{
		fprintf ( stderr,"(EE): Couldn't read maxr.log!\n Please check file/directory permissions\n" );

		if ( logfile != NULL ) return true;
		else return false;
	}
	else return true;

}

int cLog::write ( const char *str, int TYPE )
{
	return write ( std::string ( str ) , TYPE );
}

int cLog::write ( std::string str, int TYPE )
{
	cMutex::Lock l(mutex);

	if ( (TYPE == LOG_TYPE_DEBUG || TYPE == LOG_TYPE_NET_DEBUG) && !cSettings::getInstance().isDebug() ) //in case debug is disabled we skip message
	{
		return 0;
	}

	if ( open(TYPE) )
	{
		switch ( TYPE ) //Attach log message type to tmp
		{
			case LOG_TYPE_NET_WARNING :
			case LOG_TYPE_WARNING : str = str.insert ( 0 , WW ); break;
			case LOG_TYPE_NET_ERROR :
			case LOG_TYPE_ERROR :   str = str.insert ( 0 , EE ); std::cout << str << "\n"; break;
			case LOG_TYPE_NET_DEBUG :
			case LOG_TYPE_DEBUG :   str = str.insert ( 0 , DD ); break;
			case LOG_TYPE_INFO :    str = str.insert ( 0 , II ); break;
			case LOG_TYPE_MEM :	 str = str.insert ( 0 , MM ); break;
			default :				str = str.insert ( 0 , II );
		}
		str += TEXT_FILE_LF;
		return writeMessage ( str ); //add log message itself to tmp and send it for writing
	}
	else return -1;
}

int cLog::write ( const char *str )
{
	return write ( std::string ( str ) , LOG_TYPE_INFO );
}

void cLog::mark()
{
	cMutex::Lock l(mutex);

	std::string str = "==============================(MARK)==============================";
	str += TEXT_FILE_LF;
	if ( open(-1) ) writeMessage ( str );
}

int cLog::writeMessage ( char *str )
{
	return writeMessage ( std::string ( str ) );
}

int cLog::writeMessage ( std::string str )
{
	if(logfile)
	{
		int const wrote = SDL_RWwrite(logfile, str.c_str(), 1, (int)str.length());
		std::cout << str;
		if ( wrote<0 ) //sanity check - was file writable?
		{
			fprintf ( stderr,"Couldn't write to maxr.log\nPlease check permissions for maxr.log\nLog message was:\n%s", str.c_str() );
			return -1;
		}
		else close(); //after successful writing of all information we close log here and nowhere else!
		return 0;
	}
	else
	{
		fprintf ( stderr,"Couldn't write to maxr.log\nPlease check permissions for maxr.log\nLog message was:\n%s", str.c_str() );
	}
	return -1;
}


void cLog::close()
{
	SDL_RWclose ( logfile ); //function RWclose always returns 0 in SDL <= 1.2.9 - no sanity check possible
}
