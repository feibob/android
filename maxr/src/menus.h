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
#ifndef menusH
#define menusH

#include "autosurface.h"
#include "defines.h"
#include "menuitems.h"
#include "input.h"
#include "server.h"
#include "base.h" // for sSubBase

// forward declarations
int GetColorNr(SDL_Surface *sf);
class cMapReceiver;
class cMapSender;
class cServer;

struct sColor
{
	unsigned char cBlue, cGreen, cRed;
};

struct sLandingUnit
{
	sID unitID;
	int cargo;
};

enum eLandingState
{
	LANDING_STATE_UNKNOWN,		//initial state
	LANDING_POSITION_OK,		//there are no other players near the position
	LANDING_POSITION_WARNING,	//there are players within the warning distance
	LANDING_POSITION_TOO_CLOSE,	//the position is too close to another player
	LANDING_POSITION_CONFIRMED	//warnings about nearby players will be ignored, because the player has confirmed his position
};

struct sClientLandData
{
	int iLandX, iLandY;
	int iLastLandX, iLastLandY;
	eLandingState landingState;
	bool receivedOK;

	sClientLandData() : iLandX(0), iLandY(0), iLastLandX(0), iLastLandY(0), landingState ( LANDING_STATE_UNKNOWN ), receivedOK(true) {}
};

enum eSettingResourceValue
{
	SETTING_RESVAL_LOW,
	SETTING_RESVAL_NORMAL,
	SETTING_RESVAL_MUCH,
	SETTING_RESVAL_MOST
};

enum eSettingResFrequency
{
	SETTING_RESFREQ_THIN,
	SETTING_RESFREQ_NORMAL,
	SETTING_RESFREQ_THICK,
	SETTING_RESFREQ_MOST
};

enum eSettingsCredits
{
	SETTING_CREDITS_LOWEST = 0,
	SETTING_CREDITS_LOWER = 50,
	SETTING_CREDITS_LOW = 100,
	SETTING_CREDITS_NORMAL = 150,
	SETTING_CREDITS_MUCH = 200,
	SETTING_CREDITS_MORE = 250
};

enum eSettingsBridgeHead
{
	SETTING_BRIDGEHEAD_MOBILE,
	SETTING_BRIDGEHEAD_DEFINITE
};

enum eSettingsAlienTech
{
	SETTING_ALIENTECH_ON,
	SETTING_ALIENTECH_OFF
};

enum eSettingsClans
{
	SETTING_CLANS_ON,
	SETTING_CLANS_OFF
};

enum eSettingsGameType
{
	SETTINGS_GAMETYPE_SIMU,
	SETTINGS_GAMETYPE_TURNS
};

enum eSettingsVictoryType
{
	SETTINGS_VICTORY_TURNS,
	SETTINGS_VICTORY_POINTS,
	SETTINGS_VICTORY_ANNIHILATION
};

enum eSettingsDuration
{
	SETTINGS_DUR_SHORT = 100,
	SETTINGS_DUR_MEDIUM = 200,
	SETTINGS_DUR_LONG = 400
};

/**
 * A class that containes all settings for a new game.
 *@author alzi
 */
struct sSettings
{
	eSettingResourceValue metal, oil, gold;
	eSettingResFrequency resFrequency;
	eSettingsCredits credits;
	eSettingsBridgeHead bridgeHead;
	eSettingsAlienTech alienTech;
	eSettingsClans clans;
	eSettingsGameType gameType;
	eSettingsVictoryType victoryType;
	eSettingsDuration duration;

	sSettings() : metal(SETTING_RESVAL_NORMAL), oil(SETTING_RESVAL_NORMAL), gold(SETTING_RESVAL_NORMAL), resFrequency(SETTING_RESFREQ_NORMAL), credits(SETTING_CREDITS_NORMAL),
	bridgeHead (SETTING_BRIDGEHEAD_DEFINITE), alienTech(SETTING_ALIENTECH_OFF), clans(SETTING_CLANS_ON), gameType(SETTINGS_GAMETYPE_SIMU), victoryType(SETTINGS_VICTORY_POINTS),
	duration(SETTINGS_DUR_MEDIUM) {}

