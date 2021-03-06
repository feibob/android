/*
 * Copyright (c) 1997 - 2001 Hansj�rg Malthaner
 *
 * This file is part of the Simutrans project under the artistic licence.
 * (see licence.txt)
 */

#include <stdio.h>

#include "convoi_info_t.h"

#include "../simunits.h"
#include "../simdepot.h"
#include "../vehicle/simvehikel.h"
#include "../simcolor.h"
#include "../simgraph.h"
#include "../simworld.h"
#include "../simmenu.h"
#include "../simwin.h"

#include "../dataobj/fahrplan.h"
#include "../dataobj/translator.h"
#include "../dataobj/umgebung.h"
#include "../dataobj/loadsave.h"
#include "fahrplan_gui.h"
// @author hsiegeln
#include "../simlinemgmt.h"
#include "../simline.h"
#include "messagebox.h"

#include "../player/simplay.h"

#include "../utils/simstring.h"



#include "convoi_detail_t.h"

static const char cost_type[convoi_t::MAX_CONVOI_COST][64] =
{
	"Free Capacity", "Transported", "Revenue", "Operation", "Profit", "Distance", "Maxspeed"
};

static const int cost_type_color[convoi_t::MAX_CONVOI_COST] =
{
	COL_FREE_CAPACITY,
	COL_TRANSPORTED,
	COL_REVENUE,
	COL_OPERATION,
	COL_PROFIT,
	COL_DISTANCE,
	COL_MAXSPEED
};

static const bool cost_type_money[convoi_t::MAX_CONVOI_COST] =
{
	false, false, true, true, true, false
};


karte_t *convoi_info_t::welt = NULL;

bool convoi_info_t::route_search_in_progress=false;

/**
 * This variable defines by which column the table is sorted
 * Values: 0 = destination
 *                 1 = via
 *                 2 = via_amount
 *                 3 = amount
 * @author prissi
 */
const char *convoi_info_t::sort_text[SORT_MODES] = {
	"Zielort",
	"via",
	"via Menge",
	"Menge"
};



