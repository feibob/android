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
#include "player.h"
#include "client.h"
#include "server.h"
#include "serverevents.h"
#include "hud.h"
#include "buildings.h"
#include "vehicles.h"

using namespace std;

//-----------------------------------------------------------------------
// Implementation cPlayer class
//-----------------------------------------------------------------------

//-----------------------------------------------------------------------
cPlayer::cPlayer(string Name, SDL_Surface* Color, int nr, int iSocketNum) :
	name(Name),
	color(Color),
	Nr(nr),
	numEcos(0),
	lastDeletedUnit(0),
	clan(-1)
{
	// copy the vehicle stats
	VehicleData = new sUnitData[UnitsData.getNrVehicles ()];
	for (unsigned int i = 0; i < UnitsData.getNrVehicles (); i++)
		VehicleData[i] = UnitsData.getVehicle (i).data; // get the default (no clan) vehicle data

	// copy the building stats
	BuildingData = new sUnitData[UnitsData.getNrBuildings ()];
	for (unsigned int i = 0; i < UnitsData.getNrBuildings (); i++)
		BuildingData[i] = UnitsData.getBuilding (i).data; // get the default (no clan) building data

	DetectLandMap = NULL;
	DetectSeaMap = NULL;
	DetectMinesMap = NULL;
	ScanMap = NULL;
	SentriesMapAir = NULL;
	SentriesMapGround = NULL;
	VehicleList = NULL;
	BuildingList = NULL;
	ResourceMap = NULL;

	ResearchCount = 0;
	for (int i = 0; i < cResearch::kNrResearchAreas; i++)
		researchCentersWorkingOnArea[i] = 0;
	Credits=0;

	this->iSocketNum = iSocketNum;
	isDefeated = false;
	isRemovedFromGame = false;
	bFinishedTurn = false;

	savedHud = new sHudStateContainer;
	researchFinished = false;
}

//-----------------------------------------------------------------------
cPlayer::cPlayer(const cPlayer &Player)
{
	name = Player.name;
	color = Player.color;
	Nr = Player.Nr;
	iSocketNum = Player.iSocketNum;
	clan = Player.clan;
	pointsHistory = Player.pointsHistory;
	numEcos = Player.numEcos;
	lastDeletedUnit = Player.lastDeletedUnit;

	// copy vehicle and building datas
	VehicleData = new sUnitData[UnitsData.getNrVehicles ()];
	for ( unsigned int i = 0; i < UnitsData.getNrVehicles (); i++)
		VehicleData[i] = Player.VehicleData[i];
	BuildingData = new sUnitData[UnitsData.getNrBuildings ()];
	for ( unsigned int i = 0; i < UnitsData.getNrBuildings (); i++)
		BuildingData[i] = Player.BuildingData[i];

	DetectLandMap = NULL;
	DetectSeaMap = NULL;
	DetectMinesMap = NULL;
	ScanMap = NULL;
	SentriesMapAir = NULL;
	SentriesMapGround = NULL;
	VehicleList = NULL;
	BuildingList = NULL;
	ResourceMap = NULL;

	Credits = Player.Credits;
	ResearchCount = Player.ResearchCount;
	for (int i = 0; i < cResearch::kNrResearchAreas; i++)
		researchCentersWorkingOnArea[i] = Player.researchCentersWorkingOnArea[i];
	for (int i = 0; i < cResearch::kNrResearchAreas; i++)
	{
		researchLevel.setCurResearchLevel(Player.researchLevel.getCurResearchLevel(i), i);
		researchLevel.setCurResearchPoints(Player.researchLevel.getCurResearchPoints(i), i);
	}
	reportResearchAreasFinished = Player.reportResearchAreasFinished;

	this->iSocketNum = iSocketNum;
	isDefeated = false;
	bFinishedTurn = Player.bFinishedTurn;

	savedHud = new sHudStateContainer;

	researchFinished = Player.researchFinished;
}

//-----------------------------------------------------------------------
cPlayer::~cPlayer ()
{
	// Erst alle geladenen Vehicles lˆschen:
	cVehicle *ptr=VehicleList;
	while ( ptr )
	{
		if ( ptr->storedUnits.Size() )
		{
			ptr->deleteStoredUnits ();
		}
		ptr = (cVehicle*)ptr->next;
	}
	// Jetzt alle Vehicles lˆschen:
	while ( VehicleList )
	{
		cVehicle *ptr;
		ptr = (cVehicle*)VehicleList->next;
		VehicleList->sentryActive = false;
		delete VehicleList;
		VehicleList=ptr;
	}
	while ( BuildingList )
	{
		cBuilding *ptr;
		ptr = (cBuilding*)BuildingList->next;
		BuildingList->sentryActive = false;

		// Stored Vehicles are already deleted; just clear the list
		while( BuildingList->storedUnits.Size() > 0 )
		{
			BuildingList->storedUnits.Delete( BuildingList->storedUnits.Size() - 1 );
		}

		delete BuildingList;
		BuildingList=ptr;
	}
	delete [] VehicleData;
	delete [] BuildingData;
	if ( ScanMap ) delete [] ScanMap;
	if ( SentriesMapAir ) delete [] SentriesMapAir;
	if ( SentriesMapGround ) delete [] SentriesMapGround;
	if ( ResourceMap ) delete [] ResourceMap;

	if ( DetectLandMap ) delete [] DetectLandMap;
	if ( DetectSeaMap ) delete [] DetectSeaMap;
	if ( DetectMinesMap ) delete [] DetectMinesMap;

	while ( ReportVehicles.Size() )
	{
		delete ReportVehicles[0];
		ReportVehicles.Delete ( 0 );
	}
	while ( ReportBuildings.Size() )
	{
		delete ReportBuildings[0];
		ReportBuildings.Delete ( 0 );
	}
	while ( LockList.Size() )
	{
		delete LockList[0];
		LockList.Delete ( 0 );
	}

	delete savedHud;
}