	std::string getResValString ( eSettingResourceValue type );
	std::string getResFreqString();
	std::string getVictoryConditionString();

};

/**
 * A class that containes all information to start a new or loaded game.
 * This class can run games automaticaly out of this information.
 *@author alzi
 */
class cGameDataContainer
{
public:

	/** The type of the game. See eGameTypes*/
	eGameTypes type;
	/** Should this instance of maxr act as the server for a TCP/IP game. */
	bool isServer;

	/** Number of the savegame or -1 for no savegame*/
	int savegameNum;
	/** name of the savegame if the savefile is only on the server and this container is set by an client*/
	std::string savegame;

	/** The settings for the game*/
	sSettings *settings;
	/** The map for the game*/
	cMap *map;

	/** list with all players for the game*/
	cList<cPlayer*> players;
	/** list with the selected landing units by each player*/
	cList<cList<sLandingUnit>*> landingUnits;
	/** the client landing data (landing positions) of the players*/
	cList<sClientLandData*> landData;
	/** indicates, whether all players have landed */
	bool allLanded;

	cGameDataContainer() :
		type(GAME_TYPE_SINGLE),
		isServer(false),
		savegameNum(-1),
		settings(0),
		map(0),
		allLanded(false)
	{}

	~cGameDataContainer();

	/** Runs the game. If isServer is true, which means that he is the host, a server will be started.
	 * Else only a client will be started. When reconnect is true, it will be reconnected to a running game.
	 * When the conatainer contains a savegamenumber, the savegame will be loaded
	 *@author alzi
	 */
	void runGame( int playerNr, bool reconnect = false );

	/** handles incoming clan information
	 *  @author pagra */
	void receiveClan ( cNetMessage *message );

	/** handles incoming landing units
	 *@author alzi
	 */
	void receiveLandingUnits ( cNetMessage *message );

	/** handles incoming unit upgrades
	 *@author alzi
	 */
	void receiveUnitUpgrades ( cNetMessage *message );

	/** handles an incoming landing position
	 *@author alzi
	 */
	void receiveLandingPosition ( cNetMessage *message );

private:
	/** checks whether the landing positions are okay
	 *@author alzi
	 */
	eLandingState checkLandingState( int playerNr );
	/** loads and runs a saved game
	 *@author alzi
	 */
	void runSavedGame( int player );
};

enum eMenuBackgrounds
{
	MNU_BG_BLACK,
	MNU_BG_ALPHA,
	MNU_BG_TRANSPARENT
};

/**
 * The main menu class. This class handles the background, the position, the input from mouse and keyboard
 * and all the menu items as buttons, images, labels, etc. All menuclasses in maxr should be a child of
 * this class.
 *@author alzi
 */
class cMenu
{
protected:
	bool drawnEveryFrame;
	/** When this is true the show-loop will be end and give 0 as return.
	 * Should be used when the menu is closed by ok or done.
	 */
	bool end;
	/** When this is true the show-loop will be ended and give 1 as return.
	 * Should be used when the menu is closed by abort or back.
	 */
	bool terminate;

	/** The background of the menu. Can be smaller than the screen. */
	AutoSurface background;
	/** The type of the background behind the menu background image, when the image is smaller then the screen. */
	eMenuBackgrounds backgroundType;
	/** The position of the menu on the screen when it is smaller than the screen. The position will be
	 * calculated in the constructor of cMenu and set to the center of the screen.
	 */
	SDL_Rect position;

	cList<cMenuTimerBase*> menuTimers;
	/** The list with all menuitems (buttons, images, etc.) of this menu. */
	cList<cMenuItem*> menuItems;
	/** Pointer to the currently active menuitem. This one will receive keyboard input. */
	cMenuItem *activeItem;

	/**
	 * initializes members and calculates the menu position on the screen.
	 *@author alzi
	 *@param background_ The background of the surface. Automatically gets deleted
	 *                   when the menu is destroyed.
	 */
	cMenu( SDL_Surface *background_, eMenuBackgrounds backgroundType_ = MNU_BG_BLACK );

	virtual void preDrawFunction() {};

