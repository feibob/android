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
#ifndef menuitemsH
#define menuitemsH

#include <vector>
#include "autosurface.h"
#include "defines.h"
#include "unifonts.h"
#include "upgradecalculator.h"
#include "sound.h"
#include "clist.h"
#include "main.h" // for sID
#include "notifications.h"

class cMenu;
class cHangarMenu;
class cUpgradeHangarMenu;
class cNetworkMenu;
class cMenuRadioGroup;
class cMenuUnitsList;
class cReportsMenu;
class cVehicle;
class cBuilding;
class cUnit;

/**
 * A struct that contains information of a savegame.
 *@author alzi
 */
struct sSaveFile
{
	/** the filename of the savegame*/
	std::string filename;
	/** the displayed name of the savegame*/
	std::string gamename;
	/** the type of the savegame (SIN, HOT, NET)*/
	std::string type;
	/** the time and date when this savegame was saved*/
	std::string time;
	/** the number of the savegame*/
	int number;
};

/**
 * A struct that contains information about the upgrades of a unit.
 *@author alzi
 */
struct sUnitUpgrade
{
	/** The diffrent values of a unit that can be upgraded*/
	enum eUpgradeTypes
	{
		UPGRADE_TYPE_DAMAGE,
		UPGRADE_TYPE_SHOTS,
		UPGRADE_TYPE_RANGE,
		UPGRADE_TYPE_AMMO,
		UPGRADE_TYPE_ARMOR,
		UPGRADE_TYPE_HITS,
		UPGRADE_TYPE_SCAN,
		UPGRADE_TYPE_SPEED,
		UPGRADE_TYPE_NONE
	};

	/** is this upgrade buyable for the player */
	bool active;
	/** what will the next upgrade cost */
	int nextPrice;
	/** how many upgrades of this type has the player purchased */
	int purchased;
	/** what is the current value */
	int curValue;
	/** the value that this unit would have without all upgrades */
	int startValue;
	/** the type of the upgrade */
	eUpgradeTypes type;

	sUnitUpgrade() : active(false), nextPrice(0), purchased(0), curValue(-1), startValue(0), type(UPGRADE_TYPE_NONE) {}
};

/**
 * The basic class for menuitems such as buttons, images, labels, etc. All menuitems have to be children
 * of this class to be addable to the menu's item list. This class handles the clicks etc. on the item.
 *@author alzi
 */
class cMenuItem
{
friend class cMenuItemContainer;
protected:
	/** The sound that should be played then the item is clicked or NULL for no sound*/
	sSOUND *clickSound;
	/** The sound that should be played then mousebutton is released over the item or NULL for no sound*/
	sSOUND *releaseSound;

	/** pointer to the extern function that will be called when the item was clicked. The void* parameter should
	* be a pointer to the menu or item that contains this function, so it can be used as "this-pointer" in
	* a static function. (The function has to be static to get a pointer on the function)*/
	void (*click)(void *);
	/** pointer to the extern function that will be called when the item was released */
	void (*release)(void *);
	/** pointer to the extern  function that will be called when the mouse was moved from somewhere outside the item
	* to a position over the item */
	void (*hoverOn)(void *);
	/** pointer to the extern  function that will be called when the mouse was moved from a position over the item to
	* somewhere outside the item */
	void (*hoverAway)(void *);
	/** pointer to the extern  function that will be called when the mouse has been moved over the item */
	void (*moveMouseOver)(void *);
	/** pointer to the extern  function that will be called when the item is active and keyboard input has been received */
	void (*wasKeyInput)(void *);
	/** pointer to the extern function that will be called when the item is right clicked */
	void (*rightClick)(void *);

	cMenuItem( int x, int y );

	/** position and size of the item */
	SDL_Rect position;
	/** true then the item is the currently active item of the menu */
	bool active;

	/** intern handler for the clicked status */
	bool wasClicked;
	/** intern handler for the clicked status */
	bool isClicked;
	/** if this is true, the item is always in clicked state and no clicked, release, etc. functions will be called anymore */
	bool locked;
	/** if this is true the menu will not handle any clicks or keys on this item */
	bool disabled;

	/**
	 * Function that will be called when the item has been clicked.
	 *@author alzi
	 *@return if 'false' the click-pointer-function will not be called.
	 */
	virtual bool preClicked() { return true; }
	/**
	 * Function that will be called when the mouse has been released over this item before calling the release-pointer-function.
	 *@author alzi
	 *@return if 'false' the release-pointer-function will not be called.
	 */
	virtual bool preReleased();
	/**
	 * Function that will be called when the mouse has been released over this item after calling the release-pointer-function.
	 *@author alzi
	 */
	virtual void postReleased() {}
	/**
	 * Function that will be called when the mouse hovers onto the item.
	 *@author alzi
	 *@return if 'false' the hoverOn-pointer-function will not be called.
	 */
	virtual bool preHoveredOn() { return true; }
	/**
	 * Function that will be called when the mouse hovers away from the item.
	 *@author alzi
	 *@return if 'false' the hoverAway-pointer-function will not be called.
	 */
	virtual bool preHoveredAway() { return true; }
	/**
	 * Function that will be called when the item will be locked.
	 *@author alzi
	 *@return if 'false' the item will not be locked.
	 */
	virtual bool preSetLocked( bool locked_ ) { return true; }

public:
	/**
	 * virtual functions that should draw the item.
	 *@author alzi
	 */
	virtual void draw() = 0;

