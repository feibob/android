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

#include <math.h>
#include "attackJobs.h"
#include "server.h"
#include "client.h"
#include "serverevents.h"
#include "clientevents.h"
#include "netmessage.h"
#include "settings.h"
#include "vehicles.h"
#include "buildings.h"
#include "player.h"

using namespace std;

//--------------------------------------------------------------------------
void selectTarget( cVehicle*& targetVehicle, cBuilding*& targetBuilding, int x, int y, char attackMode, cMap* map)
{
	targetVehicle = NULL;
	targetBuilding = NULL;
	int offset = x + y * map->size;

	if ( (attackMode & TERRAIN_AIR) && (attackMode & TERRAIN_GROUND) )
	{
		targetVehicle = (*map)[offset].getPlanes();

		if ( !targetVehicle ) targetVehicle = (*map)[offset].getVehicles();
		if ( targetVehicle && (targetVehicle->data.isStealthOn&TERRAIN_SEA) && map->isWater(x, y, true) ) targetVehicle = NULL;

		if ( !targetVehicle ) targetBuilding = (*map)[offset].getBuildings();
	}
	else if ( (attackMode & TERRAIN_GROUND) && (attackMode & AREA_SUB) )
	{
		targetVehicle = (*map)[offset].getPlanes();
		if ( targetVehicle && targetVehicle->FlightHigh > 0 ) targetVehicle = NULL;

		if ( !targetVehicle ) targetVehicle = (*map)[offset].getVehicles();

		if ( !targetVehicle ) targetBuilding = (*map)[offset].getBuildings();
	}
	else if ( attackMode & TERRAIN_GROUND )
	{
		targetVehicle = (*map)[offset].getPlanes();
		if ( targetVehicle && targetVehicle->FlightHigh > 0 ) targetVehicle = NULL;

		if ( !targetVehicle ) targetVehicle = (*map)[offset].getVehicles();
		if ( targetVehicle && (targetVehicle->data.isStealthOn&TERRAIN_SEA) && map->isWater(x, y, true) ) targetVehicle = NULL;

		if ( !targetVehicle ) targetBuilding = (*map)[offset].getBuildings();
	}
	else if ( attackMode & TERRAIN_AIR )
	{
		targetVehicle = (*map)[offset].getPlanes();
		if ( targetVehicle && targetVehicle->FlightHigh == 0 ) targetVehicle = NULL;
	}

	//check for rubble
	if ( targetBuilding && !targetBuilding->owner )
		targetBuilding = NULL;

}



//--------------------------------------------------------------------------
// cServerAttackJob Implementation
//--------------------------------------------------------------------------

//--------------------------------------------------------------------------
int cServerAttackJob::iNextID = 0;

//--------------------------------------------------------------------------
cServerAttackJob::cServerAttackJob (cUnit* _unit, int targetOff, bool sentry)
{
	unit = _unit;

	iID = iNextID;
	iNextID++;
	bMuzzlePlayed = false;
	iTargetOff = targetOff;
	damage = unit->data.damage;
	attackMode = unit->data.canAttack;
	sentryFire = sentry;

	iMuzzleType = unit->data.muzzleType;
	iAgressorOff = unit->PosX + unit->PosY * Server->Map->size;

	//lock targets
	if (unit->data.muzzleType == sUnitData::MUZZLE_TYPE_ROCKET_CLUSTER)
		lockTargetCluster ();
	else
		lockTarget (targetOff);

	unit->data.shotsCur--;
	unit->data.ammoCur--;
	if (unit->isVehicle () && unit->data.canDriveAndFire == false)
		unit->data.speedCur -= (int)(((float)unit->data.speedMax) / unit->data.shotsMax);
	unit->attacking = true;

	sendFireCommand ();

	if (unit->isBuilding () && unit->data.explodesOnContact )
	{
		Server->deleteUnit (unit, false);
		unit = 0;
	}
}

//--------------------------------------------------------------------------
cServerAttackJob::~cServerAttackJob ()
{
	if (unit != 0)
		unit->attacking = false;
}

