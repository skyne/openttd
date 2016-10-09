#ifndef AUTOMIGRATEBOT_H
#define AUTOMIGRATEBOT_H

#include "stdafx.h"
#include "debug.h"
#include "station_base.h"

#include <set>
#include <vector>

#include "safeguards.h"
#include <queue>
#include "pathfinder/follow_track.hpp"

struct PathJunctionPoint
{
	TrackdirBits direction;
	TileIndex tile;
	bool isProcessed;

	inline bool operator==(PathJunctionPoint a) const
	{
		if (a.direction == direction && a.tile == tile && a.isProcessed == isProcessed)
			return true;
		else
			return false;
	}
};

class AutoMigrateRailsBot {
private:
	std::deque<TileIndex> tiles = std::deque<TileIndex>();
	std::deque<PathJunctionPoint> junctions = std::deque<PathJunctionPoint>();
	void MigrateTiles();
	void FollowOneLine(Owner o, Trackdir dir, TileIndex startTile, CFollowTrackT<TRANSPORT_RAIL, Train, true > ft);
	void ProcessJunction(Owner owner, PathJunctionPoint junction, CFollowTrackT<TRANSPORT_RAIL, Train, true > ft);

public:
	std::vector<const Station*> SearchForStations(const Owner owner);
	void BuildRailLines(std::vector<const Station*> stationList,Owner owner);
	void FollowLines(Owner owner, Trackdir dir, TileIndex startingTile);
};

#endif