	/**
	 * Sets the activity status of this item
	 *@author alzi
	 */
	virtual void setActivity ( bool active_ ) { active = active_; }
	/**
	 * returns whether the position is over the item or not.
	 *@author alzi
	 */
	virtual bool overItem( int x, int y ) const;

	/**
	 * This function will be called by the menus when the item has been clicked. Regularly the parent pointer
	 * should point to the calling object because this pointer will be passed to the click-pointer-function.
	 *@author alzi
	 */
	virtual void clicked( void *parent );
	/**
	 * This function will be called by the menus when the item has been released. Regularly the parent pointer
	 * should point to the calling object because this pointer will be passed to the release-pointer-function.
	 *@author alzi
	 */
	virtual void released( void *parent );
	/**
	 * This function will be called by the menus when the mouse hovered on this item. Regularly the parent pointer
	 * should point to the calling object because this pointer will be passed to the hoverOn-pointer-function.
	 *@author alzi
	 */
	virtual void hoveredOn( void *parent );
	/**
	 * This function will be called by the menus when the mouse hovered away from this item. Regularly the parent pointer
	 * should point to the calling object because this pointer will be passed to the hoverAway-pointer-function.
	 *@author alzi
	 */
	virtual void hoveredAway( void *parent );
	/**
	 * This function will be called by the menus when the mouse has moved over the item. Regularly the parent pointer
	 * should point to the calling object because this pointer will be passed to the moveMouseOver-pointer-function.
	 *@author alzi
	 */
	virtual void movedMouseOver( int lastMouseX, int lastMouseY, void *parent );

	/**
	 * This function will be called by the menus when the mousebutton has been released somewhere else then over this item.
	 *@author alzi
	 */
	virtual void somewhereReleased();
	/**
	 * This function will be called by the menus when this item is the active one and the mouse has been moved
	 * somewhere on the screen, but not over this item.
	 *@author alzi
	 */
	virtual void somewhereMoved() {}

	/**
	 * function that will be called when this item is the currently active one and there has been keyboard input.
	 *@author alzi
	 *@param keysym the SDL keysym with the information about the pressed key
	 *@param ch the encoded key
	 *@param parent pointer to the calling menu
	 */
	virtual bool handleKeyInput( SDL_keysym keysym, std::string ch, void *parent ) { return false; }


	/**
	 *@author eiko
	 * This function will be called by the menus when this item is clicked with the right mouse button
	 */
	virtual void rightClicked (void *parent);

	/**
	 * sets a new position of the item
	 *@author alzi
	 */
	virtual void move ( int x, int y );
	/**
	 * returns the current position of the item
	 *@author alzi
	 */
	SDL_Rect getPosition();
	/**
	 * locks or unlocks the item.
	 *@author alzi
	 */
	void setLocked( bool locked_ );
	/**
	 * disables or enables the item.
	 *@author alzi
	 */
	void setDisabled( bool disabled_ );
	/**
	 * sets a new clicked sound.
	 *@author alzi
	 */
	void setClickSound ( sSOUND *clickSound_ );
	/**
	 * sets a new released sound.
	 *@author alzi
	 */
	void setReleaseSound ( sSOUND *releaseSound_ );

	/**
	 * sets the click-pointer-function.
	 *@author alzi
	 */
	void setClickedFunction ( void (*click_)(void *) );

	/**
	 * sets the click-pointer-function.
	 *@author alzi
	 */
	void setRightClickedFunction ( void (*click_)(void *) );

	/**
	 * sets the release-pointer-function.
	 *@author alzi
	 */
	void setReleasedFunction ( void (*release_)(void *) );
	/**
	 * sets the moveMouseOver-pointer-function.
	 *@author alzi
	 */
	void setMovedOverFunction ( void (*moveMouseOver_)(void *) );
	/**
	 * sets the wasKeyInput-pointer-function.
	 *@author alzi
	 */
	void setWasKeyInputFunction ( void (*wasKeyInput_)(void *) );

	/**
	 * returns the status of the isClicked variable.
	 *@author alzi
	 */
	bool getIsClicked();
	/**
	 * returns the status of the wasClicked variable.
	 *@author alzi
	 */
	bool getWasClicked();
	/**
	 * returns whether the item is disabled or not.
	 *@author alzi
	 */
	bool isDisabled() const { return disabled; }

	bool isLocked() const { return locked; }
};

/**
 * a menu item that contains more menuitems. Mouse and keyboardevents will be given to the child items.
 *@author alzi
 */
class cMenuItemContainer : public cMenuItem
{
protected:
	cList<cMenuItem*> itemList;
public:
	cMenuItemContainer( int x, int y );
	virtual void draw();

	virtual void clicked( void *parent );
	virtual void released( void *parent );
	virtual void hoveredOn( void *parent );
	virtual void hoveredAway( void *parent );

	virtual void movedMouseOver( int lastMouseX, int lastMouseY, void *parent );
	virtual void somewhereReleased();