//--------------------------------------------------------------------------
void cServerAttackJob::lockTarget (int offset)
{
	//make sure, that the unit data has been send to all clients
	Server->checkPlayerUnits ();

	cVehicle* targetVehicle;
	cBuilding* targetBuilding;
	selectTarget (targetVehicle, targetBuilding, offset%Server->Map->size, offset/Server->Map->size, attackMode, Server->Map);
	if (targetVehicle)
		targetVehicle->isBeeingAttacked = true;

	bool isAir = (targetVehicle && targetVehicle->data.factorAir > 0);

	//if the agressor can attack air and land units, decide whether it is currently attacking air or land targets
	if ( (attackMode & TERRAIN_AIR) && (attackMode & TERRAIN_GROUND) )
	{
		if (isAir)
			//TODO: can alien units attack submarines?
			attackMode = TERRAIN_AIR;
		else
			attackMode = TERRAIN_GROUND;
	}

	if ( !isAir )
	{
		cBuildingIterator buildings = (*Server->Map)[offset].getBuildings();
		while ( !buildings.end )
		{
			targetBuilding = buildings;
			buildings->isBeeingAttacked = true;
			buildings++;
		}
	}

	if (targetVehicle == 0 && targetBuilding == 0)
		return;

	//change offset, to match the upper left field of big vehicles
	if (targetVehicle != 0 && targetVehicle->data.isBig)
		offset = targetVehicle->PosX + targetVehicle->PosY * Server->Map->size;

	for (unsigned int i = 0; i < Server->PlayerList->Size (); i++)
	{
		cPlayer* player = (*Server->PlayerList)[i];

		//targed in sight?
		if (player->ScanMap[offset] == 0)
			continue;

		cNetMessage* message = new cNetMessage (GAME_EV_ATTACKJOB_LOCK_TARGET);
		if (targetVehicle != 0)
		{
			message->pushChar (targetVehicle->OffX);
			message->pushChar (targetVehicle->OffY);
			message->pushInt32 (targetVehicle->iID);
		}
		else
		{
			//ID 0 for 'no vehicle'
			message->pushInt32 (0);
		}
		message->pushInt32 (offset);
		message->pushBool (isAir);

		Server->sendNetMessage (message, player->Nr);
	}
}

//--------------------------------------------------------------------------
void cServerAttackJob::lockTargetCluster ()
{
	int PosX = iTargetOff % Server->Map->size;
	int PosY = iTargetOff / Server->Map->size;

	for (int x = PosY - 2; x <= PosY + 2; x++)
	{
		if (x < 0 || x >= Server->Map->size)
			continue;
		for (int y = PosY - 2; y <= PosY + 2; y++)
		{
			if (y < 0 || y >= Server->Map->size)
				continue;
			if (abs (PosX - x) + abs (PosY - y) > 2)
				continue;
			lockTarget (x + y * Server->Map->size);
		}
	}
}

//--------------------------------------------------------------------------
void cServerAttackJob::sendFireCommand ()
{
	if (unit == 0)
		return;

	//make the agressor visible on all clients who can see the agressor offset
	for (unsigned int i = 0; i < Server->PlayerList->Size (); i++)
	{
		cPlayer* player = (*Server->PlayerList)[i];
		if (player->ScanMap[iAgressorOff] == false)
			continue;

		if (unit->owner == player)
			continue;

		unit->setDetectedByPlayer (player);
	}
	Server->checkPlayerUnits ();

	//calculate fire direction
	int targetX = iTargetOff % Server->Map->size;
	int targetY = iTargetOff / Server->Map->size;
	int agressorX = iAgressorOff % Server->Map->size;
	int agressorY = iAgressorOff / Server->Map->size;

	float dx = (float)targetX - (float)agressorX;
	float dy =(float)-( targetY - agressorY );
	float r = sqrt ( dx*dx + dy*dy );

	int fireDir = unit->dir;
	if (r <= 0.001)
	{
		//do not rotate agressor
	}
	else
	{
		dx /= r;
		dy /= r;
		r = (float)(asin (dx) * 57.29577951);
		if (dy >= 0)
		{
			if (r < 0)
				r += 360;
		}
		else
			r = 180 - r;

		if (r >= 337.5 || r <= 22.5) fireDir = 0;
		else if (r >= 22.5  && r <= 67.5)  fireDir = 1;
		else if (r >= 67.5  && r <= 112.5) fireDir = 2;
		else if (r >= 112.5 && r <= 157.5) fireDir = 3;
		else if (r >= 157.5 && r <= 202.5) fireDir = 4;
		else if (r >= 202.5 && r <= 247.5) fireDir = 5;
		else if (r >= 247.5 && r <= 292.5) fireDir = 6;
		else if (r >= 292.5 && r <= 337.5) fireDir = 7;
	}
	unit->dir = fireDir;

	// send the fire message to all clients who can see the attack
	for (unsigned int i = 0; i < Server->PlayerList->Size (); i++)
	{
		cPlayer* player = (*Server->PlayerList)[i];

		if (player->ScanMap[iAgressorOff]
			|| (player->ScanMap[iTargetOff] && isMuzzleTypeRocket ()))
		{
			sendFireCommand (player);
			executingClients.Add (player);
		}
	}
}

