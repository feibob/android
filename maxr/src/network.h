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
#ifndef networkH
#define networkH
#include <SDL_net.h>
#include <string>
#include "defines.h"
#include "cmutex.h"

class cNetMessage;

#define MAX_CLIENTS				10			// maximal number of clients that can connect to the server
#define PACKAGE_LENGTH			1024		// maximal length of a TCP/IP package
#define START_CHAR				(char)0xFF	// start character in netmessages

// the first client message must be smaller then the first menu message!
#define FIRST_SERVER_MESSAGE	10
#define FIRST_CLIENT_MESSAGE	100
#define FIRST_MENU_MESSAGE		1000

/**
* Callback for the networkthread
*@author alzi alias DoctorDeath
*/
int CallbackHandleNetworkThread( void *arg );

enum SOCKET_TYPES
{
	FREE_SOCKET,
	SERVER_SOCKET,
	CLIENT_SOCKET
};

enum SOCKET_STATES
{
	STATE_UNUSED,
	STATE_READY,
	STATE_NEW,
	STATE_DYING,
	STATE_DELETE
};

//------------------------------------------------------------------------
/**
* Structure with data and its length.
*@author alzi alias DoctorDeath
*/
struct sDataBuffer
{
	Uint32 iLength;
	char data[5*PACKAGE_LENGTH];

	char* getWritePointer();
	int getFreeSpace();
	void deleteFront(int n);

	/**
	* Clears the data buffer and sets his lenght to 0.
	*@author alzi alias DoctorDeath
	*/
	void clear();
};

//------------------------------------------------------------------------
/**
* Structure for Sockets used by the TCP-Class.
*@author alzi alias DoctorDeath
*/
struct sSocket
{
	sSocket();
	~sSocket();

	int iType;
	int iState;

	TCPsocket socket;
	sDataBuffer buffer;
	unsigned int messagelength;
};

//------------------------------------------------------------------------
/**
* Class for the handling of events over TCP/IP
*@author alzi alias DoctorDeath
*/
class cTCP
{
public:
	/**
	 * Creates the mutexes, initialises some variables and the sockets and starts the network thread.
	 *@author alzi alias DoctorDeath
	 */
	cTCP();

	/**
	 * Destroys the mutexes, the sockets and exits the network thread.
	 *@author alzi alias DoctorDeath
	 */
	~cTCP();

private:
	cMutex TCPMutex;


	SDL_Thread *TCPHandleThread;
	bool bExit;
	bool bHost;

	int iPort;
	int iLast_Socket;
	std::string sIP;
	sSocket Sockets[MAX_CLIENTS];
	SDLNet_SocketSet SocketSet;
	IPaddress ipaddr;

	/**
	* Searchs for the first unused socket and allocates memory for a new one if there are no free sockets.
	*@author alzi alias DoctorDeath
	*@return index of found socket
	*/
	int getFreeSocket();
	/**
	* Deletes the socket, frees its memory and sorts the rest sockets in the list.
	*@author alzi alias DoctorDeath
	*/
	void deleteSocket( int iNum );

	int pushEvent( cNetMessage* message);
public:
	/**
	* Creates a new server on the port which has to be set before.
	*@author alzi alias DoctorDeath
	*return 0 on succes, -1 if an error occurs
	*/
	int create();
	/**
	* Connects as client to the IP on the port which both have to be set before.
	*@author alzi alias DoctorDeath
	*return 0 on succes, -1 if an error occurs
	*/
	int connect();
	/**
	* Closes the connection to the socket.
	*@author alzi alias DoctorDeath
	*param iClientNumber Number of client/socket to which the connection should be closed.
	*/
	void close( int iClientNumber );

	/**
	* Sends data of an given lenght to the client/socket.
	*@author alzi alias DoctorDeath
	*param iClientNumber Number of client/socket to which the data should be send.
	*param iLength Length of data to be send.
	*param buffer buffer with data to be send.
	*return 0 on succes, -1 if an error occurs
	*/
	int sendTo( int iClientNumber, int iLength, char *buffer );
	/**
	* Sends the data to all sockets to which this machine is connected.
	*@author alzi alias DoctorDeath
	*param iLength Length of data to be send.
	*param buffer buffer with data to be send.
	*return 0 on succes, -1 if an error occurs
	*/
	int send( int iLength, char *buffer );

	/**
	* Sets a new port.
	*@author alzi alias DoctorDeath
	*param iPort New port number.
	*/
	void setPort( int iPort );
	/**
	* Sets a new IP.
	*@author alzi alias DoctorDeath
	*param iPort New IP.
	*/
	void setIP ( std::string sIP );
	/**
	* Gets the number of currently connected sockets.
	*@author alzi alias DoctorDeath
	*return Number of sockets.
	*/
	int getSocketCount();
	/**
	* Gets the status of the connection.
	*@author alzi alias DoctorDeath
	*return 1 if connected, 0 if not.
	*/
	int getConnectionStatus();
	/**
	* looks if this machine is the host.
	*@author alzi alias DoctorDeath
	*return true if is host, false if not.
	*/
	bool isHost();

	/**
	* Thread function which handles new incoming connections and data.
	*@author alzi alias DoctorDeath
	*/
	void HandleNetworkThread();
} EX *network;

#endif // networkH