//--------------------------------------------------------------------------
void cPlayer::setClan (int newClan)
{
	if (newClan == clan || newClan < -1 || 7 < newClan)
		return;

	clan = newClan;

	for (unsigned int i = 0; i < UnitsData.getNrVehicles (); i++)
		VehicleData[i] = UnitsData.getVehicle (i, clan).data;

	for (unsigned int i = 0; i < UnitsData.getNrBuildings (); i++)
		BuildingData[i] = UnitsData.getBuilding (i, clan).data;
}

//--------------------------------------------------------------------------
/** Adds the vehicle to the list of the player */
//--------------------------------------------------------------------------
cVehicle *cPlayer::AddVehicle (int posx, int posy, sVehicle *v)
{
	cVehicle *n;

	n=new cVehicle ( v,this );
	n->PosX=posx;
	n->PosY=posy;
	n->prev=NULL;
	if ( VehicleList!=NULL )
	{
		VehicleList->prev=n;
	}
	n->next=VehicleList;
	VehicleList=n;
	drawSpecialCircle ( n->PosX,n->PosY,n->data.scan,ScanMap, (int)sqrt ( (double)MapSize ) );
	if ( n->data.canDetectStealthOn&TERRAIN_GROUND ) drawSpecialCircle ( n->PosX, n->PosY, n->data.scan, DetectLandMap, (int)sqrt ( (double)MapSize ) );
	if ( n->data.canDetectStealthOn&TERRAIN_SEA  ) drawSpecialCircle ( n->PosX, n->PosY, n->data.scan, DetectSeaMap, (int)sqrt ( (double)MapSize )  );
	if ( n->data.canDetectStealthOn&AREA_EXP_MINE )
	{
		for ( int x = n->PosX - 1; x <= n->PosX + 1; x++ )
		{
			if ( x < 0 || x >= (int)sqrt ( (double)MapSize ) ) continue;
			for ( int y = n->PosY - 1; y <= n->PosY + 1; y++ )
			{
				if ( y < 0 || y >= (int)sqrt ( (double)MapSize ) ) continue;
				DetectMinesMap[x + (int)sqrt ( (double)MapSize )*y] = 1;
			}
		}
	}
	return n;
}

//--------------------------------------------------------------------------
/** initialize the maps */
//--------------------------------------------------------------------------
void cPlayer::InitMaps ( int MapSizeX, cMap *map )
{
	MapSize=MapSizeX*MapSizeX;
	// Scanner-Map:
	ScanMap = new char[MapSize];
	memset ( ScanMap,0,MapSize );
	// Ressource-Map
	ResourceMap = new char[MapSize];
	memset ( ResourceMap,0,MapSize );

	base.map = map;
	// Sentry-Map:
	SentriesMapAir = new char[MapSize];
	memset ( SentriesMapAir,0,MapSize );
	SentriesMapGround = new char[MapSize];
	memset ( SentriesMapGround,0,MapSize );

	// Detect-Maps:
	DetectLandMap = new char[MapSize];
	memset ( DetectLandMap,0,MapSize );
	DetectSeaMap = new char[MapSize];
	memset ( DetectSeaMap,0,MapSize );
	DetectMinesMap = new char[MapSize];
	memset ( DetectMinesMap, 0, MapSize );
}

//--------------------------------------------------------------------------
/** Adds the building to the list of the player */
//--------------------------------------------------------------------------
cBuilding *cPlayer::addBuilding ( int posx, int posy, sBuilding *b )
{
	cBuilding *Building;

	Building = new cBuilding ( b, this, &base );
	Building->PosX = posx;
	Building->PosY = posy;
	Building->prev = NULL;
	if ( BuildingList != NULL )
	{
		BuildingList->prev = Building;
	}
	Building->next = BuildingList;
	BuildingList = Building;
	if ( Building->data.scan )
	{
		if ( Building->data.isBig ) drawSpecialCircleBig ( Building->PosX, Building->PosY, Building->data.scan, ScanMap, (int)sqrt ( (double)MapSize ) );
		else drawSpecialCircle ( Building->PosX, Building->PosY, Building->data.scan, ScanMap, (int)sqrt ( (double)MapSize ) );
	}
	return Building;
}

//--------------------------------------------------------------------------
void cPlayer::addSentry ( cUnit *u )
{
	u->sentryActive = true;
	if ( u->data.canAttack & TERRAIN_AIR )
	{
		drawSpecialCircle ( u->PosX, u->PosY, u->data.range, SentriesMapAir, (int)sqrt ( (double)MapSize ) );
	}
	if ( ( u->data.canAttack & TERRAIN_GROUND ) || ( u->data.canAttack & TERRAIN_SEA ) )
	{
		drawSpecialCircle ( u->PosX, u->PosY, u->data.range, SentriesMapGround, (int)sqrt ( (double)MapSize ) );
	}
}



//--------------------------------------------------------------------------
void cPlayer::deleteSentry ( cUnit *u )
{
	u->sentryActive = false;
	if ( u->data.canAttack & TERRAIN_AIR )
	{
		refreshSentryAir();
	}
	else if ( (u->data.canAttack & TERRAIN_GROUND) || (u->data.canAttack & TERRAIN_SEA) )
	{
		refreshSentryGround();
	}
}