//--------------------------------------------------------------------------
void cServerAttackJob::sendFireCommand (cPlayer* player)
{
	if (unit == 0)
		return;

	cNetMessage* message = new cNetMessage (GAME_EV_ATTACKJOB_FIRE);

	message->pushBool (sentryFire);
	message->pushInt16 (unit->data.speedCur);
	message->pushInt16 (unit->data.ammoCur);
	message->pushInt16 (unit->data.shotsCur);
	message->pushChar (unit->dir);
	if (isMuzzleTypeRocket ())
		message->pushInt32 (iTargetOff);

	if (player->ScanMap[iAgressorOff])
		message->pushInt32 (unit->iID);
	else
	{
		//when the agressor is out of sight, send the position and muzzle type to the client
		message->pushInt32 (iAgressorOff);
		message->pushChar (unit->data.muzzleType);
		message->pushInt32 (0);
	}
	message->pushInt16 (iID);

	Server->sendNetMessage (message, player->Nr);
}

//--------------------------------------------------------------------------
bool cServerAttackJob::isMuzzleTypeRocket () const
{
	return ((iMuzzleType == sUnitData::MUZZLE_TYPE_ROCKET)
			|| (iMuzzleType == sUnitData::MUZZLE_TYPE_TORPEDO)
			|| (iMuzzleType == sUnitData::MUZZLE_TYPE_ROCKET_CLUSTER));
}

//--------------------------------------------------------------------------
void cServerAttackJob::clientFinished (int playerNr)
{
	for (unsigned int i = 0; i < executingClients.Size (); i++)
	{
		if (executingClients[i]->Nr == playerNr)
			executingClients.Delete (i);
	}

	Log.write (" Server: waiting for " + iToStr ((int)executingClients.Size ()) + " clients", cLog::eLOG_TYPE_NET_DEBUG);

	if (executingClients.Size () == 0)
	{
		if (unit && unit->data.muzzleType == sUnitData::MUZZLE_TYPE_ROCKET_CLUSTER )
			makeImpactCluster ();
		else
			makeImpact (iTargetOff % Server->Map->size, iTargetOff / Server->Map->size);
	}
}