	/** Recalculates the position and size of the menu.
	 */
	virtual void recalcPosition( bool resetItemPositions );

public:
	/**
	* virtual destructor
	*/
	virtual ~cMenu();
	/**
	 * redraws the menu background, the cursor and all menuitems.
	 *@author alzi
	 */
	void draw( bool firstDraw = false, bool showScreen = true );
	/**
	 * displays the menu and focuses all input on this menu until end or terminate are set to true.
	 *@author alzi
	 */
	virtual int show();
	/**
	 * sets end to true what will close the menu.
	 *@author alzi
	 */
	void close();
	virtual void returnToCallback();

	/**
	 * will the menu be closed after finishing the current action?
	 *@author eiko
	 */
	bool exiting();

	/**
	 * handles mouseclicks, delegates them to the matching menuitem and handles the activity of the menuitems.
	 *@author alzi
	 */
	void handleMouseInput( sMouseState mouseState );
	/**
	 * gives the opinion to handle the mouse input to childclasses.
	 * This function is called at the end of handleMouseInput().
	 *@author alzi
	 */
	virtual void handleMouseInputExtended( sMouseState mouseState ) {}
	/**
	 * delegates the keyinput to the active menuitem.
	 *@author alzi
	 */
	virtual void handleKeyInput( SDL_KeyboardEvent &key, std::string ch );

	/**
	 * sends a netmessage to the given player.
	 *@author alzi
	 */
	static void sendMessage ( cNetMessage *message, sMenuPlayer *player = NULL, int fromPlayerNr = -1 );
	/**
	 * this method will receive the menu-net-messages when this menu is active in the moment the message
	 * has been received. If the message should be handled overwrite this virtual method.
	 *@author alzi
	 */
	virtual void handleNetMessage( cNetMessage *message ) {}
	virtual void handleDestroyUnit( cBuilding *building = NULL, cVehicle *vehicle = NULL ) {}

	void addTimer(cMenuTimerBase* timer);

private:
	int lastScreenResX, lastScreenResY;
};

/** pointer to the currently active menu or NULL if no menu is active */
EX cMenu *ActiveMenu;

/**
 * A main menu with unit info image and a credits label at the bottom.
 *@author alzi
 */
class cMainMenu : public cMenu
{
	cMenuImage *infoImage;
	cMenuLabel *creditsLabel;

public:
	cMainMenu();
	~cMainMenu();

	SDL_Surface *getRandomInfoImage();
	static void infoImageReleased( void* parent );
};

/**
 * The menu in the very beginning.
 *@author alzi
 */
class cStartMenu : public cMainMenu
{
	cMenuLabel *titleLabel;
	cMenuButton *singleButton;
	cMenuButton *multiButton;
	cMenuButton *preferenceButton;
	cMenuButton *licenceButton;
	cMenuButton *exitButton;

public:
	cStartMenu();
	~cStartMenu();

	static void singlePlayerReleased( void* parent );
	static void multiPlayerReleased( void* parent );
	static void preferencesReleased( void* parent );
	static void licenceReleased( void* parent );
	static void exitReleased( void* parent );
};

/**
 * The singleplayer menu.
 *@author alzi
 */
class cSinglePlayerMenu : public cMainMenu
{
	cMenuLabel *titleLabel;
	cMenuButton *newGameButton;
	cMenuButton *loadGameButton;
	cMenuButton *backButton;
public:
	cSinglePlayerMenu();
	~cSinglePlayerMenu();

	static void newGameReleased( void* parent );
	static void loadGameReleased( void* parent );
	static void backReleased( void* parent );
};

/**
 * The multiplayer menu.
 *@author alzi
 */
class cMultiPlayersMenu : public cMainMenu
{
	cMenuLabel *titleLabel;
	cMenuButton *tcpHostButton;
	cMenuButton *tcpClientButton;
	cMenuButton *newHotseatButton;
	cMenuButton *loadHotseatButton;
	cMenuButton *backButton;
public:
	cMultiPlayersMenu();
	~cMultiPlayersMenu();