	void addItem ( cMenuItem* item );
	void removeItem ( cMenuItem* item );
};

class cMenuTimerBase
{
public:
	virtual void callback() = 0;
	bool getState();

protected:
	static Uint32 sdlTimerCallback( Uint32 intervall, void* param);

	bool state;
	cMenuTimerBase(Uint32 intervall);
	~cMenuTimerBase();
	SDL_TimerID timerID;

};

/**
* a timer class for adding timers to menus.
* This class is for adding timers with a
* callback on a non static member function.
*@author eiko
*/
template <typename T> class cMenuTimer : public cMenuTimerBase
{
public:
	/**
	* creates a new menu timer. After creation, the timer must be added to the menus timer list by calling menuTimers.Add()
	*@param obj_ the object, on which the callback will be called
	*@param method a pointer to the method, which will be executed
	*@intervall the timer intervall in milliseconds
	*@author eiko
	*/
	cMenuTimer(T& obj_, void (T::*method_)(), Uint32  intervall);

	/**
	* this function will be called by the menu to run the callback.
	*/
	void callback();

private:
	T& object;
	void (T::*method)();
};


template <typename T> cMenuTimer<T>::cMenuTimer(T& obj_, void (T::*method_)(), Uint32 intervall) :
	cMenuTimerBase(intervall),
	object(obj_),
	method(method_)
{}

template <typename T> void cMenuTimer<T>::callback()
{
	(object.*method)();
}


/**
 * a simple image.
 *@author alzi
 */
class cMenuImage : public cMenuItem
{
public:
	cMenuImage(int x, int y, SDL_Surface* image_ = NULL);
	void setImage( SDL_Surface *image_ );
	void draw();

private:
	AutoSurface image;
};

/**
 * a simple label that displays text.
 *@author alzi
 */
class cMenuLabel : public cMenuItem
{
protected:
	SDL_Rect textPosition;
	std::string text;
	eUnicodeFontType fontType;

	bool flagCentered;
	bool flagBox;

public:
	cMenuLabel ( int x, int y, std::string text_ = "", eUnicodeFontType fontType_= FONT_LATIN_NORMAL );
	void setText( std::string text_ );
	std::string getText() { return text; }
	void setFontType ( eUnicodeFontType fontType_ );
	/**
	 * if centered is true the text will be centered at the in the constructor overgiven position.
	 *@author alzi
	 */
	void setCentered( bool centered );
	/**
	 * sets the label to a fixed width and height and displays the text in this box.
	 *@author alzi
	 */
	void setBox( int width, int height );
	void draw();
	void move( int x, int y );
};

/**
 * a simple button.
 *@author alzi
 */
class cMenuButton : public cMenuItem
{
public:
	/** the diffrent button types that are provided by the graphics in menu_stuff.pcx*/
	enum eButtonTypes
	{
		BUTTON_TYPE_STANDARD_BIG,
		BUTTON_TYPE_STANDARD_SMALL,
		BUTTON_TYPE_HUGE,
		BUTTON_TYPE_ARROW_UP_BIG,
		BUTTON_TYPE_ARROW_DOWN_BIG,
		BUTTON_TYPE_ARROW_LEFT_BIG,
		BUTTON_TYPE_ARROW_RIGHT_BIG,
		BUTTON_TYPE_ARROW_UP_SMALL,
		BUTTON_TYPE_ARROW_DOWN_SMALL,
		BUTTON_TYPE_ARROW_LEFT_SMALL,
		BUTTON_TYPE_ARROW_RIGHT_SMALL,
		BUTTON_TYPE_ARROW_UP_BAR,
		BUTTON_TYPE_ARROW_DOWN_BAR,
		BUTTON_TYPE_ANGULAR,

		BUTTON_TYPE_HUD_HELP,
		BUTTON_TYPE_HUD_CENTER,
		BUTTON_TYPE_HUD_NEXT,
		BUTTON_TYPE_HUD_PREV,
		BUTTON_TYPE_HUD_DONE,
		BUTTON_TYPE_HUD_REPORT,
		BUTTON_TYPE_HUD_CHAT,

		BUTTON_TYPE_HUD_PREFERENCES,
		BUTTON_TYPE_HUD_FILES,
		BUTTON_TYPE_HUD_END,
		BUTTON_TYPE_HUD_PLAY,
		BUTTON_TYPE_HUD_STOP,
	};
protected:

	AutoSurface surface;
	std::string text;
	eUnicodeFontType fontType;
	eButtonTypes buttonType;

	void renewButtonSurface();
	void redraw();
	int getTextYOffset();
	int getBordersSize();

	bool preClicked();
	bool preReleased();
	void postReleased();
	bool preHoveredOn();
	bool preHoveredAway();

	bool preSetLocked( bool locked_ );
public:
	cMenuButton ( int x, int y, std::string text_ = "", eButtonTypes buttonType_ = BUTTON_TYPE_STANDARD_BIG, eUnicodeFontType fontType_ = FONT_LATIN_BIG, sSOUND *clickSound_ = SoundData.SNDHudButton );
	void draw();
};

/**
 * the big red destroy botton
 *@author eiko
 */