//--------------------------------------------------------------------------
void cServerAttackJob::makeImpact (int x, int y)
{
	if (x < 0 || x >= Server->Map->size || y < 0 || y >= Server->Map->size)
		return;
	int offset = x + y * Server->Map->size;

	cVehicle* targetVehicle = 0;
	cBuilding* targetBuilding = 0;
	selectTarget (targetVehicle, targetBuilding, x, y, attackMode, Server->Map);

	//check, whether the target is allready in the target list.
	//this is needed, to make sure, that a cluster attack doesn't hit the same unit multible times
	for (unsigned int i = 0; i < vehicleTargets.Size (); i++)
	{
		if (vehicleTargets[i] == targetVehicle)
			return;
	}
	for (unsigned int i = 0; i < buildingTargets.Size (); i++)
	{
		if (buildingTargets[i] == targetBuilding)
			return;
	}
	if (targetVehicle != 0)
		vehicleTargets.Add (targetVehicle );
	if (targetBuilding != 0)
		buildingTargets.Add (targetBuilding);

	int remainingHP = 0;
	int id = 0;
	cPlayer* owner = 0;
	bool isAir = (targetVehicle != 0 && targetVehicle->data.factorAir > 0);

	// in the time between the first locking and the impact, it is possible that a vehicle drove onto the target field
	// so relock the target, to ensure synchronity
	if (targetVehicle != 0 && targetVehicle->isBeeingAttacked == false)
	{
		Log.write (" Server: relocking target", cLog::eLOG_TYPE_NET_DEBUG);
		lockTarget (offset);
	}

	// if target found, make the impact
	cUnit* targetUnit = ((targetVehicle != 0) ? (cUnit*)targetVehicle : (cUnit*)targetBuilding);
	if (targetUnit != 0)
	{
		// if taget is a stealth unit, make it visible on all clients
		if (targetUnit->data.isStealthOn != TERRAIN_NONE)
		{
			for (unsigned int i = 0; i < Server->PlayerList->Size (); i++)
			{
				cPlayer* player = (*Server->PlayerList)[i];
				if (targetUnit->owner == player)
					continue;
				if (!player->ScanMap[offset])
					continue;
				targetUnit->setDetectedByPlayer (player);
			}
			Server->checkPlayerUnits ();
		}

		id = targetUnit->iID;
		targetUnit->data.hitpointsCur = targetUnit->calcHealth (damage);
		remainingHP = targetUnit->data.hitpointsCur;
		targetUnit->hasBeenAttacked = true;
		owner = targetUnit->owner;
		Log.write (" Server: unit '" + targetUnit->getDisplayName () + "' (ID: " + iToStr (targetUnit->iID) + ") hit. Remaining HP: " + iToStr (targetUnit->data.hitpointsCur), cLog::eLOG_TYPE_NET_DEBUG);
	}

	//workaround
	//make sure, the owner gets the impact message
	if (owner)
		owner->ScanMap[offset] = 1;

	sendAttackJobImpact (offset, remainingHP, id);

	//remove the destroyed units
	if (targetBuilding && targetBuilding->data.hitpointsCur <= 0)
	{
		Server->destroyUnit (targetBuilding);
		targetBuilding = 0;
	}
	else if (targetVehicle && targetVehicle->data.hitpointsCur <= 0)
	{
		Server->destroyUnit (targetVehicle);
		targetVehicle = 0;
	}

	//attack finished. reset attacking and isBeeingAttacked flags
	if (targetVehicle)
		targetVehicle->isBeeingAttacked = false;

	if (isAir == false)
	{
		cBuildingIterator buildings = (*Server->Map)[offset].getBuildings ();
		while (buildings.end == false)
		{
			buildings->isBeeingAttacked = false;
			buildings++;
		}
	}
	if (unit)
		unit->attacking = false;

	//check whether a following sentry mode attack is possible
	if (targetVehicle)
		targetVehicle->InSentryRange ();
	//check whether the agressor itself is in sentry range
	if (unit && unit->isVehicle ())
		static_cast<cVehicle*>(unit)->InSentryRange ();
}

//--------------------------------------------------------------------------
void cServerAttackJob::makeImpactCluster()
{
	int clusterDamage = damage;
	int PosX = iTargetOff % Server->Map->size;
	int PosY = iTargetOff / Server->Map->size;

	//full damage
	makeImpact (PosX, PosY);

	// 3/4 damage
	damage = (clusterDamage * 3) / 4;
	makeImpact (PosX - 1, PosY    );
	makeImpact (PosX + 1, PosY    );
	makeImpact (PosX    , PosY - 1);
	makeImpact (PosX    , PosY + 1);

	// 1/2 damage
	damage = clusterDamage / 2;
	makeImpact (PosX + 1, PosY + 1);
	makeImpact (PosX + 1, PosY - 1);
	makeImpact (PosX - 1, PosY + 1);
	makeImpact (PosX - 1, PosY - 1);

	// 1/3 damage
	damage = clusterDamage / 3;
	makeImpact (PosX - 2, PosY    );
	makeImpact (PosX + 2, PosY    );
	makeImpact (PosX    , PosY - 2);
	makeImpact (PosX    , PosY + 2);

}

