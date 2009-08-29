#ifndef __FACTORY_H__
#define __FACTORY_H__

#include <map> // for std::map
#include <string> // for std::string
#include <jutils.h>
using std::make_pair;

#define FACORY_ID_MAXLEN 64

#define FACTORY_ALLOWED \
    static int isRegistered;

#define FACTORY_MAKE_TAG(__category, __id) #__category "::" #__id

#define FACTORY_REGISTER_INSTANTIATOR(__base_class, __class_name, __category, __id) \
    static __class_name * get##__class_name() \
    { \
        func("Creating %s -- %s\n", #__base_class, #__class_name);\
        return new __class_name(); \
    } \
    int __class_name::isRegistered = Factory<__base_class>::register_instantiator( \
        FACTORY_MAKE_TAG(__category, __id), (Instantiator)get##__class_name \
    );

typedef void *(*Instantiator)();
#define FInstantiatorsMap std::map<std::string, Instantiator>
#define FInstantiatorPair std::pair<std::string, Instantiator>
#define FTagPair std::pair<std::string, const char *>
#define FTagMap std::map<std::string, const char *>
#define FMapsMap std::map<std::string, FInstantiatorsMap *>
#define FDefaultClassesMap std::map<std::string, const char *> 
#define FMapPair std::pair<std::string, FInstantiatorsMap *> 

template <class T>
class Factory 
{
  private:
      static FInstantiatorsMap *instantiators_map;
      static FDefaultClassesMap *classes_map;
  public:
    
    static int set_default_classtype(const char *classname, const char *id)
    {
        if (!classes_map)
            classes_map = new FDefaultClassesMap();
        FTagMap::iterator pair = classes_map->find(classname);
        if (pair != classes_map->end()) // remove old default (if any)
            classes_map->erase(pair);
        // set new default
        classes_map->insert(FTagPair(classname, id));
        return 1;
    }

    static T *new_instance(const char *classname)
    {
        FTagMap::iterator pair = classes_map->find(classname);
        if (pair != classes_map->end())
            return new_instance(classname, pair->second);
        return NULL;
    }

    static T *new_instance(const char *classname, const char *id)
    {
        char tag[FACORY_ID_MAXLEN];
        if (!classname || !id) // safety belts
            return NULL;

        func("Looking for %s::%s \n", classname, id);

        if (strlen(classname)+strlen(id)+3 > sizeof(tag)) { // check the size of the requested id
            error("Factory::new_instance : requested ID (%s::%s) exceedes maximum size", classname, id);
            return NULL;
        }
        snprintf(tag, sizeof(tag), "%s::%s", classname, id);
        func("Looking for %s in instantiators_map (%d)\n", tag, instantiators_map->size());
        FInstantiatorsMap::iterator instantiators_pair = instantiators_map->find(tag);
        if (instantiators_pair != instantiators_map->end()) { // check if we have 
            func("id %s found\n", id);
            Instantiator create_instance = instantiators_pair->second;
            if (create_instance) 
                return (T*)create_instance();
        }
        return NULL;
    };
    static int register_instantiator(const char *tag, Instantiator func)
    {
        if (!instantiators_map) {
            instantiators_map = new FInstantiatorsMap();
        }
        if (instantiators_map) {
            FInstantiatorsMap::iterator instantiators_pair = instantiators_map->find(tag);
            if (instantiators_pair != instantiators_map->end()) {
                error("Can't register new class. Tag '%s' already exists!", tag);
                return 0;
            }
            instantiators_map->insert(FInstantiatorPair(tag, func));
            return 1;
        }
        return 0;
    };
};

template <class T> FInstantiatorsMap *Factory<T>::instantiators_map = NULL;
template <class T> FDefaultClassesMap *Factory<T>::classes_map = NULL;

#endif
