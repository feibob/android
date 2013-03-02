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
#include "serverevents.h"
#include "clientevents.h"
#include "menuevents.h"
#include "network.h"
#include "netmessage.h"
#include "events.h"
#include "server.h"
#include "movejobs.h"
#include "upgradecalculator.h"
#include "hud.h"
#include "buildings.h"
#include "vehicles.h"
#include "player.h"
#include "casualtiestracker.h"

//-------------------------------------------------------------------------------------
void sendAddUnit ( int iPosX, int iPosY, int iID, bool bVehicle, sID UnitID, int iPlayer, bool bInit, bool bAddToMap )
{
	cNetMessage* message;

	if ( bVehicle ) message = new cNetMessage ( GAME_EV_ADD_VEHICLE );
	else message = new cNetMessage ( GAME_EV_ADD_BUILDING );

	message->pushBool( bAddToMap );
	message->pushInt16( iID );
	message->pushInt16( iPosX );
	message->pushInt16( iPosY );
	message->pushInt16( UnitID.iSecondPart );
	message->pushInt16( UnitID.iFirstPart );
	message->pushInt16( iPlayer );
	message->pushBool( bInit );

	Server->sendNetMessage( message, iPlayer );
}

//-------------------------------------------------------------------------------------
void sendAddRubble( cBuilding* building, int iPlayer )
{
	cNetMessage* message = new cNetMessage( GAME_EV_ADD_RUBBLE );

	message->pushInt16( building->PosX );
	message->pushInt16( building->PosY );
	message->pushInt16( building->iID );
	message->pushInt16( building->RubbleValue );
	message->pushInt16( building->RubbleTyp );
	message->pushBool( building->data.isBig );

	Server->sendNetMessage( message, iPlayer );
}

//-------------------------------------------------------------------------------------
void sendDeleteUnitMessage (cUnit* unit, int playerNr)
{
	cNetMessage* message = new cNetMessage (unit->isBuilding () ? GAME_EV_DEL_BUILDING : GAME_EV_DEL_VEHICLE);
	message->pushInt16 (unit->iID);
	Server->sendNetMessage (message, playerNr);
}

//-------------------------------------------------------------------------------------
void sendDeleteUnit (cUnit* unit, int iClient)
{
	if (iClient == -1)
	{
		for (unsigned int i = 0; i < unit->seenByPlayerList.Size (); i++)
			sendDeleteUnitMessage (unit, unit->seenByPlayerList[i]->Nr);

		//send message to owner, since he is not in the seenByPlayerList
		if (unit->owner != 0)
			sendDeleteUnitMessage (unit, unit->owner->Nr);
	}
	else
		sendDeleteUnitMessage(unit, iClient);
}

//-------------------------------------------------------------------------------------
void sendAddEnemyUnit (cUnit* unit, int iClient)
{
	cNetMessage* message = new cNetMessage (unit->isBuilding () ? GAME_EV_ADD_ENEM_BUILDING : GAME_EV_ADD_ENEM_VEHICLE);

	message->pushInt16 (unit->data.version);
	message->pushInt16 (unit->iID);
	if (unit->isVehicle ())
		message->pushInt16 (unit->dir);
	message->pushInt16 (unit->PosX);
	message->pushInt16 (unit->PosY);
	message->pushInt16 (unit->data.ID.iSecondPart);
	message->pushInt16 (unit->data.ID.iFirstPart);
	message->pushInt16 (unit->owner->Nr);

	Server->sendNetMessage (message, iClient);
}

//-------------------------------------------------------------------------------------
void sendMakeTurnEnd ( bool bEndTurn, bool bWaitForNextPlayer, int iNextPlayerNum, int iPlayer )
{
	cNetMessage* message = new cNetMessage ( GAME_EV_MAKE_TURNEND );

	message->pushBool ( bEndTurn );
	message->pushBool ( bWaitForNextPlayer );
	message->pushInt16( iNextPlayerNum );

	Server->sendNetMessage( message, iPlayer );
}

//-------------------------------------------------------------------------------------
void sendTurnFinished ( int iPlayerNum, int iTimeDelay, cPlayer *Player )
{
	cNetMessage* message = new cNetMessage ( GAME_EV_FINISHED_TURN );

	message->pushInt16 ( iTimeDelay );
	message->pushInt16( iPlayerNum );

	if ( Player != NULL ) Server->sendNetMessage( message, Player->Nr );
	else Server->sendNetMessage( message );
}