convoi_info_t::convoi_info_t(convoihandle_t cnv)
:	gui_frame_t( cnv->get_name(), cnv->get_besitzer() ),
	scrolly(&text),
	text(&freight_info),
	view(cnv->front(), koord(max(64, get_base_tile_raster_width()), max(56, (get_base_tile_raster_width() * 7) / 8))),
	sort_label(translator::translate("loaded passenger/freight"))
{
	this->cnv = cnv;
	welt = cnv->get_welt();
	this->mean_convoi_speed = speed_to_kmh(cnv->get_akt_speed()*4);
	this->max_convoi_speed = speed_to_kmh(cnv->get_min_top_speed()*4);

	const sint16 offset_below_viewport = D_MARGIN_TOP + D_BUTTON_HEIGHT + D_V_SPACE + view.get_groesse().y;
	const sint16 total_width = D_MARGIN_LEFT + 3*(D_BUTTON_WIDTH + D_H_SPACE) + max( D_BUTTON_WIDTH, view.get_groesse().x ) + D_MARGIN_RIGHT;

	input.set_pos( koord(D_MARGIN_LEFT,D_MARGIN_TOP) );
	reset_cnv_name();
	input.add_listener(this);
	add_komponente(&input);

	add_komponente(&view);

	// this convoi doesn't belong to an AI
	button.init(button_t::roundbox, "Fahrplan", koord(BUTTON1_X,offset_below_viewport), koord(D_BUTTON_WIDTH, D_BUTTON_HEIGHT));
	button.set_tooltip("Alters a schedule.");
	button.add_listener(this);
	add_komponente(&button);

	go_home_button.init(button_t::roundbox, "go home", koord(BUTTON2_X,offset_below_viewport), koord(D_BUTTON_WIDTH, D_BUTTON_HEIGHT));
	go_home_button.set_tooltip("Sends the convoi to the last depot it departed from!");
	go_home_button.add_listener(this);
	add_komponente(&go_home_button);

	no_load_button.init(button_t::roundbox, "no load", koord(BUTTON3_X,offset_below_viewport), koord(D_BUTTON_WIDTH, D_BUTTON_HEIGHT));
	no_load_button.set_tooltip("No goods are loaded onto this convoi.");
	no_load_button.add_listener(this);
	add_komponente(&no_load_button);

	follow_button.init(button_t::roundbox_state, "follow me", koord(view.get_pos().x, view.get_groesse().y + 21), koord(view.get_groesse().x, D_BUTTON_HEIGHT));
	follow_button.set_tooltip("Follow the convoi on the map.");
	follow_button.add_listener(this);
	add_komponente(&follow_button);

	chart.set_pos(koord(88,offset_below_viewport+D_BUTTON_HEIGHT+11));
	chart.set_groesse(koord(total_width-88-10, 88));
	chart.set_dimension(12, 10000);
	chart.set_visible(false);
	chart.set_background(MN_GREY1);
	for (int cost = 0; cost<convoi_t::MAX_CONVOI_COST; cost++) {
		chart.add_curve( cost_type_color[cost], cnv->get_finance_history(), convoi_t::MAX_CONVOI_COST, cost, MAX_MONTHS, cost_type_money[cost], false, true, cost_type_money[cost]*2 );
		filterButtons[cost].init(button_t::box_state, cost_type[cost],
			koord(BUTTON1_X+(D_BUTTON_WIDTH+D_H_SPACE)*(cost%4), view.get_groesse().y+164+(D_BUTTON_HEIGHT+2)*(cost/4)),
			koord(D_BUTTON_WIDTH, D_BUTTON_HEIGHT));
		filterButtons[cost].add_listener(this);
		filterButtons[cost].background = cost_type_color[cost];
		filterButtons[cost].set_visible(false);
		filterButtons[cost].pressed = false;
		add_komponente(filterButtons + cost);
	}
	add_komponente(&chart);

	add_komponente(&sort_label);

	const sint16 yoff = offset_below_viewport+46-D_BUTTON_HEIGHT-2;

	sort_button.init(button_t::roundbox, sort_text[umgebung_t::default_sortmode], koord(BUTTON1_X,yoff), koord(D_BUTTON_WIDTH, D_BUTTON_HEIGHT));
	sort_button.set_tooltip("Sort by");
	sort_button.add_listener(this);
	add_komponente(&sort_button);

	toggler.init(button_t::roundbox_state, "Chart", koord(BUTTON3_X,yoff), koord(D_BUTTON_WIDTH, D_BUTTON_HEIGHT));
	toggler.set_tooltip("Show/hide statistics");
	toggler.add_listener(this);
	add_komponente(&toggler);

	details_button.init(button_t::roundbox, "Details", koord(BUTTON4_X,yoff), koord(D_BUTTON_WIDTH, D_BUTTON_HEIGHT));
	details_button.set_tooltip("Vehicle details");
	details_button.add_listener(this);
	add_komponente(&details_button);

	text.set_pos( koord(0,D_V_SPACE) );
	scrolly.set_pos(koord(0, yoff+D_BUTTON_HEIGHT));
	scrolly.set_show_scroll_x(true);
	add_komponente(&scrolly);

	filled_bar.add_color_value(&cnv->get_loading_limit(), COL_YELLOW);
	filled_bar.add_color_value(&cnv->get_loading_level(), COL_GREEN);
	add_komponente(&filled_bar);

	speed_bar.set_base(max_convoi_speed);
	speed_bar.set_vertical(false);
	speed_bar.add_color_value(&mean_convoi_speed, COL_GREEN);
	add_komponente(&speed_bar);

	// we update this ourself!
	route_bar.add_color_value(&cnv_route_index, COL_GREEN);
	add_komponente(&route_bar);

	// goto line button
	line_button.init( button_t::posbutton, NULL, koord(D_MARGIN_LEFT, D_MARGIN_TOP + D_BUTTON_HEIGHT + D_V_SPACE + LINESPACE*4 ) );
	line_button.set_targetpos( koord(0,0) );
	line_button.add_listener( this );
	line_bound = false;

	cnv->set_sortby( umgebung_t::default_sortmode );

	set_fenstergroesse(koord(total_width, view.get_groesse().y+208+scrollbar_t::BAR_SIZE));
	set_min_windowsize(koord(total_width, view.get_groesse().y+131+scrollbar_t::BAR_SIZE));

	set_resizemode(diagonal_resize);
	resize(koord(0,0));
}