//--------------------------------------------------------------------------
void cServerAttackJob::sendAttackJobImpact (int offset, int remainingHP, int id)
{
	for (unsigned int i = 0; i < Server->PlayerList->Size (); i++)
	{
		cPlayer* player = (*Server->PlayerList)[i];

		//targed in sight?
		if (player->ScanMap[offset] == 0)
			continue;

		cNetMessage* message = new cNetMessage (GAME_EV_ATTACKJOB_IMPACT);
		message->pushInt32 (offset);
		message->pushInt16 (remainingHP);
		message->pushInt16 (id);

		Server->sendNetMessage (message, player->Nr);
	}
}



//--------------------------------------------------------------------------
// cClientAttackJob implementation
//--------------------------------------------------------------------------

//--------------------------------------------------------------------------
void cClientAttackJob::lockTarget( cNetMessage* message )
{
	bool bIsAir = message->popBool();
	int offset = message->popInt32();
	int x = offset % Client->Map->size;
	int y = offset / Client->Map->size;
	int ID = message->popInt32();
	if ( ID != 0 )
	{
		cVehicle* vehicle = Client->getVehicleFromID( ID );
		if ( vehicle == NULL )
		{
			Log.write(" Client: vehicle with ID " + iToStr(ID) + " not found", cLog::eLOG_TYPE_NET_ERROR );
			return;	//we are out of sync!!!
		}

		vehicle->isBeeingAttacked = true;

		//synchonize position
		if ( vehicle->PosX != x || vehicle->PosY != y )
		{
			Log.write(" Client: changed vehicle position to (" + iToStr( x ) + ":" + iToStr( y ) + ")", cLog::eLOG_TYPE_NET_DEBUG );
			Client->Map->moveVehicle( vehicle, x, y );
			vehicle->owner->DoScan();

			vehicle->OffY = message->popChar();
			vehicle->OffX = message->popChar();
		}
	}
	if ( !bIsAir )
	{
		cBuildingIterator buildings = (*Client->Map)[offset].getBuildings();
		while ( !buildings.end )
		{
			buildings->isBeeingAttacked = true;
			buildings++;
		}
	}
}

//--------------------------------------------------------------------------
void cClientAttackJob::handleAttackJobs()
{
	for (unsigned int i = 0; i < Client->attackJobs.Size(); i++)
	{
		cClientAttackJob* job = Client->attackJobs[i];
		switch ( job->state )
		{
		case FINISHED:
			{
				job->sendFinishMessage();
				if (job->vehicle)  job->vehicle->attacking  = false;
				if (job->building) job->building->attacking = false;
				delete job;
				Client->attackJobs.Delete(i);
				break;
			}
		case PLAYING_MUZZLE:
			{
				job->playMuzzle();
				break;
			}
		case ROTATING:
			{
				job->rotate();
				break;
			}

		}
	}
}