class cMenuDestroyButton : public cMenuItem
{

protected:
	int glassHeight;
	cMenuTimer<cMenuDestroyButton>* timer;

	void animationCallback();

	bool preClicked();
	bool preReleased();
	void postReleased();
	bool preHoveredOn();
	bool preHoveredAway();
	bool preSetLocked( bool locked_ );

public:
	cMenuDestroyButton ( int x, int y, cMenu* menu );
	~cMenuDestroyButton();
	void draw();

	bool opening;
};

/**
 * a simple checkbox or radiobutton.
 *@author alzi
 */
class cMenuCheckButton : public cMenuItem
{
friend class cMenuRadioGroup;

public:
	/** the diffrent button types that are provided by the graphics in menu_stuff.pcx*/
	enum eCheckButtonTypes
	{
		RADIOBTN_TYPE_TEXT_ONLY,
		RADIOBTN_TYPE_BTN_ROUND,
		RADIOBTN_TYPE_ANGULAR_BUTTON,
		CHECKBOX_TYPE_STANDARD,
		CHECKBOX_TYPE_TANK,
		CHECKBOX_TYPE_PLANE,
		CHECKBOX_TYPE_SHIP,
		CHECKBOX_TYPE_BUILD,
		CHECKBOX_TYPE_TNT,

		CHECKBOX_HUD_INDEX_00,
		CHECKBOX_HUD_INDEX_01,
		CHECKBOX_HUD_INDEX_02,
		CHECKBOX_HUD_INDEX_10,
		CHECKBOX_HUD_INDEX_11,
		CHECKBOX_HUD_INDEX_12,
		CHECKBOX_HUD_INDEX_20,
		CHECKBOX_HUD_INDEX_21,
		CHECKBOX_HUD_INDEX_22,

		CHECKBOX_HUD_LOCK,

		CHECKBOX_HUD_TNT,
		CHECKBOX_HUD_2X,
		CHECKBOX_HUD_PLAYERS,
	};

	/** deffines whether the text will be displayed at the left or the right of the item.
	 * Will not be used when the checkbox is a button where the text is centered.*/
	enum eCheckButtonTextOriantation
	{
		TEXT_ORIENT_RIGHT,
		TEXT_ORIENT_LEFT
	};
protected:
	std::string text;
	AutoSurface surface;
	eUnicodeFontType fontType;
	eCheckButtonTypes buttonType;
	eCheckButtonTextOriantation textOrientation;

	cMenuRadioGroup *group;

	bool centered;
	bool checked;
	int textLimitWight;

	void renewButtonSurface();

	bool preClicked();

public:
	cMenuCheckButton( int x, int y, std::string text_ = "", bool checked_ = false, bool centered_ = false,eCheckButtonTypes buttonType_ = RADIOBTN_TYPE_BTN_ROUND, eCheckButtonTextOriantation textOrientation = TEXT_ORIENT_RIGHT, eUnicodeFontType fontType_ = FONT_LATIN_NORMAL, sSOUND *clickSound_ = SoundData.SNDObjectMenu);
	void draw();

	void setChecked ( bool checked_ );
	bool isChecked();

	void limitTextSize ( int w );
};

/**
 * an item that handles a group of radiobuttons, and makes sure that alway only one of his childs is checked.
 *@author alzi
 */
class cMenuRadioGroup : public cMenuItem
{
friend class cMenuCheckButton;
protected:
	cList<cMenuCheckButton*> buttonList;

	void checkedButton ( cMenuCheckButton* button );

	bool overItem( int x, int y ) const;

	void clicked ( void *parent );
public:
	cMenuRadioGroup () : cMenuItem ( 0, 0 ) {}
	~cMenuRadioGroup();
	void draw();

	void addButton( cMenuCheckButton* button );
	bool buttonIsChecked ( int index );
};

/** The diffrent display types for a unit list item */
enum eMenuUnitListDisplayTypes
{
	/** Displays the unitimage, -name and -costs.*/
	MUL_DIS_TYPE_COSTS,
	/** Displays the unitimage, unitname and the acctual and maximum cargo.*/
	MUL_DIS_TYPE_CARGO,
	/** Displays only the unitimage and -name*/
	MUL_DIS_TYPE_NOEXTRA
};

/**
 * the item of a unitslist. This item handles some additional information as the cargo and handles the drawing.
 *@author alzi
 */
class cMenuUnitListItem : public cMenuItem
{
friend class cMenuUnitsList;
protected:
	eMenuUnitListDisplayTypes displayType;
	cMenuUnitsList* parentList;

	sID unitID;
	sUnitData* unitData;
	cPlayer *owner;

	AutoSurface surface;

	int resValue;
	int minResValue;
	sUnitUpgrade *upgrades;

	bool fixed;
	bool selected;
	bool fixedResValue;
	bool marked;

	int drawName( bool withNumber );
	void drawCargo( int destY );

	void released( void *parent );
	void init ();
public:
	cMenuUnitListItem( sID unitID_, cPlayer *owner_, sUnitUpgrade *upgrades_, eMenuUnitListDisplayTypes displayType_, cMenuUnitsList* parent, bool fixedResValue_ );
	cMenuUnitListItem( sUnitData *unitData_, cPlayer *owner_, sUnitUpgrade *upgrades_, eMenuUnitListDisplayTypes displayType_, cMenuUnitsList* parent, bool fixedResValue_ );
	void draw();