	static void tcpHostReleased( void* parent );
	static void tcpClientReleased( void* parent );
	static void newHotseatReleased( void* parent );
	static void loadHotseatReleased( void* parent );
	static void backReleased( void* parent );
};

/**
 * The settings menu.
 *@author alzi
 */
class cSettingsMenu : public cMenu
{
protected:
	cGameDataContainer *gameDataContainer;
	sSettings settings;

	cMenuLabel *titleLabel;
	cMenuButton *okButton;
	cMenuButton *backButton;

	cMenuLabel *metalLabel;
	cMenuLabel *oilLabel;
	cMenuLabel *goldLabel;
	cMenuLabel *creditsLabel;
	cMenuLabel *bridgeheadLabel;
	//cMenuLabel *alienTechLabel;
	cMenuLabel *clansLabel;
	cMenuLabel *resFrequencyLabel;
	cMenuLabel *gameTypeLabel;
	cMenuLabel *victoryLabel;

	cMenuRadioGroup *metalGroup;
	cMenuRadioGroup *oilGroup;
	cMenuRadioGroup *goldGroup;
	cMenuRadioGroup *creditsGroup;
	cMenuRadioGroup *bridgeheadGroup;
	//cMenuRadioGroup *aliensGroup;
	cMenuRadioGroup *clansGroup;
	cMenuRadioGroup *resFrequencyGroup;
	cMenuRadioGroup *gameTypeGroup;
	cMenuRadioGroup *victoryGroup;

	void updateSettings();
public:
	cSettingsMenu( cGameDataContainer *gameDataContainer_ );
	~cSettingsMenu();

	static void backReleased( void* parent );
	static void okReleased( void* parent );
};

/**
 * The planet selection.
 *@author alzi
 */
class cPlanetsSelectionMenu : public cMenu
{
protected:
	cGameDataContainer *gameDataContainer;

	cMenuLabel *titleLabel;

	cMenuButton *okButton;
	cMenuButton *backButton;

	cMenuButton *arrowUpButton;
	cMenuButton *arrowDownButton;

	cMenuImage *planetImages[8];
	cMenuLabel *planetTitles[8];

	cList<std::string> *maps;
	int selectedMapIndex;
	int offset;

public:
	cPlanetsSelectionMenu( cGameDataContainer *gameDataContainer_ );
	~cPlanetsSelectionMenu();

	void loadMaps();
	void showMaps();

	static void backReleased( void* parent );
	static void okReleased( void* parent );
	static void arrowDownReleased( void* parent );
	static void arrowUpReleased( void* parent );
	static void mapReleased( void* parent );
};


/**
 * The clan selection.
 * @author pagra
 */
class cClanSelectionMenu : public cMenu
{
protected:
	cGameDataContainer *gameDataContainer;

	cMenuLabel *titleLabel;

	cMenuImage *clanImages[8];
	cMenuLabel *clanNames[8];
	cMenuLabel *clanDescription1;
	cMenuLabel *clanDescription2;
	cMenuLabel *clanShortDescription;

	cMenuButton *okButton;
	cMenuButton *backButton;

	cPlayer *player;
	int clan;

	void updateClanDescription ();

public:
	cClanSelectionMenu (cGameDataContainer* gameDataContainer_, cPlayer *player, bool noReturn );
	~cClanSelectionMenu ();

	static void clanSelected (void* parent);
	static void okReleased (void* parent);
	static void backReleased (void* parent);

	void handleNetMessage( cNetMessage *message );
};

/**
 * A standard hangar menu with one unit selection table, unit info image, unit description, unit details window
 * and two buttons (done and back).
 *@author alzi
 */
class cHangarMenu : public cMenu
{
protected:
	cPlayer *player;

	cMenuImage *infoImage;
	cMenuLabel *infoText;
	cMenuCheckButton *infoTextCheckBox;

	cMenuUnitDetailsBig *unitDetails;

	cMenuUnitsList *selectionList;
	cMenuUnitListItem *selectedUnit;

	cMenuButton *selListUpButton;
	cMenuButton *selListDownButton;

	cMenuButton *doneButton;
	cMenuButton *backButton;

	void drawUnitInformation();

