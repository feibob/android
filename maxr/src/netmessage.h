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

#ifndef _net_message_h
#define _net_message_h

#include <SDL.h>
#include <string>
#include "network.h"

enum eNetStatusMsg
{
	TCP_ACCEPT = 0,
	TCP_CLOSE
};

enum eNetMessageClass { NET_MSG_SERVER, NET_MSG_CLIENT, NET_MSG_MENU, NET_MSG_STATUS };


/**
* This class represents a message between server and client
* It provides some usefull interfaces to its containing data
*@author Eiko
*/
class cNetMessage
{
public:
	char data[PACKAGE_LENGTH];		//the data of the netMessage

	/** length of the containing data in bytes */
	int iLength;
	/** should be set by the send function. Client->Host: message source, Host->Client: message destiantion */
	int iPlayerNr;
	/** the type of the message */
	int iType;

	/** creates a new netMessage with iType
	* @author Eiko
	* @param iType the type of the message
	*/
	cNetMessage(int iType);

	/** creates a netMessage from a former serialized netMessage
	* @author Eiko
	* @param c the serial data from a netMessage
	*/
	cNetMessage( char* c );

	//~cNetMessage();

	/** return a pointer to a platform independed serial representation of the NetMessage
	* Byte 0: START_CHAR
	* Bytes 1 - 2: Total length of the message, in little endian
	* Bytes 3 - 4: Type of the message, little endian
	* Byte  5: the Playernumber
	* following Bytes: the pushed data
	* @author Eiko
	*/
	char* serialize();

	eNetMessageClass getClass();

	/** rewinds a received and already read (via popXYZ) msg, so that it's content can be popped a second time.
	 * @author Pagra
	 */
	void rewind ();

	/** pushes a char to the end of the netMessage
	* @author Eiko
	* @param c the char to push to the message
	*/
	void pushChar( char c);

	/** pops an char from the end of the netMessage
	* @author Eiko
	* @return the char poped from the message
	*/
	char popChar();

	/** pushes an Sint16 to the end of the netMessage
	* @author Eiko
	* @param i the Sint16 to push to the message
	*/
	void pushInt16( Sint16 i );

	/** pops a Sint16 from the end of the netMessage
	* @author Eiko
	* @return the Sint16 poped from the message
	*/
	Sint16 popInt16();

	/** pushes a Sint32 to the end of the netMessage
	* @author Eiko
	* @param i the Sint32 to push to the message
	*/
	void pushInt32( Sint32 i );

	/** pops a Sint 32 from the end of the netMessage
	* @author Eiko
	* @return the Sint32 poped from the message
	*/
	Sint32 popInt32 ();

	/** pushes a string to the end of the netMessage
	* @author Eiko
	* @param s the string to push to the message
	*/
	void pushString( std::string s );

	/** pops a string from the end of the netMessage
	* @author Eiko
	* @return the string poped from the message
	*/
	std::string popString();

	/** pushes a bool to the end of the netMessage
	* @author Eiko
	* @param b the bool to push to the message
	*/
	void pushBool( bool b );

	/** pops a bool from the end of the netMessage
	* @author Eiko
	* @return the bool poped from the message
	*/
	bool popBool();

	#define BITS 32
	#define EXPBITS 8
	/** pushes a float to the end of the netMessage
	* @author Eiko
	* @param f the float to push to the message
	*/
	void pushFloat( float f );

	/** pops a float from the end of the netMessage
	* @author Eiko
	* @return the float poped from the message
	*/
	float popFloat();

	/** returns the string representation of iType
	* this is only for better readability of the netlog
	* @author Eiko
	*/
	std::string getTypeAsString();
	int getType () const {return iType;}

	/** returns the serial hexadecimal representation of the netMessage
	* @author Eiko
	*/
	std::string getHexDump();
};

#endif //#ifndef _net_message_h
