#include <SFML/Graphics.hpp> 
#include "EntitySystem.h"
#include "Utilities.h"

#include <algorithm>
#include <math.h>

Entity player; 
Entity object; 
EntitySystem entitysystem; 

sf::RenderWindow App;
sf::Clock Clock;

enum {
	DRAW=1, 
	POSITION,
	PLAYER,
	PHYSICS,
	CONTROLLER
};

enum {
	NONE  = 0x0000,
	RIGHT = 1 << 0, 
	LEFT  = 1 << 1, 
	UP	  = 1 << 8,
	DOWN  = 1 << 16
};

class SubSystem 
{
public:
	sf::RenderWindow *app; 
	EntitySystem *entitySystem; 

	SubSystem(sf::RenderWindow *wnd, EntitySystem *es) : app(wnd), entitySystem(es) {}
	virtual void Tick(sf::Time elapsed) = 0;
};

struct CompDraw : public Component
{
	static const FamilyId familyId = DRAW;
	sf::Texture texture; 
	sf::Sprite sprite; 
};

struct CompPosition : public Component
{
	static const FamilyId familyId = POSITION; 
	sf::Vector2f position;

	CompPosition() : position(0.0f,0.0f) {}
};

struct CompPlayer : public Component
{
	static const FamilyId familyId = PLAYER; 

	CompPlayer() {}
}; 

struct CompPhysics : public Component
{
	static const FamilyId familyId = PHYSICS;

	float speedx; 
	float speedy;

	int direction; 

	sf::Vector2f target; 

	sf::Rect<float> bbox; 

	CompPhysics () : speedx(200), speedy(200), direction(NONE) {target=sf::Vector2f(0,0);}
};

struct CompFart : public Component
{
	static const FamilyId familyId = 12; 
};

struct CompController : public Component 
{
	static const FamilyId familyId = CONTROLLER; 
};

void keyboardControl(Entity *ent, sf::Time elapsed)
{
	if(sf::Keyboard::IsKeyPressed(sf::Keyboard::Right))
		ent->getAs<CompPhysics>()->direction  = RIGHT;
	else if(sf::Keyboard::IsKeyPressed(sf::Keyboard::Left))
		ent->getAs<CompPhysics>()->direction = LEFT;
	else ent->getAs<CompPhysics>()->direction = NONE; 
}

void mouseControl(Entity* ent, sf::Time elapsed)
{
	if(sf::Mouse::IsButtonPressed(sf::Mouse::Left))
	{
		ent->getAs<CompPhysics>()->target = sf::Vector2f((float)sf::Mouse::GetPosition(App).x, (float)sf::Mouse::GetPosition(App).y); 
		std::cout << "mouse: " << ent->getAs<CompPhysics>()->target.x << ", " << ent->getAs<CompPhysics>()->target.y << std::endl;
	}
}

void simplePhysicsStep(Entity* ent, sf::Time elapsed)
{
	CompPhysics  *phy = ent->getAs<CompPhysics>();
	CompPosition *pos = ent->getAs<CompPosition>();

	CompDraw *draw = ent->getAs<CompDraw>();

	phy->bbox = draw->sprite.GetGlobalBounds();

	float speedx = phy->speedx; 
	float speedy = phy->speedy; 

	if( pos->position.x < phy->target.x )
	{
		pos->position.x += speedx*elapsed.AsSeconds(); 
		if( pos->position.x > phy->target.x)
			pos->position.x = phy->target.x; 
	}
	else if (pos->position.x > phy->target.x)
	{
		pos->position.x   -= speedx*elapsed.AsSeconds(); 
		if( pos->position.x < phy->target.x)
			pos->position.x = phy->target.x; 
	}

	if( pos->position.y < phy->target.y )
	{
		pos->position.y += speedy*elapsed.AsSeconds(); 
		if( pos->position.y > phy->target.y)
			pos->position.y = phy->target.y; 
	}
	else if (pos->position.y > phy->target.y)
	{
		pos->position.y   -= speedy*elapsed.AsSeconds(); 
		if( pos->position.y < phy->target.y)
			pos->position.y = phy->target.y; 
	}
	
}

void DrawEntities(Entity* ent, sf::Time elapsed)
{
	sf::Vector2f pos(ent->getAs<CompPosition>()->position.x,ent->getAs<CompPosition>()->position.y);

	ent->getAs<CompDraw>()->sprite.SetPosition(pos);

	App.Draw(ent->getAs<CompDraw>()->sprite); 
}

