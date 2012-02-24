#pragma once

#include <SFML/Graphics.hpp>
#include <SFML/Audio.hpp>

//Standard libraries
#include <map>
#include <string>
#include <cassert>
#include <vector>

/*===============================

	CONSTANTS/TYPEDEFS
================================*/

#define PI 3.14

//window stuff
const int WINDOW_WIDTH = 512;
const int WINDOW_HEIGHT = 320;

//map stuff
const int TILE_SIZE = 16; 
const int MAP_WIDTH = WINDOW_WIDTH/TILE_SIZE; //32
const int MAP_HEIGHT = WINDOW_HEIGHT/TILE_SIZE; //20
const int PALLETE_MAX = 64; 

const int GRAVITY = 10;

//constants for Player
const int PLAYER_WIDTH = 15;
const int PLAYER_HEIGHT = 43; 

const int MAX_ANIMS  = 2; 
const int MAX_FRAMES = 12; 

enum { RIGHT=0, LEFT, UP, DOWN, NONE };

typedef sf::IntRect frame_t;


const int MAX_LASERS = 10;
const int MAX_BACON  = 20;

enum EntCode
{
	PLAYER,
	BACON,
	BEAVER
};

//used by entities
struct PhysData_t
{
	float velX;
	float velY;

	bool grounded;

	sf::IntRect collisionBox;

	PhysData_t() :velX(0), velY(0) {}
};

//Interface
class Entity
{
public:

	//public variables
	int flags;

	bool physicsObject;

	PhysData_t physData;

	bool inUse;

	sf::Sprite sprite;

	float nextThink;

	Entity() : physicsObject(false), inUse(false), nextThink(0.001) {}
	
	//virtual ~Entity() {}

	virtual void Think(float dt, const sf::Input & input) = 0;

	virtual void Draw(const sf::RenderWindow & wnd) {}
};

class Player : public Entity
{
	private:
	//animations indexes
	enum { ANIM_LEFT=0, ANIM_RIGHT, ANIM_JUMP };

	int m_currentFrame; //current frame
	int m_currentAnim;
	
	frame_t m_frames [MAX_ANIMS][MAX_FRAMES];
	float m_nextAnim;

	//jumping stuff

	int m_jumpPosition;
	bool m_jumpKey;
	int jumpHeight;
	bool m_lockJump; 

	//sounds
	sf::Sound m_soundJump;
	sf::Sound m_soundPickUp;
	sf::Sound m_soundShoot;

	void UpdateAnimations(float dt);

	inline void Jump()
	{
		physData.velY = -3;
	}

public:

	Player();

	virtual void Think(float dt, const sf::Input & input);

	virtual void Draw(const sf::RenderWindow & wnd);
};

class ItemBacon :public Entity
{

private:

	float m_impulse;

	float m_limit;

	float m_rate;

	bool m_change;

public:

	ItemBacon();

	virtual void Think(float dt, const sf::Input &input);

	virtual void Draw(sf::RenderWindow & wnd);
};

/*=========================
	Level/Game Code
=========================*/

const int MAX_ENTITIES = 256; //total number of entities allowed per level
const int MAX_CLIENTS  = 32;  //the first 32 entites are reserved for clients

template <typename T> class Singleton;

class EntityFactory :public Singleton<EntityFactory>
{
private:

	static std::vector<Entity*> m_entities;

public:

	~EntityFactory();

	Entity* Spawn(int entCode);
	
	Entity* GetEntity(int entIndex);

	void Update();

	void Draw(const sf::RenderWindow & wnd);
};

#define  gEntityFactory EntityFactory.GetSingleton() 

class Level
{
private:
	EntityFactory m_entFactory;

public:
	
	Level();

	bool Init();

	void Run();
};


/*====================================
	Resource Management
====================================*/

#define MAX_RESOURCES = 64;

//Interface
template < typename res > 
class ResourceManager
{
protected:
	std::map< const char*, res > m_resources;  
public:
	virtual res & Load( const char* file ) = 0;
};

/*==================================
	Singleton snippet
==================================*/

template <typename T> class Singleton
{
    static T* ms_Singleton;
protected:
    Singleton( void )
    {
        assert( !ms_Singleton );
		ms_Singleton = static_cast<T*>(this);
	}
   ~Singleton( void )
        {  assert( ms_Singleton );  ms_Singleton = 0;  }

public:
	static inline void Create()
	{
		if(ms_Singleton) return;
		new(T);
	}
    static inline T& GetSingleton( void )
	{ assert( ms_Singleton );  return ( *ms_Singleton );  }
    static inline T* GetSingletonPtr( void )
        {  return ( ms_Singleton );  }
};

template <typename T> T* Singleton <T>::ms_Singleton = 0;

/*=================
	Image Manager
=================*/

class ImageMan : public ResourceManager<sf::Image>, public Singleton<ImageMan>
{
public:

	virtual sf::Image & Load( const char * file);
};

#define g_ImageMan ImageMan::GetSingleton()

