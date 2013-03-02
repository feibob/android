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

#include "movejobs.h"
#include "attackJobs.h"
#include "player.h"
#include "serverevents.h"
#include "clientevents.h"
#include "server.h"
#include "client.h"
#include "netmessage.h"
#include "math.h"
#include "settings.h"
#include "buildings.h"
#include "vehicles.h"

cPathDestHandler::cPathDestHandler(ePathDestinationTypes type_, int destX_, int destY_, cVehicle* srcVehicle_, cBuilding* destBuilding_, cVehicle* destVehicle_) :
	type(type_),
	srcVehicle(srcVehicle_),
	destBuilding(destBuilding_),
	destVehicle(destVehicle_),
	destX(destX_),
	destY(destY_)
{}

bool cPathDestHandler::hasReachedDestination( int x, int y )
{
	switch ( type )
	{
	case PATH_DEST_TYPE_POS:
		return ( x == destX && y == destY );
	case PATH_DEST_TYPE_LOAD:
		return ( ( destBuilding && destBuilding->isNextTo ( x, y ) ) ||
				( destVehicle && destVehicle->isNextTo ( x, y ) ) );
	case PATH_DEST_TYPE_ATTACK:
		x -= destX;
		y -= destY;
		return ( sqrt ( ( double ) ( x*x + y*y ) ) <= srcVehicle->data.range );
	default:
		return true;
	}
	return false;
}

int cPathDestHandler::heuristicCost ( int srcX, int srcY )
{
	switch ( type )
	{
	case PATH_DEST_TYPE_POS:
	case PATH_DEST_TYPE_LOAD:
		return 0;
	case PATH_DEST_TYPE_ATTACK:
	default:
		{
			int deltaX = destX - srcX;
			int deltaY = destY - srcY;

			return Round ( sqrt ( (double)deltaX*deltaX + deltaY*deltaY ) );
		}
	}
}

cPathCalculator::cPathCalculator( int ScrX, int ScrY, int DestX, int DestY, cMap *Map, cVehicle *Vehicle, cList<cVehicle*> *group )
{
	destHandler = new cPathDestHandler ( PATH_DEST_TYPE_POS, DestX, DestY, NULL, NULL, NULL );
	init ( ScrX, ScrY, Map, Vehicle, group );
}


cPathCalculator::cPathCalculator( int ScrX, int ScrY, cVehicle *destVehicle, cBuilding *destBuilding, cMap *Map, cVehicle *Vehicle, bool load )
{
	destHandler = new cPathDestHandler ( load ? PATH_DEST_TYPE_LOAD : PATH_DEST_TYPE_ATTACK, 0, 0, Vehicle, destBuilding, destVehicle );
	init ( ScrX, ScrY, Map, Vehicle, NULL );
}

cPathCalculator::cPathCalculator( int ScrX, int ScrY, cMap *Map, cVehicle *Vehicle, int attackX, int attackY )
{
	destHandler = new cPathDestHandler ( PATH_DEST_TYPE_ATTACK, attackX, attackY, Vehicle, NULL, NULL );
	init ( ScrX, ScrY, Map, Vehicle, NULL );
}

void cPathCalculator::init( int ScrX, int ScrY, cMap *Map, cVehicle *Vehicle, cList<cVehicle*> *group )
{
	this->ScrX = ScrX;
	this->ScrY = ScrY;
	this->Map = Map;
	this->Vehicle = Vehicle;
	this->group = group;
	bPlane = Vehicle->data.factorAir > 0;
	bShip = Vehicle->data.factorSea > 0 && Vehicle->data.factorGround == 0;

	Waypoints = NULL;
	MemBlocks = NULL;
	nodesHeap = NULL;
	openList = NULL;
	closedList = NULL;

	blocknum = 0;
	blocksize = 0;
	heapCount = 0;
}

cPathCalculator::~cPathCalculator()
{
	delete destHandler;
	if ( MemBlocks != NULL )
	{
		for ( int i = 0; i < blocknum; i++ )
		{
			delete[] MemBlocks[i];
		}
		free ( MemBlocks );
	}
	if ( nodesHeap != NULL ) delete [] nodesHeap;
	if ( openList != NULL ) delete [] openList;
	if ( closedList != NULL ) delete [] closedList;
}

sWaypoint* cPathCalculator::calcPath ()
{
	// generate open and closed list
	nodesHeap = new sPathNode*[Map->size*Map->size+1];
	openList = new sPathNode*[Map->size*Map->size+1];
	closedList = new sPathNode*[Map->size*Map->size+1];
	std::fill <sPathNode**, sPathNode*> ( nodesHeap, &nodesHeap[Map->size*Map->size+1], NULL );
	std::fill <sPathNode**, sPathNode*> ( openList, &openList[Map->size*Map->size+1], NULL );
	std::fill <sPathNode**, sPathNode*> ( closedList, &closedList[Map->size*Map->size+1], NULL );

	// generate startnode
	sPathNode *StartNode = allocNode ();
	StartNode->x = ScrX;
	StartNode->y = ScrY;
	StartNode->costG = 0;
	StartNode->costH = destHandler->heuristicCost ( ScrX, ScrY );
	StartNode->costF = StartNode->costG+StartNode->costH;

	StartNode->prev = NULL;
	openList[ScrX+ScrY*Map->size] = StartNode;
	insertToHeap ( StartNode, false );

	while ( heapCount > 0 )
	{
		// get the node with the lowest F value
		sPathNode *CurrentNode = nodesHeap[1];

		// move it from the open to the closed list
		openList[CurrentNode->x+CurrentNode->y*Map->size] = NULL;
		closedList[CurrentNode->x+CurrentNode->y*Map->size] = CurrentNode;
		deleteFirstFromHeap();

		// generate waypoints when destination has been reached
		if ( destHandler->hasReachedDestination ( CurrentNode->x, CurrentNode->y ) )
		{
			sWaypoint *NextWaypoint;
			Waypoints = new sWaypoint;

			sPathNode *NextNode = CurrentNode;
			NextNode->next = NULL;
			do
			{
				NextNode->prev->next = NextNode;
				NextNode = NextNode->prev;
			}
			while ( NextNode->prev != NULL );


			NextWaypoint = Waypoints;
			NextWaypoint->X = NextNode->x;
			NextWaypoint->Y = NextNode->y;
			NextWaypoint->Costs = 0;
			do
			{
				NextNode = NextNode->next;

				NextWaypoint->next = new sWaypoint;
				NextWaypoint->next->X = NextNode->x;
				NextWaypoint->next->Y = NextNode->y;
				NextWaypoint->next->Costs = calcNextCost ( NextNode->prev->x, NextNode->prev->y, NextWaypoint->next->X, NextWaypoint->next->Y );
				NextWaypoint = NextWaypoint->next;
			}
			while ( NextNode->next != NULL );

			NextWaypoint->next = NULL;

			return Waypoints;
		}

		// expand node
		expandNodes ( CurrentNode );
	}

	// there is no path to the destination field
	Waypoints = NULL;


	return Waypoints;
}