//-------------------------------------------------------------------------------------
void sendUnitData (cUnit* unit, int iPlayer)
{
	cNetMessage* message = new cNetMessage (GAME_EV_UNIT_DATA);

	// The unit data values
	if (unit->isVehicle ())
	{
		message->pushInt16( ((cVehicle*) unit)->FlightHigh );
		message->pushInt16 (unit->data.speedMax);
		message->pushInt16 (unit->data.speedCur);
	}
	message->pushInt16 (unit->data.version);
	message->pushInt16 (unit->data.hitpointsMax);
	message->pushInt16 (unit->data.hitpointsCur);
	message->pushInt16 (unit->data.armor);
	message->pushInt16 (unit->data.scan);
	message->pushInt16 (unit->data.range);
	message->pushInt16 (unit->data.shotsMax);
	message->pushInt16 (unit->data.shotsCur);
	message->pushInt16 (unit->data.damage);
	message->pushInt16 (unit->data.storageUnitsMax);
	message->pushInt16 (unit->data.storageUnitsCur);
	message->pushInt16 (unit->data.storageResMax);
	message->pushInt16 (unit->data.storageResCur);
	message->pushInt16 (unit->data.ammoMax);
	message->pushInt16 (unit->data.ammoCur);
	message->pushInt16 (unit->data.buildCosts);

	// Current state of the unit
	//TODO: remove information such sentrystatus, build or clearrounds from normal data
	//		because this data will be received by enemys, too
	if (unit->isBuilding ())
		message->pushInt16 (((cBuilding*)unit)->points);

	message->pushBool (unit->manualFireActive);
	message->pushBool (unit->sentryActive);

	if (unit->isVehicle ())
	{
		cVehicle* vehicle = (cVehicle*)unit;
		message->pushInt16 (vehicle->ClearingRounds);
		message->pushInt16 (vehicle->BuildRounds);
		message->pushBool (vehicle->IsBuilding);
		message->pushBool (vehicle->IsClearing);
		message->pushInt16 ((int)vehicle->CommandoRank);
	}
	else
	{
		cBuilding* building = (cBuilding*)unit;
		message->pushBool ( building->IsWorking );
		message->pushInt16 ( building->researchArea );
	}

	message->pushInt16 (unit->turnsDisabled);

	if (unit->isVehicle ())
		message->pushBool (unit->isBeeingAttacked);

	if (unit->isNameOriginal () == false)
	{
		message->pushString (unit->getName ());
		message->pushBool (true);
	}
	else
		message->pushBool (false);

	if (unit->isVehicle ())
		message->pushBool (unit->data.isBig);

	// Data for identifying the unit by the client
	message->pushInt16 (unit->PosX);
	message->pushInt16 (unit->PosY);
	message->pushBool  (unit->isVehicle ());
	message->pushInt16 (unit->iID);
	message->pushInt16 (unit->owner->Nr);

	Server->sendNetMessage (message, iPlayer);
}

//-------------------------------------------------------------------------------------
void sendSpecificUnitData ( cVehicle *Vehicle )
{
	cNetMessage* message = new cNetMessage( GAME_EV_SPECIFIC_UNIT_DATA );
	message->pushInt16 ( Vehicle->BandY );
	message->pushInt16 ( Vehicle->BandX );
	message->pushBool ( Vehicle->BuildPath );
	message->pushInt16 ( Vehicle->BuildingTyp.iSecondPart );
	message->pushInt16 ( Vehicle->BuildingTyp.iFirstPart );
	message->pushInt16 ( Vehicle->dir );
	message->pushInt16 ( Vehicle->iID );
	Server->sendNetMessage( message, Vehicle->owner->Nr );
}

//-------------------------------------------------------------------------------------
void sendChatMessageToClient( std::string message, int iType, int iPlayer, std::string inserttext )
{
	cNetMessage* newMessage;
	newMessage = new cNetMessage( GAME_EV_CHAT_SERVER );
	newMessage->pushString( inserttext );
	newMessage->pushString( message );
	newMessage->pushChar( iType );
	Server->sendNetMessage( newMessage, iPlayer );
}

//-------------------------------------------------------------------------------------
void sendDoStartWork( cBuilding* building )
{
	int offset = building->PosX + building->PosY * Server->Map->size;

	//check all players
	for ( unsigned int i = 0; i < Server->PlayerList->Size(); i++)
	{
		cPlayer* player = (*Server->PlayerList)[i];

		//do not send to players who can't see the building
		if ( !player->ScanMap[offset] && player!=building->owner ) continue;

		cNetMessage* message = new cNetMessage( GAME_EV_DO_START_WORK );
		message->pushInt32( building->iID );
		Server->sendNetMessage( message, player->Nr );
	}
}

//-------------------------------------------------------------------------------------
void sendDoStopWork( cBuilding* building )
{
	int offset = building->PosX + building->PosY * Server->Map->size;

	//check all players
	for ( unsigned int i = 0; i < Server->PlayerList->Size(); i++)
	{
		cPlayer* player = (*Server->PlayerList)[i];

		//do not send to players who can't see the building
		if ( !player->ScanMap[offset] && player!=building->owner ) continue;

		cNetMessage* message = new cNetMessage( GAME_EV_DO_STOP_WORK );
		message->pushInt32( building->iID );
		Server->sendNetMessage( message, player->Nr );
	}
}