void InitEntities()
{
	//Player
	CompDraw *draw = new CompDraw();
	CompPosition *pos = new CompPosition(); 
	CompPlayer *ply = new CompPlayer();
	CompPhysics *phy = new CompPhysics();
	CompController *cont = new CompController();

	entitysystem.addComponent<CompDraw>(&player, draw); 
	entitysystem.addComponent<CompPosition>(&player, pos);
	entitysystem.addComponent<CompPlayer>(&player, ply);
	entitysystem.addComponent<CompPhysics>(&player, phy); 
	entitysystem.addComponent<CompController>(&player, cont);

	phy->func  = &simplePhysicsStep; 
	cont->func = &mouseControl; 
	draw->func = &DrawEntities;

	phy->speedx = 300.0f; 
	phy->speedy = 300.0f;

	draw->texture.LoadFromFile("Data/Doctor.png");
	draw->sprite.SetTexture(draw->texture);

	/*
	//object
	CompDraw *draw1 = new CompDraw();
	CompPosition *pos1 = new CompPosition(); 
	CompPhysics *phy1 = new CompPhysics();

	entitysystem.addComponent<CompDraw>(&object, draw1); 
	entitysystem.addComponent<CompPosition>(&object, pos1);
	entitysystem.addComponent<CompPhysics>(&object, phy1); 

	phy1->func  = &simplePhysicsStep; 
	draw1->func = &DrawEntities;

	phy1->speedx = 300.0f; 
	phy1->speedy = 300.0f;

	pos1->position = sf::Vector2i(0, 300); 

	draw1->sprite.SetTexture(draw->texture);
	*/
}

void PlayerControl(sf::Time elapsed)
{
	std::vector<Entity*> players; 
	entitysystem.getEntities<CompPlayer>(players);

	for(auto iter = players.begin(); iter != players.end(); ++iter)
	{
		(*iter)->getAs<CompController>()->func(*iter, elapsed);
	}
}

void PhysicsTick(sf::Time elapsed)
{
	std::vector<Entity*> physObjects;
	entitysystem.getEntities<CompPhysics>(physObjects);

	for(auto iter = physObjects.begin(); iter != physObjects.end(); ++iter)
	{
		(*iter)->getAs<CompPhysics>()->func(*iter, elapsed);
	}
}

template <typename Comp> 
void ComponentTick(sf::Time elapsed)
{
	std::vector<Entity*> ents; 
	entitysystem.getEntities<Comp>(ents);

	for(auto iter = ents.begin(); iter != ents.end(); ++iter)
	{
		(*iter)->getAs<Comp>()->func(*iter, elapsed); 
	}
	
}

class PhysicsSub :public SubSystem
{
public:
	PhysicsSub(sf::RenderWindow *wnd, EntitySystem *es) : SubSystem(wnd, es) {}
	virtual void Tick(sf::Time elapsed) 
	{
		std::vector<Entity*> ents;
		entitySystem->getEntities<CompPhysics>(ents);

		CompPhysics  *phys; 
		CompPosition *pos; 

		for(auto iter = ents.begin(); iter < ents.end(); ++iter)
		{
			phys = (*iter)->getAs<CompPhysics>(); 
			pos  = (*iter)->getAs<CompPosition>(); 

			//separate function
			phys->func(*iter, elapsed); 

			
			//separate function
			if( (pos->position.x + phys->bbox.Width) > App.GetWidth() )
			{
				phys->direction = NONE; 
				pos->position.x = App.GetWidth() - phys->bbox.Width;
			}
			else if( (pos->position.x) < 0.0f )
			{
				phys->direction = NONE; 
				pos->position.x       = 0.0f;
			}

			std::vector<Entity*>::iterator iter2; 
			if( iter != (ents.end()-1))
			{	
				iter2 = ++iter;
				iter--;
			}

			//separate function
			if( (ents.size() > 1) && (iter != (ents.end()-1)) )
				if( collides<float>(phys->bbox, (*iter2)->getAs<CompPhysics>()->bbox ) )
				{
					//do stuff
				}
				

		}
	}
};

int main()
{
    // Create the main rendering window
    App.Create(sf::VideoMode(800, 600, 32), "SFML Graphics");

	InitEntities();
	PhysicsSub physSub(&App, &entitysystem); 

    // Start game loop
	while (App.IsOpen())
    {
        // Process events
        sf::Event Event;
		while (App.PollEvent(Event))
        {
            // Close window : exit
            if (Event.Type == sf::Event::Closed)
                App.Close();
		}
		
		ComponentTick<CompController>(Clock.GetElapsedTime());
		//ComponentTick<CompPhysics>(Clock.GetElapsedTime());
		physSub.Tick(Clock.GetElapsedTime());

        // Clear the screen with red color
		App.Clear(sf::Color::Black);

		ComponentTick<CompDraw>(Clock.GetElapsedTime());

        // Display window contents on screen
        App.Display();

		Clock.Restart();
    }

    return EXIT_SUCCESS;
}