void cPathCalculator::expandNodes ( sPathNode *ParentNode )
{
	// add all nearby nodes
	for ( int y = ParentNode->y-1; y <= ParentNode->y+1; y++ )
	{
		if ( y < 0 || y >= Map->size ) continue;

		for ( int x = ParentNode->x-1; x <= ParentNode->x+1; x++ )
		{
			if ( x < 0 || x >= Map->size ) continue;
			if ( x == ParentNode->x && y == ParentNode->y ) continue;

			if ( !Map->possiblePlace( Vehicle, x, y, true) )
			{
				// when we have a group of units, the units will not block each other
				if ( group )
				{
					bool isInGroup = false;
					// get the blocking unit
					cVehicle *blockingUnit;
					if ( Vehicle->data.factorAir > 0 ) blockingUnit = (*Map)[x+y*Map->size].getPlanes();
					else blockingUnit = (*Map)[x+y*Map->size].getVehicles();
					// check whether the blocking unit is the group
					for ( unsigned int i = 0; i < group->Size(); i++ )
					{
						if ( (*group)[i] == blockingUnit )
						{
							isInGroup = true;
							break;
						}
					}
					if ( !isInGroup ) continue;
				}
				else continue;
			}
			if ( closedList[x+y*Map->size] != NULL ) continue;

			if ( openList[x+y*Map->size] == NULL )
			{
				// generate new node
				sPathNode *NewNode = allocNode ();
				NewNode->x = x;
				NewNode->y = y;
				NewNode->costG = calcNextCost( ParentNode->x, ParentNode->y, x, y ) + ParentNode->costG;
				NewNode->costH = destHandler->heuristicCost ( x, y );
				NewNode->costF = NewNode->costG+NewNode->costH;
				NewNode->prev = ParentNode;
				openList[x+y*Map->size] = NewNode;
				insertToHeap ( NewNode, false );
			}
			else
			{
				// modify existing node
				int costG, costH, costF;
				costG = calcNextCost( ParentNode->x, ParentNode->y, x,y ) + ParentNode->costG;
				costH = destHandler->heuristicCost ( x, y );
				costF = costG+costH;
				if ( costF < openList[x+y*Map->size]->costF )
				{
					openList[x+y*Map->size]->costG = costG;
					openList[x+y*Map->size]->costH = costH;
					openList[x+y*Map->size]->costF = costF;
					openList[x+y*Map->size]->prev = ParentNode;
					insertToHeap ( openList[x+y*Map->size], true );
				}
			}
		}
	}
}

sPathNode *cPathCalculator::allocNode()
{
	// alloced new memory block if necessary
	if ( blocksize <= 0 )
	{
		MemBlocks = (sPathNode**) realloc ( MemBlocks, (blocknum+1)*sizeof(sPathNode*) );
		MemBlocks[blocknum] = new sPathNode[10];
		blocksize = MEM_BLOCK_SIZE;
		blocknum++;
	}
	blocksize--;
	return &MemBlocks[blocknum-1][blocksize];
}

void cPathCalculator::insertToHeap( sPathNode *Node, bool exists )
{
	int i;
	if ( exists )
	{
		// get the number of the existing node
		for ( int j = 1; j <= heapCount; j++ )
		{
			if ( nodesHeap[j] == Node )
			{
				i = j;
				break;
			}
		}
	}
	else
	{
		// add the new node in the end
		heapCount++;
		nodesHeap[heapCount] = Node;
		i = heapCount;
	}
	// resort the nodes
	while ( i > 1 )
	{
		if ( Node->costF < nodesHeap[i/2]->costF )
		{
			sPathNode *TempNode = nodesHeap[i/2];
			nodesHeap[i/2] = Node;
			nodesHeap[i] = TempNode;
			i = i/2;
		}
		else break;
	}
}

void cPathCalculator::deleteFirstFromHeap()
{
	// overwrite the first node by the last one
	nodesHeap[1] = nodesHeap[heapCount];
	nodesHeap[heapCount] = NULL;
	heapCount--;
	int v = 1, u;
	while ( true )
	{
		u = v;
		if ( 2*u+1 <= heapCount )	// both children in the heap exists
		{
			if ( nodesHeap[u]->costF >= nodesHeap[u*2]->costF ) v = 2*u;
			if ( nodesHeap[v]->costF >= nodesHeap[u*2+1]->costF ) v = 2*u+1;
		}
		else if ( 2*u <= heapCount )	// only one children exists
		{
			if ( nodesHeap[u]->costF >= nodesHeap[u*2]->costF ) v = 2*u;
		}
		// do the resort
		if ( u != v )
		{
			sPathNode *TempNode = nodesHeap[u];
			nodesHeap[u] = nodesHeap[v];
			nodesHeap[v] = TempNode;
		}
		else break;
	}
}

int cPathCalculator::calcNextCost( int srcX, int srcY, int destX, int destY )
{
	int costs, offset;
	// first we check whether the unit can fly
	if ( Vehicle->data.factorAir > 0 )
	{
		if ( srcX != destX && srcY != destY ) return (int)(4*Vehicle->data.factorAir*1.5);
		else return (int)(4*Vehicle->data.factorAir);
	}
	offset = destX+destY*Map->size;
	cBuilding* building = Map->fields[offset].getBaseBuilding();
	// moving on water will cost more
	if ( Map->terrain[Map->Kacheln[offset]].water && ( !building || ( building->data.surfacePosition == sUnitData::SURFACE_POS_BENEATH_SEA || building->data.surfacePosition == sUnitData::SURFACE_POS_ABOVE ) )&& Vehicle->data.factorSea > 0 ) costs = (int)(4*Vehicle->data.factorSea);
	else if ( Map->terrain[Map->Kacheln[offset]].coast && !building && Vehicle->data.factorCoast > 0 ) costs = (int)(4*Vehicle->data.factorCoast);
	else if ( Vehicle->data.factorGround > 0 ) costs = (int)(4*Vehicle->data.factorGround);
	else
	{
		Log.write ( "Where can this unit move? " + iToStr ( Vehicle->iID ), cLog::eLOG_TYPE_NET_WARNING );
		costs = 4;
	}
	// moving on a road is cheaper
	if ( building && building->data.modifiesSpeed != 0 ) costs = (int)(costs*building->data.modifiesSpeed);

	// mutliplicate with the factor 1.5 for diagonal movements
	if ( srcX != destX && srcY != destY ) costs = (int)(costs*1.5);
	return costs;
}