	void (*selectionChangedFunc)(void *);
public:
	cHangarMenu( SDL_Surface *background_, cPlayer *player_, eMenuBackgrounds backgroundType_ = MNU_BG_BLACK );
	~cHangarMenu();

	static void infoCheckBoxClicked( void* parent );
	static void selListUpReleased( void* parent );
	static void selListDownReleased( void* parent );

	void setSelectedUnit( cMenuUnitListItem *selectedUnit_ );
	cMenuUnitListItem *getSelectedUnit();

	virtual void generateSelectionList() {}
};

/**
 * A hangar menu with a second unit table, where you can add units by double clicking in the first list.
 *@author alzi
 */
class cAdvListHangarMenu : virtual public cHangarMenu
{
protected:
	cMenuUnitsList *secondList;

	cMenuButton *secondListUpButton;
	cMenuButton *secondListDownButton;

	virtual bool checkAddOk ( cMenuUnitListItem *item ) { return true; }
	virtual void addedCallback ( cMenuUnitListItem *item ) {}
	virtual void removedCallback ( cMenuUnitListItem *item ) {}
public:
	cAdvListHangarMenu( SDL_Surface *background_, cPlayer *player_ );
	~cAdvListHangarMenu();

	static bool selListDoubleClicked( cMenuUnitsList* list, void* parent );
	static bool secondListDoubleClicked( cMenuUnitsList* list, void* parent );

	static void secondListUpReleased( void* parent );
	static void secondListDownReleased( void* parent );
};

/**
 * An upgrade hangar menu with filter checkbuttons for the unit selection list, goldbar and buttons for
 * upgrading units.
 *@author alzi
 */
class cUpgradeHangarMenu : virtual public cHangarMenu
{
protected:
	int credits;

	cMenuUpgradeFilter *upgradeFilter;
	cMenuUpgradeHandler *upgradeButtons;
	cMenuMaterialBar *goldBar;
	cMenuLabel *goldBarLabel;

	sUnitUpgrade (*unitUpgrades)[8];
	void initUpgrades( cPlayer *player );
public:
	cUpgradeHangarMenu( cPlayer *owner );
	~cUpgradeHangarMenu();

	void setCredits( int credits_ );
	int getCredits();
};

/**
 * The hangar menu where you select your landing units in the beginning of a new game.
 *@author alzi
 */
class cStartupHangarMenu : public cUpgradeHangarMenu, public cAdvListHangarMenu
{
protected:
	cGameDataContainer *gameDataContainer;

	cMenuRadioGroup *upgradeBuyGroup;

	cMenuMaterialBar *materialBar;

	cMenuLabel *materialBarLabel;

	cMenuButton *materialBarUpButton;
	cMenuButton *materialBarDownButton;

	bool isInLandingList( cMenuUnitListItem *item );

	bool checkAddOk ( cMenuUnitListItem *item );
	void addedCallback ( cMenuUnitListItem *item );
	void removedCallback ( cMenuUnitListItem *item );

	void updateUnitData();
public:
	cStartupHangarMenu( cGameDataContainer *gameDataContainer_, cPlayer *player_, bool noReturn );
	~cStartupHangarMenu();

	static void selectionChanged( void* parent );

	static void backReleased( void* parent );
	static void doneReleased( void* parent );
	static void subButtonsChanged( void* parent );
	static void materialBarUpReleased( void* parent );
	static void materialBarDownReleased( void* parent );
	static void materialBarClicked( void* parent );

	void handleNetMessage( cNetMessage *message );

	void generateSelectionList();
};

/**
 * The landingposition selection menu.
 *@author alzi
 */
class cLandingMenu : public cMenu
{
protected:
	cGameDataContainer *gameDataContainer;

	cPlayer *player;

	cMap *map;

	AutoSurface hudSurface;
	AutoSurface mapSurface;

	cMenuImage *hudImage;
	cMenuImage *mapImage;
	cMenuImage *circlesImage;
	cMenuLabel *infoLabel;

	sClientLandData landData;

	void createHud();
	void createMap();
	sTerrain *getMapTile ( int x, int y );
	void hitPosition();
public:
	cLandingMenu( cGameDataContainer *gameDataContainer_, cPlayer *player_ );
	~cLandingMenu();

