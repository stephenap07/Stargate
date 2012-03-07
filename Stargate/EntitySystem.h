#pragma once

#include "map"
#include "vector"
#include <assert.h>
#include <boost/function.hpp>

typedef int FamilyId;
typedef int EventId;

struct Component {
};

struct EntitySystem; 

struct Entity {
   static EntitySystem *entitySystem;
   Entity();
   
   ~Entity() {
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
	   if(e->mComponents.find(T::familyId) != e->mComponents.end())
		return (T*)e->mComponents[T::familyId];
	  else return 0;
   }
   template<typename T> std::vector<Entity*> getEntities() {
      if(mComponentStore.find(T::familyId) != mComponentStore.end())
		  return mComponentStore[T::familyId]; 
	  else {
		  EntityVector a; 
		  return a;
	  }
   }
   template<typename T> void addComponent(Entity *e, T* comp) {
      mComponentStore[T::familyId].push_back(e);
      e->mComponents.insert(std::pair<FamilyId, Component*>(T::familyId, comp));
   }

   void deleteEntity(Entity *e) {
	   for(auto it = e->mComponents.begin(); it != e->mComponents.end(); ++it)
	   {
		   for(auto t = mComponentStore[it->first].begin(); t != mComponentStore[it->first].end(); ++t)
		   {
			   if ( (*t) == e) {
				   delete e->mComponents[it->first];
				   mComponentStore[it->first].erase(t); 
			   }
		   }
	   }
   }

   void deleteAllEntities() {

   }
protected:
	typedef std::vector<Entity*> EntityVector; 
	std::map<FamilyId, EntityVector> mComponentStore;
};

Entity::Entity() {
}

template<typename Type> Type *Entity::getAs() {
   return entitySystem->getComponent<Type>(this);
}
