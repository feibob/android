#include <string>
#include "../../dataobj/tabfile.h"
#include "../roadsign_besch.h"
#include "obj_node.h"
#include "text_writer.h"
#include "imagelist_writer.h"
#include "roadsign_writer.h"
#include "get_waytype.h"
#include "skin_writer.h"

using std::string;

void roadsign_writer_t::write_obj(FILE* fp, obj_node_t& parent, tabfileobj_t& obj)
{
	obj_node_t node(this, 14, &parent);

	// Hajodoc: Preferred height of this tree type
	// Hajoval: int (useful range: 0-14)
	uint32                  const cost      = obj.get_int("cost",      500) * 100;
	uint16                  const min_speed = obj.get_int("min_speed",   0);
	roadsign_besch_t::types const flags     =
		(obj.get_int("single_way",         0) > 0 ? roadsign_besch_t::ONE_WAY               : roadsign_besch_t::NONE) |
		(obj.get_int("free_route",         0) > 0 ? roadsign_besch_t::CHOOSE_SIGN           : roadsign_besch_t::NONE) |
		(obj.get_int("is_private",         0) > 0 ? roadsign_besch_t::PRIVATE_ROAD          : roadsign_besch_t::NONE) |
		(obj.get_int("is_signal",          0) > 0 ? roadsign_besch_t::SIGN_SIGNAL           : roadsign_besch_t::NONE) |
		(obj.get_int("is_presignal",       0) > 0 ? roadsign_besch_t::SIGN_PRE_SIGNAL       : roadsign_besch_t::NONE) |
		(obj.get_int("no_foreground",      0) > 0 ? roadsign_besch_t::ONLY_BACKIMAGE        : roadsign_besch_t::NONE) |
		(obj.get_int("is_longblocksignal", 0) > 0 ? roadsign_besch_t::SIGN_LONGBLOCK_SIGNAL : roadsign_besch_t::NONE) |
		(obj.get_int("end_of_choose",      0) > 0 ? roadsign_besch_t::END_OF_CHOOSE_AREA    : roadsign_besch_t::NONE);
	uint8                   const wtyp      = get_waytype(obj.get("waytype"));

	// Hajo: write version data
	node.write_uint16(fp, 0x8003,    0);
	node.write_uint16(fp, min_speed, 2);
	node.write_uint32(fp, cost,      4);
	node.write_uint8 (fp, flags,     8);
	node.write_uint8 (fp, wtyp,      9);

	uint16 intro  = obj.get_int("intro_year", DEFAULT_INTRO_DATE) * 12;
	intro += obj.get_int("intro_month", 1) - 1;
	node.write_uint16(fp,          intro,           10);

	uint16 retire  = obj.get_int("retire_year", DEFAULT_RETIRE_DATE) * 12;
	retire += obj.get_int("retire_month", 1) - 1;
	node.write_uint16(fp,          retire,          12);

	write_head(fp, node, obj);

	// add the images
	slist_tpl<string> keys;
	string str;

	for (int i = 0; i < 24; i++) {
		char buf[40];

		sprintf(buf, "image[%i]", i);
		str = obj.get(buf);
		// make sure, there are always 4, 8, 12, ... images (for all directions)
		if (str.empty() && i % 4 == 0) {
			break;
		}
		keys.append(str);
	}
	imagelist_writer_t::instance()->write_obj(fp, node, keys);

	// probably add some icons, if defined
	slist_tpl<string> cursorkeys;

	string c = string(obj.get("cursor")), i=string(obj.get("icon"));
	cursorkeys.append(c);
	cursorkeys.append(i);
	if (!c.empty() || !i.empty()) {
		cursorskin_writer_t::instance()->write_obj(fp, node, obj, cursorkeys);
	}

	node.write(fp);
}