	static void mapClicked( void* parent );
	static void mouseMoved( void* parent );

	void handleKeyInput( SDL_KeyboardEvent &key, std::string ch );
	void handleNetMessage( cNetMessage *message );
};

/**
 * A standard menu for network TCP/IP games with ip, port, and playername lineedits,
 * chat lineedit and chat window, color selection and playerlist and map image.
 *@author alzi
 */
class cNetworkMenu : public cMenu
{
protected:
	std::string ip;
	int port;
	cList<sMenuPlayer*> players;
	sMenuPlayer *actPlayer;

	cMenuButton *backButton;
	cMenuButton *sendButton;

	cMenuImage *mapImage;
	cMenuLabel *mapLabel;

	cMenuLabel *settingsText;

	cMenuListBox *chatBox;
	cMenuLineEdit *chatLine;

	cMenuLabel *ipLabel;
	cMenuLabel *portLabel;
	cMenuLabel *nameLabel;
	cMenuLabel *colorLabel;

	cMenuButton *nextColorButton;
	cMenuButton *prevColorButton;
	cMenuImage *colorImage;

	cMenuLineEdit *ipLine;
	cMenuLineEdit *portLine;
	cMenuImage *setDefaultPortImage;
	cMenuLineEdit *nameLine;

	cMenuPlayersBox *playersBox;

	cGameDataContainer gameDataContainer;
	std::string saveGameString;
	std::string triedLoadMap;

	void showSettingsText();
	void showMap();
	void setColor( int color );
	void saveOptions();
	void changePlayerReadyState( sMenuPlayer *player );
	bool enteredCommand( std::string text );

public:
	cNetworkMenu();
	~cNetworkMenu();

	void playerReadyClicked ( sMenuPlayer *player );

	static void backReleased( void* parent );
	static void sendReleased( void* parent );

	static void nextColorReleased( void* parent );
	static void prevColorReleased( void* parent );

	static void wasNameImput( void* parent );
	static void portIpChanged( void* parent );
	static void setDefaultPort (void* parent );

	virtual void playerSettingsChanged () {}
};

/**
 * The host network menu.
 *@author alzi
 */
class cNetworkHostMenu : public cNetworkMenu
{
protected:
	cMenuLabel *titleLabel;

	cMenuButton *okButton;

	cMenuButton *mapButton;
	cMenuButton *settingsButton;
	cMenuButton *loadButton;
	cMenuButton *startButton;

	int checkAllPlayersReady();
	void checkTakenPlayerAttr( sMenuPlayer *player );
	bool runSavedGame();

	std::vector<cMapSender*> mapSenders;

public:
	cNetworkHostMenu();
	~cNetworkHostMenu();

	static void okReleased( void* parent );
	static void mapReleased( void* parent );
	static void settingsReleased( void* parent );
	static void loadReleased( void* parent );
	static void startReleased( void* parent );

	void handleNetMessage( cNetMessage *message );
	void playerSettingsChanged ();
};

/**
 * The client network menu
 *@author alzi
 */
class cNetworkClientMenu : public cNetworkMenu
{
	cMenuLabel *titleLabel;
	cMenuButton *connectButton;

	cMapReceiver* mapReceiver;
	std::string lastRequestedMap;
	void initMapDownload (cNetMessage* message);
	void receiveMapData (cNetMessage* message);
	void canceledMapDownload (cNetMessage* message);
	void finishedMapDownload (cNetMessage* message);

public:
	cNetworkClientMenu();
	~cNetworkClientMenu();

	static void connectReleased( void* parent );
	void handleNetMessage( cNetMessage *message );
	void playerSettingsChanged ();
};

/**
 * The load menu.
 *@author alzi
 */
class cLoadMenu : public cMenu
{
protected:
	cGameDataContainer *gameDataContainer;

	cMenuLabel *titleLabel;

	cMenuButton *backButton;
	cMenuButton *loadButton;

	cMenuButton *upButton;
	cMenuButton *downButton;

	cMenuSaveSlot *saveSlots[10];

