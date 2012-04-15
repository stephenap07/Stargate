#ifndef ENTITY_H_
#define ENTITY_H_

class Entity
{
private:
	sf::Texture m_texture;
	sf::Sprite m_sprite;

	static sf::RenderWindow* m_window; 
	
public:
	Entity(sf::Image const& img) { m_texture.LoadFromImage(img); m_sprite.SetTexture(m_texture); }

	void SetPosition(float x, float y) { m_sprite.SetPosition(x,y); }

	void Draw() { m_window->Draw(m_sprite); }
};

#endif