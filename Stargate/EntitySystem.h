#pragma once

#include "map"
#include "vector"
#include <assert.h>
#include <boost/function.hpp>

typedef int FamilyId;
typedef int EventId;

struct EntitySystem;

struct Entity;

struct Event_t
{

};

struct Listener_t
{

};

typedef std::map<EventId, Event_t*> EventMapType; 
typedef boost::function<const EventMapType & (Component*, float elapsed)> behaviorFunction;

struct ComponentSystem;

struct Component {
	static ComponentSystem *componentSystem;

	template<typename T> 
	const std::vector<behaviorFunction> & getAs()
	{
		return componentSystem->getBehaviorRange<T>(this); 
	}

	std::multimap< EventId, behaviorFunction > behaviors; 
};

ComponentSystem *Component::componentSystem = 0;

struct ComponentSystem
{
	ComponentSystem() {
		if(!Component::componentSystem) 
			Component::componentSystem = this;
	}
	
	template<typename eventType> 
	void AddBehavior(Component *comp, behaviorFunction func) {
		comp->behaviors.insert(std::pair<eventType::familyId, behaviorFunction>(eventId, func) );
	}

	template<typename eventType> 
	const std::vector<behaviorFunction> & getBehaviorRange(Component* comp)
	{
		std::vector<behaviorFunction> funcs; 

		auto iter = comp->behaviors.equal_range(T::familyId);
		for(auto it = iter.first; it != iter.second; ++it)
			funcs.push_back(it->second);

		return funcs; 
	}

};

struct Entity {
   static EntitySystem *entitySystem;
   Entity();
   
   ~Entity() {
	   std::cout << "entity destroyed\n";
   }

   template<typename Type> Type *getAs();
   std::map<FamilyId, Component*> mComponents;
};

EntitySystem *Entity::entitySystem = 0;

struct EntitySystem {
   EntitySystem() {
      Entity::entitySystem = this;
   }
   template<typename T> T *getComponent(Entity *e) {
	  assert(e->mComponents[T::familyId]);
      return (T*)e->mComponents[T::familyId];
   }
   template<typename T> void getEntities(std::vector<Entity*> &result) {
      auto iterPair = mComponentStore.equal_range(T::familyId);
      for(auto iter = iterPair.first; iter != iterPair.second; ++iter) {
         result.push_back(iter->second);
      }
   }
   template<typename T> void addComponent(Entity *e, T* comp) {
      mComponentStore.insert(std::pair<FamilyId, Entity*>(T::familyId, e));
      e->mComponents.insert(std::pair<FamilyId, Component*>(T::familyId, comp));
   }

   void deleteEntity(Entity *e) {
	   /*
	   for(auto it = e->mComponents.begin(); it != e->mComponents.end(); ++it)
	   {
		   auto iterPair = mComponentStore.equal_range(it->first);
		   iterPair
		   delete it->second; 
	   }
	   e->mComponents.erase(e->mComponents.begin(), e->mComponents.end());
	   delete e;
	   */
   }

   void deleteAllEntities() {
	   for(auto it = mComponentStore.begin(); it != mComponentStore.end(); ++it)
	   {
		   Entity *ent = it->second;
		   mComponentStore.erase(it);
		   deleteEntity(ent);
	   }
   }
protected:
   std::multimap<FamilyId, Entity*> mComponentStore;
};

Entity::Entity() {
}

template<typename Type> Type *Entity::getAs() {
   return entitySystem->getComponent<Type>(this);
}
