#include <SFML/Graphics.hpp> 
#include "EntitySystem.h"
#include "Utilities.h"
#include "imgui.h"

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

inline sf::Vector2f interpolate(const sf::Vector2f &a, const sf::Vector2f &b, float weight)
{
	return a + weight*(b-a); 
}

void mouseControl(Entity* ent, sf::Time elapsed)
{
	if(sf::Mouse::IsButtonPressed(sf::Mouse::Left))
	{
		ent->getAs<CompPhysics>()->target = sf::Vector2f((float)sf::Mouse::GetPosition(App).x, (float)sf::Mouse::GetPosition(App).y); 
	}
	ent->getAs<CompPosition>()->position = interpolate(ent->getAs<CompPosition>()->position, ent->getAs<CompPhysics>()->target, ent->getAs<CompPhysics>()->speedx*elapsed.AsSeconds()); 
}

void simplePhysicsStep(Entity* ent, sf::Time elapsed)
{
	CompPhysics  *phy = ent->getAs<CompPhysics>();

	CompDraw *draw = ent->getAs<CompDraw>();

	phy->bbox = draw->sprite.GetGlobalBounds();
}

void DrawEntities(Entity* ent, sf::Time elapsed)
{
	sf::Vector2f pos(ent->getAs<CompPosition>()->position.x,ent->getAs<CompPosition>()->position.y);

	ent->getAs<CompDraw>()->sprite.SetPosition(pos);

	App.Draw(ent->getAs<CompDraw>()->sprite); 
}

template <typename Comp> 
void ComponentTick(sf::Time elapsed)
{
	std::vector<Entity*> ents; 
	entitysystem.getEntities<Comp>(ents);

	for(auto iter = ents.begin(); iter != ents.end(); ++iter)
	{
		for(size_t i = 0; i < (*iter)->getAs<Comp>()->funcs.size(); ++i)
			(*iter)->getAs<Comp>()->funcs[i](*iter, elapsed); 		
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

			//process functions from physics components
			for( size_t i = 0; i < phys->funcs.size(); ++i )
			{
				phys->funcs[i]((*iter), elapsed); 
			}

			
			//check wall collisions
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

			//check entity collision
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

	phy->funcs.push_back(&simplePhysicsStep); 
	cont->funcs.push_back(&mouseControl); 
	draw->funcs.push_back(&DrawEntities);

	phy->speedx = 10.0f; 
	phy->speedy = 300.0f;

	sf::Image image;
	image.LoadFromFile("Data/doctor.png");
	image.CreateMaskFromColor(sf::Color::White); 
	draw->texture.LoadFromImage(image);
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

int main()
{
    // Create the main rendering window
    App.Create(sf::VideoMode(640,480,32), "SFML Graphics");
	UIState uistate;
	uistate.renderer = &App;

	InitEntities();
	PhysicsSub physSub(&App, &entitysystem); 

    // Start game loop
	while (App.IsOpen())
    {
        // Process events
        sf::Event Event;
		while (App.PollEvent(Event))
        {
			uistate.uiEvents(Event);
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

		uistate.imgui_prepare();
		widget::button(uistate, GEN_ID, sf::Vector2f(100, 30), sf::Vector2f(10, 10), "button", 24);
		uistate.imgui_finish();

        // Display window contents on screen
        App.Display();

		Clock.Restart();
    }

    return EXIT_SUCCESS;
}
