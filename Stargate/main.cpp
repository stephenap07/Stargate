#include <SFML/Graphics.hpp> 
#include "EntitySystem.h"
#include "Utilities.h"

#include <algorithm>

Entity player; 
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
	RIGHT, 
	LEFT, 
	UP,
	DOWN,
	NONE
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
	float x;
	float y; 

	CompPosition() : x(0), y(0) {}
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

	sf::FloatRect bbox; 

	CompPhysics () : speedx(200), speedy(200), direction(NONE) {}
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

void simplePhysicsStep(Entity* ent, sf::Time elapsed)
{
	CompPhysics  *phy = ent->getAs<CompPhysics>();
	CompPosition *pos = ent->getAs<CompPosition>();

	CompDraw *draw = ent->getAs<CompDraw>();

	phy->bbox = draw->sprite.GetGlobalBounds();

	float speedx = phy->speedx; 

	switch(phy->direction) 
	{
	case RIGHT : 
		pos->x += speedx*elapsed.AsSeconds(); 
		break;
	case LEFT  : 
		pos->x -= speedx*elapsed.AsSeconds();
		break;
	}
}

void DrawEntities(Entity* ent, sf::Time elapsed)
{
	float x = ent->getAs<CompPosition>()->x;
	float y = ent->getAs<CompPosition>()->y;

	ent->getAs<CompDraw>()->sprite.SetPosition(x,y);

	App.Draw(ent->getAs<CompDraw>()->sprite); 
}

void InitEntities()
{
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
	cont->func = &keyboardControl; 
	draw->func = &DrawEntities;

	phy->speedx = 300.0f; 
	phy->speedy = 300.0f;

	draw->texture.LoadFromFile("Data/Doctor.png");
	draw->sprite.SetTexture(draw->texture);
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

		for(auto iter = ents.begin(); iter != ents.end(); ++iter)
		{
			phys = (*iter)->getAs<CompPhysics>(); 
			pos  = (*iter)->getAs<CompPosition>(); 

			//separate function
			phys->func(*iter, elapsed); 

			//separate function
			if( (pos->x + phys->bbox.Width) > App.GetWidth() )
			{
				phys->direction = NONE; 
				pos->x = App.GetWidth() - phys->bbox.Width;
			}
			else if( (pos->x) < 0.0f )
			{
				phys->direction = NONE; 
				pos->x       = 0.0f;
			}

			//separate function
			if( phys->bbox.Contains( (*iter)->getAs<CompPhysics>()->bbox ) )


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
        App.Clear(sf::Color(250, 250, 250));

		ComponentTick<CompDraw>(Clock.GetElapsedTime());

        // Display window contents on screen
        App.Display();

		Clock.Restart();
    }

    return EXIT_SUCCESS;
}