//--------------------------------------------------------------------------
void cPlayer::refreshSentryAir ()
{
	memset ( SentriesMapAir,0,MapSize );
	cUnit* unit = (cUnit*) VehicleList;
	while (unit)
	{
		if ( unit->sentryActive && unit->data.canAttack & TERRAIN_AIR )
		{
			drawSpecialCircle ( unit->PosX, unit->PosY, unit->data.range, SentriesMapAir, (int)sqrt ( (double)MapSize ) );
		}
		unit = unit->next;
	}
	unit = (cUnit*) BuildingList;
	while (unit)
	{
		if ( unit->sentryActive && unit->data.canAttack & TERRAIN_AIR )
		{
			drawSpecialCircle ( unit->PosX, unit->PosY, unit->data.range, SentriesMapAir, (int)sqrt ( (double)MapSize ) );
		}
		unit = unit->next;
	}
}

//--------------------------------------------------------------------------
void cPlayer::refreshSentryGround ()
{
	memset ( SentriesMapGround,0,MapSize );
	cUnit* unit = (cUnit*) VehicleList;
	while (unit)
	{
		if ( unit->sentryActive && ((unit->data.canAttack & TERRAIN_GROUND) || (unit->data.canAttack & TERRAIN_SEA)) )
		{
			drawSpecialCircle ( unit->PosX, unit->PosY, unit->data.range, SentriesMapGround, (int)sqrt ( (double)MapSize ) );
		}
		unit = unit->next;
	}
	unit = (cUnit*) BuildingList;
	while (unit)
	{
		if ( unit->sentryActive && ((unit->data.canAttack & TERRAIN_GROUND) || (unit->data.canAttack & TERRAIN_SEA)) )
		{
			drawSpecialCircle ( unit->PosX, unit->PosY, unit->data.range, SentriesMapGround, (int)sqrt ( (double)MapSize ) );
		}
		unit = unit->next;
	}
}

//--------------------------------------------------------------------------
/** Does a scan for all units of the player */
//--------------------------------------------------------------------------
void cPlayer::DoScan ()
{
	cVehicle *vp;
	cBuilding *bp;

	if ( isDefeated ) return;
	memset ( ScanMap,       0, MapSize );
	memset ( DetectLandMap, 0, MapSize );
	memset ( DetectSeaMap,  0, MapSize );
	memset ( DetectMinesMap,0, MapSize );

	// iterate the vehicle list
	vp = VehicleList;
	while ( vp )
	{
		if ( vp->Loaded )
		{
			vp = (cVehicle*)vp->next;
			continue;
		}

		if ( vp->turnsDisabled )
			ScanMap[vp->PosX+vp->PosY*(int)sqrt ( (double)MapSize )] = 1;
		else
		{
			if ( vp->data.isBig )
				drawSpecialCircleBig ( vp->PosX, vp->PosY, vp->data.scan, ScanMap, (int)sqrt ( (double)MapSize ) );
			else
				drawSpecialCircle ( vp->PosX,vp->PosY,vp->data.scan,ScanMap, (int)sqrt ( (double)MapSize ) );

			//detection maps
			if ( vp->data.canDetectStealthOn&TERRAIN_GROUND ) drawSpecialCircle ( vp->PosX, vp->PosY, vp->data.scan, DetectLandMap, (int)sqrt ( (double)MapSize ) );
			else if ( vp->data.canDetectStealthOn&TERRAIN_SEA ) drawSpecialCircle ( vp->PosX, vp->PosY, vp->data.scan, DetectSeaMap, (int)sqrt ( (double)MapSize ) );
			if ( vp->data.canDetectStealthOn&AREA_EXP_MINE )
			{
				for ( int x = vp->PosX - 1; x <= vp->PosX + 1; x++ )
				{
					if ( x < 0 || x >= (int)sqrt ( (double)MapSize ) )
						continue;
					for ( int y = vp->PosY - 1; y <= vp->PosY + 1; y++ )
					{
						if ( y < 0 || y >= (int)sqrt ( (double)MapSize ) )
							continue;
						DetectMinesMap[x + (int)sqrt ( (double)MapSize )*y] = 1;
					}
				}
			}
		}
		vp = (cVehicle*)vp->next;
	}

	// iterate the building list
	bp = BuildingList;
	while ( bp )
	{

		if ( bp->turnsDisabled )
			ScanMap[bp->PosX+bp->PosY*(int)sqrt ( (double)MapSize )]=1;
		else
		{
			if ( bp->data.scan )
			{
				if ( bp->data.isBig )
					drawSpecialCircleBig ( bp->PosX,bp->PosY,bp->data.scan,ScanMap, (int)sqrt ( (double)MapSize ) );
				else
					drawSpecialCircle ( bp->PosX,bp->PosY,bp->data.scan,ScanMap, (int)sqrt ( (double)MapSize ) );
			}
		}
		bp = (cBuilding*)bp->next;
	}
}


