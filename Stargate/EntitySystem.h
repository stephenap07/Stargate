#pragma once

#include "map"
#include "vector"
#include <assert.h>
#include <boost/function.hpp>

typedef int FamilyId;
struct EntitySystem;

struct Entity;

struct Component {
	boost::function<void(Entity* ent, sf::Time elapsed)> func; 
};

struct Entity {
   static EntitySystem *entitySystem;
   Entity();
   
   ~Entity() {
	   for( auto iter = mComponents.begin(); iter != mComponents.end(); ++iter)
	   {
		   delete iter->second; 
	   }
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
      //Scan Entity Components and get familyIds, then I know what to delete in the ComponentStore
      delete e;
   }
protected:
   std::multimap<FamilyId, Entity*> mComponentStore;
};

Entity::Entity() {
}

template<typename Type> Type *Entity::getAs() {
   return entitySystem->getComponent<Type>(this);
}
