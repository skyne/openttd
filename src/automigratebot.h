#ifndef AUTOMIGRATEBOT_H
#define AUTOMIGRATEBOT_H

#include "stdafx.h"
#include "debug.h"
#include "station_base.h"

#include <set>
#include <vector>

#include "safeguards.h"

class AutoMigrateRailsBot {

public:
	std::vector<const Station*> SearchForStations(const Owner owner);
	void BuildRailLines(std::vector<const Station*> stationList,Owner owner);
};
#endif