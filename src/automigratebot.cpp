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
#include "table/strings.h"

#include "safeguards.h"
#include "newgrf_station.h"
#include <queue>

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

//TODO: move this to an extension file
/*  returns 1 iff str ends with suffix  */
int str_ends_with(const char * str, const char * suffix) {

	if (str == NULL || suffix == NULL)
		return 0;

	size_t str_len = strlen(str);
	size_t suffix_len = strlen(suffix);

	if (suffix_len > str_len)
		return 0;

	return 0 == strncmp(str + str_len - suffix_len, suffix, suffix_len);
}

void AutoMigrateRailsBot::BuildRailLines(std::vector<const Station*> stationList, Owner owner)
{
	std::vector<TileIndex> tiles;

	const Station* selectedStation = nullptr;

	//TODO: find a way to handle multiple stations
	for (std::vector<int>::size_type i = 0; i != stationList.size(); i++)
	{
		if (str_ends_with(stationList[i]->name, " *"))
		{
			selectedStation = stationList[i];
			break;
		}
	}

	if (!selectedStation)
		return;

	//keressük meg a stationbõl kimutató síneket
	TileArea ta;
	selectedStation->GetTileArea(&ta, STATION_RAIL);
	TileIndex centerTile = ta.tile;


	//TileIndex t = centerTile;

	TileIndex cur_tile = 0;
	TILE_AREA_LOOP(cur_tile, ta)
	{
		for (int i = TRACKDIR_BEGIN; i != TRACKDIR_END; i++)
		{
			FollowLines(owner, (Trackdir)i, cur_tile);
		}
	}

	MigrateTiles();
}

void AutoMigrateRailsBot::FollowOneLine(Owner o, Trackdir dir, TileIndex startTile, CFollowTrackT<TRANSPORT_RAIL, Train, true > ft)
{
	TileIndex currentTile = startTile;

	while (ft.Follow(currentTile, dir))
	{
		TrackdirBits dirBits = TRACKDIR_BIT_NONE;
		if (IsPlainRailTile(ft.m_new_tile))
		{
			TrackBits bits = GetTrackBits(ft.m_new_tile);
			dirBits = TrackBitsToTrackdirBits(bits);
			RemoveFirstTrackdir(&dirBits);
			dirBits = (TrackdirBits)(dirBits & 0xFF);
		}

		TrackdirBits second_dir_bits = KillFirstBit(ft.m_new_td_bits);
		if (second_dir_bits != TRACKDIR_BIT_NONE || dirBits != TRACKDIR_BIT_NONE)
		{
			if (ft.m_new_tile != startTile)
			{
				//there is a junction
				PathJunctionPoint junction = { dirBits, ft.m_new_tile, false };

				if (std::find(junctions.begin(), junctions.end(), junction) == junctions.end())
					junctions.push_back(junction);

				currentTile = ft.m_new_tile;
				dir = RemoveFirstTrackdir(&ft.m_new_td_bits);
				continue;
			}
		}

		if (ft.m_is_station == true)
		{
			//there is a station
		}

		if (IsDepotTile(ft.m_new_tile))
		{
			TileIndexDiff diff = TileOffsByDiagDir(TrackdirToExitdir(ft.m_old_td));
			TileIndex newDepoEntry = TILE_ADD(ft.m_new_tile, diff);
		}

		//VpStartPlaceSizing(t, VPM_X_AND_Y, DDSP_CONVERT_RAIL);
		//TODO: select last available from ai engine
		//TODO: detect cycles
		if (std::find(tiles.begin(), tiles.end(), ft.m_new_tile) == tiles.end())
			tiles.push_back(ft.m_new_tile);

		currentTile = ft.m_new_tile;
		dir = RemoveFirstTrackdir(&ft.m_new_td_bits);
	}
}

void AutoMigrateRailsBot::ProcessJunction(Owner owner, PathJunctionPoint junction, CFollowTrackT<TRANSPORT_RAIL, Train, true > ft)
{
	Trackdir trackDirection = RemoveFirstTrackdir(&junction.direction);

	if (trackDirection == INVALID_TRACKDIR)
	{
		return;
	}

	Track exitTrack = DiagDirToDiagTrack(TrackdirToExitdir(trackDirection));

	TileIndexDiff diff = TileOffsByDiagDir(TrackdirToExitdir(trackDirection));
	//TileIndex newStartingPoint = TILE_ADD(junction.tile, diff);

	if (IsDepotTile(junction.tile))
	{
		DoCommandP(ft.m_new_tile, ft.m_new_tile, RAILTYPE_MAGLEV | (false ? 0x10 : 0), CMD_CONVERT_RAIL | CMD_MSG(STR_ERROR_CAN_T_CONVERT_RAIL), CcPlaySound1E);
		return;
	}

	if (IsTileType(junction.tile, MP_RAILWAY))
	{
		TrackBits bits = GetTrackBits(junction.tile);
		TrackdirBits dirBits = TrackBitsToTrackdirBits(bits);
		Trackdir direction = RemoveFirstTrackdir(&dirBits);
		DiagDirection exitDir = TrackdirToExitdir(trackDirection);

		if (exitDir == DIAGDIR_SW && direction == TRACKDIR_X_NE)
		{
			direction = TRACKDIR_X_SW;
		}
		else if (exitDir == DIAGDIR_SE && direction == TRACKDIR_Y_NW)
		{
			direction = TRACKDIR_Y_SE;
		}
		else if (exitDir == DIAGDIR_NE && direction == TRACKDIR_X_SW)
		{
			direction = TRACKDIR_X_NE;
		}
		else if (exitDir == DIAGDIR_NW && direction == TRACKDIR_Y_SE)
		{
			direction = TRACKDIR_Y_NW;
		}

		FollowOneLine(owner, direction, junction.tile, ft);

		//if (std::find(tiles.begin(), tiles.end(), junction.tile) == tiles.end())
		//	tiles.push_back(junction.tile);
	}
}

void AutoMigrateRailsBot::FollowLines(Owner owner, Trackdir dir, TileIndex startingTile)
{
	CFollowTrackT<TRANSPORT_RAIL, Train, true > ft(owner, RAILTYPES_RAIL);

	FollowOneLine(owner, dir, startingTile, ft);
	
	for(int i = 0; i < junctions.size(); i++)
	{
		PathJunctionPoint junction = junctions.at(i);

		if (junction.isProcessed)
			continue;

		junction.isProcessed = true;
		ProcessJunction(owner, junction, ft);
	}
}

void AutoMigrateRailsBot::MigrateTiles()
{
	std::deque<TileIndex>::iterator it = tiles.begin();
	while (it != tiles.end())
	{
		TileIndex currentTile = *it++;

		RailType railtype = RAILTYPE_MAGLEV;
		DoCommandP(currentTile, currentTile, railtype | (false ? 0x10 : 0), CMD_CONVERT_RAIL | CMD_MSG(STR_ERROR_CAN_T_CONVERT_RAIL), CcPlaySound1E);
	}
}