//-------------------------------------------------------------------------------------
void sendNextMove( cVehicle* vehicle, int iType, int iSavedSpeed )
{
	char height = 0;
	cVehicleIterator planes = (*Server->Map)[vehicle->PosX + vehicle->PosY * Server->Map->size].getPlanes();
	while ( !planes.end )
	{
		if ( planes == vehicle )
			height = planes.getIndex();
		planes++;
	}

	for ( unsigned int i = 0; i < vehicle->seenByPlayerList.Size(); i++ )
	{
		cNetMessage* message = new cNetMessage( GAME_EV_NEXT_MOVE );
		if ( iSavedSpeed >= 0 ) message->pushChar( iSavedSpeed );
		message->pushChar( height );
		message->pushChar( iType );
		message->pushInt16( vehicle->PosY );
		message->pushInt16( vehicle->PosX );
		message->pushInt16( vehicle->iID );
		Server->sendNetMessage( message, vehicle->seenByPlayerList[i]->Nr );
	}

	cNetMessage* message = new cNetMessage( GAME_EV_NEXT_MOVE );
	if ( iSavedSpeed >= 0 ) message->pushChar( iSavedSpeed );
	message->pushChar( height );
	message->pushChar( iType );
	message->pushInt16(vehicle->PosY);
	message->pushInt16(vehicle->PosX);
	message->pushInt16( vehicle->iID );
	Server->sendNetMessage( message, vehicle->owner->Nr);
}

//-------------------------------------------------------------------------------------
void sendMoveJobServer( cServerMoveJob *MoveJob, int iPlayer )
{

	cNetMessage* message = new cNetMessage( GAME_EV_MOVE_JOB_SERVER );

	sWaypoint *waypoint = MoveJob->Waypoints;
	int iCount = 0;
	while ( waypoint )
	{
		message->pushInt16( waypoint->Costs );
		message->pushInt16( waypoint->X );
		message->pushInt16( waypoint->Y );

		if ( message->iLength > PACKAGE_LENGTH - 19 )
		{
			Log.write(" Server: Error sending movejob: message too long", cLog::eLOG_TYPE_NET_ERROR );
			delete message;
			return;	// don't send movejobs that are to long
		}

		waypoint = waypoint->next;
		iCount++;
	}

	message->pushInt16( iCount );
	message->pushInt16( MoveJob->iSavedSpeed );
	message->pushInt32( MoveJob->DestX+MoveJob->DestY*MoveJob->Map->size );
	message->pushInt32( MoveJob->SrcX+MoveJob->SrcY*MoveJob->Map->size );
	message->pushInt32( MoveJob->Vehicle->iID );

	Server->sendNetMessage( message, iPlayer );
}

//-------------------------------------------------------------------------------------
void sendVehicleResources( cVehicle *Vehicle, cMap *Map )
{
	int iCount = 0;
	cNetMessage* message = new cNetMessage( GAME_EV_RESOURCES );

	// only send new scaned resources
	for ( int iX = Vehicle->PosX-1, iY = Vehicle->PosY-1; iY <= Vehicle->PosY+1; iX++ )
	{
		if ( iX > Vehicle->PosX+1 )
		{
			iX = Vehicle->PosX-1;
			iY++;
		}

		if ( iY > Vehicle->PosY+1 ) break;
		if ( iX < 0 || iX >= Map->size || iY < 0 || iY >= Map->size ) continue;
		if ( Vehicle->owner->ResourceMap[iX+iY*Map->size] != 0 ) continue;

		message->pushInt16( Map->Resources[iX+iY*Map->size].value );
		message->pushInt16( Map->Resources[iX+iY*Map->size].typ );
		message->pushInt32( iX+iY*Map->size );
		iCount++;
	}
	message->pushInt16( iCount );

	Server->sendNetMessage( message, Vehicle->owner->Nr );
}

//-------------------------------------------------------------------------------------
void sendResources( cPlayer *Player )
{
	int iCount = 0;
	cNetMessage* message = new cNetMessage( GAME_EV_RESOURCES );
	for ( int i = 0; i < Player->MapSize; i++ )
	{
		if (  Player->ResourceMap[i] == 1 )
		{
			message->pushInt16( Server->Map->Resources[i].value );
			message->pushInt16( Server->Map->Resources[i].typ );
			message->pushInt32( i );
			iCount++;
		}
		if ( message->iLength >= PACKAGE_LENGTH-10 )
		{
			message->pushInt16( iCount );
			Server->sendNetMessage( message, Player->Nr );
			message = new cNetMessage( GAME_EV_RESOURCES );
			iCount = 0;
		}
	}
	if ( iCount > 0 )
	{
		message->pushInt16( iCount );
		Server->sendNetMessage( message, Player->Nr );
	}
}

//-------------------------------------------------------------------------------------
void sendScore( cPlayer *Subject, int turn, cPlayer *Receiver)
{
	if(!Receiver)
		for ( unsigned int n = 0; n < Server->PlayerList->Size(); n++ )
			sendScore(Subject, turn, (*Server->PlayerList)[n]);
	else
	{
		cNetMessage *msg = new cNetMessage( GAME_EV_SCORE );
		msg->pushInt16( Subject->getScore(turn));
		msg->pushInt16( turn );
		msg->pushInt16( Subject->Nr );

		Server->sendNetMessage(msg, Receiver->Nr);
	}
}

void sendUnitScore(cBuilding *b)
{
	cNetMessage *msg = new cNetMessage( GAME_EV_UNIT_SCORE );
	msg->pushInt16(b->points);
	msg->pushInt16(b->iID);
	Server->sendNetMessage(msg, b->owner->Nr);
}

