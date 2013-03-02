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
#ifndef playerH
#define playerH

#include <SDL.h>
#include <vector>
#include "defines.h"
#include "base.h"
#include "main.h" // for sID
#include "upgradecalculator.h"

struct sVehicle;
struct sUnitData;
struct sBuilding;
class cVehicle;
class cBuilding;
class cHud;
class cMapField;
class cUnit;
struct sHudStateContainer;
struct sTurnstartReport;

// Eintrag in der Lock-Liste /////////////////////////////////////////////////
struct sLockElem{
  cVehicle *v;
  cBuilding *b;
};

struct sSavedReportMessage
{
	enum eReportTypes
	{
		REPORT_TYPE_COMP,
		REPORT_TYPE_UNIT,
		REPORT_TYPE_CHAT
	};

	std::string message;
	eReportTypes type;
	sID unitID;
	int xPos, yPos;
	int colorNr;
};

typedef std::vector<int> PointsHistory;


// Die Player-Klasse /////////////////////////////////////////////////////////
class cPlayer{

friend class cServer;
friend class cClient;
public:
	cPlayer(std::string Name,SDL_Surface *Color,int nr, int iSocketNum = -1 );
	~cPlayer();
	cPlayer(const cPlayer &Player);

	std::string name;
	SDL_Surface *color;
	int Nr;

	sUnitData *VehicleData; // Daten aller Vehicles f¸r diesen Player.
	cVehicle *VehicleList;     // Liste aller Vehicles des Spielers.
	sUnitData *BuildingData; // Daten aller Buildings f¸r diesen Player.
	cBuilding *BuildingList;     // Liste aller Buildings des Spielers.
	int MapSize;               // Kartengrˆﬂe
	char *ScanMap;             // Map mit dem Scannerflags.
	char *ResourceMap;         // Map mit aufgedeckten Resourcen. / Map with explored resources.
	cBase base;               // Die Basis dieses Spielers.
	//cList<sSentry*> SentriesAir;		/** list with all units on sentry that can attack planes */
	char *SentriesMapAir;				/** the covered air area */
	//cList<sSentry*> SentriesGround;	/** list with all units on sentry that can attack ground units */
	char *SentriesMapGround;			/** the covered ground area */
	char *DetectLandMap;       // Map mit den Gebieten, die an Land gesehen werden kˆnnen.
	char *DetectSeaMap;        // Map mit den Gebieten, die im Wasser gesehen werden kˆnnen.
	char *DetectMinesMap;				/** the area where the player can detect mines */
	cResearch researchLevel;	///< stores the current research level of the player
	int researchCentersWorkingOnArea[cResearch::kNrResearchAreas]; ///< counts the number of research centers that are currently working on each area
	int ResearchCount;         ///< number of working research centers
	int Credits;               // Anzahl der erworbenen Credits.
	mutable PointsHistory pointsHistory; // history of player's total score (from eco-spheres) for graph
	sHudStateContainer *savedHud;
	cList<sTurnstartReport*> ReportVehicles; // Reportlisten.
	cList<sTurnstartReport*> ReportBuildings; // Reportlisten.
	cList<sSavedReportMessage> savedReportsList;
	cList<int> reportResearchAreasFinished; ///< stores, which research areas were just finished (for reporting at turn end)
	cList<sLockElem*> LockList;           // Liste mit gelockten Objekten.
	int iSocketNum;			// Number of socket over which this player is connected in network game
	// if MAX_CLIENTS its the lokal connected player; -1 for unknown
	bool bFinishedTurn;			//true when player send his turn end
	bool isDefeated;			// true if the player has been defeated
	bool isRemovedFromGame;		// true if the player has been removed from the game.
	int numEcos;                // number of ecospheres. call CountEcoSpheres on server to update.
	bool researchFinished;
	unsigned int lastDeletedUnit;  /**used for detecting ownerchanges of a unit, e.g. a unit is readded with different player*/

	void InitMaps(int MapSizeX, cMap *map = NULL ); // TODO: remove ' = NULL'
	void DoScan();
	cUnit *getNextUnit ();
	cUnit *getPrevUnit ();
	void addSentry ( cUnit *u );
	void deleteSentry ( cUnit *u );
	void startAResearch (int researchArea);
	void stopAResearch (int researchArea);
	void doResearch (); ///< proceed with the research at turn end
	void accumulateScore(); // at turn end
	void upgradeUnitTypes (cList<int>& areasReachingNextLevel, cList<sUnitData*>& resultUpgradedUnitDatas);
	void refreshResearchCentersWorkingOnArea();
	void AddLock(cBuilding *b);
	void AddLock(cVehicle *v);
	void DeleteLock(cBuilding *b);
	void DeleteLock(cVehicle *v);
	bool InLockList(cBuilding *b);
	bool InLockList(cVehicle *v);
	void ToggelLock(cMapField *OverUnitField);
	void DrawLockList();
	void CountEcoSpheres();
	int getScore(int turn) const;
	void setScore(int score, int turn);
	void clearDone();

	/**
	* draws a circle on the map for the fog
	*@author alzi alias DoctorDeath
	*@param iX X coordinate to the center of the circle
	*@param iY Y coordinate to the center of the circle
	*@param iRadius radius of the circle
	*@param map map were to store the data of the circle
	*/
	void drawSpecialCircle( int iX, int iY, int iRadius, char *map, int mapsize );
	/**
	* draws a big circle on the map for the fog
	*@author alzi alias DoctorDeath
	*@param iX X coordinate to the center of the circle
	*@param iY Y coordinate to the center of the circle
	*@param iRadius radius of the circle
	*@param map map were to store the data of the circle
	*/
	void drawSpecialCircleBig( int iX, int iY, int iRadius, char *map, int mapsize );

	void addSavedReport ( std::string message, sSavedReportMessage::eReportTypes type, sID unitID = sID(), int xPos = -1, int yPos = -1, int colorNr = -1 );

	void setClan (int newClan);
	int getClan () const { return clan; }

private:
	void refreshSentryAir();
	void refreshSentryGround();

	cVehicle *AddVehicle( int posx, int posy, sVehicle *v );
	cBuilding *addBuilding( int posx, int posy, sBuilding *b );

	int clan;
};

#endif
