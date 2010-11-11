#include <SFML/Graphics.hpp>
#include "Constants.h"
#include "TileMap.h"
#include "Utilities.h"
#include "imgui.h"
#include "boost/lexical_cast.hpp"

#include <iostream>
//#include <vector>

sf::RenderWindow app;

sf::RenderWindow *UIState::renderer = &app; //renderer
UIState ui; //gui
cMap map; //map 
bool initial; // is the prompt screen up still?

std::string tilefile;
std::string sw;
std::string sh;
int iw, ih;
sf::Image tilesetImage;
sf::Sprite tilesetSprites[TOTAL_TILES];
int id;

//intital tile map properties like width, height and path to image file 
inline void intitialQuery()
{
	static int width = 200;
	static int height = 30; 
	static int x = width/2; 
	static int y = CAMERA_HEIGHT/4;

	static sf::String textFile;
	static sf::String textWidth;
	static sf::String textHeight;

	textFile.SetText("path to tile image file");
	textWidth.SetText("Width");
	textHeight.SetText("Height");

	textFile.SetScale(.5, .5);
	textWidth.SetScale(.5,.5);
	textHeight.SetScale(.5,.5);

	textFile.SetPosition(x+width, y);
	textWidth.SetPosition(x+width, y+height*1.6);
	textHeight.SetPosition(x+width, y+height*2.6);

	app.Draw(textFile);
	app.Draw(textWidth);
	app.Draw(textHeight);

	if(widget::textfield(ui, GEN_ID, sf::IntRect(x,y,x+width, y+height), tilefile))
	{
	}
	if(widget::textfield(ui, GEN_ID, sf::IntRect(x,y+height*1.5,x+width, y+height*1.5 + height), sw))
	{
	
	}
	if(widget::textfield(ui, GEN_ID, sf::IntRect(x,y+height*2.5,x+width, y+height*2.5 + height), sh))
	{
	
	}
	if(widget::button(ui, GEN_ID, sf::IntRect(x,y+height*4.5, x+60, y+height*4.5 + 20), "ok"))
	{
		initial = false;
	}

	if(initial == false)
	{
		if(sw.size() > 0)
			iw = boost::lexical_cast<int>(sw);
		if(sh.size() > 0)
			ih = boost::lexical_cast<int>(sh);

		map.setWidth(iw);
		map.setHeight(ih);

		if(!map.loadTileSheet(tilefile))
			initial = true;
		else
		{
			map.initializeMap();
			tilesetImage.LoadFromFile(tilefile);
		}
	}
}

inline void setupguiTiles()
{
	int n = 0;
	for(int j = 0; j < 2; j++)
	{
		for(int k = 0; k < TOTAL_TILES/2; k++)
		{
			tilesetSprites[n].SetImage(tilesetImage);

			tilesetSprites[n].SetSubRect(sf::IntRect(k*TILE_WIDTH, //left
												j*TILE_HEIGHT, //top
												k*TILE_WIDTH + TILE_WIDTH,   //right
												j*TILE_HEIGHT +  TILE_HEIGHT)); //bottom
			n++;
		}
	}
}

inline void doTileGui(int ax, int ay)
{
	int n = 0;
	for(int j = 0; j < 2; j++)
	{
		for(int k = 0; k < TOTAL_TILES/2; k++)
		{
			if(widget::buttonImage(ui, GEN_ID+n, ax+k*TILE_WIDTH, ay+j*TILE_HEIGHT, tilesetSprites[n]))
			{
				id = n;
			}

			n++;
		}
	}
}


int main()
{
	app.Create(sf::VideoMode(CAMERA_WIDTH, CAMERA_HEIGHT, 32), "Test Window");
	app.SetFramerateLimit(30);

	sf::Vector2f Center(CAMERA_WIDTH/2,CAMERA_HEIGHT/2);
	sf::Vector2f Half(CAMERA_WIDTH/2,CAMERA_HEIGHT/2);

	setupguiTiles();

	initial = true;
	tilefile = "data/Tileset1.png";
	sw = "30";
	sh = "20";

	sf::View view(Center, Half);

	sf::Event ev;
	bool quit = false; 
	id = 0;
	bool rightMouseToggle = false;
	bool canAddTile = true;
	int rightMouseX = 0;
	int rightMouseY = 0;
	int value = 255/2;


	while(!quit)
	{
		const sf::Input &input = app.GetInput();

	    while( app.GetEvent(ev) )
	    {
            if(ev.Type == sf::Event::Closed)
			{
	            quit = true; 
			    break; 
			}

			if(ev.Type == sf::Event::MouseWheelMoved)
			{
				if(ev.MouseWheel.Delta > 0)
				{
					if(id > TOTAL_TILES-1) break;
					id++;
				}
				else
				{
					if(id < 1) break; 
					id--;
				}
			}

			if( ev.Type == sf::Event::KeyPressed )
			{
				switch(ev.Key.Code)
				{
				case sf::Key::Left:
					view.Move(-TILE_WIDTH,0);
					break;
				case sf::Key::Right:
					view.Move(TILE_WIDTH,0);
					break;
				case sf::Key::Up:
					view.Move(0, -TILE_HEIGHT);
					break;
				case sf::Key::Down:
					view.Move(0, TILE_HEIGHT);
					break;
				case sf::Key::J:
					map.setWidth(map.GetWidth()+1);
					break;
				case sf::Key::K:
					map.setHeight(map.GetHeight()-1);
				}
			}

			if( (ev.Type == sf::Event::MouseButtonPressed) && (ev.MouseButton.Button == sf::Mouse::Right) && !initial)
			{
				rightMouseToggle = !rightMouseToggle;
				rightMouseX = app.GetInput().GetMouseX(); //app.ConvertCoords(app.GetInput().GetMouseX(), app.GetInput().GetMouseY()).x;
				rightMouseY = app.GetInput().GetMouseY(); //app.ConvertCoords(app.GetInput().GetMouseX(), app.GetInput().GetMouseY()).y;
			}

			ui.uiEvents(ev, input);
      }

		
		if(input.IsMouseButtonDown(sf::Mouse::Left) && !initial && !ui.hotitem )
		{
			int _x = app.ConvertCoords(app.GetInput().GetMouseX(), app.GetInput().GetMouseY()).x/TILE_WIDTH;
			int _y = app.ConvertCoords(app.GetInput().GetMouseX(), app.GetInput().GetMouseY()).y/TILE_HEIGHT;

			map.addTile(_x,_y, Layer::MAIN, id, Collision::NORMAL);
		}

		ui.imgui_prepare();
		
		app.Clear();

		map.draw(app);

		app.SetView(app.GetDefaultView());

		if(initial)
			intitialQuery();
		else 
		{
			if(rightMouseToggle)
			{
				doTileGui(rightMouseX, rightMouseY);
			}

			if(widget::scrollbar(ui, GEN_ID, CAMERA_WIDTH-16, 0, 255, value, 16, CAMERA_HEIGHT))
			{

			}
		}

		ui.imgui_finish();

		app.SetView(view); 

		app.Display();
	}

	app.Close(); 
	return 0;
}