void sendNumEcos(cPlayer *Subject, cPlayer *Receiver)
{
	Subject->CountEcoSpheres();

	if(!Receiver)
		for ( unsigned int n = 0; n < Server->PlayerList->Size(); n++ )
			sendNumEcos(Subject, (*Server->PlayerList)[n]);
	else
	{
		cNetMessage *msg = new cNetMessage( GAME_EV_NUM_ECOS );
		msg->pushInt16( Subject->numEcos );
		msg->pushInt16( Subject->Nr );

		Server->sendNetMessage(msg, Receiver->Nr);
	}
}

void sendVictoryConditions(int turnLimit, int scoreLimit, cPlayer *Receiver)
{
	if(!Receiver)
		for ( unsigned int n = 0; n < Server->PlayerList->Size(); n++ )
			sendVictoryConditions(turnLimit, scoreLimit, (*Server->PlayerList)[n]);
	else
	{
		cNetMessage *msg = new cNetMessage( GAME_EV_VICTORY_CONDITIONS );
		msg->pushInt16( turnLimit );
		msg->pushInt16( scoreLimit );
		Server->sendNetMessage(msg, Receiver->Nr);
	}
}

//-------------------------------------------------------------------------------------
void sendBuildAnswer( bool bOK, cVehicle* vehicle )
{
	//message for the owner
	cNetMessage* message = new cNetMessage( GAME_EV_BUILD_ANSWER );
	if ( bOK )
	{
		message->pushInt16( vehicle->BandY );
		message->pushInt16( vehicle->BandX );
		message->pushBool(  vehicle->BuildPath );
		message->pushInt16( vehicle->BuildRounds );
		message->pushInt16( vehicle->BuildingTyp.iSecondPart );
		message->pushInt16( vehicle->BuildingTyp.iFirstPart );
		message->pushBool ( vehicle->BuildingTyp.getUnitDataOriginalVersion()->isBig );
		message->pushInt16( vehicle->PosY );
		message->pushInt16( vehicle->PosX );
	}

	message->pushInt16( vehicle->iID );
	message->pushBool ( bOK );
	Server->sendNetMessage( message, vehicle->owner->Nr );

	//message for the enemys
	for ( unsigned int i = 0; i < vehicle->seenByPlayerList.Size(); i++ )
	{
		cNetMessage* message = new cNetMessage( GAME_EV_BUILD_ANSWER );
		if ( bOK )
		{
			message->pushBool ( vehicle->BuildingTyp.getUnitDataOriginalVersion()->isBig );
			message->pushInt16( vehicle->PosY );
			message->pushInt16( vehicle->PosX );
		}
		message->pushInt16( vehicle->iID );
		message->pushBool ( bOK );
		Server->sendNetMessage( message, vehicle->seenByPlayerList[i]->Nr );
	}
}

//-------------------------------------------------------------------------------------
void sendStopBuild ( int iVehicleID, int iNewPos, int iPlayer  )
{
	cNetMessage* message = new cNetMessage( GAME_EV_STOP_BUILD );
	message->pushInt32( iNewPos );
	message->pushInt16( iVehicleID );
	Server->sendNetMessage( message, iPlayer );
}

//-------------------------------------------------------------------------------------
void sendSubbaseValues ( sSubBase *SubBase, int iPlayer )
{
	//temporary debug check
	if ( SubBase->getGoldProd() < SubBase->getMaxAllowedGoldProd() ||
		 SubBase->getMetalProd() < SubBase->getMaxAllowedMetalProd() ||
		 SubBase->getOilProd() < SubBase->getMaxAllowedOilProd() )
	{
		Log.write(" Server: Mine distribution values are not a maximum", cLog::eLOG_TYPE_NET_WARNING);
	}

	cNetMessage* message = new cNetMessage( GAME_EV_SUBBASE_VALUES );

	message->pushInt16 ( SubBase->EnergyProd );
	message->pushInt16 ( SubBase->EnergyNeed );
	message->pushInt16 ( SubBase->MaxEnergyProd );
	message->pushInt16 ( SubBase->MaxEnergyNeed );
	message->pushInt16 ( SubBase->Metal );
	message->pushInt16 ( SubBase->MaxMetal );
	message->pushInt16 ( SubBase->MetalNeed );
	message->pushInt16 ( SubBase->MaxMetalNeed );
	message->pushInt16 ( SubBase->getMetalProd() );
	message->pushInt16 ( SubBase->Gold );
	message->pushInt16 ( SubBase->MaxGold );
	message->pushInt16 ( SubBase->GoldNeed );
	message->pushInt16 ( SubBase->MaxGoldNeed );
	message->pushInt16 ( SubBase->getGoldProd() );
	message->pushInt16 ( SubBase->Oil );
	message->pushInt16 ( SubBase->MaxOil );
	message->pushInt16 ( SubBase->OilNeed );
	message->pushInt16 ( SubBase->MaxOilNeed );
	message->pushInt16 ( SubBase->getOilProd() );
	message->pushInt16 ( SubBase->HumanNeed );
	message->pushInt16 ( SubBase->MaxHumanNeed );
	message->pushInt16 ( SubBase->HumanProd );
	message->pushInt16 ( SubBase->getID() );
	Server->sendNetMessage( message, iPlayer );
}

