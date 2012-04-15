#include <SFML/Graphics.hpp> 
#include "imgui.h"
#include "Entity.h"
#include "Command.h"

#include <algorithm>
#include <math.h>

sf::RenderWindow App;
sf::Clock Clock;

sf::RenderWindow* Entity::m_window = &App;

int doEntity(float x, float y, Entity& ent, Command const &commands)
{
	ent.SetPosition(x,y);
	
	// Do some other stuff

	ent.Draw();

	return 1; 
}

int main()
{
	// Create the main rendering window
	App.Create(sf::VideoMode(640,480,32), "SFML Graphics");
	UIState uistate;
	uistate.renderer = &App;

	sf::Image image; 
	image.LoadFromFile("Data/doctor.png");

	Entity ent(image);

	Command commands; 

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

		App.Clear(sf::Color::Black);

		doEntity(30.0f, 40.0f, ent, commands);

		App.Display();

		Clock.Restart();
	}

	return EXIT_SUCCESS;
}
