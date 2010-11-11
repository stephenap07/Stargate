#include "imgui.h"

#include <SFML/Audio.hpp>

#include <iostream>
#include <string>
#include <cmath>

static float scale = .5f;

//functions

 int UIState::regionhit(const sf::IntRect &rect)
{
	if ( rect.Contains(mousex, mousey) )
	 return 1;

   return 0;
}

 void UIState::imgui_prepare()
{
	hotitem = 0;
}

 void UIState::imgui_finish()
{
	if(mousedown == 0)
		activeitem = 0;
	else
	{
		if(activeitem == 0)
			activeitem = -1;
	}

	if(keyentered == sf::Key::Tab)
		kbfocus = 0;

	keyentered = 0;
	keychar = 0;
}

 void UIState::uiEvents(const sf::Event &aEvent, const sf::Input &input)
{
	switch(aEvent.Type)
	{
	case sf::Event::MouseMoved :
		{
			mousex = input.GetMouseX();
			mousey = input.GetMouseY();
		} break;

	case sf::Event::MouseButtonPressed:
		{
			if(aEvent.MouseButton.Button == sf::Mouse::Left)
			{
				mousedown = 1;
			}
		} break;

	case sf::Event::MouseButtonReleased:
		{
			if(aEvent.MouseButton.Button == sf::Mouse::Left)
				mousedown =0;
		} break;

	case sf::Event::KeyPressed:
		{
			keyentered = aEvent.Key.Code;
			keymod = aEvent.Key.Shift;
		}break;
	case sf::Event::TextEntered:
		{
			if( (aEvent.Text.Unicode & 0xFF80) == 0)
				keychar = aEvent.Key.Code;
		}break;
	}
}

//helper functions for widgets
 bool UIState::processkeyboard(int id)
{
	if(kbfocus == id)
		{
			switch(keyentered)
			{
				//if tab was pressed, lose keyboard focus for the next widget to pick it up
			case sf::Key::Tab:
				{
					kbfocus = 0;

					if( keymod )
						kbfocus = lastwidget;

					keyentered = 0;
				}break;

			case sf::Key::Return:
				return true; 
			}
		}

	return false;
}

 inline void RenderText(const char* text, float x, float y, const sf::Color &color = sf::Color::White)
 {
	 //set string parameters
	 sf::String string(text);
	 string.SetCenter(string.GetRect().GetWidth()/2, string.GetRect().GetHeight()/2);
	 string.SetScale(scale, scale);
	 string.SetPosition(x, y);

	 //drop shadow
	 string.SetColor(sf::Color(0,0,0,255/2));
	 string.Move(3,3);
	 UIState::renderer->Draw(string);

	 //draw the string
	 string.SetColor(color);
	 string.Move(-3,-3);
	 
	 UIState::renderer->Draw(string);
 }

bool UIState::checkhot(int id, const sf::IntRect & rect)
{
	if(regionhit(rect))
		{
			hotitem =id;

			if(activeitem == 0 && mousedown)
			{
				activeitem = id;
			}

			return true;
		}

		else if(lasthotitem == id)
		{
			lasthotitem  = 0;
		}

	return false;
}

namespace widget
{
	int button(UIState &ui, int id, const sf::IntRect &rect, const char* text)
	{

		if(ui.checkhot(id, rect))
		{
			if(ui.lasthotitem != id)
			{
				ui.lasthotitem = id;
			}
		}

		float outline = 0;
		sf::Color outlinecolor(sf::Color(255,0,255));

		//checking keyboard focus
		if(ui.kbfocus == 0)
			ui.kbfocus = id;

		//if we have keyboard focus, draw the border
		if(ui.kbfocus == id)
		{
			outline = 2.5f;
		}
	

		//render the button

		sf::Shape button = sf::Shape::Rectangle(rect.Left, rect.Top, rect.Right, rect.Bottom, sf::Color(127,127,127), outline, outlinecolor);
		button.EnableOutline(false);

		button.Move(4,4);

		UIState::renderer->Draw(button);

		button.Move(-4,-4);

		if(ui.hotitem == id)
		{
			if(ui.activeitem == id)
			{
				button.Move(4,4);

				UIState::renderer->Draw(button);
				RenderText(text, rect.Left + rect.GetWidth()/2 + 4, rect.Top + rect.GetHeight()/2 + 4, sf::Color(255,255,255));
			}
			else 
			{
				button.SetColor(sf::Color(255,255,255));
				UIState::renderer->Draw(button);
				RenderText(text, rect.Left + rect.GetWidth()/2, rect.Top + rect.GetHeight()/2, sf::Color(255,255,255));
			}
		}

		else 
		{
			button.SetColor(sf::Color(127,127,127,255/1.5));
			button.EnableOutline(true);

			UIState::renderer->Draw(button);
			RenderText(text,rect.Left + rect.GetWidth()/2, rect.Top + rect.GetHeight()/2, sf::Color(255,255,255));
		}

		//if enter was pressed and we have keyboard focus
		if(ui.processkeyboard(id))
			return 1;

		ui.lastwidget = id;

		//button was pressed
		if (ui.mousedown == 0
		&& ui.hotitem == id
		&& ui.activeitem == id)
			return 1;

		return 0;
	}