//-------------------------------------------------------------------------------------
void sendBuildList ( cBuilding *Building )
{
	cNetMessage* message = new cNetMessage( GAME_EV_BUILDLIST );
	message->pushBool ( Building->RepeatBuild );
	message->pushInt16 ( Building->BuildSpeed );
	message->pushInt16 ( Building->MetalPerRound );
	for ( int i = (int)Building->BuildList->Size()-1; i >= 0; i-- )
	{
		message->pushInt16((*Building->BuildList)[i]->metall_remaining);
		message->pushInt16((*Building->BuildList)[i]->type.iSecondPart);
		message->pushInt16((*Building->BuildList)[i]->type.iFirstPart);
	}
	message->pushInt16 ( (int)Building->BuildList->Size() );
	message->pushInt16 ( Building->iID );
	Server->sendNetMessage( message, Building->owner->Nr );
}

//-------------------------------------------------------------------------------------
void sendProduceValues ( cBuilding *Building )
{
	cNetMessage* message = new cNetMessage( GAME_EV_MINE_PRODUCE_VALUES );
	message->pushInt16 ( Building->MaxGoldProd );
	message->pushInt16 ( Building->MaxOilProd );
	message->pushInt16 ( Building->MaxMetalProd );
	message->pushInt16 ( Building->iID );
	Server->sendNetMessage( message, Building->owner->Nr );
}

//-------------------------------------------------------------------------------------
void sendTurnReport ( cPlayer *Player )
{
	// TODO: make sure, that the message size is not exceeded!

	cNetMessage* message = new cNetMessage( GAME_EV_TURN_REPORT );
	int iCount = 0;
	sTurnstartReport *Report;

	int nrResearchAreasFinished = Player->reportResearchAreasFinished.Size ();
	for (int i = nrResearchAreasFinished - 1; i >= 0; i--) // count down to get the correct order at the client conveniently
		message->pushChar (Player->reportResearchAreasFinished[i]);
	message->pushChar (nrResearchAreasFinished);

	while ( Player->ReportBuildings.Size() )
	{
		Report = Player->ReportBuildings[0];
		message->pushInt16 ( Report->iAnz );
		message->pushInt16 ( Report->Type.iSecondPart );
		message->pushInt16 ( Report->Type.iFirstPart );
		Player->ReportBuildings.Delete ( 0 );
		delete Report;
		iCount++;
	}
	while ( Player->ReportVehicles.Size() )
	{
		Report = Player->ReportVehicles[0];
		message->pushInt16 ( Report->iAnz );
		message->pushInt16 ( Report->Type.iSecondPart );
		message->pushInt16 ( Report->Type.iFirstPart );
		Player->ReportVehicles.Delete ( 0 );
		delete Report;
		iCount++;
	}
	message->pushInt16 ( iCount );
	Server->sendNetMessage( message, Player->Nr );
}

//-------------------------------------------------------------------------------------
void sendSupply ( int iDestID, bool bDestVehicle, int iValue, int iType, int iPlayerNum )
{
	cNetMessage* message = new cNetMessage( GAME_EV_SUPPLY );
	message->pushInt16 ( iValue );
	message->pushInt16 ( iDestID );
	message->pushBool ( bDestVehicle );
	message->pushChar ( iType );
	Server->sendNetMessage( message, iPlayerNum );
}

//-------------------------------------------------------------------------------------
void sendDetectionState( cVehicle* vehicle )
{
	cNetMessage* message = new cNetMessage( GAME_EV_DETECTION_STATE );
	message->pushBool( vehicle->detectedByPlayerList.Size() > 0 );
	message->pushInt32( vehicle->iID );
	Server->sendNetMessage( message, vehicle->owner->Nr );
}

//-------------------------------------------------------------------------------------
void sendClearAnswer ( int answertype, cVehicle *Vehicle, int turns, int bigoffset, int iPlayer )
{
	cNetMessage* message = new cNetMessage( GAME_EV_CLEAR_ANSWER );
	message->pushInt16( bigoffset );
	message->pushInt16( turns );
	message->pushInt16( Vehicle->iID );
	message->pushInt16( answertype );
	Server->sendNetMessage( message, iPlayer );
}

//-------------------------------------------------------------------------------------
void sendStopClear ( cVehicle *Vehicle, int bigoffset, int iPlayer )
{
	cNetMessage* message = new cNetMessage( GAME_EV_STOP_CLEARING );
	message->pushInt16( bigoffset );
	message->pushInt16( Vehicle->iID );
	Server->sendNetMessage( message, iPlayer );
}

//-------------------------------------------------------------------------------------
void sendNoFog ( int iPlayer )
{
	cNetMessage* message = new cNetMessage( GAME_EV_NOFOG );
	Server->sendNetMessage( message, iPlayer );
}

//-------------------------------------------------------------------------------------
void sendDefeated ( cPlayer *Player, int iPlayer )
{
	cNetMessage* message = new cNetMessage( GAME_EV_DEFEATED );
	message->pushInt16( Player->Nr );
	Server->sendNetMessage( message, iPlayer );
}