/*=================
	Sound Manager
=================*/

class SoundMan : public ResourceManager<sf::SoundBuffer>, public Singleton<SoundMan>
{
public:
	virtual sf::SoundBuffer & Load( const char* file );
};

#define g_SoundMan SoundMan::GetSingleton()

void InitGlobals();

/*===========================================
	Collision stuff
===========================================*/

inline bool Collision_Horz_Down(int x, int y, int width, int & tileY)
{
	int tilePixels = x - (x % TILE_SIZE);
	int testEnd = x + width;

	tileY = y/TILE_SIZE;

	int tileX = tilePixels/TILE_SIZE;

	while( tilePixels <= testEnd) {
		if( map[tileY][tileX] < 16 )
			return true;

		tileX++;
		tilePixels += TILE_SIZE;
	}

	return false;
}

inline bool Collision_Horz_Up(int x, int y, int width, int & tileY)
{
	int tilePixels = x-(x % TILE_SIZE);
	int testEnd = x + width;

	tileY = y/TILE_SIZE;

	int tileX = tilePixels/TILE_SIZE;

	while( tilePixels <= testEnd) {
		if( map[tileY][tileX] > 16 )
			return true;

		tileX++;
		tilePixels += TILE_SIZE;
	}

	return false;
}

inline bool Collision_Ver(int x, int y, int height, int & tileX)
{
	int tilePixels = y - ( y % TILE_SIZE );
	int testEnd = y + height;

	tileX = x/TILE_SIZE;

	int tileY = tilePixels/TILE_SIZE;

	while( tilePixels < testEnd) {
		if(map[tileY][tileX] < 16)
			return true;

		tileY++;
		tilePixels += TILE_SIZE;
	}

	return false;
}

inline bool RectCollision(const sf::Sprite & a, const sf::Sprite & b)
{
	sf::IntRect ar = a.GetSubRect();
	sf::IntRect br = b.GetSubRect();

	if( a.GetPosition().x + ar.Width < b.GetPosition().x  ||
		a.GetPosition().x > b.GetPosition().x + br.Width  ||
		a.GetPosition().y + ar.Height < b.GetPosition().y ||
		a.GetPosition().y > b.GetPosition().y + br.Height )
		return false;

	return true;
}

inline void Gravity(float &velY)
{
	velY += GRAVITY*app.GetFrameTime(); 
}

void PhysicsDos( Entity* ply )
{
	int tileCoord;

	//X AXIS
	
	if( ply->physData.velX > 0 ) { //moving right

		if( Collision_Ver(ply->sprite.GetPosition().x + ply->sprite.GetSubRect().Width + ply->physData.velX*app.GetFrameTime(), ply->sprite.GetPosition().y, ply->sprite.GetSubRect().Height, tileCoord ) ) {
			ply->sprite.SetX( tileCoord*TILE_SIZE - ply->sprite.GetSubRect().Width );
			//std::cout << "Collision right\n";
		}
		else
			ply->sprite.Move( ply->physData.velX*app.GetFrameTime(), 0 );
	}

	else if( ply->physData.velX < 0) { //moving left

		if( Collision_Ver( ply->sprite.GetPosition().x + ply->physData.velX*app.GetFrameTime(), ply->sprite.GetPosition().y, ply->sprite.GetSubRect().Height, tileCoord ) ) { //collision on the left side
			ply->sprite.SetX( (tileCoord + 1)*TILE_SIZE );	//move to the edge of the tile
			//std::cout << "Collision Left\n";
		}

		else  ply->sprite.Move( ply->physData.velX*app.GetFrameTime(), 0 ); 
	}

	//Y AXIS

	if( ply->physData.velY < 0) { //moving up

		ply->physData.grounded = false;

		if( Collision_Horz_Up(ply->sprite.GetPosition().x, ply->sprite.GetPosition().y + ply->physData.velY*app.GetFrameTime(), ply->sprite.GetSubRect().Width, tileCoord ) ) {
			ply->sprite.SetY( ( tileCoord + 1 ) * TILE_SIZE );
			ply->physData.velY = 0;
		}
		
		else {
			ply->sprite.Move( 0, ply->physData.velY );
			Gravity(ply->physData.velY);
		}
	}

	else { //moving down / on the ground

		if( Collision_Horz_Down( ply->sprite.GetPosition().x, ply->sprite.GetPosition().y + ply->sprite.GetSubRect().Height + ply->physData.velY*app.GetFrameTime(), ply->sprite.GetSubRect().Width, tileCoord ) ) { //on the ground
			ply->sprite.SetY( tileCoord * TILE_SIZE - ply->sprite.GetSubRect().Height );
			ply->physData.velY = 0;
			ply->physData.grounded = true;
		}
		else { //falling (in air)
			ply->sprite.Move( 0, ply->physData.velY );
			Gravity(ply->physData.velY);
			ply->physData.grounded = false;

			if(ply->physData.velY >= TILE_SIZE)
				ply->physData.velY = TILE_SIZE;

		}
	}

}