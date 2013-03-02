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
#ifndef serverH
#define serverH
#include <SDL.h>
#include "defines.h"
#include "clist.h"
#include "ringbuffer.h"
#include "main.h" // for sID
#include "map.h"

class cPlayer;
class cServerAttackJob;
class cServerMoveJob;
class cNetMessage;
class cBuilding;
class cUnit;
struct sVehicle;
class cCasualtiesTracker;

/**
* The Types which are possible for a game
*/
enum eGameTypes
{
	GAME_TYPE_SINGLE,		// a singleplayer game
	GAME_TYPE_HOTSEAT,		// a hotseat multiplayer game
	GAME_TYPE_TCPIP			// a multiplayergame over TCP/IP
};

/**
 * Structure for the reports
 */
struct sTurnstartReport
{
	/** unit type of the report */
	sID Type;
	/** counter for this report */
	int iAnz;
};

/**
* Callback funktion for the serverthread
*@author alzi alias DoctorDeath
*/
int CallbackRunServerThread( void *arg );


Uint32 ServerTimerCallback(Uint32 interval, void *arg);

struct sLandingUnit;

/**
* Server class which takes all calculations for the game and has the data of all players
*@author alzi alias DoctorDeath
*/
class cServer
{
	friend class cSavegame;
public:
	/**
	 * initialises the server class. turnLimit and scoreLimit should not
	 both be set. If both are zero, it's last man standing.
	 *@author alzi alias DoctorDeath
	 *@param map The Map for the game
	 *@param PlayerList The list with all players
	 *@param iGameType The type of the game. Can be GAME_TYPE_SINGLE, GAME_TYPE_HOTSEAT or GAME_TYPE_TCPIP
	 *@param turnLimit Game ends after this many turns
	 *@param scoreLimit First player to this many points wins
	 */
	cServer(cMap* map, cList<cPlayer*>* PlayerList, eGameTypes gameType, bool bPlayTurns, int turnLimit=0, int scoreLimit=0);
	void setDeadline(int iDeadline);
	~cServer();

private:
	/** a list with all events for the server */
	cRingbuffer<cNetMessage*> eventQueue;
	/** the event that was polled last from the eventQueue*/
	cNetMessage* lastEvent;

	/** the thread the server runs in */
	SDL_Thread *ServerThread;
	/** true if the server should exit and end his thread */
	bool bExit;


	/** list with buildings without owner, e. g. rubble fields */
	cBuilding* neutralBuildings;
	/** true if this is a hotseat game */
	bool bHotSeat;
	/** number of active player in hotseat */
	int iHotSeatPlayer;
	/** true if the game should be played in turns */
	bool bPlayTurns;
	/** number of active player in turn based multiplayer game */
	int iActiveTurnPlayerNr;
	/** name of the savegame to load or to save */
	std::string sSaveLoadFile;
	/** index number of the savegame to load or to save */
	int iSaveLoadNumber;
	/** the type of the current game */
	eGameTypes gameType;
	/** a list with the numbers of all players who have ended theire turn */
	cList<cPlayer*> PlayerEndList;
	/** number of current turn */
	int iTurn;
	/** deadline in seconds if the first player has finished his turn*/
	int iTurnDeadline;
	/** Ticks when the deadline has been initialised*/
	unsigned int iDeadlineStartTime;
	/** Number of the Player who wants to end his turn; -1 for no player, -2 for undifined player */
	int iWantPlayerEndNum;
	/** The ID for the next unit*/
	unsigned int iNextUnitID;
	/** will be incremented by the Timer */
	unsigned int iTimerTime;
	/** diffrent timers */
	bool timer50ms, timer100ms, timer400ms;
	/** ID of the timer */
	SDL_TimerID TimerID;
	/** if this is true the map will be opened for a defeated player */
	bool openMapDefeat;
	/** List with disconnected players */
	cList<cPlayer*> DisconnectedPlayerList;
	/** a sequential id for identifying additional save information from clients */
	int savingID;
	/** the index of the saveslot where additional save info should be added */
	int savingIndex;

	/** victory conditions. One or both must be zero. **/
	int turnLimit, scoreLimit;

	cCasualtiesTracker* casualtiesTracker;
public:
	cCasualtiesTracker* getCasualtiesTracker () {return casualtiesTracker;}

private:
	/**
	* returns a pointer to the next event of the eventqueue. If the queue is empty it will return NULL.
	* the returned message and its data structures are valid until the next call of pollEvent()
	*@author eiko
	*@return the next net message or NULL if queue is empty
	*/
	cNetMessage* pollEvent();

public:
	/**
	* Handels all incoming netMessages from the clients
	*@author Eiko
	*@param message The message to be prozessed
	*@return 0 for success
	*/
	int HandleNetMessage( cNetMessage* message );
private:

	/**
	* lands the vehicle at a free position in the radius
	*@author alzi alias DoctorDeath
	*@param iX The X coordinate to land.
	*@param iY The Y coordinate to land.
	*@param iWidth Width of the field.
	*@param iHeight iHeight of the field.
	*@param Vehicle Vehicle to land.
	*@param Player Player whose vehicle should be land.
	*@return NULL if the vehicle could not be landed, else a pointer to the vehicle.
	*/
	cVehicle *landVehicle ( int iX, int iY, int iWidth, int iHeight, sVehicle *Vehicle, cPlayer *Player );

	/**
	* handles the pressed end of a player
	*@author alzi alias DoctorDeath
	*/
	void handleEnd ( int iPlayerNum );
	/**
	* executes everthing for a turnend
	*@author alzi alias DoctorDeath
	*@param iPlayerNum Number of player who has pressed the end turn button
	*@param bChangeTurn true if all players have ended their turn and the turnnumber has changed
	*/
	void makeTurnEnd ();
	/**
	* checks whether a player is defeated
	*@author alzi alias DoctorDeath
	*/
	void checkDefeats ();

public:
	/**
	* rechecks the end actions when a player wanted to finish his turn
	*@author alzi alias DoctorDeath
	*/
	void handleWantEnd();
private:
	/**
	* checks whether some units are moving and restarts remaining movements
	*@author alzi alias DoctorDeath
	*@param iPlayer The player who will receive the messages when the turn can't be finished now; -1 for all players
	*@return true if there were found some moving units
	*/
	bool checkEndActions ( int iPlayer );
public:
	/**
	* checks wether the deadline has run down
	*@author alzi alias DoctorDeath
	*/
	void checkDeadline ();
	/**
	* handles the timers timer50ms, timer100ms and timer400ms
	*@author alzi alias DoctorDeath
	*/
	void handleTimer();
	/**
	* handles all active movejobs
	*@author alzi alias DoctorDeath
	*/
	void handleMoveJobs();

private:
	/**
	* Calculates the cost, that this upgrade would have for the given player.
	*@author Paul Grathwohl
	*/
	int getUpgradeCosts (sID& ID, cPlayer* player, bool bVehicle,
						 int newDamage, int newMaxShots, int newRange, int newMaxAmmo,
						 int newArmor, int newMaxHitPoints, int newScan, int newMaxSpeed);
	/**
	* changes the owner of a vehicle
	*@author alzi alias DoctorDeath
	*/
	void changeUnitOwner ( cVehicle *vehicle, cPlayer *newOwner );
	/**
	* stops the buildingprocess of a working vehicle.
	*@author alzi alias DoctorDeath
	*/
	void stopVehicleBuilding ( cVehicle *vehicle );

	/**
	 * Helper for destroyUnit(cBuilding) that deletes all buildings in the iterator and returns the generated rubble value.
	 * @author Paul Grathwohl
	 */
	int deleteBuildings(cBuildingIterator building);

public:
	/** the map */
	cMap *Map;
	/** List with all attackjobs */
	cList<cServerAttackJob*> AJobs;
	/** List with all active movejobs */
	cList<cServerMoveJob*> ActiveMJobs;
	/** List with all players */
	cList<cPlayer*> *PlayerList;
	/** true if the game has been started */
	bool bStarted;

	/**
	 * gets the unit with the ID
	 *@param iID The ID of the unit
	 */
	cUnit* getUnitFromID (unsigned int iID) const;
	/**
	* gets the vehicle with the ID
	*@author alzi alias DoctorDeath
	*@param iID The ID of the vehicle
	*/
	cVehicle* getVehicleFromID (unsigned int iID) const;
	/**
	* gets the bulding with the ID
	*@author alzi alias DoctorDeath
	*@param iID The ID of the building
	*/
	cBuilding* getBuildingFromID (unsigned int iID) const;

	/**
	* checks whether a player has detected some new enemy units
	*@author alzi alias DoctorDeath
	*/
	void checkPlayerUnits ();

	/**
	* returns the player with the given number
	*@author alzi alias DoctorDeath
	*@param iNum The number of the player.
	*@return The wanted player.
	*/
	cPlayer *getPlayerFromNumber ( int iNum );

	/**
	 * returns if the player is on the disconnected players list
	 *@author pagra
	 */
	bool isPlayerDisconnected (cPlayer* player) const;