	cList<std::string> *files;
	cList<sSaveFile*> savefiles;

	int offset;
	int selected;

	void loadSaves();
	void displaySaves();

public:
	cLoadMenu( cGameDataContainer *gameDataContainer_, eMenuBackgrounds backgroundType_ = MNU_BG_BLACK );
	~cLoadMenu();

	static void backReleased( void* parent );
	static void loadReleased( void* parent );

	static void upReleased( void* parent );
	static void downReleased( void* parent );

	static void slotClicked( void* parent );

	virtual void extendedSlotClicked( int oldSelection ) {}
};

/**
 * The load and save menu (ingame data menu).
 *@author alzi
 */
class cLoadSaveMenu : public cLoadMenu
{
protected:
	cMenuButton *exitButton;
	cMenuButton *saveButton;

public:
	cLoadSaveMenu( cGameDataContainer *gameDataContainer_ );
	~cLoadSaveMenu();

	static void exitReleased( void* parent );
	static void saveReleased( void* parent );

	void extendedSlotClicked( int oldSelection );
};

/**
 * The menu to build buildings.
 *@author alzi
 */
class cBuildingsBuildMenu : public cHangarMenu
{
protected:
	cVehicle *vehicle;

	cMenuLabel *titleLabel;
	cMenuButton *pathButton;
	cMenuBuildSpeedHandler *speedHandler;

public:
	cBuildingsBuildMenu( cPlayer *player_, cVehicle *vehicle_ );
	~cBuildingsBuildMenu();

	static void doneReleased ( void *parent );
	static void backReleased ( void *parent );
	static void pathReleased ( void *parent );
	static void selectionChanged ( void *parent );
	static bool selListDoubleClicked ( cMenuUnitsList* list, void *parent );

	void generateSelectionList();
	void handleDestroyUnit( cBuilding *destroyedBuilding = NULL, cVehicle *destroyedVehicle = NULL );
};

/**
 * The menu to build vehicles.
 *@author alzi
 */
class cVehiclesBuildMenu : public cAdvListHangarMenu
{
protected:
	cBuilding *building;

	cMenuLabel *titleLabel;
	cMenuBuildSpeedHandler *speedHandler;

	cMenuCheckButton *repeatButton;

	void createBuildList();
public:
	cVehiclesBuildMenu( cPlayer *player_, cBuilding *building_ );
	~cVehiclesBuildMenu();

	static void doneReleased ( void *parent );
	static void backReleased ( void *parent );
	static void selectionChanged ( void *parent );

	void generateSelectionList();
	void handleDestroyUnit( cBuilding *destroyedBuilding = NULL, cVehicle *destroyedVehicle = NULL );
};

/**
 * The upgrade menu.
 *@author alzi
 */
class cUpgradeMenu : public cUpgradeHangarMenu
{
	static bool tank;
	static bool plane;
	static bool ship;
	static bool build;
	static bool tnt;
protected:
public:
	cUpgradeMenu( cPlayer *player );

	static void doneReleased ( void *parent );
	static void backReleased ( void *parent );
	static void selectionChanged ( void *parent );

	void generateSelectionList();
};

class cUnitHelpMenu : public cMenu
{
protected:
	cMenuLabel *titleLabel;

	cMenuImage *infoImage;
	cMenuLabel *infoText;

	cMenuUnitDetailsBig *unitDetails;

	cMenuButton *doneButton;

	cMenuUnitListItem *unit;

	void init(sID unitID);
public:
	cUnitHelpMenu( sID unitID, cPlayer *owner );
	cUnitHelpMenu( sUnitData* unitData, cPlayer *owner );

	~cUnitHelpMenu();

	static void doneReleased( void *parent );
	void handleDestroyUnit( cBuilding *destroyedBuilding = NULL, cVehicle *destroyedVehicle = NULL );
};

class cStorageMenu : public cMenu
{
friend class cClient;
protected:
	cVehicle *ownerVehicle;
	cBuilding *ownerBuilding;
	cList<cUnit*>& storageList;
	sUnitData unitData;
	sSubBase *subBase;

	bool canStorePlanes;
	bool canStoreShips;
	bool canRepairReloadUpgrade;