void setOffset( cVehicle* Vehicle, int nextDir, int offset )
{
	switch ( nextDir )
	{
		case 0:
			Vehicle->OffY -= offset;
			break;
		case 1:
			Vehicle->OffY -= offset;
			Vehicle->OffX += offset;
			break;
		case 2:
			Vehicle->OffX += offset;
			break;
		case 3:
			Vehicle->OffX += offset;
			Vehicle->OffY += offset;
			break;
		case 4:
			Vehicle->OffY += offset;
			break;
		case 5:
			Vehicle->OffX -= offset;
			Vehicle->OffY += offset;
			break;
		case 6:
			Vehicle->OffX -= offset;
			break;
		case 7:
			Vehicle->OffX -= offset;
			Vehicle->OffY -= offset;
			break;
	}

}

cServerMoveJob::cServerMoveJob ( int srcX_, int srcY_, int destX_, int destY_, cVehicle *vehicle )
{
	if ( !Server ) return;

	Map = Server->Map;
	this->Vehicle = vehicle;
	SrcX = srcX_;
	SrcY = srcY_;
	DestX = destX_;
	DestY = destY_;
	bPlane = (Vehicle->data.factorAir > 0);
	bFinished = false;
	bEndForNow = false;
	iSavedSpeed = 0;
	Waypoints = NULL;
	endAction = NULL;

	// unset sentry status when moving vehicle
	if ( Vehicle->sentryActive )
	{
		Vehicle->owner->deleteSentry (Vehicle);
	}
	sendUnitData ( Vehicle, Vehicle->owner->Nr );
	for ( unsigned int i = 0; i < Vehicle->seenByPlayerList.Size(); i++ )
	{
		sendUnitData ( Vehicle, Vehicle->seenByPlayerList[i]->Nr );
	}

	if ( Vehicle->ServerMoveJob )
	{
		iSavedSpeed = Vehicle->ServerMoveJob->iSavedSpeed;
		Vehicle->ServerMoveJob->release();
		Vehicle->moving = false;
		Vehicle->MoveJobActive = false;
		Vehicle->ServerMoveJob->Vehicle = NULL;
	}
	Vehicle->ServerMoveJob = this;
}

cServerMoveJob::~cServerMoveJob()
{
	sWaypoint *NextWaypoint;
	while ( Waypoints )
	{
		NextWaypoint = Waypoints->next;
		delete Waypoints;
		Waypoints = NextWaypoint;
	}
	Waypoints = NULL;

	if ( endAction ) delete endAction;
}

void cServerMoveJob::stop()
{
	//an already started movement step will be finished

	//delete all waypoint of the movejob except the next one, so the vehicle stops on the next field
	if ( Waypoints && Waypoints->next && Waypoints->next->next )
	{
		sWaypoint* wayPoint = Waypoints->next->next;
		Waypoints->next->next = NULL;
		while ( wayPoint )
		{
			sWaypoint* nextWayPoint = wayPoint->next;
			delete wayPoint;
			wayPoint = nextWayPoint;
		}
	}

	//if the vehicle is not moving, it has to stop immediately
	if ( !Vehicle->moving )
	{
		release();
	}
}

void cServerMoveJob::resume()
{
	if ( Vehicle && Vehicle->data.speedCur > 0 && !Vehicle->moving )
	{
		// restart movejob
		calcNextDir();
		bEndForNow = false;
		SrcX = Vehicle->PosX;
		SrcY = Vehicle->PosY;
		Server->addActiveMoveJob ( this );
	}
}

void cServerMoveJob::addEndAction(int destID, eEndMoveActionType type)
{
	if ( endAction ) delete endAction;

	endAction = new cEndMoveAction(Vehicle, destID, type);
	sendEndMoveActionToClient( Vehicle, destID, type );

}

cServerMoveJob* cServerMoveJob::generateFromMessage ( cNetMessage *message )
{
	if ( message->iType != GAME_EV_MOVE_JOB_CLIENT ) return NULL;

	int iVehicleID = message->popInt32();
	cVehicle *vehicle = Server->getVehicleFromID ( iVehicleID );
	if ( vehicle == NULL )
	{
		Log.write(" Server: Can't find vehicle with id " + iToStr ( iVehicleID ), cLog::eLOG_TYPE_NET_WARNING);
		return NULL;
	}

	//TODO: is this check really needed?
	if ( vehicle->isBeeingAttacked )
	{
		Log.write(" Server: cannot move a vehicle currently under attack", cLog::eLOG_TYPE_NET_DEBUG );
		return NULL;
	}
	if ( vehicle->attacking )
	{
		Log.write(" Server: cannot move a vehicle currently attacking", cLog::eLOG_TYPE_NET_DEBUG );
		return NULL;
	}
	if ( vehicle->IsBuilding || (vehicle->BuildPath && vehicle->ServerMoveJob ))
	{
		Log.write(" Server: cannot move a vehicle currently building", cLog::eLOG_TYPE_NET_DEBUG );
		return NULL;
	}
	if ( vehicle->IsClearing )
	{
		Log.write(" Server: cannot move a vehicle currently building", cLog::eLOG_TYPE_NET_DEBUG );
		return NULL;
	}

	//reconstruct path
	sWaypoint* path = NULL;
	sWaypoint* dest = NULL;
	int iCount = 0;
	int iReceivedCount = message->popInt16();

	if ( iReceivedCount == 0 )
	{
		return NULL;
	}

	while ( iCount < iReceivedCount )
	{
		sWaypoint* waypoint = new sWaypoint;
		waypoint->Y = message->popInt16();
		waypoint->X = message->popInt16();
		waypoint->Costs = message->popInt16();

		if ( !dest ) dest = waypoint;

		waypoint->next = path;
		path = waypoint;

		iCount++;
	}

	//is the vehicle position equal to the begin of the path?
	if ( vehicle->PosX != path->X || vehicle->PosY != path->Y )
	{
		Log.write(" Server: Vehicle with id " + iToStr ( iVehicleID ) + " is at wrong position (" + iToStr (vehicle->PosX) + "x" + iToStr(vehicle->PosY) + ") for movejob from " +  iToStr (path->X) + "x" + iToStr (path->Y) + " to " + iToStr (dest->X) + "x" + iToStr (dest->Y), cLog::eLOG_TYPE_NET_WARNING);

		while ( path )
		{
			sWaypoint* waypoint = path;
			path = path->next;
			delete waypoint;
		}
		return NULL;
	}

	//everything is ok. Construct the movejob
	Log.write(" Server: Received MoveJob: VehicleID: " + iToStr( vehicle->iID ) + ", SrcX: " + iToStr( path->X ) + ", SrcY: " + iToStr( path->Y ) + ", DestX: " + iToStr( dest->X ) + ", DestY: " + iToStr( dest->Y ) + ", WaypointCount: " + iToStr( iReceivedCount ), cLog::eLOG_TYPE_NET_DEBUG);
	cServerMoveJob* mjob = new cServerMoveJob(path->X, path->Y, dest->X, dest->Y, vehicle);
	mjob->Waypoints = path;

	mjob->calcNextDir ();

	return mjob;
}