// only handle a pending renaming ...
convoi_info_t::~convoi_info_t()
{
	// rename if necessary
	rename_cnv();
}


/**
 * komponente neu zeichnen. Die �bergebenen Werte beziehen sich auf
 * das Fenster, d.h. es sind die Bildschirkoordinaten des Fensters
 * in dem die Komponente dargestellt wird.
 * @author Hj. Malthaner
 */
void convoi_info_t::zeichnen(koord pos, koord gr)
{
	if(!cnv.is_bound()) {
		destroy_win(this);
	}
	else {
		// make titlebar dirty to display the correct coordinates
		if(cnv->get_besitzer()==cnv->get_welt()->get_active_player()) {
			if(  line_bound  &&  !cnv->get_line().is_bound()  ) {
				remove_komponente( &line_button );
				line_bound = false;
			}
			else if(  !line_bound  &&  cnv->get_line().is_bound()  ) {
				add_komponente( &line_button );
				line_bound = true;
			}
			button.enable();
			go_home_button.pressed = route_search_in_progress;
			details_button.pressed = win_get_magic( magic_convoi_detail+cnv.get_id() );
			if (!cnv->get_schedule()->empty()) {
				const grund_t* g = cnv->get_welt()->lookup(cnv->get_schedule()->get_current_eintrag().pos);
				if (g != NULL && g->get_depot()) {
					go_home_button.disable();
				}
				else {
					goto enable_home;
				}
			}
			else {
enable_home:
				go_home_button.enable();
			}
			no_load_button.pressed = cnv->get_no_load();
			no_load_button.enable();
		}
		else {
			if(  line_bound  ) {
				// do not jump to other player line window
				remove_komponente( &line_button );
				line_bound = false;
			}
			button.disable();
			go_home_button.disable();
			no_load_button.disable();
		}
		follow_button.pressed = (cnv->get_welt()->get_follow_convoi()==cnv);

		// buffer update now only when needed by convoi itself => dedicated buffer for this
		const int old_len=freight_info.len();
		cnv->get_freight_info(freight_info);
		if(  old_len!=freight_info.len()  ) {
			text.recalc_size();
		}

		route_bar.set_base(cnv->get_route()->get_count()-1);
		cnv_route_index = cnv->front()->get_route_index() - 1;

		// all gui stuff set => display it
		gui_frame_t::zeichnen(pos, gr);
		set_dirty();

		PUSH_CLIP(pos.x+1,pos.y+D_TITLEBAR_HEIGHT,gr.x-2,gr.y-D_TITLEBAR_HEIGHT);

		// convoi information
		char tmp[256];
		cbuffer_t buf;
		static cbuffer_t info_buf;

		KOORD_VAL xpos = pos.x+D_MARGIN_LEFT;
		KOORD_VAL ypos = pos.y+D_TITLEBAR_HEIGHT+D_MARGIN_TOP+D_BUTTON_HEIGHT+D_V_SPACE;

		// use median speed to avoid flickering
		mean_convoi_speed += speed_to_kmh(cnv->get_akt_speed()*4);
		mean_convoi_speed /= 2;
		buf.printf( translator::translate("%i km/h (max. %ikm/h)"), (mean_convoi_speed+3)/4, speed_to_kmh(cnv->get_min_top_speed()) );
		display_proportional( xpos, ypos, buf, ALIGN_LEFT, COL_BLACK, true );
		ypos += LINESPACE;

		// next important: income stuff
		int len = display_proportional( xpos, ypos, translator::translate("Gewinn"), ALIGN_LEFT, COL_BLACK, true ) + 5;
		money_to_string( tmp, cnv->get_jahresgewinn()/100.0 );
		len += display_proportional( xpos+len, ypos, tmp, ALIGN_LEFT, cnv->get_jahresgewinn()>0?MONEY_PLUS:MONEY_MINUS, true )+5;
		money_to_string( tmp+1, cnv->get_running_cost()/100.0 );
		strcat( tmp, "/km)" );
		tmp[0] = '(';
		display_proportional( xpos+len, ypos, tmp, ALIGN_LEFT, COL_BLACK, true );
		ypos += LINESPACE;

		// the weight entry
		info_buf.clear();
		info_buf.append( translator::translate("Gewicht") );
		info_buf.append( ": " );
		info_buf.append( cnv->get_sum_gesamtgewicht()/1000.0, 1 );
		info_buf.append( "t (" );
		info_buf.append( (cnv->get_sum_gesamtgewicht()-cnv->get_sum_gewicht())/1000.0, 1 );
		info_buf.append( "t)" );
		display_proportional( xpos, ypos, info_buf, ALIGN_LEFT, COL_BLACK, true );
		ypos += LINESPACE;

		// next stop
		const schedule_t * fpl = cnv->get_schedule();
		info_buf.clear();
		info_buf.append(translator::translate("Fahrtziel"));
		fahrplan_gui_t::gimme_short_stop_name(info_buf, cnv->get_welt(), cnv->get_besitzer(), fpl->get_current_eintrag(), 34);
		len = display_proportional_clip( xpos, ypos, info_buf, ALIGN_LEFT, COL_BLACK, true );

		// convoi load indicator
		const KOORD_VAL offset = max( len+D_MARGIN_LEFT, 167)+3;
		route_bar.set_pos( koord( offset,ypos-pos.y-D_TITLEBAR_HEIGHT+(LINESPACE-D_INDICATOR_HEIGHT)/2) );
		route_bar.set_groesse( koord(view.get_pos().x-offset-D_H_SPACE, D_INDICATOR_HEIGHT) );
		ypos += LINESPACE;

		/*
		 * only show assigned line, if there is one!
		 * @author hsiegeln
		 */
		if(  cnv->get_line().is_bound()  ) {
			len = display_proportional( xpos+D_BUTTON_HEIGHT, ypos, translator::translate("Serves Line:"), ALIGN_LEFT, COL_BLACK, true );
			display_proportional_clip( xpos+D_BUTTON_HEIGHT+D_H_SPACE+len, ypos, cnv->get_line()->get_name(), ALIGN_LEFT, cnv->get_line()->get_state_color(), true );
		}
		POP_CLIP();
	}
}