	bool voiceTypeAll;
	bool voicePlayed;

	int metalValue;

	int offset;

	cMenuButton *doneButton;
	cMenuButton *downButton;
	cMenuButton *upButton;

	cMenuImage *unitImages[6];
	cMenuLabel *unitNames[6];
	cMenuStoredUnitDetails *unitInfo[6];

	cMenuButton *activateButtons[6];
	cMenuButton *relaodButtons[6];
	cMenuButton *repairButtons[6];
	cMenuButton *upgradeButtons[6];

	cMenuButton *activateAllButton;
	cMenuButton *reloadAllButton;
	cMenuButton *repairAllButton;
	cMenuButton *upgradeAllButton;

	cMenuMaterialBar *metalBar;

	void generateItems();

	void resetInfos();

	int getClickedButtonVehIndex ( cMenuButton *buttons[6] );
public:
	cStorageMenu( cList<cUnit*>& storageList_, cVehicle *vehicle, cBuilding *building );
	~cStorageMenu();

	static void doneReleased( void *parent );

	static void upReleased( void *parent );
	static void downReleased( void *parent );

	static void activateReleased ( void *parent );
	static void reloadReleased ( void *parent );
	static void repairReleased ( void *parent );
	static void upgradeReleased ( void *parent );

	static void activateAllReleased ( void *parent );
	static void reloadAllReleased ( void *parent );
	static void repairAllReleased ( void *parent );
	static void upgradeAllReleased ( void *parent );

	void playVoice( int Type );
	void handleDestroyUnit( cBuilding *destroyedBuilding = NULL, cVehicle *destroyedVehicle = NULL );
};

class cMineManagerMenu : public cMenu
{
	cBuilding *building;
	sSubBase subBase;

	cMenuLabel *titleLabel;
	cMenuButton *doneButton;

	cMenuMaterialBar *metalBars[3];
	cMenuMaterialBar *oilBars[3];
	cMenuMaterialBar *goldBars[3];

	cMenuLabel *metalBarLabels[3];
	cMenuLabel *oilBarLabels[3];
	cMenuLabel *goldBarLabels[3];

	cMenuMaterialBar *noneBars[3];

	cMenuLabel *resourceLabels[3];
	cMenuLabel *usageLabels[3];
	cMenuLabel *reserveLabels[3];

	cMenuButton *incButtons[3];
	cMenuButton *decButtons[3];

	void setBarValues();
	void setBarLabels();

	std::string secondBarText( int prod, int need );
public:
	cMineManagerMenu( cBuilding *building_ );
	~cMineManagerMenu();

	static void doneReleased( void *parent );

	static void increaseReleased( void *parent );
	static void decreseReleased( void *parent );

	static void barReleased( void *parent );

	void handleDestroyUnit( cBuilding *destroyedBuilding = NULL, cVehicle *destroyedVehicle = NULL );
};

class cReportsMenu : public cMenu
{
	cPlayer *owner;

	cMenuRadioGroup *typeButtonGroup;

	cMenuLabel *includedLabel;
	cMenuCheckButton *planesCheckBtn;
	cMenuCheckButton *groundCheckBtn;
	cMenuCheckButton *seaCheckBtn;
	cMenuCheckButton *stationaryCheckBtn;

	cMenuLabel *borderedLabel;
	cMenuCheckButton *buildCheckBtn;
	cMenuCheckButton *fightCheckBtn;
	cMenuCheckButton *damagedCheckBtn;
	cMenuCheckButton *stealthCheckBtn;

	cMenuButton *doneButton;
	cMenuButton *upButton;
	cMenuButton *downButton;

	cMenuReportsScreen *dataScreen;
public:
	cReportsMenu( cPlayer *owner_ );
	~cReportsMenu();

	static void doneReleased( void *parent );

	static void upReleased( void *parent );
	static void downReleased( void *parent );

	static void typeChanged( void *parent );

	static void filterClicked( void *parent );

	void scrollCallback ( bool upPossible, bool downPossible );

	void doubleClicked ( cVehicle *vehicle, cBuilding *building );
};

#endif //menusH