//--------------------------------------------------------------------------
/** Returns the next unit that can still fire/shoot */
//--------------------------------------------------------------------------
cUnit *cPlayer::getNextUnit ()
{
	//find the first unit to look at
	cUnit *start = Client->gameGUI.getSelectedUnit ();
	if (start)
	{
		if (start->next)
		{
			start = start->next;
		}
		else
		{
			if (start->isBuilding ())
				start = VehicleList;
			else
				start = BuildingList;
		}
	}
	if (!start || start->owner != this)
	{
		start = VehicleList;
	}
	if (!start)
	{
		start = BuildingList;
	}
	if (!start)
	{
		return NULL;
	}


	cUnit *unit = start;
	do
	{
		//check if this unit is the next one to be selected
		if (unit->isBuilding ())
		{
			cBuilding* building = (cBuilding*) unit;
			if ( !building->isMarkedAsDone && !building->IsWorking && !building->sentryActive && ( !building->data.canBuild.empty() || building->data.shotsCur || building->data.canMineMaxRes > 0 || building->data.convertsGold > 0 || building->data.canResearch))
				return unit;
		}
		else
		{
			cVehicle* vehicle = (cVehicle*) unit;
			if ( !vehicle->isMarkedAsDone && (!vehicle->IsBuilding || vehicle->BuildRounds == 0) && !vehicle->IsClearing && !vehicle->sentryActive && !vehicle->Loaded && ( vehicle->data.speedCur || vehicle->data.shotsCur ))
				return unit;
		}

		//otherwise get next unit
		if (unit->next)
		{
			unit = unit->next;
		}
		else
		{
			if (unit->isBuilding ())
			{
				unit = VehicleList ? (cUnit*)VehicleList : (cUnit*)BuildingList;
			}
			else
			{
				unit = BuildingList ? (cUnit*)BuildingList : (cUnit*)VehicleList;
			}
		}
	}
	while (unit != start);

	//when no unit is found, go to mining station
	unit = BuildingList;
	while (unit)
	{
		if (unit->data.canMineMaxRes > 0)
			return unit;
		unit = unit->next;
	}

	return NULL;
}

//--------------------------------------------------------------------------
/** Returns the previous vehicle, that can still move / shoot */
//--------------------------------------------------------------------------
cUnit *cPlayer::getPrevUnit ()
{
	//find the first unit to look at
	cUnit *start = Client->gameGUI.getSelectedUnit ();
	if (start)
	{
		if (start->prev)
		{
			start = start->prev;
		}
		else
		{
			if (start->isBuilding ())
			{
				start = VehicleList;
				while (start && start->next)
					start = start->next;
			}
			else
			{
				start = BuildingList;
				while (start && start->next)
					start = start->next;
			}
		}
	}
	if (!start || start->owner != this)
	{
		start = VehicleList;
		while (start && start->next)
			start = start->next;
	}
	if (!start)
	{
		start = BuildingList;
		while (start && start->next)
			start = start->next;
	}
	if (!start)
	{
		return NULL;
	}


	cUnit *unit = start;
	do
	{
		//check if this unit is the next one to be selected
		if (unit->isBuilding ())
		{
			cBuilding* building = (cBuilding*) unit;
			if ( !building->isMarkedAsDone && !building->IsWorking && !building->sentryActive && ( !building->data.canBuild.empty() || building->data.shotsCur || building->data.canMineMaxRes > 0 || building->data.convertsGold > 0 || building->data.canResearch))
				return unit;
		}
		else
		{
			cVehicle* vehicle = (cVehicle*) unit;
			if ( !vehicle->isMarkedAsDone && (!vehicle->IsBuilding || vehicle->BuildRounds == 0) && !vehicle->IsClearing && !vehicle->sentryActive && !vehicle->Loaded && ( vehicle->data.speedCur || vehicle->data.shotsCur ))
				return unit;
		}

		//otherwise get next unit
		if (unit->prev)
		{
			unit = unit->prev;
		}
		else
		{
			if (unit->isBuilding ())
			{
				unit = VehicleList ? (cUnit*)VehicleList : (cUnit*)BuildingList;
				while (unit && unit->next)
					unit = unit->next;
			}
			else
			{
				unit = BuildingList ? (cUnit*)BuildingList : (cUnit*)VehicleList;
				while (unit && unit->next)
					unit = unit->next;
			}
		}
	}
	while (unit != start);

	//when no unit is found, go to mining station
	unit = BuildingList;
	while (unit)
	{
		if (unit->data.canMineMaxRes > 0)
			return unit;
		unit = unit->next;
	}

	return NULL;
}

//--------------------------------------------------------------
/** Starts a research center. */
//--------------------------------------------------------------
void cPlayer::startAResearch (int researchArea)
{
	if (0 <= researchArea && researchArea <= cResearch::kNrResearchAreas)
	{
		ResearchCount++;
		researchCentersWorkingOnArea[researchArea] += 1;
	}
}

//--------------------------------------------------------------
/** Stops a research center. */
//--------------------------------------------------------------
void cPlayer::stopAResearch (int researchArea)
{
	if (0 <= researchArea && researchArea <= cResearch::kNrResearchAreas)
	{
		ResearchCount--;
		if (researchCentersWorkingOnArea[researchArea] > 0)
			researchCentersWorkingOnArea[researchArea] -= 1;
	}
}

//--------------------------------------------------------------
/** At turnend update the research level */
//--------------------------------------------------------------
void cPlayer::doResearch()
{
	bool researchFinished = false;
	cList<sUnitData*> upgradedUnitDatas;
	cList<int> areasReachingNextLevel;
	reportResearchAreasFinished.Reserve (0); // clear
	for (int area = 0; area < cResearch::kNrResearchAreas; area++)
	{
		if (researchCentersWorkingOnArea[area] > 0)
		{
			if (researchLevel.doResearch(researchCentersWorkingOnArea[area], area))
			{ // next level reached
				areasReachingNextLevel.Add(area);
				reportResearchAreasFinished.Add(area);
				researchFinished = true;
			}
		}
	}
	if (researchFinished)
	{
		upgradeUnitTypes (areasReachingNextLevel, upgradedUnitDatas);

		for (unsigned int i = 0; i < upgradedUnitDatas.Size(); i++)
			sendUnitUpgrades (upgradedUnitDatas[i], Nr);
	}
	sendResearchLevel (&researchLevel, Nr);
}