	sID getUnitID();
	sUnitData *getUnitData();
	cPlayer *getOwner();

	/**
	 * returns the res-value saved by the listitem.
	 *@author alzi
	 */
	int getResValue();
	/**
	 * returns whether the res-value is fixed or not.
	 *@author alzi
	 */
	bool getFixedResValue ();
	/**
	 * sets a new res-value to the unit. The value will automatically set to minResValue if it's smaller than this value.
	 *@author alzi
	 *@param resValue_ the new res-value
	 *@param cargoCheck if this is true the new res-value will automatically set to the maximum cargo that the unit can cargo,
	 * or to 0 when it's negative.
	 */
	void setResValue( int resValue_, bool cargoCheck = true );
	/**
	 * sets the minimum res-value.
	 *@author alzi
	 */
	void setMinResValue ( int minResValue_ );
	/**
	 * if the unititem is marked then name will be displayed red instead of white.
	 *@author alzi
	 */
	void setMarked ( bool marked_ );
	/**
	 * sets whether the res-value is fixed and can not be changed by setResValue.
	 *@author alzi
	 */
	void setFixedResValue ( bool fixedResValue_ );
	/**
	 * sets the fixed status of the item. If this is true it will for example not be removable of the secondList of the AdvListHangaMenu.
	 *@author alzi
	 */
	void setFixed ( bool fixed_ );
	/**
	 * returns the fixed status of the listitem.
	 *@author alzi
	 */
	bool getFixedStatus();

	sUnitUpgrade *getUpgrades();
	sUnitUpgrade *getUpgrade( sUnitUpgrade::eUpgradeTypes type );
};

/**
 * a list that displays units.
 *@author alzi
 */
class cMenuUnitsList : public cMenuItem
{
friend class cMenuUnitListItem;
protected:
	bool (*doubleClicked)(cMenuUnitsList *, void *parent );

	cHangarMenu *parentMenu;

	eMenuUnitListDisplayTypes displayType;

	cList<cMenuUnitListItem*> unitsList;
	cMenuUnitListItem *selectedUnit;
	int offset;
	int maxDisplayUnits;

	void released( void *parent );
	void addUnit ( cMenuUnitListItem *unitItem, bool scroll = false );

public:
	cMenuUnitsList( int x, int y, int w, int h, cHangarMenu *parent, eMenuUnitListDisplayTypes displayType_ );
	~cMenuUnitsList();
	void draw();

	int getSize();
	cMenuUnitListItem* getItem ( int index );
	cMenuUnitListItem* getSelectedUnit();

	void resize ( int w, int h );
	void setDoubleClickedFunction( bool (*doubleClicked_)(cMenuUnitsList *, void *parent ) );
	void scrollUp();
	void scrollDown();
	void setSelection ( cMenuUnitListItem *selectedUnit_ );
	/**
	 * adds a new unit to the list.
	 *@author alzi
	 *@param unitID the it of the new unit
	 *@param owner the owner of the unit
	 *@param upgrades the struct with the upgrade-values for this unit
	 *@param scroll	if this is true the list will automatically scrolled to the new added item.
	 *@param fixedCargo if this is true the new unit will set with a fixed cargo.
	 */
	cMenuUnitListItem *addUnit ( sID unitID, cPlayer *owner, sUnitUpgrade *upgrades = NULL, bool scroll = false, bool fixedCargo = false );
	cMenuUnitListItem *addUnit ( sUnitData *unitData, cPlayer *owner, sUnitUpgrade *upgrades = NULL, bool scroll = false, bool fixedCargo = false );
	void removeUnit ( cMenuUnitListItem *item );
	void clear();
	void setDisplayType ( eMenuUnitListDisplayTypes displayType_ );
};

class cUnitDataSymbolHandler
{
public:
	/** the diffrent symbol types */
	enum eUnitDataSymbols
	{
		MENU_SYMBOLS_SPEED,
		MENU_SYMBOLS_HITS,
		MENU_SYMBOLS_AMMO,
		MENU_SYMBOLS_ATTACK,
		MENU_SYMBOLS_SHOTS,
		MENU_SYMBOLS_RANGE,
		MENU_SYMBOLS_ARMOR,
		MENU_SYMBOLS_SCAN,
		MENU_SYMBOLS_METAL,
		MENU_SYMBOLS_OIL,
		MENU_SYMBOLS_GOLD,
		MENU_SYMBOLS_ENERGY,
		MENU_SYMBOLS_HUMAN,
		MENU_SYMBOLS_TRANS_TANK,
		MENU_SYMBOLS_TRANS_AIR
	};
	static void drawNumber ( int x, int y, int value, int maximalValue );
	static void drawSymbols ( eUnitDataSymbols symType, int x, int y, int maxX, bool big, int value1, int value2 );

	static SDL_Rect getBigSymbolPosition ( eUnitDataSymbols symType );
	static SDL_Rect getSmallSymbolPosition ( eUnitDataSymbols symType );
};

/**
 * an item that displays the data-values (attack, range, scan, etc.) of unit with the corresponding symbols for the hud.
 *@author alzi
 */
