#pragma once

#include "Constants.h"
#include <SFML/Graphics.hpp>
#include <vector>
#include <map>
#include <string>

class cTile
{
public:

	cTile() :id(0){}

	int id;
};

class cMapChunk
{
public:
	cMapChunk() :m_x(0), m_y(0){}
	~cMapChunk() {}

	//intitialize id to a certain id
	void initialize(int id=MapEdit::EDIT);

	//position relative to the world map 
	void setX(int x) {m_x = x;}
	void setY(int y) {m_y = y;}

	//set the id of a specified tile
	void setID(int x, int y, int id =0);

	int getX() {return m_x;}
	int getY() {return m_y;}

	int getID(int x, int y);

private:

	int m_x;
	int m_y;
	cTile m_chunk[MAP_WIDTH*MAP_HEIGHT];  
};


//the map made out of map chunks
class cMap
{
    public:
   	 cMap() {}
   	 ~cMap() {}

        bool load( const char* file );

        bool save( const char* file );

        void draw(sf::RenderWindow & window);

        int getTileID( int x, int y);


	//editor interface
	//ugly
	public:

		void setWidth(int width) {m_width = width;}
		void setHeight(int height) {m_height = height;}
		void initializeMap();

		void setTileID(int x, int y, int layer, int id);
		void removeTile(int x, int y, int layer);
		void setCollision(int x, int y, int layer, int collision);
		
		bool loadTileSheet(std::string file);

	public:

		int GetWidth() {return m_width;}
		int GetHeight() {return m_height;}
		
	private:

		//typedefs
		//layer and map<(x,y), mapchunk>
		typedef std::map< sf::Vector2i, cMapChunk > tchunk;
		typedef std::map< int, tchunk > tilemap;

        int m_width;
        int m_height;

		//our tile set: 32 tiles
		sf::Sprite m_tileset[TOTAL_TILES];
		std::string m_tilesetname;
		sf::Image m_tilesetImage;

		//the big map itself
		tilemap m_map;

		//private methods

		void drawTile(int id, int x, int y, sf::RenderWindow & window); 

		//free the tiles
        void freetiles()
		{
			m_map.clear();
		}

		//helper methods
		sf::Vector2i transformToLocalTile(const sf::Vector2i & tile)
		{
			//get the map chunk the coordinate is in
			sf::Vector2i chunk = transformChunktoLocal(tile);

			//transform by subtracting how many tiles over the global tile is by how many tiles over the 
			//chunk is; this returns the coordinates local to the chunk MAP_WIDTH*MAP_HEIGHT
			sf::Vector2i value(tile.x/TILE_WIDTH - chunk.x*MAP_WIDTH, tile.y/TILE_HEIGHT - chunk.y*MAP_HEIGHT);

			return value;
		}

		sf::Vector2i transformChunkToLocal(const sf::Vector2i &chunk)
		{
			sf::Vector2i value(chunk.x/CAMERA_WIDTH, chunk.y/CAMERA_HEIGHT);

			return value;
		}
};