void cPlayer::accumulateScore()
{
	const int now = Server->getTurn();
	int deltaScore = 0;

	for(cBuilding *bp = BuildingList; bp; bp = (cBuilding*)bp->next)
	{
		if ( bp->typ->data.canScore && bp->IsWorking )
		{
			bp->points ++;
			deltaScore ++;

			sendUnitScore(bp);
		}
	}
	setScore(getScore(now) + deltaScore, now);
	sendScore(this, now);
}

void cPlayer::CountEcoSpheres()
{
	numEcos = 0;

	for(cBuilding *bp = BuildingList; bp; bp = (cBuilding*)bp->next)
	{
		if ( bp->typ->data.canScore && bp->IsWorking )
			numEcos ++;
	}
}

void cPlayer::setScore(int s, int turn)
{
	unsigned int t = turn ? turn : (Client ? Client->iTurn : 1);

	if(pointsHistory.size() < t)
		pointsHistory.resize(t);
	pointsHistory[t - 1] = s;
}

void cPlayer::clearDone()
{
	cUnit *unit = VehicleList;
	while (unit)
	{
		unit->isMarkedAsDone = false;
		unit = unit->next;
	}

	unit = BuildingList;
	while (unit)
	{
		unit->isMarkedAsDone = false;
		unit = unit->next;
	}
}

int cPlayer::getScore(int turn) const
{
	unsigned int t = turn;

	if(pointsHistory.size() < t)
	{
		int score = pointsHistory.empty() ?
			0 : pointsHistory[pointsHistory.size() - 1];
		pointsHistory.resize(t);
		pointsHistory[t - 1] = score;
	}
	return pointsHistory[t - 1];
}

//--------------------------------------------------------------
void cPlayer::upgradeUnitTypes (cList<int>& areasReachingNextLevel, cList<sUnitData*>& resultUpgradedUnitDatas)
{
	for (unsigned int i = 0; i < UnitsData.getNrVehicles (); i++)
	{
		bool incrementVersion = false;
		for (unsigned int areaCounter = 0; areaCounter < areasReachingNextLevel.Size(); areaCounter++)
		{
			int researchArea = areasReachingNextLevel[areaCounter];
			int newResearchLevel = researchLevel.getCurResearchLevel (researchArea);

			int startValue = 0;
			switch (researchArea)
			{
				case cResearch::kAttackResearch: startValue = UnitsData.getVehicle (i, getClan ()).data.damage; break;
				case cResearch::kShotsResearch: startValue = UnitsData.getVehicle (i, getClan ()).data.shotsMax; break;
				case cResearch::kRangeResearch: startValue = UnitsData.getVehicle (i, getClan ()).data.range; break;
				case cResearch::kArmorResearch: startValue = UnitsData.getVehicle (i, getClan ()).data.armor; break;
				case cResearch::kHitpointsResearch: startValue = UnitsData.getVehicle (i, getClan ()).data.hitpointsMax; break;
				case cResearch::kScanResearch: startValue = UnitsData.getVehicle (i, getClan ()).data.scan; break;
				case cResearch::kSpeedResearch: startValue = UnitsData.getVehicle (i, getClan ()).data.speedMax; break;
				case cResearch::kCostResearch: startValue = UnitsData.getVehicle (i, getClan ()).data.buildCosts; break;
			}
			int oldResearchBonus = cUpgradeCalculator::instance().calcChangeByResearch(startValue, newResearchLevel - 10,
																					   researchArea == cResearch::kCostResearch ? cUpgradeCalculator::kCost : -1,
																					   UnitsData.getVehicle (i, getClan ()).data.isHuman ? cUpgradeCalculator::kInfantry : cUpgradeCalculator::kStandardUnit);
			int newResearchBonus = cUpgradeCalculator::instance().calcChangeByResearch(startValue, newResearchLevel,
																					   researchArea == cResearch::kCostResearch ? cUpgradeCalculator::kCost : -1,
																					   UnitsData.getVehicle (i, getClan ()).data.isHuman ? cUpgradeCalculator::kInfantry : cUpgradeCalculator::kStandardUnit);
			if (oldResearchBonus != newResearchBonus)
			{
				switch (researchArea)
				{
					case cResearch::kAttackResearch: VehicleData[i].damage += newResearchBonus - oldResearchBonus; break;
					case cResearch::kShotsResearch: VehicleData[i].shotsMax += newResearchBonus - oldResearchBonus; break;
					case cResearch::kRangeResearch: VehicleData[i].range += newResearchBonus - oldResearchBonus; break;
					case cResearch::kArmorResearch: VehicleData[i].armor += newResearchBonus - oldResearchBonus; break;
					case cResearch::kHitpointsResearch: VehicleData[i].hitpointsMax += newResearchBonus - oldResearchBonus; break;
					case cResearch::kScanResearch: VehicleData[i].scan += newResearchBonus - oldResearchBonus; break;
					case cResearch::kSpeedResearch: VehicleData[i].speedMax += newResearchBonus - oldResearchBonus; break;
					case cResearch::kCostResearch: VehicleData[i].buildCosts += newResearchBonus - oldResearchBonus; break;
				}
				if (researchArea != cResearch::kCostResearch) // don't increment the version, if the only change are the costs
					incrementVersion = true;
				if (!resultUpgradedUnitDatas.Contains(&(VehicleData[i])))
					resultUpgradedUnitDatas.Add(&(VehicleData[i]));
			}
		}
		if (incrementVersion)
			VehicleData[i].version += 1;
	}

	for (unsigned int i = 0; i < UnitsData.getNrBuildings (); i++)
	{
		bool incrementVersion = false;
		for (unsigned int areaCounter = 0; areaCounter < areasReachingNextLevel.Size(); areaCounter++)
		{
			int researchArea = areasReachingNextLevel[areaCounter];
			int newResearchLevel = researchLevel.getCurResearchLevel (researchArea);

			int startValue = 0;
			switch (researchArea)
			{
				case cResearch::kAttackResearch: startValue = UnitsData.getBuilding (i, getClan ()).data.damage; break;
				case cResearch::kShotsResearch: startValue = UnitsData.getBuilding (i, getClan ()).data.shotsMax; break;
				case cResearch::kRangeResearch: startValue = UnitsData.getBuilding (i, getClan ()).data.range; break;
				case cResearch::kArmorResearch: startValue = UnitsData.getBuilding (i, getClan ()).data.armor; break;
				case cResearch::kHitpointsResearch: startValue = UnitsData.getBuilding (i, getClan ()).data.hitpointsMax; break;
				case cResearch::kScanResearch: startValue = UnitsData.getBuilding (i, getClan ()).data.scan; break;
				case cResearch::kCostResearch: startValue = UnitsData.getBuilding (i, getClan ()).data.buildCosts; break;
			}
			int oldResearchBonus = cUpgradeCalculator::instance().calcChangeByResearch(startValue, newResearchLevel - 10,
																					   researchArea == cResearch::kCostResearch ? cUpgradeCalculator::kCost : -1,
																					   cUpgradeCalculator::kBuilding);
			int newResearchBonus = cUpgradeCalculator::instance().calcChangeByResearch(startValue, newResearchLevel,
																					   researchArea == cResearch::kCostResearch ? cUpgradeCalculator::kCost : -1,
																					   cUpgradeCalculator::kBuilding);
			if (oldResearchBonus != newResearchBonus)
			{
				switch (researchArea)
				{
					case cResearch::kAttackResearch: BuildingData[i].damage += newResearchBonus - oldResearchBonus; break;
					case cResearch::kShotsResearch: BuildingData[i].shotsMax += newResearchBonus - oldResearchBonus; break;
					case cResearch::kRangeResearch: BuildingData[i].range += newResearchBonus - oldResearchBonus; break;
					case cResearch::kArmorResearch: BuildingData[i].armor += newResearchBonus - oldResearchBonus; break;
					case cResearch::kHitpointsResearch: BuildingData[i].hitpointsMax += newResearchBonus - oldResearchBonus; break;
					case cResearch::kScanResearch: BuildingData[i].scan += newResearchBonus - oldResearchBonus; break;
					case cResearch::kCostResearch: BuildingData[i].buildCosts += newResearchBonus - oldResearchBonus; break;
				}
				if (researchArea != cResearch::kCostResearch) // don't increment the version, if the only change are the costs
					incrementVersion = true;
				if (!resultUpgradedUnitDatas.Contains(&(BuildingData[i])))
					resultUpgradedUnitDatas.Add(&(BuildingData[i]));
			}
		}
		if (incrementVersion)
			BuildingData[i].version += 1;
	}
}