bool cServerMoveJob::calcPath()
{
	if ( SrcX == DestX && SrcY == DestY ) return false;

	cPathCalculator PathCalculator( SrcX, SrcY, DestX, DestY, Map, Vehicle );
	Waypoints = PathCalculator.calcPath();
	if ( Waypoints )
	{
		calcNextDir();
		return true;
	}
	return false;
}

void cServerMoveJob::release()
{
	bEndForNow = false;
	bFinished = true;
	Log.write ( " Server: Released old movejob", cLog::eLOG_TYPE_NET_DEBUG );
	for ( unsigned int i = 0; i < Server->ActiveMJobs.Size(); i++ )
	{
		if ( this == Server->ActiveMJobs[i] ) return;
	}
	Server->addActiveMoveJob ( this );
	Log.write ( " Server: Added released movejob to avtive ones", cLog::eLOG_TYPE_NET_DEBUG );
}

bool cServerMoveJob::checkMove()
{
	bool bInSentryRange;
	if ( !Vehicle || !Waypoints || !Waypoints->next )
	{
		bFinished = true;
		return false;
	}

	// not enough waypoints for this move?
	if ( Vehicle->data.speedCur < Waypoints->next->Costs )
	{
		Log.write( " Server: Vehicle has not enough waypoints for the next move -> EndForNow: ID: " + iToStr ( Vehicle->iID ) + ", X: " + iToStr ( Waypoints->next->X ) + ", Y: " + iToStr ( Waypoints->next->Y ), LOG_TYPE_NET_DEBUG );
		iSavedSpeed += Vehicle->data.speedCur;
		Vehicle->data.speedCur = 0;
		bEndForNow = true;
		return true;
	}

	bInSentryRange = Vehicle->InSentryRange();

	if ( !Server->Map->possiblePlace( Vehicle, Waypoints->next->X, Waypoints->next->Y) && !bInSentryRange )
	{
		Server->sideStepStealthUnit( Waypoints->next->X, Waypoints->next->Y, Vehicle );
	}

	//when the next field is still blocked, inform the client
	if ( !Server->Map->possiblePlace( Vehicle, Waypoints->next->X, Waypoints->next->Y) || bInSentryRange ) //TODO: bInSentryRange?? Why?
	{
		Log.write( " Server: Next point is blocked: ID: " + iToStr ( Vehicle->iID ) + ", X: " + iToStr ( Waypoints->next->X ) + ", Y: " + iToStr ( Waypoints->next->Y ), LOG_TYPE_NET_DEBUG );
		// if the next point would be the last, finish the job here
		if ( Waypoints->next->X == DestX && Waypoints->next->Y == DestY )
		{
			bFinished = true;
		}
		// else delete the movejob and inform the client that he has to find a new path
		else
		{
			sendNextMove ( Vehicle, MJOB_BLOCKED );
		}
		return false;
	}

	//next step can be executed. start the move and set the vehicle to the next field
	calcNextDir();
	Vehicle->MoveJobActive = true;
	Vehicle->moving = true;

	Vehicle->data.speedCur += iSavedSpeed;
	iSavedSpeed = 0;
	Vehicle->DecSpeed ( Waypoints->next->Costs );

	//reset detected flag, when a water stealth unit drives into the water
	if ( Vehicle->data.isStealthOn & TERRAIN_SEA && Vehicle->data.factorGround )
	{
		bool wasOnLand = !Server->Map->isWater(Waypoints->X, Waypoints->Y, true);
		bool driveIntoWater = Server->Map->isWater(Waypoints->next->X, Waypoints->next->Y, true);

		if ( wasOnLand && driveIntoWater )
		{
			while( Vehicle->detectedByPlayerList.Size() )
				Vehicle->resetDetectedByPlayer( Vehicle->detectedByPlayerList[0] );
		}
	}
	// resetDetected for players, that can't _detect_ the unit anymore and if the unit was not in the detected area of that player in this turn, too
	Vehicle->tryResetOfDetectionStateAfterMove ();

	// send move command to all players who can see the unit
	sendNextMove ( Vehicle, MJOB_OK );

	Map->moveVehicle(Vehicle, Waypoints->next->X, Waypoints->next->Y );
	Vehicle->owner->DoScan();
	Vehicle->OffX = 0;
	Vehicle->OffY = 0;
	setOffset( Vehicle, iNextDir, -64 );

	return true;
}

void cServerMoveJob::moveVehicle()
{
	int iSpeed;
	if ( !Vehicle )
		return;
	if ( Vehicle->data.animationMovement )
		iSpeed = MOVE_SPEED/2;
	else if ( !(Vehicle->data.factorAir > 0) && !(Vehicle->data.factorSea > 0 && Vehicle->data.factorGround == 0) )
	{
		iSpeed = MOVE_SPEED;
		cBuilding* building = Map->fields[Waypoints->next->X+Waypoints->next->Y*Map->size].getBaseBuilding();
		if (building && building->data.modifiesSpeed)
			iSpeed = (int)(iSpeed/building->data.modifiesSpeed);
	}
	else if ( Vehicle->data.factorAir > 0 )
		iSpeed = MOVE_SPEED*2;
	else
		iSpeed = MOVE_SPEED;

	setOffset(Vehicle, iNextDir, iSpeed );

	// check whether the point has been reached:
	if ( abs( Vehicle->OffX ) < iSpeed && abs( Vehicle->OffY ) < iSpeed )
		doEndMoveVehicle();
}