//--------------------------------------------------------------------------
cClientAttackJob::cClientAttackJob( cNetMessage* message )
{
	state = ROTATING;
	wait = 0;
	this->iID = message->popInt16();
	iTargetOffset = -1;
	vehicle = NULL;
	building = NULL;

	//check for duplicate jobs
	for ( unsigned int i = 0; i < Client->attackJobs.Size(); i++)
	{
		if ( Client->attackJobs[i]->iID == this->iID )
		{
			state = FINISHED;
			return;
		}
	}

	int unitID = message->popInt32();

	//get muzzle type and agressor position
	if ( unitID == 0 )
	{
		iMuzzleType = message->popChar();
		if ( iMuzzleType != sUnitData::MUZZLE_TYPE_ROCKET && iMuzzleType != sUnitData::MUZZLE_TYPE_ROCKET_CLUSTER && iMuzzleType != sUnitData::MUZZLE_TYPE_TORPEDO )
		{
			state = FINISHED;
			return;
		}
		iAgressorOffset = message->popInt32();
	}
	else
	{
		vehicle  = Client->getVehicleFromID( unitID );
		building = Client->getBuildingFromID( unitID );
		if ( !vehicle && !building )
		{
			state = FINISHED;
			Log.write(" Client: agressor with id " + iToStr( unitID ) + " not found", cLog::eLOG_TYPE_NET_ERROR );
			return; //we are out of sync!!!
		}

		iMuzzleType = vehicle ? vehicle->data.muzzleType : building->data.muzzleType;

		if ( vehicle )
		{
			iAgressorOffset = vehicle->PosX + vehicle->PosY * Client->Map->size;
		}
		else
		{
			iAgressorOffset = building->PosX + building->PosY * Client->Map->size;
		}
	}

	if ( iMuzzleType == sUnitData::MUZZLE_TYPE_ROCKET || iMuzzleType == sUnitData::MUZZLE_TYPE_ROCKET_CLUSTER || iMuzzleType == sUnitData::MUZZLE_TYPE_TORPEDO )
	{
		iTargetOffset = message->popInt32();
	}
	iFireDir = message->popChar();

	//get remaining shots, ammo and movement points
	if ( vehicle )
	{
		vehicle->data.shotsCur = message->popInt16();
		vehicle->data.ammoCur = message->popInt16();
		vehicle->data.speedCur = message->popInt16();
		vehicle->attacking = true;
	}
	else if ( building )
	{
		building->data.shotsCur = message->popInt16();
		building->data.ammoCur = message->popInt16();
		building->attacking = true;
	}

	bool sentryReaction = message->popBool();
	if ( sentryReaction && ( (building && building->owner == Client->ActivePlayer) || (vehicle && vehicle->owner == Client->ActivePlayer) ))
	{
		int x = vehicle?vehicle->PosX:building->PosX;
		int y = vehicle?vehicle->PosY:building->PosY;
		string name = vehicle?vehicle->getDisplayName():building->getDisplayName();
		sID id = vehicle?vehicle->data.ID:building->data.ID;
		Client->ActivePlayer->addSavedReport ( Client->addCoords( lngPack.i18n("Text~Comp~AttackingEnemy", name), x, y ), sSavedReportMessage::REPORT_TYPE_UNIT, id, x, y );
		if (random(2))
			PlayVoice(VoiceData.VOIAttackingEnemy1 );
		else
			PlayVoice(VoiceData.VOIAttackingEnemy2 );
	}
	Client->gameGUI.checkMouseInputMode();
}

//--------------------------------------------------------------------------
void cClientAttackJob::rotate()
{
	if ( vehicle )
	{
		if ( vehicle->dir != iFireDir )
		{
			vehicle->rotateTo( iFireDir );
		}
		else
		{
			state = PLAYING_MUZZLE;
		}
	}
	else if ( building )
	{
		if ( building->dir != iFireDir && !building->data.explodesOnContact )
		{
			building->rotateTo( iFireDir );
		}
		else
		{
			state = PLAYING_MUZZLE;
		}
	}
	else
	{
		state = PLAYING_MUZZLE;
	}
}

