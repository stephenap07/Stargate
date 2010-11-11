#include "TileMap.h"
#include "Utilities.h"
#include "boost/lexical_cast.hpp"
#include <fstream>
#include <iostream>
#include <algorithm>

void cMapChunk::initialize(int id)
{
	if (id > TOTAL_TILES) 
		id = 0;

	for(int y = 0; y < MAP_HEIGHT; ++y)
	{
		for(int x = 0; x < MAP_WIDTH; x++)
		{
			m_chunk[x + y*MAP_WIDTH].id = id; 
		}
	}
}


int cMapChunk::getID(int x, int y)
{
	int index = x + y*MAP_WIDTH;

	if( index > MAP_WIDTH*MAP_HEIGHT)
		return -1;

	return m_chunk[index].id;
}

void cMapChunk::setID(int x, int y, int id)
{
	int index = x + y*MAP_WIDTH;

	if( index > MAP_WIDTH*MAP_HEIGHT)
		return;
	
	m_chunk[index].id = id;
}

bool cMap::load(const char* file)
{
	//get tile sheet location

	//get width and height of the map

	//get get the tile types and if they can collide with player or not

	//convert everything into the tile class and map data structure

	std::ifstream fin;

	fin.open(file, std::ios_base::binary); 

	if(fin.fail())
	{
		return false;
	}

	char imagename[128];
	char w[8];
	char h[8];
	char la[8];

	//read in width, height, image name, and layer 
	fin >> w;
	fin >> h;
	fin >> imagename;
	fin >> la;

	m_width = boost::lexical_cast<int>(w);
	m_height = boost::lexical_cast<int>(h);

	if(!loadTileSheet(imagename))
		return false;

	return true;
}


bool cMap::save(const char* file)
{
	std::ofstream fout;
	fout.open(file);

	if(!fout)
		return false;

	fout << m_width << std::endl;
	fout << m_height << std::endl;
	fout << m_tilesetname << std::endl;
	
	fout << Layer::BACKGROUND << std::endl;

	return true;
}

//THIS USES GLOBAL COORDINATES
int cMap::getTileID( int x, int y)
{
	//map chunk coordinates in relation to map eg (0,1)
	sf::Vector2i vec(x/MAP_WIDTH, y/MAP_HEIGHT);

	//tile coordinates in relation to map chunk
	int _x = x - vec.x*MAP_WIDTH;  //coordinates in pixels eg (0,16)
	int _y = y - vec.y*MAP_HEIGHT;

	//coordinates in tile format ex (0,1)
	_x /= TILE_WIDTH;  
	_y /= TILE_HEIGHT;

	tchunk::iterator it; 

	//search for the map chunk at the specified coordinate vector
	it = m_map[Layer::COLLISION].find(vec); 

	//if the chunk exists, return the ID
	if( it != m_map[Layer::COLLISION].end())
		return m_map[Layer::COLLISION][vec].getID(_x,_y);
	else
		return -1; // or else, return error
}

void cMap::drawTile(int id, int x, int y, sf::RenderWindow & window)
{
	//if the id is out of range, just return
	if(id < 0 || id > 32) return;

	m_tileset[id].SetPosition(x*TILE_WIDTH,y*TILE_HEIGHT);

	window.Draw( m_tileset[id] );
}

void cMap::draw(sf::RenderWindow & window)
{
	const sf::View &view = window.GetView();
	
	for(int l = Layer::TOTAL-1; l > -1; l--)
	{
		//loop through all the map chunks
		for(tchunk::iterator it = m_map[l].begin(); it != m_map[l].end(); ++it)
		{
			for(int y = 0; y < MAP_HEIGHT; y++)
			{
				for(int x = 0; x < MAP_WIDTH; x++)
				{
					//x,y coordinates relative to the map
							//coor of chunk    + coor of tile of chunk 
					int _x = it->second.getX() + x*TILE_WIDTH;
					int _y = it->second.getY() + y*TILE_HEIGHT;

					//get the tile id from the tile chunk
					int id = it->second.getID(x,y);

					//if the tile is in range of the camera, then draw it 
					//the tile cannot be TILE_NONE to be drawn
					bool cameraCollide = collides(_x, view.GetRect().Left, _y, view.GetRect().Top, TILE_WIDTH,
						view.GetRect().GetWidth(), TILE_HEIGHT, view.GetRect().GetHeight() ) 
						&& id != TILE_NONE;

					if( cameraCollide )
						drawTile(id, x, y, window);
				}
			}
		}
	}
}

bool cMap::loadTileSheet(std::string file)
{
	if(!m_tilesetImage.LoadFromFile(file))
		return false;
	m_tilesetname = file;
	
	int n = 0;
	int numrows = 2;

	for(int j = 0; j < numrows; j++)
	{
		for(int k = 0; k < TOTAL_TILES/numrows; k++)
		{
			m_tileset[n].SetImage(m_tilesetImage);

			m_tileset[n].SetSubRect(sf::IntRect(k*TILE_WIDTH, //left
												j*TILE_HEIGHT, //top
												k*TILE_WIDTH + TILE_WIDTH,   //right
												j*TILE_HEIGHT +  TILE_HEIGHT)); //bottom
			n++;
		}
	}

	return true;
}

void cMap::addTile(int x, int y, int layer, int id, int collision)
{
	if(m_map[layer].size() < (x+y*m_width))
		return;
	if(m_map[layer][x+y*m_width].id == id)
		return;

	m_map[layer][x+y*m_width].id = id;
	m_map[layer][x+y*m_width].collision = collision;
}

void cMap::removeTile(int x, int y, int layer)
{
	if(m_map[layer].size() < (x+y*m_width))
		return;
	if(m_map[layer][x+y*m_width].id == TILE_NONE)
		return;

	m_map[layer][x+y*m_width].id = TILE_NONE;
	m_map[layer][x+y*m_width].collision = Collision::NONE;
}

void cMap::initializeMap()
{
	for(int l = 0; l < Layer::TOTAL; l++)
	{
		for(tchunk::iterator it = m_map[l].begin(); it != m_map[l].end(); it++)
		{
			it->second.initialize();
		}
	}
}

void cMap::setCollision(int x, int y, int layer, int collision)
{
	int _x = x/MAP_WIDTH;
	int _y = y/MAP_HEIGHT;

	m_map[Layer::COLLISION][sf::Vector2i(_x, _y)].setID( = collision;
}