void cServerMoveJob::doEndMoveVehicle()
{
	Log.write(" Server: Vehicle reached the next field: ID: " + iToStr ( Vehicle->iID )+ ", X: " + iToStr ( Waypoints->next->X ) + ", Y: " + iToStr ( Waypoints->next->Y ), cLog::eLOG_TYPE_NET_DEBUG);

	sWaypoint *Waypoint;
	Waypoint = Waypoints->next;
	delete Waypoints;
	Waypoints = Waypoint;

	Vehicle->OffX = 0;
	Vehicle->OffY = 0;

	if ( Waypoints->next == NULL )
	{
		bFinished = true;
	}

	// check for results of the move

	// make mines explode if necessary
	cBuilding* mine = Map->fields[Vehicle->PosX+Vehicle->PosY*Map->size].getMine();
	if ( Vehicle->data.factorAir == 0 && mine && mine->owner != Vehicle->owner )
	{
		Server->AJobs.Add( new cServerAttackJob( mine, Vehicle->PosX+Vehicle->PosY*Map->size, false ));
		bEndForNow = true;
	}

	// search for resources if necessary
	if ( Vehicle->data.canSurvey )
	{
		sendVehicleResources( Vehicle, Map );
		Vehicle->doSurvey();
	}

	//handle detection
	Vehicle->makeDetection();

	// let other units fire on this one
	Vehicle->InSentryRange();

	// lay/clear mines if necessary
	if ( Vehicle->data.canPlaceMines )
	{
		bool bResult = false;
		if ( Vehicle->LayMines ) bResult = Vehicle->layMine();
		else if ( Vehicle->ClearMines ) bResult = Vehicle->clearMine();
		if ( bResult )
		{
			// send new unit values
			sendUnitData( Vehicle, Vehicle->owner->Nr );
			for ( unsigned int i = 0; i < Vehicle->seenByPlayerList.Size(); i++ )
			{
				sendUnitData(Vehicle, Vehicle->seenByPlayerList[i]->Nr );
			}
		}
	}

	Vehicle->moving = false;
	calcNextDir();

	if ( Vehicle->canLand( *Server->Map ) )
	{
		Vehicle->FlightHigh = 0;
	}
	else
	{
		Vehicle->FlightHigh = 64;
	}
}

void cServerMoveJob::calcNextDir()
{
	if ( !Waypoints || !Waypoints->next ) return;
#define TESTXY_CND(a,b) if( Waypoints->X a Waypoints->next->X && Waypoints->Y b Waypoints->next->Y )
	TESTXY_CND ( ==,> ) iNextDir = 0;
	else TESTXY_CND ( <,> ) iNextDir = 1;
	else TESTXY_CND ( <,== ) iNextDir = 2;
	else TESTXY_CND ( <,< ) iNextDir = 3;
	else TESTXY_CND ( ==,< ) iNextDir = 4;
	else TESTXY_CND ( >,< ) iNextDir = 5;
	else TESTXY_CND ( >,== ) iNextDir = 6;
	else TESTXY_CND ( >,> ) iNextDir = 7;
}

cEndMoveAction::cEndMoveAction( cVehicle* vehicle, int destID, eEndMoveActionType type)
{
	destID_ = destID;
	type_ = type;
	vehicle_ = vehicle;
}

void cEndMoveAction::execute()
{
	switch ( type_ )
	{
		case EMAT_LOAD:
			executeLoadAction();
			break;
		case EMAT_GET_IN:
			executeGetInAction();
			break;
		case EMAT_ATTACK:
			executeAttackAction();
			break;
	}
}

void cEndMoveAction::executeLoadAction()
{
	cVehicle* destVehicle = Server->getVehicleFromID( destID_ );
	if ( !destVehicle ) return;

	if ( vehicle_->canLoad ( destVehicle ) )
	{
		vehicle_->storeVehicle ( destVehicle, Server->Map );
		if ( destVehicle->ServerMoveJob ) destVehicle->ServerMoveJob->release();

		//vehicle is removed from enemy clients by cServer::checkPlayerUnits()
		sendStoreVehicle ( vehicle_->iID, true, destVehicle->iID, vehicle_->owner->Nr );
	}
}

void cEndMoveAction::executeGetInAction()
{
	cVehicle* destVehicle = Server->getVehicleFromID( destID_ );
	cBuilding* destBuilding = Server->getBuildingFromID( destID_ );

	// execute the loading if possible
	if ( destVehicle && destVehicle->canLoad ( vehicle_ ) )
	{
		destVehicle->storeVehicle ( vehicle_, Server->Map );
		if ( vehicle_->ServerMoveJob ) vehicle_->ServerMoveJob->release();
		//vehicle is removed from enemy clients by cServer::checkPlayerUnits()
		sendStoreVehicle ( destVehicle->iID, true, vehicle_->iID, destVehicle->owner->Nr );
	}
	else if ( destBuilding && destBuilding->canLoad ( vehicle_ ) )
	{
		destBuilding->storeVehicle ( vehicle_, Server->Map );
		if ( vehicle_->ServerMoveJob ) vehicle_->ServerMoveJob->release();
		//vehicle is removed from enemy clients by cServer::checkPlayerUnits()
		sendStoreVehicle ( destBuilding->iID, false, vehicle_->iID, destBuilding->owner->Nr );

	}

}

void cEndMoveAction::executeAttackAction()
{
	//get the target unit
	cVehicle* destVehicle = Server->getVehicleFromID( destID_ );
	cBuilding* destBuilding = Server->getBuildingFromID( destID_ );

	int x, y;
	if ( destVehicle )
	{
		x = destVehicle->PosX;
		y = destVehicle->PosY;
	}
	else if ( destBuilding )
	{
		x = destBuilding->PosX;
		y = destBuilding->PosY;
	}
	else
		return;

	int offset = x + y * Server->Map->size;

	//check, whether the attack is now possible
	if ( !vehicle_->canAttackObjectAt ( x, y, Server->Map, true, true ) ) return;

	//is the target in sight?
	if ( !vehicle_->owner->ScanMap[offset] ) return;

	Server->AJobs.Add( new cServerAttackJob( vehicle_, offset, false ));
}

cClientMoveJob::cClientMoveJob ( int iSrcOff, int iDestOff, cVehicle *Vehicle ) :
	Waypoints(NULL),
	lastWaypoints(NULL)
{
	DestX = iDestOff%Client->Map->size;
	DestY = iDestOff/Client->Map->size;
	init ( iSrcOff, Vehicle );
}

cClientMoveJob::cClientMoveJob ( int iSrcOff, sWaypoint *Waypoints, cVehicle *Vehicle )
{
	this->Waypoints = Waypoints;
	sWaypoint *nextWaypoint = Waypoints;
	while ( nextWaypoint )
	{
		if ( !nextWaypoint->next )
		{
			DestX = nextWaypoint->X;
			DestY = nextWaypoint->Y;
		}
		nextWaypoint = nextWaypoint->next;
	}

	if ( Waypoints ) calcNextDir();

	init ( iSrcOff, Vehicle );
}