//-------------------------------------------------------------------------------------
void sendWaitReconnect ( int iPlayer )
{
	cNetMessage* message = new cNetMessage( GAME_EV_WAIT_RECON );
	Server->sendNetMessage( message, iPlayer );
}

//-------------------------------------------------------------------------------------
void sendAbortWaitReconnect ( int iPlayer )
{
	cNetMessage* message = new cNetMessage( GAME_EV_ABORT_WAIT_RECON );
	Server->sendNetMessage( message, iPlayer );
}

//-------------------------------------------------------------------------------------
void sendFreeze ( bool sendNotification, int iPlayer )
{
	cNetMessage* message = new cNetMessage( GAME_EV_FREEZE );
	message->pushBool ( sendNotification );
	Server->sendNetMessage( message, iPlayer );
}

//-------------------------------------------------------------------------------------
void sendWaitFor ( int waitForPlayerNr, int iPlayer )
{
	cNetMessage* message = new cNetMessage( GAME_EV_WAIT_FOR );
	message->pushInt16 ( waitForPlayerNr );
	Server->sendNetMessage( message, iPlayer );
}

//-------------------------------------------------------------------------------------
void sendUnfreeze ( int iPlayer )
{
	cNetMessage* message = new cNetMessage( GAME_EV_UNFREEZE );
	Server->sendNetMessage( message, iPlayer );
}

//-------------------------------------------------------------------------------------
void sendDeletePlayer ( cPlayer *Player, int iPlayer )
{
	cNetMessage* message = new cNetMessage( GAME_EV_DEL_PLAYER );
	message->pushInt16( Player->Nr );
	Server->sendNetMessage( message, iPlayer );
}

//-------------------------------------------------------------------------------------
void sendRequestIdentification ( int iSocket )
{
	cNetMessage* message = new cNetMessage( GAME_EV_REQ_RECON_IDENT );
	message->pushInt16 ( iSocket );
	Log.write("Server: <-- " + message->getTypeAsString() + ", Hexdump: " + message->getHexDump(), cLog::eLOG_TYPE_NET_DEBUG );
	network->sendTo( iSocket, message->iLength, message->serialize() );
}

//-------------------------------------------------------------------------------------
void sendReconnectAnswer ( bool okay, int socketNumber, cPlayer *Player )
{
	cNetMessage* message = new cNetMessage( GAME_EV_RECONNECT_ANSWER );
	if ( okay && Player != NULL )
	{
		for ( unsigned int i = 0; i < Server->PlayerList->Size(); i++ )
		{
			cPlayer const *SecondPlayer = (*Server->PlayerList)[i];
			if ( Player == SecondPlayer ) continue;
			message->pushInt16 ( SecondPlayer->Nr );
			message->pushInt16 ( GetColorNr( SecondPlayer->color ) );
			message->pushString ( SecondPlayer->name );
		}
		message->pushInt16 ( (int)Server->PlayerList->Size() );
		message->pushString ( Server->Map->MapName );
		message->pushInt16 ( GetColorNr( Player->color ) );
		message->pushInt16 ( Player->Nr );
	}
	message->pushBool ( okay );

	Log.write("Server: <-- " + message->getTypeAsString() + ", Hexdump: " + message->getHexDump(), cLog::eLOG_TYPE_NET_DEBUG );
	network->sendTo( socketNumber, message->iLength, message->serialize() );
}

//-------------------------------------------------------------------------------------
void sendTurn ( int turn, cPlayer *Player )
{
	cNetMessage* message = new cNetMessage( GAME_EV_TURN );
	message->pushInt16 ( turn );
	Server->sendNetMessage( message, Player->Nr );
}

//-------------------------------------------------------------------------------------
void sendHudSettings ( sHudStateContainer hudStates, cPlayer *Player )
{
	cNetMessage* message = new cNetMessage( GAME_EV_HUD_SETTINGS );
	message->pushBool ( hudStates.tntChecked );
	message->pushBool ( hudStates.hitsChecked );
	message->pushBool ( hudStates.lockChecked );
	message->pushBool ( hudStates.surveyChecked );
	message->pushBool ( hudStates.statusChecked );
	message->pushBool ( hudStates.scanChecked );
	message->pushBool ( hudStates.rangeChecked );
	message->pushBool ( hudStates.twoXChecked );
	message->pushBool ( hudStates.fogChecked );
	message->pushBool ( hudStates.ammoChecked );
	message->pushBool ( hudStates.gridChecked );
	message->pushBool ( hudStates.colorsChecked );
	message->pushFloat ( hudStates.zoom );
	message->pushInt16 ( hudStates.offY );
	message->pushInt16 ( hudStates.offX );
	message->pushInt16 ( hudStates.selUnitID );
	Server->sendNetMessage( message, Player->Nr );
}

//-------------------------------------------------------------------------------------
void sendStoreVehicle ( int unitid, bool vehicle, int storedunitid, int player )
{
	cNetMessage* message = new cNetMessage( GAME_EV_STORE_UNIT );
	message->pushInt16 ( unitid );
	message->pushBool ( vehicle );
	message->pushInt16 ( storedunitid );
	Server->sendNetMessage( message, player );
}