//--------------------------------------------------------------------------
void cClientAttackJob::playMuzzle()
{

	int offx=0,offy=0;

	if ( building && building->data.explodesOnContact )
	{
		state = FINISHED;
		PlayFX ( building->typ->Attack );
		if ( Client->Map->isWater( building->PosX, building->PosY) )
		{
			Client->addFX( fxExploWater, building->PosX*64 + 32, building->PosY*64 + 32, 0);
		}
		else
		{
			Client->addFX( fxExploSmall, building->PosX*64 + 32, building->PosY*64 + 32, 0);
		}
		Client->deleteUnit( building );
		return;
	}

	switch ( iMuzzleType )
	{
		case sUnitData::MUZZLE_TYPE_BIG:
			if ( wait++!=0 )
			{
				if ( wait>2 ) state = FINISHED;
				return;
			}
			switch ( iFireDir )
			{
				case 0:
					offy-=40;
					break;
				case 1:
					offx+=32;
					offy-=32;
					break;
				case 2:
					offx+=40;
					break;
				case 3:
					offx+=32;
					offy+=32;
					break;
				case 4:
					offy+=40;
					break;
				case 5:
					offx-=32;
					offy+=32;
					break;
				case 6:
					offx-=40;
					break;
				case 7:
					offx-=32;
					offy-=32;
					break;
			}
			if ( vehicle )
			{
				Client->addFX ( fxMuzzleBig,vehicle->PosX*64+offx,vehicle->PosY*64+offy,iFireDir );
			}
			else if ( building )
			{
				Client->addFX ( fxMuzzleBig,building->PosX*64+offx,building->PosY*64+offy,iFireDir );
			}
			break;
		case sUnitData::MUZZLE_TYPE_SMALL:
			if ( wait++!=0 )
			{
				if ( wait>2 ) state = FINISHED;
				return;
			}
			if ( vehicle )
			{
				Client->addFX ( fxMuzzleSmall,vehicle->PosX*64,vehicle->PosY*64,iFireDir );
			}
			else if ( building )
			{
				Client->addFX ( fxMuzzleSmall,building->PosX*64,building->PosY*64,iFireDir );
			}
			break;
		case sUnitData::MUZZLE_TYPE_ROCKET:
		case sUnitData::MUZZLE_TYPE_ROCKET_CLUSTER:
		{
			if ( wait++!=0 ) return;

			int PosX = iAgressorOffset%Client->Map->size;
			int PosY = iAgressorOffset/Client->Map->size;
			Client->addFX ( fxRocket, PosX*64, PosY*64, this, iTargetOffset, iFireDir );

			break;
		}
		case sUnitData::MUZZLE_TYPE_MED:
		case sUnitData::MUZZLE_TYPE_MED_LONG:
			if ( wait++!=0 )
			{
				if ( wait>2 ) state = FINISHED;
				return;
			}
			switch ( iFireDir )
			{
				case 0:
					offy-=20;
					break;
				case 1:
					offx+=12;
					offy-=12;
					break;
				case 2:
					offx+=20;
					break;
				case 3:
					offx+=12;
					offy+=12;
					break;
				case 4:
					offy+=20;
					break;
				case 5:
					offx-=12;
					offy+=12;
					break;
				case 6:
					offx-=20;
					break;
				case 7:
					offx-=12;
					offy-=12;
					break;
			}
			if ( iMuzzleType == sUnitData::MUZZLE_TYPE_MED )
			{
				if ( vehicle )
				{
					Client->addFX ( fxMuzzleMed,vehicle->PosX*64+offx,vehicle->PosY*64+offy,iFireDir );
				}
				else if ( building )
				{
					Client->addFX ( fxMuzzleMed,building->PosX*64+offx,building->PosY*64+offy,iFireDir );
				}
			}
			else
			{
				if ( vehicle )
				{
					Client->addFX ( fxMuzzleMedLong,vehicle->PosX*64+offx,vehicle->PosY*64+offy,iFireDir );
				}
				else if ( building )
				{
					Client->addFX ( fxMuzzleMedLong,building->PosX*64+offx,building->PosY*64+offy,iFireDir );
				}
			}
			break;
		case sUnitData::MUZZLE_TYPE_TORPEDO:
		{
			if ( wait++!=0 ) return;

			int PosX = iAgressorOffset%Client->Map->size;
			int PosY = iAgressorOffset/Client->Map->size;
			Client->addFX ( fxTorpedo, PosX*64, PosY*64, this, iTargetOffset, iFireDir );

			break;
		}
		case sUnitData::MUZZLE_TYPE_SNIPER:
			state = FINISHED;
			break;
	}
	if ( vehicle )
	{
		PlayFX ( vehicle->typ->Attack );
	}
	else if ( building )
	{
		PlayFX ( building->typ->Attack );
	}

}

//--------------------------------------------------------------------------
void cClientAttackJob::sendFinishMessage()
{
	cNetMessage* message = new cNetMessage( GAME_EV_ATTACKJOB_FINISHED );
	message->pushInt16( iID );
	Client->sendNetMessage( message );
}

