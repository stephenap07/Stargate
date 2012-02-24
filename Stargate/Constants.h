#pragma once

const int WINDOW_WIDTH = 480;
const int WINDOW_HEIGHT = 320;

const int TILE_WIDTH = 16;
const int TILE_HEIGHT = 16; 

const int MAP_WIDTH = WINDOW_WIDTH/TILE_WIDTH;
const int MAP_HEIGHT = WINDOW_HEIGHT/TILE_HEIGHT;

const int TILE_TOTAL = 32;

enum LAYERS
{
	COLLISION =0,
	MAIN,
	ENTITIES,
	BACKGROUND,

	TOTAL
};