bool convoi_info_t::is_weltpos()
{
	return (cnv->get_welt()->get_follow_convoi()==cnv);
}


koord3d convoi_info_t::get_weltpos( bool set )
{
	if(  set  ) {
		if(  !is_weltpos()  )  {
			cnv->get_welt()->set_follow_convoi( cnv );
		}
		else {
			cnv->get_welt()->set_follow_convoi( convoihandle_t() );
		}
		return koord3d::invalid;
	}
	else {
		return cnv->get_pos();
	}
}


// activate the statistic
void convoi_info_t::show_hide_statistics( bool show )
{
	toggler.pressed = show;
	const koord offset = show ? koord(0, 155) : koord(0, -155);
	set_min_windowsize(get_min_windowsize() + offset);
	scrolly.set_pos(scrolly.get_pos() + offset);
	chart.set_visible(show);
	set_fenstergroesse(get_fenstergroesse() + offset + koord(0,show?LINESPACE:-LINESPACE));
	resize(koord(0,0));
	for(  int i = 0;  i < convoi_t::MAX_CONVOI_COST;  i++  ) {
		filterButtons[i].set_visible(toggler.pressed);
	}
}


/**
 * This method is called if an action is triggered
 * @author Hj. Malthaner
 */
bool convoi_info_t::action_triggered( gui_action_creator_t *komp,value_t /* */)
{
	// follow convoi on map?
	if(komp == &follow_button) {
		if(cnv->get_welt()->get_follow_convoi()==cnv) {
			// stop following
			cnv->get_welt()->set_follow_convoi( convoihandle_t() );
		}
		else {
			cnv->get_welt()->set_follow_convoi(cnv);
		}
		return true;
	}

	// details?
	if(komp == &details_button) {
		create_win(20, 20, new convoi_detail_t(cnv), w_info, magic_convoi_detail+cnv.get_id() );
		return true;
	}

	if(  komp == &line_button  ) {
		cnv->get_besitzer()->simlinemgmt.show_lineinfo( cnv->get_besitzer(), cnv->get_line() );
		cnv->get_welt()->set_dirty();
	}

	if(  komp == &input  ) {
		// rename if necessary
		rename_cnv();
	}

	// sort by what
	if(komp == &sort_button) {
		// sort by what
		umgebung_t::default_sortmode = (sort_mode_t)((int)(cnv->get_sortby()+1)%(int)SORT_MODES);
		sort_button.set_text(sort_text[umgebung_t::default_sortmode]);
		cnv->set_sortby( umgebung_t::default_sortmode );
	}

	// some actions only allowed, when I am the player
	if(cnv->get_besitzer()==cnv->get_welt()->get_active_player()) {

		if(komp == &button) {
			cnv->call_convoi_tool( 'f', NULL );
			return true;
		}

		if(komp == &no_load_button    &&    !route_search_in_progress) {
			cnv->call_convoi_tool( 'n', NULL );
			return true;
		}

		if(komp == &go_home_button  &&  !route_search_in_progress) {
			// limit update to certain states that are considered to be save for fahrplan updates
			int state = cnv->get_state();
			if(state==convoi_t::FAHRPLANEINGABE) {
DBG_MESSAGE("convoi_info_t::action_triggered()","convoi state %i => cannot change schedule ... ", state );
				return true;
			}
			route_search_in_progress = true;

			// iterate over all depots and try to find shortest route
			route_t * shortest_route = new route_t();
			route_t * route = new route_t();
			koord3d home = koord3d(0,0,0);
			FOR(slist_tpl<depot_t*>, const depot, depot_t::get_depot_list()) {
				vehikel_t& v = *cnv->front();
				if (depot->get_waytype() != v.get_besch()->get_waytype() ||
						depot->get_besitzer() != cnv->get_besitzer()) {
					continue;
				}
				koord3d pos = depot->get_pos();
				if(!shortest_route->empty()    &&    koord_distance(pos.get_2d(),cnv->get_pos().get_2d())>=shortest_route->get_count()-1) {
					// the current route is already shorter, no need to search further
					continue;
				}
				if (v.calc_route(cnv->get_pos(), pos, 50, route)) { // do not care about speed
					if(  route->get_count() < shortest_route->get_count()    ||    shortest_route->empty()  ) {
						shortest_route->kopiere(route);
						home = pos;
					}
				}
			}
			delete route;
			DBG_MESSAGE("shortest route has ", "%i hops", shortest_route->get_count()-1);

			// if route to a depot has been found, update the convoi's schedule
			const char *txt;
			if(!shortest_route->empty()) {
				schedule_t *fpl = cnv->get_schedule()->copy();
				fpl->insert(cnv->get_welt()->lookup(home));
				fpl->set_aktuell( (fpl->get_aktuell()+fpl->get_count()-1)%fpl->get_count() );
				cbuffer_t buf;
				fpl->sprintf_schedule( buf );
				cnv->call_convoi_tool( 'g', buf );
				txt = "Convoi has been sent\nto the nearest depot\nof appropriate type.\n";
			}
			else {
				txt = "Home depot not found!\nYou need to send the\nconvoi to the depot\nmanually.";
			}
			delete shortest_route;
			route_search_in_progress = false;
			create_win( new news_img(txt), w_time_delete, magic_none);
		} // end go home button
	}

	if (komp == &toggler) {
		show_hide_statistics( toggler.pressed^1 );
		return true;
	}

	for(  int i = 0;  i < convoi_t::MAX_CONVOI_COST;  i++  ) {
		if(  komp == &filterButtons[i]  ) {
			filterButtons[i].pressed = !filterButtons[i].pressed;
			if(filterButtons[i].pressed) {
				chart.show_curve(i);
			}
			else {
				chart.hide_curve(i);
			}

			return true;
		}
	}

	return false;
}