//------------------------------------------------------------
void cPlayer::refreshResearchCentersWorkingOnArea()
{
	int newResearchCount = 0;
	for (int i = 0; i < cResearch::kNrResearchAreas; i++)
		researchCentersWorkingOnArea[i] = 0;
	cBuilding* curBuilding = BuildingList;
	while (curBuilding)
	{
		if (curBuilding->data.canResearch && curBuilding->IsWorking)
		{
			researchCentersWorkingOnArea[curBuilding->researchArea] += 1;
			newResearchCount++;
		}
		curBuilding = (cBuilding*)curBuilding->next;
	}
	ResearchCount = newResearchCount;
}

//------------------------------------------------------------
/** Adds a building to the lock list (the range/scan of the building will remain displayed, even if it is unselected). */
//------------------------------------------------------------
void cPlayer::AddLock (cBuilding *b)
{
	sLockElem *elem;
	elem = new sLockElem;
	elem->b = b;
	elem->v = NULL;
	b->IsLocked = true;
	LockList.Add (elem);
}

//------------------------------------------------------------
/** Adds a vehicle to the lock list (the range/scan of the vehicle will remain displayed, even if it is unselected). */
//------------------------------------------------------------
void cPlayer::AddLock (cVehicle *v)
{
	sLockElem *elem;
	elem = new sLockElem;
	elem->v = v;
	elem->b = NULL;
	v->IsLocked = true;
	LockList.Add (elem);
}

//------------------------------------------------------------
/** Removes a vehicle from the lock list. */
//------------------------------------------------------------
void cPlayer::DeleteLock (cVehicle *v)
{
	sLockElem *elem;
	for (unsigned int i = 0; i < LockList.Size(); i++)
	{
		elem = LockList[i];
		if (elem->v == v)
		{
			v->IsLocked=false;
			delete elem;
			LockList.Delete (i);
			return;
		}
	}
}

