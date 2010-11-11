#pragma once

const int CAMERA_WIDTH = 480;
const int CAMERA_HEIGHT = 320;

const int TILE_WIDTH = 16;
const int TILE_HEIGHT = TILE_WIDTH; 

const int MAP_WIDTH = CAMERA_WIDTH/TILE_WIDTH;
const int MAP_HEIGHT = CAMERA_HEIGHT/TILE_HEIGHT;

const int TOTAL_TILES = 32; 
const int TILE_NONE = TOTAL_TILES + 1;

enum Layer 
{
	MAIN,
	COLLISION,
	BACKGROUND,

	TOTAL //2
};

enum Collision
{
	NONE,
	NORMAL,
	SLOPE_RIGHT,
	SLOPE_LEFT,
	ICE
};

enum MapEdit
{
	EDIT,
	COLLISION
};
