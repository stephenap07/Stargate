#include <SFML/Graphics.hpp> 
#include "EntitySystem.h"
#include "Utilities.h"
#include "imgui.h"

#include <algorithm>
#include <math.h>

Entity object; 
EntitySystem entitysystem; 

sf::RenderWindow App;
sf::Clock Clock;

enum {
	DRAW=1, 
	POSITION,
	PLAYER,
	PHYSICS,
	CONTROLLER,
	COLLIDABLE, 
	PROJECTILE
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
	virtual void Tick(float dt) = 0;
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

struct CompCollidable : public Component
{
	static const FamilyId familyId = COLLIDABLE;

	std::vector<Entity*> collidedEntities; 

	bool collided; 
};

struct CompProjectile :public Component
{
	const static FamilyId familyId = PROJECTILE; 
};

class PhysicsSub :public SubSystem
{
public:
	PhysicsSub(sf::RenderWindow *wnd, EntitySystem *es) : SubSystem(wnd, es) {}
	virtual void Tick(float dt) 
	{
		std::vector<Entity*> ents;
		entitySystem->getEntities<CompPhysics>(ents);

		if(ents.empty())
			return;

		CompPhysics  *phys; 
		CompPosition *pos; 
		CompCollidable *col;

		for(auto iter = ents.begin(); iter < ents.end(); ++iter)
		{
			phys = (*iter)->getAs<CompPhysics>(); 
			pos  = (*iter)->getAs<CompPosition>(); 
			col  = (*iter)->getAs<CompCollidable>();
			
			//check wall collisions
			if( (pos->position.x + phys->bbox.Width) > App.GetWidth() )
			{
				phys->direction = NONE; 
				col->collided = true; 
				pos->position.x = App.GetWidth() - phys->bbox.Width;
			}
			else if( (pos->position.x) < 0.0f )
			{
				col->collided = true; 
				phys->direction = NONE; 
				pos->position.x       = 0.0f;
			}
			else col->collided = false; 

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
					if(!(*iter2)->getAs<CompCollidable>())
						continue; 

					col->collidedEntities.push_back(*iter2); 

					(*iter2)->getAs<CompCollidable>()->collidedEntities.push_back(*iter); 
				}
				

		}
	}
};

class ControllerSystem : public SubSystem
{
public:

	ControllerSystem(sf::RenderWindow *wnd, EntitySystem *es) : SubSystem(wnd, es) {}

	virtual void Tick(float dt)
	{
		std::vector<Entity*> players; 
		entitysystem.getEntities<CompPlayer>(players);

		CompPosition *pos; 
		CompPhysics *phys; 

		for(auto it = players.begin(); it != players.end(); ++it)
		{
			pos  = (*it)->getAs<CompPosition>(); 
			phys = (*it)->getAs<CompPhysics>();

			if(sf::Keyboard::IsKeyPressed(sf::Keyboard::Key::Right))
				pos->position.x += phys->speedx * dt; 
			else if( sf::Keyboard::IsKeyPressed(sf::Keyboard::Key::Left))
				pos->position.x -= phys->speedx * dt; 

			if(sf::Keyboard::IsKeyPressed(sf::Keyboard::Key::Up))
				pos->position.y -= phys->speedy * dt; 
			else if (sf::Keyboard::IsKeyPressed(sf::Keyboard::Key::Down))
				pos->position.y += phys->speedy * dt; 
		}
	}
};

class ProjectileSystem : public SubSystem
{
public:
	ProjectileSystem(sf::RenderWindow *wnd, EntitySystem *es) : SubSystem(wnd,es) {}

	virtual void Tick(float dt)
	{
		std::vector<Entity*> ents; 
		entitysystem.getEntities<CompProjectile>(ents);

		for(auto it = ents.begin(); it != ents.end(); ++it)
		{
			(*it)->getAs<CompPosition>()->position.x += (*it)->getAs<CompPhysics>()->speedx * dt; 
			if((*it)->getAs<CompCollidable>()->collided)
				(*it)->getAs<CompPhysics>()->speedx = -(*it)->getAs<CompPhysics>()->speedx;
		}
	}
};