class cMenuUnitDetails : public cMenuItem
{
protected:
	cVehicle *vehicle;
	cBuilding *building;

	cPlayer *owner;
	bool drawLines;

public:
	cMenuUnitDetails( int x, int y, bool drawLines_, cPlayer *owner_ );
	void draw();

	void setOwner ( cPlayer *owner_ );
	void setSelection ( cVehicle *vehicle_, cBuilding *building_ );
};

/**
 * an item that displays the data-values (attack, range, scan, etc.) of a unit with the corresponding symbols.
 *@author alzi
 */
class cMenuUnitDetailsBig : public cMenuItem
{
protected:
	cMenuUnitListItem *selectedUnit;

public:
	cMenuUnitDetailsBig( int x, int y );
	void draw();

	void setSelection ( cMenuUnitListItem *selectedUnit_ );
};

/**
 * a simple vertical material bar.
 *@author alzi
 */
class cMenuMaterialBar : public cMenuItem
{
public:
	enum eMaterialBarTypes
	{
		MAT_BAR_TYPE_METAL,
		MAT_BAR_TYPE_OIL,
		MAT_BAR_TYPE_GOLD,
		MAT_BAR_TYPE_METAL_HORI_BIG,
		MAT_BAR_TYPE_OIL_HORI_BIG,
		MAT_BAR_TYPE_GOLD_HORI_BIG,
		MAT_BAR_TYPE_NONE_HORI_BIG,
		MAT_BAR_TYPE_METAL_HORI_SMALL,
		MAT_BAR_TYPE_OIL_HORI_SMALL,
		MAT_BAR_TYPE_GOLD_HORI_SMALL
	};

protected:
	AutoSurface surface;
	eMaterialBarTypes materialType;

	cMenuLabel *valueLabel;

	int maxValue, currentValue;
	bool inverted;
	bool horizontal;
	bool showLabel;

	void generateSurface();
public:
	cMenuMaterialBar( int x, int y, int labelX, int labelY, int maxValue_, eMaterialBarTypes materialType_, bool inverted_ = false, bool showLabel_ = true );
	~cMenuMaterialBar();
	void draw();

	void setType(eMaterialBarTypes);
	void setMaximalValue( int maxValue_ );
	void setCurrentValue( int currentValue_ );

	SDL_Rect getPosition();
};

/**
 * an itemconatainer that handles the many arrow buttons for upgrading units. This should be used together with the cMenuUnitDetailsBig item.
 *@author alzi
 */
class cMenuUpgradeHandler : public cMenuItemContainer
{
	cUpgradeHangarMenu *parentMenu;
	cMenuUnitListItem *selection;

	cMenuButton *decreaseButtons[8];
	cMenuButton *increaseButtons[8];
	cMenuLabel *costsLabel[8];

	cUpgradeCalculator::UpgradeTypes getUpgradeType( sUnitUpgrade upgrade );

	static void buttonReleased( void* parent );
public:
	cMenuUpgradeHandler( int x, int y, cUpgradeHangarMenu *parent );
	~cMenuUpgradeHandler();

	void setSelection ( cMenuUnitListItem *selection_ );
};

/**
 * The little dot that displays where the current position of a scrollbar is.
 *@author alzi
 */
class cMenuScroller : public cMenuItem
{
public:
	enum eMenuScrollerTypes
	{
		SCROLLER_TYPE_HORI,
		SCROLLER_TYPE_VERT,
		SCROLLER_TYPE_HUD_ZOOM
	};
private:
	cMenuItem *parent;
	eMenuScrollerTypes scrollerType;
	AutoSurface surface;

	int mouseXOff, mouseYOff;

	void (*movedCallback)(void *);
public:
	cMenuScroller ( int x, int y, eMenuScrollerTypes scrollerType_, cMenuItem *parent_, void (*movedCallback_)(void *) = NULL );
	void draw();

	SDL_Rect getPosition();
	void move ( int value );

	void mouseMoved( bool center );

	bool preClicked();
	void hoveredAway(void *parent);
	void movedMouseOver( int lastMouseX, int lastMouseY, void *parent );
	void somewhereMoved();
};

/**
 * a simple scrollbar with up and down buttons.
 *@author alzi
 */
class cMenuScrollBar : public cMenuItemContainer
{
friend class cMenuListBox;
friend class cMenuPlayersBox;
protected:
	cMenu *parentMenu;
	cMenuItem *parentItem;

	AutoSurface surface;

	int maximalScroll;
	int pageSteps;

	int offset;
	int maximalOffset;
	int scrollerSteps;

	cMenuButton *upButton;
	cMenuButton *downButton;
	cMenuScroller *scroller;

	void createSurface();
public:
	cMenuScrollBar ( int x, int y, int h, int pageSteps_, cMenu *parentMenu_, cMenuItem *parentItem_ );
	~cMenuScrollBar();
	void draw();

	void setMaximalScroll ( int maximalScroll_ );

	static void upButtonReleased( void* parent );
	static void downButtonReleased( void* parent );
};

/**
 * a simple box with many lines of text.
 *@author alzi
 */
class cMenuListBox : public cMenuItemContainer
{
protected:
	cMenu *parentMenu;