void convoi_info_t::reset_cnv_name()
{
	// change text input of selected line
	if (cnv.is_bound()) {
		tstrncpy(old_cnv_name, cnv->get_name(), sizeof(old_cnv_name));
		tstrncpy(cnv_name, cnv->get_name(), sizeof(cnv_name));
		input.set_text(cnv_name, sizeof(cnv_name));
	}
}


void convoi_info_t::rename_cnv()
{
	if (cnv.is_bound()) {
		const char *t = input.get_text();
		// only change if old name and current name are the same
		// otherwise some unintended undo if renaming would occur
		if(  t  &&  t[0]  &&  strcmp(t, cnv->get_name())  &&  strcmp(old_cnv_name, cnv->get_name())==0) {
			// text changed => call tool
			cbuffer_t buf;
			buf.printf( "c%u,%s", cnv.get_id(), t );
			werkzeug_t *w = create_tool( WKZ_RENAME_TOOL | SIMPLE_TOOL );
			w->set_default_param( buf );
			cnv->get_welt()->set_werkzeug( w, cnv->get_besitzer());
			// since init always returns false, it is safe to delete immediately
			delete w;
			// do not trigger this command again
			tstrncpy(old_cnv_name, t, sizeof(old_cnv_name));
		}
	}
}


/**
 * Set window size and adjust component sizes and/or positions accordingly
 * @author Markus Weber
 */
