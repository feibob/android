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
#ifndef movejobsH
#define movejobsH

#include <SDL.h>
#include "clist.h"

class cVehicle;
class cBuilding;
class cMap;
class cNetMessage;

/* Size of a memory block while pathfinding */
#define MEM_BLOCK_SIZE 10

enum MJOB_TYPES
{
	MJOB_OK,
	MJOB_STOP,
	MJOB_FINISHED,
	MJOB_BLOCKED
};

// structures for the calculation of the path
struct sWaypoint
{
	sWaypoint *next;
	int X,Y;
	int Costs;
};

/* node structure for pathfinding */
struct sPathNode
{
	/* x and y coords */
	int x, y;
	/* the difrent cost types */
	int costF, costG, costH;
	/* previous and next node of this one in the hole path */
	sPathNode *prev, *next;

	/* the costs to enter on this field */
	int fieldCosts;
};

enum ePathDestinationTypes
{
	PATH_DEST_TYPE_POS,
	PATH_DEST_TYPE_LOAD,
	PATH_DEST_TYPE_ATTACK
};

class cPathDestHandler
{
	ePathDestinationTypes type;

	cVehicle *srcVehicle;

	cBuilding *destBuilding;
	cVehicle *destVehicle;
	int destX, destY;
public:
	cPathDestHandler ( ePathDestinationTypes type_, int destX, int destY, cVehicle *srcVehicle_, cBuilding *destBuilding_, cVehicle *destVehicle_ );

	bool hasReachedDestination( int x, int y );
	int heuristicCost ( int srcX, int srcY );
};

class cPathCalculator
{
	void init( int ScrX, int ScrY, cMap *Map, cVehicle *Vehicle, cList<cVehicle*> *group );

public:
	cPathCalculator( int ScrX, int ScrY, int DestX, int DestY, cMap *Map, cVehicle *Vehicle, cList<cVehicle*> *group = NULL );
	cPathCalculator( int ScrX, int ScrY, cVehicle *destVehicle, cBuilding *destBuilding, cMap *Map, cVehicle *Vehicle, bool load );
	cPathCalculator( int ScrX, int ScrY, cMap *Map, cVehicle *Vehicle, int attackX, int attackY );
	~cPathCalculator();

	/**
	* calculates the best path in costs and length
	*@author alzi alias DoctorDeath
	*/
	sWaypoint* calcPath();
	/**
	* calculates the costs for moving from the source- to the destinationfield
	*@author alzi alias DoctorDeath
	*/
	int calcNextCost( int srcX, int srcY, int destX, int destY );

	/* the map on which the path will be calculated */
	cMap *Map;
	/* the moving vehicle */
	cVehicle *Vehicle;
	/* if more then one vehicle is moving in a group this is the list of all moving vehicles */
	cList<cVehicle*> *group;
	/* source and destination coords */
	int ScrX, ScrY;
	bool bPlane, bShip;
	cPathDestHandler *destHandler;


private:
	/* the waypoints of the found path*/
	sWaypoint *Waypoints;
	/* memoryblocks for the nodes */
	sPathNode **MemBlocks;
	/* number of blocks */
	int blocknum;
	/* restsize of the last block */
	int blocksize;

	/* heaplist where all nodes are sortet by there costF value */
	sPathNode **nodesHeap;
	/* open nodes map */
	sPathNode **openList;
	/* closed nodes map */
	sPathNode **closedList;
	/* number of nodes saved on the heaplist; equal to number of nodes in the openlist */
	int heapCount;
	/**
	* expands the nodes around the overgiven one
	*@author alzi alias DoctorDeath
	*/
	void expandNodes ( sPathNode *Node );
	/**
	* returns a pointer to allocated memory and allocets a new block in memory if necessary
	*@author alzi alias DoctorDeath
	*/
	sPathNode *allocNode();
	/**
	* inserts a node into the heaplist and sets it to the right position by its costF value.
	*@author alzi alias DoctorDeath
	*/
	void insertToHeap( sPathNode *Node, bool exists );
	/**
	* deletes the first node in the heaplist and resorts the rest.
	*@author alzi alias DoctorDeath
	*/
	void deleteFirstFromHeap();
};


enum eEndMoveActionType
{
	EMAT_LOAD,
	EMAT_GET_IN,
	EMAT_ATTACK
};

class cEndMoveAction
{
public:
	cVehicle* vehicle_;
	eEndMoveActionType type_;
	int destID_;		//we store the ID and not a pointer to vehicle/building,
						//so we don't have to invalidate the pointer, when the dest unit gets destroyed

	void executeLoadAction();
	void executeGetInAction();
	void executeAttackAction();

	cEndMoveAction( cVehicle* vehicle, int destID, eEndMoveActionType type);

	void execute();
};

class cServerMoveJob
{
public:
	cServerMoveJob ( int srcX_, int srcY_, int destX_, int destY_, cVehicle *Vehicle );
	~cServerMoveJob ();

	cMap *Map;
	cVehicle *Vehicle;

	int SrcX, SrcY;
	int DestX, DestY;
	bool bFinished;
	bool bEndForNow;
	int iNextDir;
	int iSavedSpeed;
	bool bPlane;
	cEndMoveAction* endAction;

	sWaypoint *Waypoints;

	static cServerMoveJob* generateFromMessage( cNetMessage *message );

	bool calcPath();
	void release();
	bool checkMove();
	void moveVehicle();
	void doEndMoveVehicle();
	void calcNextDir();
	void stop();
	void resume();
	void addEndAction(int destID, eEndMoveActionType type);
};

class cClientMoveJob
{
	void init ( int iSrcOff, cVehicle *Vehicle );
public:
	static sWaypoint* calcPath( int SrcX, int SrcY, int DestX, int DestY, cVehicle * vehicle, cList<cVehicle*> *group = NULL );

	cClientMoveJob ( int iSrcOff, int iDestOff, cVehicle *Vehicle );
	cClientMoveJob ( int iSrcOff, sWaypoint *Waypoints, cVehicle *Vehicle );
	~cClientMoveJob ();
	cMap *Map;
	cVehicle *Vehicle;
	cEndMoveAction *endMoveAction;

	int ScrX, ScrY;
	int DestX, DestY;
	bool bFinished;
	bool bEndForNow;
	bool bSuspended;
	int iNextDir;
	int iSavedSpeed;
	bool bPlane;
	bool bSoundRunning;

	sWaypoint *Waypoints;
	sWaypoint *lastWaypoints;

	void setVehicleToCoords(int x, int y, int height);
	bool generateFromMessage( cNetMessage *message );

	void release();
	void handleNextMove( int iServerPositionX, int iServerPositionY, int iType, int iSavedSpeed, int heigth );
	void moveVehicle();
	void doEndMoveVehicle ();
	void calcNextDir ();
	void drawArrow ( SDL_Rect Dest, SDL_Rect *LastDest, bool bSpezial );
	void startMoveSound();
	void stopMoveSound();
};

#endif // movejobsH