//------------------------------------------------------------
/** Removes a building from the lock list. */
//------------------------------------------------------------
void cPlayer::DeleteLock (cBuilding *b)
{
	sLockElem *elem;
	for (unsigned int i = 0; i < LockList.Size(); i++)
	{
		elem = LockList[i];
		if (elem->b == b)
		{
			b->IsLocked = false;
			delete elem;
			LockList.Delete (i);
			return;
		}
	}
}

//------------------------------------------------------------
/** Checks if the building is contained in the lock list. */
//------------------------------------------------------------
bool cPlayer::InLockList (cBuilding *b)
{
	sLockElem *elem;
	for (unsigned int i = 0; i < LockList.Size(); i++)
	{
		elem = LockList[i];
		if (elem->b == b)
			return true;
	}
	return false;
}

//------------------------------------------------------------
/** Checks if the vehicle is contained in the lock list. */
//------------------------------------------------------------
bool cPlayer::InLockList (cVehicle *v)
{
	sLockElem *elem;
	for (unsigned int i = 0; i < LockList.Size(); i++)
	{
		elem = LockList[i];
		if (elem->v == v)
			return true;
	}
	return false;
}

//--------------------------------------------------------------------------
/** Toggles the lock state of a unit under the mouse (when locked it's range and scan is displayed, although the unit is not selected). */
//--------------------------------------------------------------------------
void cPlayer::ToggelLock ( cMapField *OverUnitField )
{
	if ( OverUnitField->getBaseBuilding() && OverUnitField->getBaseBuilding()->owner!=this )
	{
		if ( InLockList ( OverUnitField->getBaseBuilding() ) ) DeleteLock ( OverUnitField->getBaseBuilding() );
		else AddLock ( OverUnitField->getBaseBuilding() );
	}
	if ( OverUnitField->getTopBuilding() && OverUnitField->getTopBuilding()->owner!=this )
	{
		if ( InLockList ( OverUnitField->getTopBuilding() ) ) DeleteLock ( OverUnitField->getTopBuilding() );
		else AddLock ( OverUnitField->getTopBuilding() );
	}
	if ( OverUnitField->getVehicles() && OverUnitField->getVehicles()->owner!=this )
	{
		if ( InLockList ( OverUnitField->getVehicles() ) ) DeleteLock ( OverUnitField->getVehicles() );
		else AddLock ( OverUnitField->getVehicles() );
	}
	if ( OverUnitField->getPlanes() && OverUnitField->getPlanes()->owner!=this )
	{
		if ( InLockList ( OverUnitField->getPlanes() ) ) DeleteLock ( OverUnitField->getPlanes() );
		else AddLock ( OverUnitField->getPlanes() );
	}
}

//--------------------------------------------------------------------------
/** Draws all entries, that are in the lock list. */
//--------------------------------------------------------------------------
void cPlayer::DrawLockList ()
{
	if ( !Client->gameGUI.lockChecked() ) return;
	sLockElem *elem;
	int spx, spy, off;


	for ( unsigned int i = 0; i < LockList.Size(); i++ )
	{
		elem = LockList[i];
		if ( elem->v )
		{
			off=elem->v->PosX+elem->v->PosY*Client->Map->size;
			if ( !ScanMap[off] )
			{
				DeleteLock ( elem->v );
				i--;
				continue;
			}
			spx=elem->v->getScreenPosX();
			spy=elem->v->getScreenPosY();

			if ( Client->gameGUI.scanChecked() )
			{
				if ( elem->v->data.isBig )
					drawCircle ( spx+ Client->gameGUI.getTileSize(), spy + Client->gameGUI.getTileSize(), elem->v->data.scan * Client->gameGUI.getTileSize(), SCAN_COLOR, buffer );
				else
					drawCircle ( spx + Client->gameGUI.getTileSize()/2, spy + Client->gameGUI.getTileSize()/2, elem->v->data.scan * Client->gameGUI.getTileSize(), SCAN_COLOR, buffer );
			}
			if ( Client->gameGUI.rangeChecked() && (elem->v->data.canAttack & TERRAIN_GROUND) )
				drawCircle ( spx+Client->gameGUI.getTileSize()/2,
				             spy+Client->gameGUI.getTileSize()/2,
				             elem->v->data.range*Client->gameGUI.getTileSize()+1,RANGE_GROUND_COLOR,buffer );
			if ( Client->gameGUI.rangeChecked() && (elem->v->data.canAttack & TERRAIN_AIR) )
				drawCircle ( spx+Client->gameGUI.getTileSize()/2,
				             spy+Client->gameGUI.getTileSize()/2,
				             elem->v->data.range*Client->gameGUI.getTileSize()+2,RANGE_AIR_COLOR,buffer );
			if ( Client->gameGUI.ammoChecked()&&elem->v->data.canAttack )
				elem->v->drawMunBar();
			if ( Client->gameGUI.hitsChecked() )
				elem->v->drawHealthBar();
		}
		else if ( elem->b )
		{
			off=elem->b->PosX+elem->b->PosY*Client->Map->size;
			if ( !ScanMap[off] )
			{
				DeleteLock ( elem->b );
				i--;
				continue;
			}
			spx=elem->b->getScreenPosX();
			spy=elem->b->getScreenPosY();

			if ( Client->gameGUI.scanChecked() )
			{
				if ( elem->b->data.isBig )
					drawCircle ( spx+Client->gameGUI.getTileSize(),
					             spy+Client->gameGUI.getTileSize(),
					             elem->b->data.scan*Client->gameGUI.getTileSize(),SCAN_COLOR,buffer );
				else
					drawCircle ( spx+Client->gameGUI.getTileSize()/2,
					             spy+Client->gameGUI.getTileSize()/2,
					             elem->b->data.scan*Client->gameGUI.getTileSize(),SCAN_COLOR,buffer );
			}
			if ( Client->gameGUI.rangeChecked() && (elem->b->data.canAttack & TERRAIN_GROUND) &&!elem->b->data.explodesOnContact )
				drawCircle ( spx+Client->gameGUI.getTileSize()/2,
				             spy+Client->gameGUI.getTileSize()/2,
				             elem->b->data.range*Client->gameGUI.getTileSize()+2,RANGE_GROUND_COLOR,buffer );
			if ( Client->gameGUI.rangeChecked() && (elem->b->data.canAttack & TERRAIN_AIR) )
				drawCircle ( spx+Client->gameGUI.getTileSize()/2,
				             spy+Client->gameGUI.getTileSize()/2,
				             elem->b->data.range*Client->gameGUI.getTileSize()+2,RANGE_AIR_COLOR,buffer );

			if ( Client->gameGUI.ammoChecked() && elem->b->data.canAttack && !elem->b->data.explodesOnContact )
				elem->b->drawMunBar();
			if ( Client->gameGUI.hitsChecked() )
				elem->b->drawHealthBar();
		}
	}
}

