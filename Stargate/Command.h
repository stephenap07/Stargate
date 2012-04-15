#ifndef COMMAND_H_
#define COMMAND_H_

#include <map>
#include <boost/shared_ptr.hpp>
#include "Entity.h"

struct Key 
{
	bool Left; 
	bool Right;
	bool Down; 
	bool Up;
};

typedef boost::shared_ptr<Entity> pEntity; 

class Command
{
private: 
	pEntity m_pEntity; 
public:
	Key key; 
};

class CommandSystem
{
private:
	std::map<float, Command> m_commands; 

	void AddCommand(float currentTime, pEntity ent, Command); 
};

#endif