//-------------------------------------------------------------------------------------
void sendActivateVehicle ( int unitid, bool vehicle, int activatunitid, int x, int y, int player )
{
	cNetMessage* message = new cNetMessage( GAME_EV_EXIT_UNIT );
	message->pushInt16 ( y );
	message->pushInt16 ( x );
	message->pushInt16 ( unitid );
	message->pushBool ( vehicle );
	message->pushInt16 ( activatunitid );
	Server->sendNetMessage( message, player );
}

//-------------------------------------------------------------------------------------
void sendDeleteEverything ( int player )
{
	cNetMessage* message = new cNetMessage( GAME_EV_DELETE_EVERYTHING );
	Server->sendNetMessage( message, player );
}

//-------------------------------------------------------------------------------------
void sendResearchLevel ( cResearch* researchLevel, int player )
{
	cNetMessage* message = new cNetMessage (GAME_EV_RESEARCH_LEVEL);
	for (int area = 0; area < cResearch::kNrResearchAreas; area++)
	{
		message->pushInt16(researchLevel->getCurResearchLevel(area));
		message->pushInt16(researchLevel->getCurResearchPoints(area));
	}
	Server->sendNetMessage (message, player);
}

//-------------------------------------------------------------------------------------
void sendRefreshResearchCount ( int player )
{
	cNetMessage* message = new cNetMessage (GAME_EV_REFRESH_RESEARCH_COUNT);
	Server->sendNetMessage ( message, player );
}

//-------------------------------------------------------------------------------------
void sendUnitUpgrades ( sUnitData *Data, int player )
{
	cNetMessage* message = new cNetMessage( GAME_EV_UNIT_UPGRADE_VALUES );
	message->pushInt16 ( Data->hitpointsMax );
	message->pushInt16 ( Data->ammoMax );
	message->pushInt16 ( Data->shotsMax );
	message->pushInt16 ( Data->speedMax );
	message->pushInt16 ( Data->armor );
	message->pushInt16 ( Data->buildCosts );
	message->pushInt16 ( Data->damage );
	message->pushInt16 ( Data->range );
	message->pushInt16 ( Data->scan );
	message->pushInt16 ( Data->version );
	message->pushInt16 ( Data->ID.iSecondPart );
	message->pushInt16 ( Data->ID.iFirstPart );
	Server->sendNetMessage( message, player );
}

//-------------------------------------------------------------------------------------
void sendCredits (int newCredits, int player)
{
	cNetMessage* message = new cNetMessage (GAME_EV_CREDITS_CHANGED);
	message->pushInt16 (newCredits);
	Server->sendNetMessage (message, player);
}

//-------------------------------------------------------------------------------------
void sendUpgradeBuildings (cList<cBuilding*>& upgradedBuildings, int totalCosts, int player)
{
	// send to owner
	cNetMessage* message = NULL;
	int buildingsInMsg = 0;
	for (unsigned int i = 0; i < upgradedBuildings.Size(); i++)
	{
		if (message == NULL)
		{
			message = new cNetMessage (GAME_EV_UPGRADED_BUILDINGS);
			buildingsInMsg = 0;
		}

		message->pushInt32(upgradedBuildings[i]->iID);
		buildingsInMsg++;
		if (message->iLength + 8 > PACKAGE_LENGTH)
		{
			message->pushInt16((totalCosts * buildingsInMsg) / (int)upgradedBuildings.Size());
			message->pushInt16(buildingsInMsg);
			Server->sendNetMessage (message, player);
			message = NULL;
		}
	}
	if (message != NULL)
	{
		message->pushInt16((int)(totalCosts * buildingsInMsg) / (int)upgradedBuildings.Size());
		message->pushInt16(buildingsInMsg);
		Server->sendNetMessage (message, player);
		message = NULL;
	}

	// send to other players
	for (unsigned int n = 0; n < Server->PlayerList->Size(); n++)
	{
		cPlayer* curPlayer = (*Server->PlayerList)[n];
		if (curPlayer == 0 || curPlayer->Nr == player) // don't send to the owner of the buildings
			continue;

		for (unsigned int buildingIdx = 0; buildingIdx < upgradedBuildings.Size(); buildingIdx++)
		{
			if (upgradedBuildings[buildingIdx]->seenByPlayerList.Contains(curPlayer)) // that player can see the building
				sendUnitData(upgradedBuildings[buildingIdx], curPlayer->Nr);
		}
	}
}

//-------------------------------------------------------------------------------------
void sendUpgradeVehicles (cList<cVehicle*>& upgradedVehicles, int totalCosts, unsigned int storingBuildingID, int player)
{
	if (upgradedVehicles.Size() * 4 > PACKAGE_LENGTH - 50)
	{
		Log.write("Server: sendUpgradeVehicles: Message would exceed messagesize!!!", cLog::eLOG_TYPE_NET_ERROR);
		return;
	}
	// send to owner
	cNetMessage* message = new cNetMessage (GAME_EV_UPGRADED_VEHICLES);
	for (unsigned int i = 0; i < upgradedVehicles.Size(); i++)
		message->pushInt32(upgradedVehicles[i]->iID);

	message->pushInt32(storingBuildingID);
	message->pushInt16(totalCosts);
	message->pushInt16((int)upgradedVehicles.Size());
	Server->sendNetMessage (message, player);

	//TODO: send to other players as well?
}

