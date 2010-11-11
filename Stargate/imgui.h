#pragma once

#include <SFML/Graphics.hpp>

//this is how to make custom IDs; should be changed to not use them at all maybe
#ifdef IMGUI_SRC_ID
#define GEN_ID ((IMGUI_SRC_ID) + (__LINE__))
#else
#define GEN_ID (__LINE__)
#endif

//GUI context
struct UIState
{
	int mousex;
	int mousey;
	int mousedown;

	int hotitem;
	int activeitem;

	int kbfocus;
	int keyentered;
	int keymod;
	int keychar;
  
    int lastwidget;
	int lasthotitem;

	void imgui_prepare();
	void imgui_finish();
	void uiEvents(const sf::Event &aEvent, const sf::Input &input);

	int regionhit(const sf::IntRect &rect);
	bool processkeyboard(int id);
	bool checkhot(int id, const sf::IntRect & rect);

	static sf::RenderWindow* renderer;
	
};


static sf::Clock uiClock;

//
///WIDGETS HERE
namespace widget
{
	int button(UIState & ui, int id, const sf::IntRect &rect, const char* text);

	int buttonImage(UIState & ui, int id, int x, int y, sf::Sprite & sprite);
	
	//textfield for writing in: i.e. high score
	int textfield(UIState & ui, int id, const sf::IntRect &rect, std::string &string);

	int scrollbar(UIState & ui, int id , int x, int y, int max, int &value, int width =16, int height=256);
	
} //widget