void convoi_info_t::set_fenstergroesse(koord groesse)
{
	gui_frame_t::set_fenstergroesse(groesse);

	input.set_groesse(koord(get_fenstergroesse().x - D_MARGIN_LEFT-D_MARGIN_RIGHT, D_BUTTON_HEIGHT));

	view.set_pos( koord(get_fenstergroesse().x - view.get_groesse().x - D_MARGIN_LEFT, D_MARGIN_TOP+D_BUTTON_HEIGHT+D_V_SPACE ));
	follow_button.set_pos( view.get_pos() + koord( 0, view.get_groesse().y ) );

	scrolly.set_groesse( get_client_windowsize()-scrolly.get_pos() );

	const sint16 yoff = scrolly.get_pos().y-D_BUTTON_HEIGHT-3;
	sort_button.set_pos(koord(BUTTON1_X,yoff));
	toggler.set_pos(koord(BUTTON3_X,yoff));
	details_button.set_pos(koord(BUTTON4_X,yoff));
	sort_label.set_pos(koord(2,yoff-LINESPACE-1));

	// convoi speed indicator
	speed_bar.set_pos( koord(170,view.get_pos().y+0*LINESPACE+(LINESPACE-D_INDICATOR_HEIGHT)/2 ));
	speed_bar.set_groesse(koord(view.get_pos().x - 175, D_INDICATOR_HEIGHT));

	// convoi load indicator
	filled_bar.set_pos(koord(170,view.get_pos().y+2*LINESPACE+(LINESPACE-D_INDICATOR_HEIGHT)/2 ));
	filled_bar.set_groesse(koord(view.get_pos().x - 175, D_INDICATOR_HEIGHT));
}