void cClientMoveJob::init( int iSrcOff, cVehicle *Vehicle )
{
	if ( !Client ) return;

	Map = Client->Map;
	this->Vehicle = Vehicle;
	ScrX = iSrcOff%Map->size;
	ScrY = iSrcOff/Map->size;
	this->bPlane = (Vehicle->data.factorAir > 0);
	bFinished = false;
	bEndForNow = false;
	bSoundRunning = false;
	iSavedSpeed = 0;
	lastWaypoints = NULL;
	bSuspended = false;

	if ( Vehicle->ClientMoveJob )
	{
		Vehicle->ClientMoveJob->release();
		Vehicle->moving = false;
		Vehicle->MoveJobActive = false;
	}
	Vehicle->ClientMoveJob = this;
	endMoveAction = NULL;
}

cClientMoveJob::~cClientMoveJob()
{
	sWaypoint *NextWaypoint;
	while ( Waypoints )
	{
		NextWaypoint = Waypoints->next;
		delete Waypoints;
		Waypoints = NextWaypoint;
	}
	while ( lastWaypoints )
	{
		NextWaypoint = lastWaypoints->next;
		delete lastWaypoints;
		lastWaypoints = NextWaypoint;
	}

	for ( unsigned int i = 0; i < Client->ActiveMJobs.Size(); i++ )
	{
		if ( Client->ActiveMJobs[i] == this )
		{
			Client->ActiveMJobs.Delete(i);
			i--;
		}
	}
	if ( endMoveAction ) delete endMoveAction;
}

void cClientMoveJob::setVehicleToCoords(int x, int y, int height)
{
	if ( x == Waypoints->X && y == Waypoints->Y ) return;

	Log.write(" Client: mjob: setting vehicle " + iToStr(Vehicle->iID) + " to position " + iToStr(x) + " : " + iToStr(y), cLog::eLOG_TYPE_NET_DEBUG );
	//determine direction
	bool bForward = false;
	sWaypoint *Waypoint = Waypoints;
	while ( Waypoint )
	{
		if ( Waypoint->X == x && Waypoint->Y == y )
		{
			bForward = true;
			break;
		}
		Waypoint = Waypoint->next;
	}


	Map->moveVehicle( Vehicle, x, y, height );

	if ( bForward )
	{
		Waypoint = Waypoints;
		while ( Waypoint )
		{
			if ( Waypoint->X != x || Waypoint->Y != y )
			{
				Vehicle->DecSpeed( Waypoint->next->Costs );
				Waypoints = Waypoints->next;
				Waypoint->next = lastWaypoints;
				lastWaypoints = Waypoint;

				Waypoint = Waypoints;
			}
			else
			{
				break;
			}
		}
	}
	else
	{

		Waypoint = lastWaypoints;
		while ( Waypoint )
		{
			Vehicle->DecSpeed( -Waypoints->Costs );
			lastWaypoints = lastWaypoints->next;
			Waypoint->next = Waypoints;
			Waypoints = Waypoint;
			if ( Waypoint->X == x && Waypoint->Y == y )
			{
				break;
			}
			Waypoint = lastWaypoints;
		}
	}

	calcNextDir();
	Vehicle->owner->DoScan();
	Client->gameGUI.updateMouseCursor();
	Client->gameGUI.callMiniMapDraw();
	Vehicle->moving = false;
	Vehicle->OffX = Vehicle->OffY = 0;

}

bool cClientMoveJob::generateFromMessage( cNetMessage *message )
{
	if ( message->iType != GAME_EV_MOVE_JOB_SERVER ) return false;
	int iCount = 0;
	int iReceivedCount = message->popInt16();

	Log.write(" Client: Received MoveJob: VehicleID: " + iToStr( Vehicle->iID ) + ", SrcX: " + iToStr( ScrX ) + ", SrcY: " + iToStr( ScrY ) + ", DestX: " + iToStr( DestX ) + ", DestY: " + iToStr( DestY ) + ", WaypointCount: " + iToStr( iReceivedCount ), cLog::eLOG_TYPE_NET_DEBUG);

	// Add the waypoints
	while ( iCount < iReceivedCount )
	{
		sWaypoint* waypoint = new sWaypoint;
		waypoint->Y = message->popInt16();
		waypoint->X = message->popInt16();
		waypoint->Costs = message->popInt16();

		waypoint->next = Waypoints;
		Waypoints = waypoint;

		iCount++;
	}

	calcNextDir ();
	return true;
}

sWaypoint* cClientMoveJob::calcPath( int SrcX, int SrcY, int DestX, int DestY, cVehicle * vehicle, cList<cVehicle*> *group  )
{
	if (SrcX == DestX && SrcY == DestY) return 0;

	cPathCalculator PathCalculator( SrcX, SrcY, DestX, DestY, Client->Map, vehicle, group );
	sWaypoint* waypoints = PathCalculator.calcPath();

	return waypoints;
}

void cClientMoveJob::release()
{
	bEndForNow = false;
	bFinished = true;
	Log.write ( " Client: Released old movejob", cLog::eLOG_TYPE_NET_DEBUG );
	Client->addActiveMoveJob ( this );
	Log.write ( " Client: Added released movejob to avtive ones", cLog::eLOG_TYPE_NET_DEBUG );
}