	int buttonImage(UIState &ui, int id, int x, int y, sf::Sprite & sprite)
	{
		sf::IntRect rect(x, y, x+sprite.GetSubRect().GetWidth(), y+sprite.GetSubRect().GetHeight());
		sprite.SetPosition(x,y);

		if(ui.checkhot(id, rect))
		{

			if(ui.lasthotitem != id)
			{
				ui.lasthotitem = id;
			}
		}

		sf::Color color = sprite.GetColor();
		color.a /= 2;
		sprite.SetColor(color);

		//sets the alpha to normal if item is hot
		if(ui.hotitem == id)
		{
			color.a *= 2;
			sprite.SetColor(color);
		}

		color.a = 255;
		UIState::renderer->Draw(sprite);

		//turns the sprite back to normal
		sprite.SetColor(color);

		ui.lastwidget = id;

		//button was pressed
		if (ui.mousedown == 0
		&& ui.hotitem == id
		&& ui.activeitem == id)
			return 1;

		return 0;
	}

	//textfield for writing in: i.e. high score
	int textfield(UIState &ui, int id, const sf::IntRect &rect, std::string &string)
	{
		static float elapsed_time = 0;

		//checking hotness
		if( ui.regionhit(rect) )
		{
			ui.hotitem = id;
			if(ui.activeitem == 0 && ui.mousedown)
				ui.activeitem = id;
		}

		//keyboard focus 
		if(ui.kbfocus == 0)
			ui.kbfocus = id;

		float outline = 0;
		sf::Color outlinecolor(sf::Color(255,0,255));

		//if it has keyboard focus, set up outline 
		if(ui.kbfocus == id)
		{
			outline = 1.5f;
		}

		//set up shapes and strings
		sf::Shape field = sf::Shape::Rectangle(rect.Left, rect.Top, rect.Right, rect.Bottom, sf::Color(127,127,127), outline, outlinecolor);
		field.EnableOutline(false);

		sf::String str;

		str.SetText(string);
		str.SetScale(scale,scale);
		str.SetPosition(rect.Left+8,rect.Top+6);

		//draw if it is active or hot
		if(ui.activeitem == id || ui.hotitem == id)
		{
			UIState::renderer->Draw(field);
		}

		//or just normal...
		else 
		{
			field.SetColor(sf::Color(127,127,127, 255/2));
			if(ui.kbfocus == id)
				field.EnableOutline(true);

			UIState::renderer->Draw(field);
		}

		//draw the text
		UIState::renderer->Draw(str);
		
		sf::String caret;

		caret.SetText("_");
		caret.SetScale(scale,scale);

		caret.SetPosition(str.GetRect().Right, rect.Top+4);

		if(ui.kbfocus == id && (uiClock.GetElapsedTime() - elapsed_time) > 0.75f) 
		{
			UIState::renderer->Draw(caret);
		}

		if( (uiClock.GetElapsedTime() - elapsed_time) > 1.0f)
			elapsed_time = uiClock.GetElapsedTime();

		int changed = 0;

		//if has keyboard focus
		if(ui.kbfocus == id)
		{
			switch(ui.keyentered)
			{
			case sf::Key::Tab:
				{
					ui.kbfocus = 0;

					if(ui.keymod)
						ui.kbfocus = ui.lastwidget;

					ui.keyentered = 0;
				} break;
			case sf::Key::Back:
				{
					if(string.length() > 0)
					{
						string.pop_back();
						changed = 1;
					}
				} break;
			}

			if(ui.keychar >= 32 && ui.keychar < 127 && str.GetRect().GetWidth() < rect.GetWidth() -18)
			{
				string.push_back(ui.keychar);
				changed = 1;
			}
		}

		if(ui.mousedown == 0 
			&& ui.hotitem == id
			&& ui.activeitem == id)

			ui.kbfocus = id;

		ui.lastwidget = id;

		return changed;
	}

	int scrollbar(UIState &ui, int id, int x, int y, int max, int &value, int width, int height)
	{
		//has a set height and width
		int ypos = ((height-width/2) * value)/max;

		if(ui.regionhit(sf::IntRect(x, y, x+width, y + height)))
		{
			ui.hotitem = id;

			if(ui.activeitem == 0 && ui.mousedown)
				ui.activeitem = id;
		}

		sf::Shape bar = sf::Shape::Rectangle(sf::Vector2f(x, y), sf::Vector2f(x+width, y+height+16), sf::Color(10,10,10,255/2));
		sf::Shape nub = sf::Shape::Rectangle(sf::Vector2f(x+width/3,y+width/3 + ypos), sf::Vector2f(x+width/3 + width/2, y+width/3 + ypos + width/2), sf::Color(255,255,255,255/2));
		UIState::renderer->Draw(bar);

		if(ui.activeitem == id || ui.hotitem == id)
		{
			nub.SetColor(sf::Color(255,255,255,255));
			UIState::renderer->Draw(nub);
		}
		else UIState::renderer->Draw(nub);

		if(ui.activeitem == id)
		{
			int mousepos = ui.mousey - (y + width/3);
			if(mousepos < 0) mousepos = 0;
			if(mousepos > height-1) mousepos = height-1;
			int v = (mousepos* max) / height;
			if(v != value)
			{
				value = v; 
				return 1;
			}
		}

		return 0;
	}

} //namespace widget