	cList<std::string> lines;
	int maxLines;
	int maxDrawLines;

	cMenuScrollBar *scrollBar;
public:
	cMenuListBox ( int x, int y, int w, int h, int maxLines_, cMenu *parentMenu_ );
	~cMenuListBox();
	void draw();

	void addLine ( std::string line );
};

/**
 * a simple editbox where you can enter text.
 *@author alzi
 */
class cMenuLineEdit : public cMenuItem
{
public:
	enum eLineEditType
	{
		LE_TYPE_IN_BOX,
		LE_TYPE_JUST_TEXT
	};

protected:
	void (*returnPressed)(void *);

	eLineEditType lineEditType;
	eUnicodeFontType fontType;
	cMenu *parentMenu;
	std::string text;
	int cursorPos;
	int startOffset, endOffset;

	bool readOnly;
	bool takeChars, takeNumerics;

	void resetTextPosition();
	void doPosIncrease( int &value, int pos );
	void doPosDecrease( int &pos );
	void scrollLeft( bool changeCursor = true );
	void scrollRight();
	void deleteLeft();
	void deleteRight();

	SDL_Rect getTextDrawOffset();
	virtual int getBorderSize();
public:
	cMenuLineEdit ( int x, int y, int w, int h, cMenu *parentMenu_, eUnicodeFontType fontType_ = FONT_LATIN_NORMAL, eLineEditType lineEditType_ = LE_TYPE_IN_BOX );
	void draw();

	bool preClicked();

	void setReadOnly ( bool readOnly_ );
	/**
	 * sets what type of input this editbox takes.
	 *@author alzi
	 *@param takeChars_ if this is true the box takes all kind of normal chars that are not numerics.
	 *@param takeNumerics_ if this is true the box takes numerics.
	 */
	void setTaking ( bool takeChars_, bool takeNumerics_ );
	void setText ( std::string text_ );
	std::string getText ();
	void setSize( int w, int h );
	bool handleKeyInput( SDL_keysym keysym, std::string ch, void *parent );

	void setReturnPressedFunc( void (*returnPressed_)(void *) );
};

class cMenuChatBox : public cMenuLineEdit
{
	AutoSurface surface;

	void generateSurface();
	int getBorderSize();
public:
	cMenuChatBox ( int x, int y, cMenu *parentMenu_ );
	void draw();
};

/**
 * a structure that includes all information about a player needed by a multiplayermenu.
 *@author alzi
 */
struct sMenuPlayer
{
	std::string name;
	int color;
	bool ready;

	int nr;
	int socket;

	sMenuPlayer (std::string name_ = "", int color_ = 0, bool ready_ = false, int nr_ = 0, int socket_ = -1)
		: name(name_), color(color_), ready(ready_), nr(nr_), socket(socket_) {}
};

/**
 * a list with players. It displays the name, the color and the readystate of the player.
 *@author alzi
 */
class cMenuPlayersBox : public cMenuItemContainer
{
	cList<sMenuPlayer*> *players;

	cNetworkMenu *parentMenu;
	int maxDrawPlayers;

	cList<cMenuImage*> playerColors;
	cList<cMenuLabel*> playerNames;
	cList<cMenuImage*> playerReadys;

	cMenuScrollBar *scrollBar;

	bool preClicked();
public:
	cMenuPlayersBox ( int x, int y, int w, int h, cNetworkMenu *parentMenu_ );
	~cMenuPlayersBox();
	void draw();

	void setPlayers ( cList<sMenuPlayer*> *player_ );
};

/**
 * a saveslot that displays the information like name, time and type of a savegame.
 *@author alzi
 */
class cMenuSaveSlot : public cMenuItem
{
	cMenuLabel *saveNumber;
	cMenuLabel *saveType;
	cMenuLabel *saveTime;
	cMenuLineEdit *saveName;
public:
	cMenuSaveSlot( int x, int y, cMenu *parent );
	~cMenuSaveSlot();
	void draw();

	void setActivity ( bool active_ ) {}

	void setSaveData ( sSaveFile saveFile, bool selected );
	void reset( int number, bool selected );
	cMenuLineEdit *getNameEdit();
};

/**
 * three buttons and labels that display the different buildspeeds of a unit.
 *@author alzi
 */
class cMenuBuildSpeedHandler : public cMenuItemContainer
{
	cMenuCheckButton *speedButtons[3];
	cMenuRadioGroup *speedGroup;

	cMenuLabel *turnsLabels[3];
	cMenuLabel *costsLabels[3];

public:
	cMenuBuildSpeedHandler( int x, int y );
	~cMenuBuildSpeedHandler();

	void setValues ( int *turboBuildTurns, int *turboBuildCosts );
	void setBuildSpeed( int buildSpeed );
	int getBuildSpeed();
};

/**
 * one checkbox each for tank, plane, ship, building or TNT to control the units displayed in a unitlist.
 *@author alzi
 */
class cMenuUpgradeFilter : public cMenuItemContainer
{
	cHangarMenu *parentMenu;

	cMenuCheckButton* checkButtonTank;
	cMenuCheckButton* checkButtonPlane;
	cMenuCheckButton* checkButtonShip;
	cMenuCheckButton* checkButtonBuilding;
	cMenuCheckButton* checkButtonTNT;