void cClientMoveJob::handleNextMove( int iServerPositionX, int iServerPositionY, int iType, int iSavedSpeed, int height )
{
	// the client is faster than the server and has already
	// reached the last field or the next will be the last,
	// then stop the vehicle
	if ( Waypoints == NULL || Waypoints->next == NULL )
	{
		Log.write ( " Client: Client has already reached the last field", cLog::eLOG_TYPE_NET_DEBUG );
		bEndForNow = true;
		Vehicle->OffX = Vehicle->OffY = 0;
	}
	else
	{
		// check whether the destination field is one of the next in the waypointlist
		// if not it must have been one that has been deleted already
		bool bServerIsFaster = false;
		sWaypoint *Waypoint = Waypoints->next->next;
		while ( Waypoint )
		{
			if ( Waypoint->X == iServerPositionX && Waypoint->Y == iServerPositionY )
			{
				bServerIsFaster = true;
				break;
			}
			Waypoint = Waypoint->next;
		}

		if ( iServerPositionX == Vehicle->PosX && iServerPositionY == Vehicle->PosY )
		{
			//the server has allready finished the current movement step
			Log.write ( " Client: Server is one field faster than client", cLog::eLOG_TYPE_NET_DEBUG );
			if ( Vehicle->moving ) doEndMoveVehicle();
		}
		else if ( iServerPositionX == Waypoints->X && iServerPositionY == Waypoints->Y )
		{
			//the server is driving towards the same field as the client. So do nothing.
		}
		else if ( bServerIsFaster )
		{
			//the server is faster than the client. So set so server position.
			Log.write ( " Client: Server is more than one field faster", cLog::eLOG_TYPE_NET_DEBUG );
			if ( Vehicle->moving ) doEndMoveVehicle();
			setVehicleToCoords( iServerPositionX, iServerPositionY, height );
		}
		else
		{
			//the client is more than one field faster, than the server.
			//So wait, until the server reaches the current position.
			Log.write ( " Client: Client is faster (one or more fields) deactivating movejob; Vehicle-ID: " + iToStr ( Vehicle->iID ), cLog::eLOG_TYPE_NET_DEBUG );
			// just stop the vehicle and wait for the next commando of the server
			for ( unsigned int i = 0; i < Client->ActiveMJobs.Size(); i++ )
			{
				if ( Client->ActiveMJobs[i] == this ) Client->ActiveMJobs.Delete ( i );
			}
			if ( Vehicle->moving ) doEndMoveVehicle();
			bEndForNow = true;
			if ( iType == MJOB_OK ) return;
		}
	}

	switch ( iType )
	{
	case MJOB_OK:
		{
			if ( !Vehicle->MoveJobActive )
			{
				startMoveSound();
				Client->addActiveMoveJob( this );
				if ( Client->gameGUI.getSelVehicle() == Vehicle ) Client->gameGUI.unitMenuActive = false;
			}
			if ( bEndForNow )
			{
				bEndForNow = false;
				Client->addActiveMoveJob ( Vehicle->ClientMoveJob );
				Log.write ( " Client: reactivated movejob; Vehicle-ID: " + iToStr ( Vehicle->iID ), cLog::eLOG_TYPE_NET_DEBUG );
			}
			Vehicle->MoveJobActive = true;
		}
		break;
	case MJOB_STOP:
		{
			Log.write(" Client: The movejob will end for now", cLog::eLOG_TYPE_NET_DEBUG);
			if ( Vehicle->moving ) doEndMoveVehicle();
			setVehicleToCoords( iServerPositionX, iServerPositionY, height );
			if ( bEndForNow ) Client->addActiveMoveJob(this);
			this->iSavedSpeed = iSavedSpeed;
			Vehicle->data.speedCur = 0;
			bSuspended = true;
			bEndForNow = true;
		}
		break;
	case MJOB_FINISHED:
		{
			Log.write(" Client: The movejob is finished", cLog::eLOG_TYPE_NET_DEBUG);
			if ( Vehicle->moving ) doEndMoveVehicle();
			setVehicleToCoords( iServerPositionX, iServerPositionY, height );
			release ();
		}
		break;
	case MJOB_BLOCKED:
		{
			if ( Vehicle->moving ) doEndMoveVehicle();
			setVehicleToCoords( iServerPositionX, iServerPositionY, height );
			Log.write(" Client: next field is blocked: DestX: " + iToStr ( Waypoints->next->X ) + ", DestY: " + iToStr ( Waypoints->next->Y ), cLog::eLOG_TYPE_NET_DEBUG);

			if ( Vehicle->owner != Client->ActivePlayer )
			{
				bFinished = true;
				break;
			}

			sWaypoint* path = calcPath(Vehicle->PosX, Vehicle->PosY, DestX, DestY, Vehicle);
			if ( path )
			{
				sendMoveJob( path, Vehicle->iID );
				if ( endMoveAction ) sendEndMoveAction(Vehicle->iID, endMoveAction->destID_, endMoveAction->type_);
			}
			else
			{
				bFinished = true;

				if ( Vehicle == Client->gameGUI.getSelVehicle() )
				{
					if ( random(2) )
						PlayVoice ( VoiceData.VOINoPath1 );
					else
						PlayVoice ( VoiceData.VOINoPath2 );
				}
			}
		}
		break;
	}
}

void cClientMoveJob::moveVehicle()
{
	if ( Vehicle == NULL || Vehicle->ClientMoveJob != this ) return;

	// do not move the vehicle, if the movejob hasn't got any more waypoints
	if ( Waypoints == NULL || Waypoints->next == NULL )
	{
		stopMoveSound();
		return;
	}

	if (!Vehicle->moving)
	{
		//check remaining speed
		if ( Vehicle->data.speedCur < Waypoints->next->Costs )
		{
			bSuspended = true;
			bEndForNow = true;
			stopMoveSound();
			return;
		}

		Map->moveVehicle(Vehicle, Waypoints->next->X, Waypoints->next->Y );
		Vehicle->owner->DoScan();
		Vehicle->OffX = 0;
		Vehicle->OffY = 0;
		setOffset(Vehicle, iNextDir, -64 );
		Vehicle->moving = true;


		//restart movesound, when drinving into or out of water
		if ( Vehicle == Client->gameGUI.getSelVehicle() )
		{
			bool wasWater = Map->isWater( Waypoints->X, Waypoints->Y, true );
			bool water = Map->isWater( Waypoints->next->X, Waypoints->next->Y, true );

			if ( wasWater != water )
			{
				Vehicle->StartMoveSound();
			}
		}

	}

	int iSpeed;
	if ( Vehicle->data.animationMovement )
	{
		Vehicle->WalkFrame++;
		if ( Vehicle->WalkFrame >= 13 ) Vehicle->WalkFrame = 1;
		iSpeed = MOVE_SPEED/2;
	}
	else if ( !(Vehicle->data.factorAir > 0) && !(Vehicle->data.factorSea > 0 && Vehicle->data.factorGround == 0) )
	{
		iSpeed = MOVE_SPEED;
		cBuilding* building = Map->fields[Waypoints->next->X+Waypoints->next->Y*Map->size].getBaseBuilding();
		if ( Waypoints && Waypoints->next && building && building->data.modifiesSpeed ) iSpeed = (int)(iSpeed/building->data.modifiesSpeed);
	}
	else if ( Vehicle->data.factorAir > 0 ) iSpeed = MOVE_SPEED*2;
	else iSpeed = MOVE_SPEED;

	// Ggf Tracks malen:
	if ( cSettings::getInstance().isMakeTracks() && Vehicle->data.makeTracks && !Map->isWater ( Vehicle->PosX, Vehicle->PosY, false ) &&!
	        ( Waypoints && Waypoints->next && Map->terrain[Map->Kacheln[Waypoints->next->X+Waypoints->next->Y*Map->size]].water ) &&
	        ( Vehicle->owner == Client->ActivePlayer || Client->ActivePlayer->ScanMap[Vehicle->PosX+Vehicle->PosY*Map->size] ) )
	{
		if ( abs(Vehicle->OffX) == 64 || abs(Vehicle->OffY) == 64 )
		{
			switch ( Vehicle->dir )
			{
				case 0:
					Client->addFX ( fxTracks,Vehicle->PosX*64+Vehicle->OffX,Vehicle->PosY*64-10+Vehicle->OffY,0 );
					break;
				case 4:
					Client->addFX ( fxTracks,Vehicle->PosX*64+Vehicle->OffX,Vehicle->PosY*64+10+Vehicle->OffY,0 );
					break;
				case 2:
					Client->addFX ( fxTracks,Vehicle->PosX*64+10+Vehicle->OffX,Vehicle->PosY*64+Vehicle->OffY,2 );
					break;
				case 6:
					Client->addFX ( fxTracks,Vehicle->PosX*64-10+Vehicle->OffX,Vehicle->PosY*64+Vehicle->OffY,2 );
					break;
				case 1:
				case 5:
					Client->addFX ( fxTracks,Vehicle->PosX*64+Vehicle->OffX,Vehicle->PosY*64+Vehicle->OffY,1 );
					break;
				case 3:
				case 7:
					Client->addFX ( fxTracks,Vehicle->PosX*64+Vehicle->OffX,Vehicle->PosY*64+Vehicle->OffY,3 );
					break;
			}
		}
		else if ( abs ( Vehicle->OffX ) == 64-(iSpeed*2) || abs ( Vehicle->OffY ) == 64-(iSpeed*2) )
		{
			switch ( Vehicle->dir )
			{
				case 1:
					Client->addFX ( fxTracks,Vehicle->PosX*64+26+Vehicle->OffX,Vehicle->PosY*64-26+Vehicle->OffY,1 );
					break;
				case 5:
					Client->addFX ( fxTracks,Vehicle->PosX*64-26+Vehicle->OffX,Vehicle->PosY*64+26+Vehicle->OffY,1 );
					break;
				case 3:
					Client->addFX ( fxTracks,Vehicle->PosX*64+26+Vehicle->OffX,Vehicle->PosY*64+26+Vehicle->OffY,3 );
					break;
				case 7:
					Client->addFX ( fxTracks,Vehicle->PosX*64-26+Vehicle->OffX,Vehicle->PosY*64-26+Vehicle->OffY,3 );
					break;
			}
		}
	}

	setOffset(Vehicle, iNextDir, iSpeed);

	// check whether the point has been reached:
	if ( abs( Vehicle->OffX ) < iSpeed && abs( Vehicle->OffY ) < iSpeed )
	{
		Log.write(" Client: Vehicle reached the next field: ID: " + iToStr ( Vehicle->iID )+ ", X: " + iToStr ( Waypoints->next->X ) + ", Y: " + iToStr ( Waypoints->next->Y ), cLog::eLOG_TYPE_NET_DEBUG);
		doEndMoveVehicle ();
	}
}