//--------------------------------------------------------------------------
void cClientAttackJob::makeImpact(int offset, int remainingHP, int id )
{
	if ( offset < 0 || offset > Client->Map->size * Client->Map->size )
	{
		Log.write(" Client: Invalid offset", cLog::eLOG_TYPE_NET_ERROR );
		return;
	}

	cVehicle* targetVehicle = Client->getVehicleFromID( id );
	cBuilding* targetBuilding = Client->getBuildingFromID( id );

	bool playImpact = false;
	bool ownUnit = false;
	bool destroyed = false;
	bool isAir = false;
	string name;
	int offX = 0, offY = 0;
	sID unitID;

	//no target found
	if ( !targetBuilding && !targetVehicle )
	{
		playImpact = true;
	}
	else
	{

		if ( targetVehicle )
		{
			unitID = targetVehicle->data.ID;
			isAir = ( targetVehicle->data.factorAir > 0 );
			targetVehicle->data.hitpointsCur = remainingHP;

			Log.write(" Client: vehicle '" + targetVehicle->getDisplayName() + "' (ID: " + iToStr(targetVehicle->iID) + ") hit. Remaining HP: " + iToStr(targetVehicle->data.hitpointsCur), cLog::eLOG_TYPE_NET_DEBUG );

			name = targetVehicle->getDisplayName();
			if ( targetVehicle->owner == Client->ActivePlayer ) ownUnit = true;

			if (targetVehicle->data.hitpointsCur <= 0)
			{
				Client->destroyUnit( targetVehicle );
				targetVehicle = NULL;
				destroyed = true;
			}
			else
			{
				playImpact = true;
				offX = targetVehicle->OffX;
				offY = targetVehicle->OffY;
			}
		}
		else
		{

			unitID = targetBuilding->data.ID;
			targetBuilding->data.hitpointsCur = remainingHP;

			Log.write(" Client: building '" + targetBuilding->getDisplayName() + "' (ID: " + iToStr(targetBuilding->iID) + ") hit. Remaining HP: " + iToStr(targetBuilding->data.hitpointsCur), cLog::eLOG_TYPE_NET_DEBUG );

			name = targetBuilding->getDisplayName();
			if ( targetBuilding->owner == Client->ActivePlayer ) ownUnit = true;

			if ( targetBuilding->data.hitpointsCur <= 0 )
			{
				Client->destroyUnit( targetBuilding );
				targetBuilding = NULL;
				destroyed = true;
			}
			else
			{
				playImpact = true;
			}
		}
		Client->gameGUI.updateMouseCursor();
	}

	int x = offset % Client->Map->size;
	int y = offset / Client->Map->size;

	if ( playImpact && cSettings::getInstance().isAlphaEffects() )
	{
		// TODO:  PlayFX ( SoundData.hit );
		Client->addFX( fxHit, x*64 + offX, y*64 + offY, 0);
	}

	string message;
	if ( ownUnit )
	{
		if ( destroyed )
		{
			message = name + " " + lngPack.i18n("Text~Comp~Destroyed");
			PlayVoice( VoiceData.VOIDestroyedUs );
		}
		else
		{
			message = name + " " + lngPack.i18n("Text~Comp~Attacked");
			int i = random(3);
			if (i==0)
				PlayVoice( VoiceData.VOIAttackingUs );
			else if ( i == 1)
				PlayVoice( VoiceData.VOIAttackingUs2 );
			else
				PlayVoice( VoiceData.VOIAttackingUs3 );
		}
		Client->ActivePlayer->addSavedReport ( Client->addCoords( message, x, y ), sSavedReportMessage::REPORT_TYPE_UNIT, unitID, x, y );
	}

	//clean up
	if ( targetVehicle ) targetVehicle->isBeeingAttacked = false;

	if ( !isAir )
	{
		cBuildingIterator buildings = (*Client->Map)[offset].getBuildings();
		while ( !buildings.end )
		{
			buildings->isBeeingAttacked = false;
			buildings++;
		}
	}
}
