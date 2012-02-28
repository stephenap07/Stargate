#ifndef UTILITIES_H_
#define UTILITIES_H_

#include <SFML/Graphics.hpp>

inline bool collides(int x1, int x2, int y1, int y2, int width1, int width2, int height1, int height2)
{
	if( (x1 + width1) < x2 || (y1+height1 < y2) ) return false;
	if( (x1 > x2 + width2) || (y1 > y2 + height2) ) return false;

	return true;
}

inline bool collides(const sf::IntRect & rect1, const sf::IntRect & rect2)
{
	if( (rect1.Left + rect1.Width) < rect2.Left || (rect1.Top+rect1.Height < rect2.Top) ) return false;
	if( (rect1.Left > rect2.Left + rect2.Width) || (rect1.Top > rect2.Top + rect2.Height) ) return false;

	return true;
}

#endif