//--------------------------------------------------------------------------
void cPlayer::drawSpecialCircle( int iX, int iY, int iRadius, char *map, int mapsize )
{
	float w = (float)(0.017453*45), step;
	int rx, ry, x1, x2;
	if ( iRadius <= 0 ) return;
	iRadius *= 10;
	step = (float)(0.017453*90-acos ( 1.0/iRadius ));
	step /= 2;
	for ( float i = 0; i <= w; i += step )
	{
		rx= ( int ) ( cos ( i ) *iRadius );
		ry= ( int ) ( sin ( i ) *iRadius);
		rx /= 10;
		ry /= 10;

		x1 = rx+iX;
		x2 = -rx+iX;
		for ( int k = x2; k <= x1; k++ )
		{
			if ( k < 0 ) continue;
			if ( k >= mapsize ) break;
			if ( iY+ry >= 0 && iY+ry < mapsize )
				map[k+ ( iY+ry ) *mapsize] |= 1;
			if ( iY-ry >= 0 && iY-ry < mapsize )
				map[k+ ( iY-ry ) *mapsize] |= 1;
		}

		x1 = ry+iX;
		x2 = -ry+iX;
		for ( int k = x2; k <= x1; k++ )
		{
			if ( k < 0 ) continue;
			if ( k >= mapsize ) break;
			if ( iY+rx >= 0 && iY+rx < mapsize )
				map[k+ ( iY+rx ) *mapsize] |= 1;
			if ( iY-rx >= 0 && iY-rx < mapsize )
				map[k+ ( iY-rx ) *mapsize] |= 1;
		}
	}
}

//--------------------------------------------------------------------------
void cPlayer::drawSpecialCircleBig( int iX, int iY, int iRadius, char *map, int mapsize )
{
	float w=(float)(0.017453*45), step;
	int rx, ry, x1, x2;
	if ( iRadius > 0 ) iRadius--;
	else return;
	iRadius *= 10;
	step = (float)(0.017453*90-acos ( 1.0/iRadius ));
	step /= 2;
	for ( float i = 0; i <= w; i += step )
	{
		rx= ( int ) ( cos ( i ) *iRadius );
		ry= ( int ) ( sin ( i ) *iRadius);
		rx /= 10;
		ry /= 10;

		x1 = rx+iX;
		x2 = -rx+iX;
		for ( int k = x2; k <= x1+1; k++ )
		{
			if ( k < 0 ) continue;
			if ( k >= mapsize ) break;
			if ( iY+ry >= 0 && iY+ry < mapsize )
				map[k+ ( iY+ry ) *mapsize] |= 1;
			if ( iY-ry >= 0 && iY-ry < mapsize )
				map[k+ ( iY-ry ) *mapsize] |= 1;

			if( iY+ry+1 >= 0&& iY+ry+1 < mapsize)
				map[k+( iY+ry+1 ) *mapsize] |= 1;
			if( iY-ry+1 >= 0 && iY-ry+1 < mapsize)
				map[k+( iY-ry+1 ) *mapsize] |= 1;
		}

		x1 = ry+iX;
		x2 = -ry+iX;
		for ( int k = x2; k <= x1+1; k++ )
		{
			if ( k < 0 ) continue;
			if ( k >= mapsize ) break;
			if ( iY+rx >= 0 && iY+rx < mapsize )
				map[k+ ( iY+rx ) *mapsize] |= 1;
			if ( iY-rx >= 0 && iY-rx < mapsize )
				map[k+ ( iY-rx ) *mapsize] |= 1;

			if( iY+rx+1 >= 0 && iY+rx+1 < mapsize)
				map[k+( iY+rx+1 ) *mapsize] |= 1;
			if( iY-rx+1 >= 0 && iY-rx+1 < mapsize)
				map[k+( iY-rx+1 ) *mapsize] |= 1;
		}

	}
}

//--------------------------------------------------------------------------
void cPlayer::addSavedReport ( string message, sSavedReportMessage::eReportTypes type, sID unitID, int xPos, int yPos, int colorNr )
{
	sSavedReportMessage savedReport;
	savedReport.message = message;
	savedReport.type = type;
	savedReport.xPos = xPos;
	savedReport.yPos = yPos;
	savedReport.unitID = unitID;
	savedReport.colorNr = colorNr;

	savedReportsList.Add ( savedReport );
}
