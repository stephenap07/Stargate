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
	  if(e->mComponents[T::familyId])
		return (T*)e->mComponents[T::familyId];
	  else return 0;
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

		for(auto iter = mComponentStore.begin(); iter != mComponentStore.end();)
		{
			auto it = iter++; 

			if(iter == mComponentStore.end()) return;

			//find components in entity and delete them
			if(it->second == e)
			{
				Component* c = e->mComponents[it->first];

				auto iterPair = e->mComponents.find(it->first); 

				if(iterPair == e->mComponents.end())
				{
					std::cout << "no find\n";
					break;
				}
				else e->mComponents.erase(iterPair);
				
				delete c; 
			}

			mComponentStore.erase(it);
		}

		delete e; 
   }

   void deleteAllEntities() {
	   for(auto it = mComponentStore.begin(); it != mComponentStore.end();)
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