class RenderingSystem : public SubSystem
{
public:
	RenderingSystem(sf::RenderWindow *wnd, EntitySystem *es) : SubSystem(wnd, es) {}
	virtual void Tick(float dt)
	{
		std::vector<Entity*> ents; 
		entitySystem->getEntities<CompDraw>(ents);

		for(auto iter = ents.begin(); iter != ents.end(); ++iter)
		{
			(*iter)->getAs<CompDraw>()->sprite.SetPosition((*iter)->getAs<CompPosition>()->position); 
			app->Draw((*iter)->getAs<CompDraw>()->sprite);
		}
	}
};

Entity *makePlayer()
{
	Entity *player = new Entity();
	CompDraw *draw = new CompDraw();
	CompPosition *pos = new CompPosition(); 
	CompPlayer *ply = new CompPlayer();
	CompPhysics *phy = new CompPhysics();
	CompController *cont = new CompController();
	CompCollidable *col = new CompCollidable();

	entitysystem.addComponent<CompDraw>(player, draw); 
	entitysystem.addComponent<CompPosition>(player, pos);
	entitysystem.addComponent<CompPlayer>(player, ply);
	entitysystem.addComponent<CompPhysics>(player, phy); 
	entitysystem.addComponent<CompController>(player, cont);
	entitysystem.addComponent<CompCollidable>(player, col); 

	phy->speedx = 300.0f; 
	phy->speedy = 300.0f;

	sf::Image image;
	image.LoadFromFile("Data/doctor.png");
	image.CreateMaskFromColor(sf::Color::White); 
	draw->texture.LoadFromImage(image);
	draw->sprite.SetTexture(draw->texture);

	return player;
}

Entity *makeProjectile()
{
	Entity *projectile = new Entity();

	CompProjectile *pro = new CompProjectile();
	CompPosition   *pos = new CompPosition(); 
	CompDraw       *drw = new CompDraw(); 
	CompPhysics    *phy = new CompPhysics();
	CompCollidable *col = new CompCollidable();
		
	entitysystem.addComponent<CompPosition>(projectile, pos);
	entitysystem.addComponent<CompDraw>(projectile, drw);
	entitysystem.addComponent<CompPhysics>(projectile, phy);
	entitysystem.addComponent<CompCollidable>(projectile, col);
	entitysystem.addComponent<CompProjectile>(projectile, pro);

	phy->speedx = 200.0f;

	sf::Image img;
	img.LoadFromFile("Data/ball.png");
	drw->texture.LoadFromImage(img);
	drw->sprite.SetTexture(drw->texture);

	return projectile;
}

void InitEntities()
{
	Entity* ply = makePlayer();
}

int main()
{
    // Create the main rendering window
    App.Create(sf::VideoMode(640,480,32), "SFML Graphics");
	UIState uistate;
	uistate.renderer = &App;

	InitEntities();
	PhysicsSub physSub(&App, &entitysystem); 
	RenderingSystem renderSys(&App, &entitysystem);
	ControllerSystem controller(&App, &entitysystem);
	ProjectileSystem projSys(&App, &entitysystem);

	bool inventory = false;
	int invX; 
	int invY;

	std::vector<Entity*> players; 
	entitysystem.getEntities<CompPlayer>(players);
	Entity* player = players[0];

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
			if(Event.Type == sf::Event::MouseButtonPressed && Event.MouseButton.Button == sf::Mouse::Right)
			{
				inventory = !inventory;
				if(inventory)
					invX = sf::Mouse::GetPosition(App).x;
					invY = sf::Mouse::GetPosition(App).y; 
			}
			if(Event.Type == sf::Event::MouseButtonPressed && Event.MouseButton.Button == sf::Mouse::Left)
				makeProjectile();
		}

		controller.Tick(Clock.GetElapsedTime().AsSeconds());
		physSub.Tick(Clock.GetElapsedTime().AsSeconds());
		projSys.Tick(Clock.GetElapsedTime().AsSeconds());

        // Clear the screen with red color
		App.Clear(sf::Color::Black);

		uistate.imgui_prepare();
		if(widget::button(uistate, GEN_ID, sf::Vector2f(100, 30), sf::Vector2f(520, 10), "button", 24))
			entitysystem.deleteEntity(player);
		if(inventory)
			if(widget::button(uistate, GEN_ID, sf::Vector2f(100, 30), sf::Vector2f(invX, invY), "inventory", 20))
				player = makePlayer();

		uistate.imgui_finish();

		renderSys.Tick(Clock.GetElapsedTime().AsSeconds());

        // Display window contents on screen
        App.Display();

		Clock.Restart();
    }

    return EXIT_SUCCESS;
}