void cClientMoveJob::doEndMoveVehicle ()
{
	if ( Vehicle == NULL || Vehicle->ClientMoveJob != this ) return;

	if ( Waypoints->next == NULL )
	{
		// this is just to avoid errors, this should normaly never happen.
		bFinished = true;
		return;
	}

	Vehicle->data.speedCur += iSavedSpeed;
	iSavedSpeed = 0;
	Vehicle->DecSpeed ( Waypoints->next->Costs );
	Vehicle->WalkFrame = 0;

	sWaypoint *Waypoint = Waypoints;
	Waypoints = Waypoints->next;
	Waypoint->next = lastWaypoints;
	lastWaypoints = Waypoint;

	Vehicle->moving = false;

	Vehicle->OffX = 0;
	Vehicle->OffY = 0;

	Client->gameGUI.callMiniMapDraw();
	Client->gameGUI.updateMouseCursor();

	calcNextDir();
}

void cClientMoveJob::calcNextDir ()
{
	if ( !Waypoints || !Waypoints->next ) return;
#define TESTXY_CND(a,b) if( Waypoints->X a Waypoints->next->X && Waypoints->Y b Waypoints->next->Y )
	TESTXY_CND ( ==,> ) iNextDir = 0;
	else TESTXY_CND ( <,> ) iNextDir = 1;
	else TESTXY_CND ( <,== ) iNextDir = 2;
	else TESTXY_CND ( <,< ) iNextDir = 3;
	else TESTXY_CND ( ==,< ) iNextDir = 4;
	else TESTXY_CND ( >,< ) iNextDir = 5;
	else TESTXY_CND ( >,== ) iNextDir = 6;
	else TESTXY_CND ( >,> ) iNextDir = 7;
}

void cClientMoveJob::drawArrow ( SDL_Rect Dest, SDL_Rect *LastDest, bool bSpezial )
{
	int iIndex = -1;
#define TESTXY_DP(a,b) if( Dest.x a LastDest->x && Dest.y b LastDest->y )
	TESTXY_DP ( >,< ) iIndex = 0;
	else TESTXY_DP ( ==,< ) iIndex = 1;
	else TESTXY_DP ( <,< ) iIndex = 2;
	else TESTXY_DP ( >,== ) iIndex = 3;
	else TESTXY_DP ( <,== ) iIndex = 4;
	else TESTXY_DP ( >,> ) iIndex = 5;
	else TESTXY_DP ( ==,> ) iIndex = 6;
	else TESTXY_DP ( <,> ) iIndex = 7;

	if ( iIndex == -1 ) return;

	if ( bSpezial )
	{
		SDL_BlitSurface ( OtherData.WayPointPfeileSpecial[iIndex][64-Client->gameGUI.getTileSize()], NULL, buffer, &Dest );
	}
	else
	{
		SDL_BlitSurface ( OtherData.WayPointPfeile[iIndex][64-Client->gameGUI.getTileSize()], NULL, buffer, &Dest );
	}
}

void cClientMoveJob::startMoveSound()
{
	if ( Vehicle == Client->gameGUI.getSelVehicle()) Vehicle->StartMoveSound();
	bSoundRunning = true;
}

void cClientMoveJob::stopMoveSound()
{
	if ( !bSoundRunning ) return;

	bSoundRunning = false;

	if ( Vehicle == Client->gameGUI.getSelVehicle() )
	{
		cBuilding* building = Client->Map->fields[Vehicle->PosX+Vehicle->PosY*Client->Map->size].getBaseBuilding();
		bool water = Client->Map->isWater ( Vehicle->PosX, Vehicle->PosY, true );
		if ( Vehicle->data.factorGround > 0 && building && ( building->data.surfacePosition == sUnitData::SURFACE_POS_BASE || building->data.surfacePosition == sUnitData::SURFACE_POS_ABOVE_BASE || building->data.surfacePosition == sUnitData::SURFACE_POS_ABOVE_SEA ) ) water = false;

		StopFXLoop ( Client->iObjectStream );
		if ( water && Vehicle->data.factorSea > 0 ) PlayFX ( Vehicle->typ->StopWater );
		else PlayFX ( Vehicle->typ->Stop );

		Client->iObjectStream = Vehicle->playStream();
	}
}