//-------------------------------------------------------------------------------------
void sendResearchSettings(cList<cBuilding*>& researchCentersToChangeArea, cList<int>& newAreasForResearchCenters, int player)
{
	if (researchCentersToChangeArea.Size() != newAreasForResearchCenters.Size())
		return;

	cNetMessage* message = NULL;
	int buildingsInMsg = 0;
	for (unsigned int i = 0; i < researchCentersToChangeArea.Size(); i++)
	{
		if (message == NULL)
		{
			message = new cNetMessage (GAME_EV_RESEARCH_SETTINGS);
			buildingsInMsg = 0;
		}

		message->pushChar(newAreasForResearchCenters[i]);
		message->pushInt32(researchCentersToChangeArea[i]->iID);
		buildingsInMsg++;
		if (message->iLength + 7 > PACKAGE_LENGTH)
		{
			message->pushInt16(buildingsInMsg);
			Server->sendNetMessage (message, player);
			message = NULL;
		}
	}
	if (message != NULL)
	{
		message->pushInt16(buildingsInMsg);
		Server->sendNetMessage (message, player);
		message = NULL;
	}
}

//-------------------------------------------------------------------------------------
void sendClans ( const cList<cPlayer*>* playerList, cPlayer* toPlayer)
{
	if (playerList == 0 || toPlayer == 0)
		return;
	cNetMessage *message = new cNetMessage (GAME_EV_PLAYER_CLANS);
	for (unsigned int i = 0; i < playerList->Size(); i++)
	{
		message->pushChar ((*playerList)[i]->getClan());
		message->pushChar ((*playerList)[i]->Nr);
	}
	Server->sendNetMessage (message, toPlayer->Nr);
}

//-------------------------------------------------------------------------------------
void sendClansToClients (const cList<cPlayer*>* playerList)
{
	for (unsigned int n = 0; n < playerList->Size(); n++)
		sendClans (playerList, (*playerList)[n]);
}

//-------------------------------------------------------------------------------------
void sendSetAutomoving ( cVehicle *Vehicle  )
{
	cNetMessage* message = new cNetMessage( GAME_EV_SET_AUTOMOVE );
	message->pushInt16 ( Vehicle->iID );
	Server->sendNetMessage( message, Vehicle->owner->Nr );
}

//-------------------------------------------------------------------------------------
void sendCommandoAnswer ( bool succsess, bool steal, cVehicle *srcUnit, int player )
{
	cNetMessage* message = new cNetMessage( GAME_EV_COMMANDO_ANSWER );
	message->pushInt16 ( srcUnit->iID );
	message->pushBool ( steal );
	message->pushBool ( succsess );
	Server->sendNetMessage( message, player );
}

//-------------------------------------------------------------------------------------
void sendRequestSaveInfo ( int saveingID )
{
	cNetMessage* message = new cNetMessage( GAME_EV_REQ_SAVE_INFO );
	message->pushInt16 ( saveingID );
	Server->sendNetMessage( message );
}

//-------------------------------------------------------------------------------------
void sendSavedReport ( sSavedReportMessage &savedReport, int player )
{
	cNetMessage* message = new cNetMessage( GAME_EV_SAVED_REPORT );
	message->pushInt16 ( savedReport.colorNr );
	message->pushInt16 ( savedReport.unitID.iSecondPart );
	message->pushInt16 ( savedReport.unitID.iFirstPart );
	message->pushInt16 ( savedReport.yPos );
	message->pushInt16 ( savedReport.xPos );
	message->pushInt16 ( savedReport.type );
	message->pushString ( savedReport.message );
	Server->sendNetMessage( message, player );
}

//-------------------------------------------------------------------------------------
void sendCasualtiesReport (int player)
{
	cCasualtiesTracker* casualtiesTracker = Server->getCasualtiesTracker ();
	if (casualtiesTracker)
	{
		cList<cNetMessage*> messages;
		casualtiesTracker->prepareNetMessagesForClient (messages, GAME_EV_CASUALTIES_REPORT);
		for (size_t i = 0; i < messages.Size (); i++)
			Server->sendNetMessage (messages[i], player);
	}
}

//-------------------------------------------------------------------------------------
void sendSelfDestroy( cBuilding* building )
{
	cNetMessage* message = new cNetMessage( GAME_EV_SELFDESTROY );
	message->pushInt16( building->iID );
	Server->sendNetMessage(message, building->owner->Nr);

	for ( unsigned int i = 0; i < building->seenByPlayerList.Size(); i++ )
	{
		cNetMessage* message = new cNetMessage( GAME_EV_SELFDESTROY );
		message->pushInt16( building->iID );
		Server->sendNetMessage(message, building->seenByPlayerList[i]->Nr);
	}
}

void sendEndMoveActionToClient(cVehicle* vehicle, int destID, eEndMoveActionType type )
{
	cNetMessage* message = new cNetMessage( GAME_EV_END_MOVE_ACTION_SERVER );
	message->pushChar( type );
	message->pushInt32(destID);
	message->pushInt32(vehicle->iID);

	Server->sendNetMessage(message, vehicle->owner->Nr);
}
