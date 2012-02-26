#include <SFML/Graphics.hpp> 
#include "EntitySystem.h"
#include <algorithm>
#include <iostream>

Entity player; 
EntitySystem entitysystem; 

sf::RenderWindow App;
sf::Clock Clock;

enum {
	DRAW=1, 
	POSITION,
	PLAYER
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

	float speedx;
	float speedy;

	CompPlayer() :speedx(0), speedy(0) {}
}; 

void InitEntities()
{
	CompDraw *draw = new CompDraw();
	CompPosition *pos = new CompPosition(); 
	CompPlayer *ply = new CompPlayer();

	entitysystem.addComponent<CompDraw>(&player, draw); 
	entitysystem.addComponent<CompPosition>(&player, pos);
	entitysystem.addComponent<CompPlayer>(&player, ply);

	draw->texture.LoadFromFile("Data/Doctor.png");
	draw->sprite.SetTexture(draw->texture);
}

void DrawEntities()
{
	std::vector<Entity*> drawableEntities;
	entitysystem.getEntities<CompDraw>(drawableEntities);

	for(auto iter=drawableEntities.begin(); iter != drawableEntities.end(); ++iter)
	{
		float x = (*iter)->getAs<CompPosition>()->x;
		float y = (*iter)->getAs<CompPosition>()->y;

		(*iter)->getAs<CompDraw>()->sprite.SetPosition(x,y);

		App.Draw((*iter)->getAs<CompDraw>()->sprite); 
	}
}

void KeyboardControl(Entity *ent)
{
	float speed = 10.0f;

	if(sf::Keyboard::IsKeyPressed(sf::Keyboard::Right))
		ent->getAs<CompPlayer>()->speedx  = speed;
	else if(sf::Keyboard::IsKeyPressed(sf::Keyboard::Left))
		ent->getAs<CompPlayer>()->speedx -= speed;
	else ent->getAs<CompPlayer>()->speedx = 0.0f;
}

void PlayerControl(sf::Time elapsed)
{
	std::vector<Entity*> players; 
	entitysystem.getEntities<CompPlayer>(players);

	for_each(players.begin(),players.end(),KeyboardControl); 

	float speedx = players[0]->getAs<CompPlayer>()->speedx; 
	players[0]->getAs<CompPosition>()->x += speedx*elapsed.AsSeconds(); 
}

int main()
{
    // Create the main rendering window
    App.Create(sf::VideoMode(800, 600, 32), "SFML Graphics");

	InitEntities();

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
		
		PlayerControl(Clock.GetElapsedTime());

        // Clear the screen with red color
        App.Clear(sf::Color(250, 250, 250));
		
		DrawEntities();

        // Display window contents on screen
        App.Display();

		Clock.Restart();
    }

    return EXIT_SUCCESS;
}