convoi_info_t::convoi_info_t(karte_t *welt)
:	gui_frame_t("", NULL),
	scrolly(&text),
	text(&freight_info),
	view( welt, koord(64,64)),
	sort_label(translator::translate("loaded passenger/freight"))
{
	this->welt = welt;
}



void convoi_info_t::rdwr(loadsave_t *file)
{
	koord3d cnv_pos;
	char name[128];
	koord gr = get_fenstergroesse();
	uint32 flags = 0;
	bool stats = toggler.pressed;
	sint32 xoff = scrolly.get_scroll_x();
	sint32 yoff = scrolly.get_scroll_y();
	if(  file->is_saving()  ) {
		cnv_pos = cnv->front()->get_pos();
		for(  int i = 0;  i < convoi_t::MAX_CONVOI_COST;  i++  ) {
			if(  filterButtons[i].pressed  ) {
				flags |= (1<<i);
			}
		}
		tstrncpy(name, cnv->get_name(), lengthof(name));
	}
	cnv_pos.rdwr( file );
	file->rdwr_str( name, lengthof(name) );
	gr.rdwr( file );
	file->rdwr_long( flags );
	file->rdwr_byte( umgebung_t::default_sortmode );
	file->rdwr_bool( stats );
	file->rdwr_long( xoff );
	file->rdwr_long( yoff );
	if(  file->is_loading()  ) {
		// find convoi by name and position
		if(  grund_t *gr = welt->lookup(cnv_pos)  ) {
			for(  uint8 i=0;  i<gr->get_top();  i++  ) {
				if(  gr->obj_bei(i)->is_moving()  ) {
					vehikel_t const* const v = ding_cast<vehikel_t>(gr->obj_bei(i));
					if(  v  &&  v->get_convoi()  ) {
						if(  strcmp(v->get_convoi()->get_name(),name)==0  ) {
							cnv = v->get_convoi()->self;
							break;
						}
					}
				}
			}
		}
		// we might be unlucky, then search all convois for a convoi with this name
		if(  !cnv.is_bound()  ) {
			FOR(vector_tpl<convoihandle_t>, const i, welt->convoys()) {
				if (strcmp(i->get_name(), name) == 0) {
					cnv = i;
					break;
				}
			}
		}
		// still not found?
		if(  !cnv.is_bound()  ) {
			dbg->error( "convoi_info_t::rdwr()", "Could not restore convoi info window of %s", name );
			destroy_win( this );
			return;
		}
		// now we can open the window ...
		koord const& pos = win_get_pos(this);
		convoi_info_t *w = new convoi_info_t(cnv);
		create_win(pos.x, pos.y, w, w_info, magic_convoi_info + cnv.get_id());
		if(  stats  ) {
			gr.y -= 170;
		}
		w->set_fenstergroesse( gr );
		if(  file->get_version()<111001  ) {
			for(  int i = 0;  i < 6;  i++  ) {
				w->filterButtons[i].pressed = (flags>>i)&1;
				if(w->filterButtons[i].pressed) {
					w->chart.show_curve(i);
				}
			}
		}
		else {
			for(  int i = 0;  i < convoi_t::MAX_CONVOI_COST;  i++  ) {
				w->filterButtons[i].pressed = (flags>>i)&1;
				if(w->filterButtons[i].pressed) {
					w->chart.show_curve(i);
				}
			}
		}
		if(  stats  ) {
			w->show_hide_statistics( true );
		}
		cnv->get_freight_info(w->freight_info);
		w->text.recalc_size();
		w->scrolly.set_scroll_position( xoff, yoff );
		// we must invalidate halthandle
		cnv = convoihandle_t();
		destroy_win( this );
	}
}