	/**
	 * puts all players on the disconnected list. This is useful for loading a
	 * game on the dedicated server.
	 *@author pagra
	 */
	void markAllPlayersAsDisconnected ();

	/**
	* pushes an event to the eventqueue of the server. This is threadsafe.
	*@author alzi alias DoctorDeath
	*@param event The SDL_Event to be pushed.
	*@return 0 for success
	*/
	int pushEvent( cNetMessage *event );


	/**
	* sends a netMessage to the client on which the player with 'iPlayerNum' is playing
	* PlayerNum -1 means all players
	* message must not be deleted after caling this function
	*@author alzi alias DoctorDeath
	*@param message The message to be send.
	*@param iPlayerNum Number of player who should receive this event.
	*/
	void sendNetMessage( cNetMessage* message, int iPlayerNum = -1 );

	/**
	* runs the server. Should only be called by the ServerThread!
	*@author alzi alias DoctorDeath
	*/
	void run();

	/**
	* deletes a Unit
	*@author alzi alias DoctorDeath
	*@param unit the unit which should be deleted.
	*@param notifyClient when false, the Unit is only removed locally on the Server. The caller must make sure to inform the clients
	*/
	void deleteUnit (cUnit* unit, bool notifyClient = true);

	/**
	* deletes an unit (and additional units on the same field if nessesarry)
	* from the game, creates rubble
	* does not notify the client! the the caller has to take care of the nessesary actions on the client
	*/
	void destroyUnit( cVehicle* vehicle );
	void destroyUnit( cBuilding* building );


	/**
	* adds the unit to the map and player.
	*@author alzi alias DoctorDeath
	*@param iPosX The X were the unit should be added.
	*@param iPosY The Y were the unit should be added.
	*@param Vehicle Vehicle which should be added.
	*@param Building Building which should be added.
	*@param Player Player whose vehicle should be added.
	*@param bInit true if this is a initialisation call.
	*/
	cVehicle *addUnit( int iPosX, int iPosY, sVehicle *Vehicle, cPlayer *Player, bool bInit = false, bool bAddToMap = true );
	cBuilding *addUnit( int iPosX, int iPosY, sBuilding *Building, cPlayer *Player, bool bInit = false );
	/**
	* lands all units at the given position
	*@author alzi alias DoctorDeath
	*@param iX The X coordinate to land.
	*@param iY The Y coordinate to land.
	*@param Player The Player who wants to land.
	*@param List List with all units to land.
	*@param bFixed true if the bridgehead is fixed.
	*/
	void makeLanding( int iX, int iY, cPlayer *Player, cList<sLandingUnit> *List, bool bFixed );
	/**
	 *
	 */
	void correctLandingPos( int &iX, int &iY);
	/**
	* increments the iTimeTimer.
	*@author alzi alias DoctorDeath
	*/
	void Timer();
	/**
	* adds a report to the reportlist
	*@author alzi alias DoctorDeath
	*@param sName the report name
	*@param bVehicle true if the report is about vehicles
	*@param iPlayerNum Number of player to whos list the report should be added
	*/
	void addReport ( sID Type, bool bVehicle, int iPlayerNum );
	/**
	* adds an new movejob
	*@author alzi alias DoctorDeath
	*@param MJob the movejob to be added
	*/
	void addActiveMoveJob ( cServerMoveJob *MoveJob );
	/**
	* generates a new movejob
	*/
	bool addMoveJob(int srcX, int srcY, int destX, int destY, cVehicle* vehicle);
	/**
	* adds a new rubble object to the game
	* @param x,y the position where the rubble is added
	* @param value the amount of material in the rubble field
	* @param big size of the rubble field
	*/
	void addRubble( int x, int y, int value, bool big );
	/**
	* deletes a rubble object from the game
	* @param rubble pointer to the rubble object which will be deleted
	*/
	void deleteRubble( cBuilding* rubble );

	void resyncPlayer ( cPlayer *Player, bool firstDelete = false );
	void resyncVehicle ( cVehicle *Vehicle, cPlayer *Player );
	/**
	* deletes a player and all his units
	*@author alzi alias DoctorDeath
	*/
	void deletePlayer( cPlayer *Player );

	void sideStepStealthUnit( int PosX, int PosY, cVehicle* vehicle, int bigOffset = -1 );
	void sideStepStealthUnit( int PosX, int PosY, sUnitData& vehicleData, cPlayer* vehicleOwner, int bigOffset = -1 );

	void makeAdditionalSaveRequest ( int saveNum );

	int getTurn() const;

	bool isTurnBasedGame () const { return bPlayTurns; }

} EX *Server;

#endif
