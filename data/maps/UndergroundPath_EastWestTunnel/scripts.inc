UndergroundPath_EastWestTunnel_MapScripts:: @ 8160ED3
	map_script MAP_SCRIPT_ON_TRANSITION, UndergroundPath_EastWestTunnel_OnTransition
	.byte 0

UndergroundPath_EastWestTunnel_OnTransition:: @ 8160ED9
	setworldmapflag FLAG_WORLD_MAP_UNDERGROUND_PATH_EAST_WEST_TUNNEL
	end