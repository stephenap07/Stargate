#include "g_local.h"



/*======================
  Player Implementation
======================*/

Player::Player() :m_currentFrame(0), m_lockJump(false), m_currentAnim(ANIM_RIGHT), m_nextAnim(0.2f), jumpHeight(64), m_jumpPosition(0), m_jumpKey(false)
			,Entity() 
{
		//set the image
		sprite.SetImage(g_ImageMan.Load("data/abe.png"));

		//set up animation frames
		m_frames[ANIM_LEFT][0] = frame_t(13, 8, PLAYER_WIDTH, PLAYER_HEIGHT); 
		m_frames[ANIM_LEFT][1] = frame_t(55, 8, PLAYER_WIDTH, PLAYER_HEIGHT);

		m_frames[ANIM_RIGHT][0] = frame_t(95, 8, PLAYER_WIDTH, PLAYER_HEIGHT);
		m_frames[ANIM_RIGHT][1] = frame_t(134, 8, PLAYER_WIDTH, PLAYER_HEIGHT);

		//set the current animations and current frame
		sprite.SetSubRect(m_frames[m_currentAnim][m_currentFrame]); 

		//set up sounds
		m_soundJump.SetBuffer(g_SoundMan.Load("data/sounds/jump.wav"));
		m_soundPickUp.SetBuffer(g_SoundMan.Load("data/sounds/pickup.wav"));
		m_soundShoot.SetBuffer(g_SoundMan.Load("data/sounds/shoot.wav"));
}
		

void Player::Think(float dt, const sf::Input & input)
{
	if(input.IsKeyDown(sf::Key::Right)) {
			physData.velX = 220;
		} 
		else if(input.IsKeyDown(sf::Key::Left)) {
			physData.velX = -220;
		}else physData.velX = 0;

		if( input.IsKeyDown( sf::Key::Z ) && m_lockJump == false && physData.grounded ) {
			m_lockJump = true;
			m_soundJump.SetPitch(sf::Randomizer::Random(0.75f, 1.0f));
			m_soundJump.Play();
		}

		static float laserTime = 0.0f;
		laserTime += dt;

		if(m_lockJump) { //if we pressed the jump button
			Jump();
			m_lockJump = false;
		}

		UpdateAnimations(dt);
}

void Player::Draw( const sf::RenderWindow & wnd )
{
	wnd.Draw( sprite );
}

void Player::UpdateAnimations( float dt )
{
	static float timer = 0.0f; 
	timer += dt;

	if( physData.velX > 0 ) {
		m_currentAnim = ANIM_RIGHT;
	}
	else if( physData.velX < 0 ) {
		m_currentAnim = ANIM_LEFT;
	}

	sprite.SetSubRect( m_frames[m_currentAnim][m_currentFrame] );

	if( timer < m_nextAnim ) { return; }
	else timer = 0.0f;

	if( physData.velX != 0 ) {
		m_currentFrame++;
		if(m_currentFrame > 1) m_currentFrame = 0;
	} 
	else if( m_currentAnim == ANIM_RIGHT ) m_currentFrame = 0;
		else m_currentFrame  = 1; 
}

/*================================
			Items
================================*/
ItemBacon::ItemBacon() :m_limit(0.5f), m_impulse(m_limit), m_rate(2.0f), m_change(false), Entity()
{
	sprite.SetImage(g_ImageMan.Load("data/bacon.png"));
}

void ItemBacon::Think(float dt, const sf::Input & input)
{
	sprite.Move( 0, m_impulse);

	if(m_impulse >= m_limit) {
		m_change = false;
	}
	else if(m_impulse <= -m_limit) {
		m_change = true;
	}
		
	if(!m_change) {
		m_impulse -= m_rate*dt;
	}
	else if(m_change) {
		m_impulse += m_rate*dt;
	}
}

void ItemBacon::Draw(sf::RenderWindow &wnd)
{
	wnd.Draw(sprite);
}

/*================================
		Entity Factory 
================================*/

Entity* EntityFactory::Spawn( int entCode )
{
	Entity * ent;

	switch( entCode )
	{
	case PLAYER : 
		ent = new Player();
		m_entities.push_back(ent);
		break;
	}
}

/*================================
	Level Implementation
================================*/

bool Level::Init()
{

}

void Level::Run()
{

}



/*======================
	Resource 
	Implementation
======================*/

sf::Image & ImageMan::Load( const char * file) 
{
	std::map< const char*, sf::Image >::iterator it;

	it = m_resources.find( file );

	if( it == m_resources.end() )
	{
		sf::Image temp;
		temp.LoadFromFile( file );
		m_resources[ file ] = temp;

		return m_resources[ file ];
	}

	else{ 
		return it->second;
		}
}

sf::SoundBuffer & SoundMan::Load( const char* file )
{
	std::map< const char*, sf::SoundBuffer >::iterator it;

	it = m_resources.find( file );

	if( it == m_resources.end() )
	{
		sf::SoundBuffer temp;
		temp.LoadFromFile( file );
		m_resources[ file ] = temp;

		return m_resources[ file ];
	}
	else return it-> second;
}

void InitGlobals()
{
	ImageMan::Create();
	SoundMan::Create();
}