	static void buttonChanged( void *parent );
public:
	cMenuUpgradeFilter( int x, int y, cHangarMenu *parentMenu );
	~cMenuUpgradeFilter();

	void setTankChecked ( bool checked );
	void setPlaneChecked ( bool checked );
	void setShipChecked ( bool checked );
	void setBuildingChecked ( bool checked );
	void setTNTChecked ( bool checked );

	bool TankIsChecked();
	bool PlaneIsChecked();
	bool ShipIsChecked();
	bool BuildingIsChecked();
	bool TNTIsChecked();
};

class cMenuStoredUnitDetails : public cMenuItem
{
protected:
	sUnitData *unitData;
public:
	cMenuStoredUnitDetails( int x, int y, sUnitData *unitData_ = NULL );
	void draw();

	void setUnitData ( sUnitData *unitData_ );
};

class cMenuSlider : public cMenuItem
{
public:
	enum eSliderType
	{
		SLIDER_TYPE_NORMAL,
		SLIDER_TYPE_HUD_ZOOM,
	};
	enum eSliderDirection
	{
		SLIDER_DIR_LEFTMIN,
		SLIDER_DIR_RIGHTMIN,
	};
protected:
	float minValue;
	float maxValue;
	float curValue;

	eSliderType type;
	eSliderDirection direction;

	AutoSurface surface;
	cMenu *parent;

	void (*movedCallback)(void *);
public:
	cMenuSlider( int x, int y, float minValue_, float maxValue_, cMenu *parent_, int wight = 58, eSliderType type_ = SLIDER_TYPE_NORMAL, eSliderDirection direction_ = SLIDER_DIR_LEFTMIN );
	~cMenuSlider();

	cMenuScroller *scroller;

	void draw();

	void setBorders( float minValue_, float maxValue_ );
	void setValue( float value );
	float getValue();
	void setMoveCallback ( void (*movedCallback_)(void *) );

	static void scrollerMoved( void *parent );

	bool preClicked();
};

class cMenuScrollerHandler : public cMenuItem
{
	int maxValue;
	int currentValue;

	cMenuScroller *scroller;

public:
	cMenuScrollerHandler( int x, int y, int w, int maxValue_ );
	~cMenuScrollerHandler();

	void draw();

	void setValue( int value );

	SDL_Rect getPosition();
};

//-----------------------------------------------------------------------------
class cMenuReportsScreen : public cMenuItem, public INotificationListener
{
	cPlayer *owner;
	cReportsMenu *parentMenu;

	cVehicle *vehicles;
	cBuilding *buildings;

	int index;
	int selected;

	int maxItems;

	bool filterPlanes, filterGround, filterSea, filterBuilding;
	bool filterBuild, filterAttack, filterDamaged, filterStealth;

	enum eReportScreenTypes
	{
		REP_SCR_TYPE_UNITS,
		REP_SCR_TYPE_DISADVA,
		REP_SCR_TYPE_SCORE,
		REP_SCR_TYPE_REPORTS
	};

	cMenuUnitDetails **unitDetails;

	eReportScreenTypes screenType;

	bool checkFilter ( sUnitData &data, bool checkInclude );
	bool goThroughUnits ( bool draw, int *count = NULL, cVehicle **vehicle = NULL, cBuilding **building = NULL );
	SDL_Surface *generateUnitSurface(cUnit* unit );

	int countDisadvantageEntries () const;
	/** Draws a disadvantage entry, if a player has losses for the unitID and the entry should be displayed on the current page.
	 * @return true, if an entry was drawn or would at least be in the list for unitImg */
	bool drawDisadvantageEntryIfNeeded (sID& unitID, SDL_Surface* unitImg, std::vector<sID>& unitTypesWithLosses, int displayedEntryIndex);

	void drawUnitsScreen();
	void drawDisadvantagesScreen();
	void drawScoreScreen();
	void drawScoreGraph();
	void drawReportsScreen();

	void updateScrollButtons();

//-----------------------------------------------------------------------------
public:
	cMenuReportsScreen ( int x, int y, int w, int h, cPlayer *owner_, cReportsMenu *parentMenu_ );
	~cMenuReportsScreen();

	void draw();

	void setIncludeFilter ( bool filterPlanes_, bool filterGround_, bool filterSea_, bool filterBuilding_ );
	void setBorderedFilter ( bool filterBuild_, bool filterAttack_, bool filterDamaged_, bool filterStealth_ );
	void setType ( bool unitsChecked, bool disadvaChecked, bool scoreChecked, bool reportsChecked );

	void scrollDown();
	void scrollUp();

	void released( void *parent );

	// INotificationListener implementation
	virtual bool notify (std::string message, void* sender = 0);
};


//-----------------------------------------------------------------------------
/**
 * A little box that displays additional information about the current state of a player.
 * (his name, his color, whether he is active in the game and whether he has ended his turn)
 * @author alzi
 */
class cMenuPlayerInfo : public cMenuItem
{
private:
	cPlayer *player;
public:
	cMenuPlayerInfo( int x, int y, cPlayer *player_ );

	void draw();
};

#endif // menuitemsH
