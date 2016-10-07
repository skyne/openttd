#include "automigratebot.h"
#include "pathfinder\follow_track.hpp"
#include "stdafx.h"
#include "debug.h"
#include "station_base.h"
#include "tilehighlight_func.h"
#include "rail_gui.h"
#include "widgets\rail_widget.h"
#include "tilearea_type.h"
#include "command_func.h"

#include <set>
#include <vector>

#include "safeguards.h"

std::vector<const Station*> AutoMigrateRailsBot::SearchForStations(const Owner owner)
{
	std::vector<const Station*> exploredStations;

	const Station *st;
	FOR_ALL_STATIONS(st)
	{
		if (st->owner == owner || (st->owner == OWNER_NONE && HasStationInUse(st->index, true, owner)))
		{
			exploredStations.push_back(st);
		}
	}

	return exploredStations;
}

void AutoMigrateRailsBot::BuildRailLines(std::vector<const Station*> stationList,Owner owner)
{
	std::vector<TileIndex> tiles;

	//TODO: select one station in a proper way
	const Station* selectedStation = stationList[0];

	//keressük meg a stationbõl kimutató síneket
	TileArea* ta;
	selectedStation->GetTileArea(ta, STATION_RAIL);
	TileIndex centerTile = ta->GetCenterTile();

	CFollowTrackT<TRANSPORT_RAIL, Train, true > ft(owner);
	TileIndex t = centerTile;

	//find rail going off from station
	uint length = selectedStation->GetPlatformLength(t, DIAGDIR_NE);
	//TODO: check all remaining directions
	if(length > 0)

	Trackdir td = TRACKDIR_X_NE;

	while (ft.Follow(t, td))
	{
		if (KillFirstBit(ft.m_new_td_bits) != TRACKDIR_BIT_NONE)
		{
			//there is a junction
		}

		if (ft.m_is_station = true)
		{
			//there is a station
		}

		//VpStartPlaceSizing(t, VPM_X_AND_Y, DDSP_CONVERT_RAIL);
		//TODO: select last available from ai engine
		RailType railtype = RAILTYPE_MAGLEV;

		DoCommandP(ft.m_new_tile, ft.m_new_tile, railtype | (false ? 0x10 : 0), CMD_CONVERT_RAIL | CMD_MSG(STR_ERROR_CAN_T_CONVERT_RAIL), CcPlaySound1E);

		td = RemoveFirstTrackdir(&ft.m_new_td_bits);
	}
}