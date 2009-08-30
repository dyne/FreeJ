#ifndef __FACTORY_H__
#define __FACTORY_H__

/* Factory implementation
 * This draft implementation aims to provide factory functionalities
 * within Freej environment.
 * It can be used to provide instances of any class, requiring a very little
 * effort to integrate the factory in your class.
 * It requires just two statements : one in the class definition and one in the implementation file.
 * FACTORY_ALLOWED must be put at the end of your class definition. For instance:
 *
 *  >   class MyClass {
 *  >      public:
 *  >        ...
 *  >      private:
 *  >        ...
 *  >
 *  >        FACTORY_ALLOWED
 *  >   };
 *
 * Then the macro FACTORY_REGISTER_INSTANTIATOR() must be put somewhere in the class implementation.
 *
 * For instance, if we want to create a new GeometryLayer implementation called MyClass,
 * we would probably put somewhere in myclass.cpp :
 *
 *  >  FACTORY_REGISTER_INSTANTIATOR(Layer, MyClass, GeometryLayer, MyImplementation);
 *
 * the tag (<category>::<id>) associated to MyClass will be GeometryLayer::MyImplementation
 * and new instances can be obtained using:
 *
 *  >  Factory<Layer>::new_instance('GeometryLayer', 'MyImplementation);
 *
 * You can also set 'defaults' anywhere in the code. For example you could put somewhere
 * in the initialization logic (or even at runtime)
 *
 *  >  Factory<Layer>::set_default("GeometryLayer", "MyImplementation");
 *
 * this will define 'MyImplementation' as default id to be used when querying for 'GeometryLayer'
 * instances without specifying any id.
 * for example:
 *  >  MyClass *newInstance = Factory<Layer>::new_instance('GeometryLayer');
 *
 * This will allow javascripts to call "var geo_layer = new GeometryLayer()" 
 * obtaining an instance of a different subclass depending on the platform and setup.
 *
 */
#include <map> // for std::map
#include <string> // for std::string
#include <jutils.h>

#define FACTORY_ID_MAXLEN 64

#define FACTORY_ALLOWED \
    static int isRegistered;

// facility to easily create a factory-tag starting from the category name 
// and the id ( for instance: FACTORY_MAKE_TAG(VideoLayer, ffmpeg) )
#define FACTORY_MAKE_TAG(__category, __id) #__category "::" #__id

#define FACTORY_REGISTER_INSTANTIATOR(__base_class, __class_name, __category, __id) \
    static __class_name * get##__class_name() \
    { \
        func("Creating %s -- %s", #__base_class, #__class_name);\
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
#define FInstancesMap std::map<std::string, void *>
#define FInstancePair std::pair<std::string, void *>

template <class T>
class Factory
{
  private:
      static FInstantiatorsMap *instantiators_map;
      static FDefaultClassesMap *defaults_map;
      static FInstancesMap *instances_map;
  public:
    
    // Define a default class for a certain category
    static int set_default_classtype(const char *category, const char *id)
    {
        if (!defaults_map) // create on first use
            defaults_map = new FDefaultClassesMap();
        FTagMap::iterator pair = defaults_map->find(category);
        if (pair != defaults_map->end()) // remove old default (if any)
            defaults_map->erase(pair);
        // set new default
        defaults_map->insert(FTagPair(category, id));
        return 1;
    }

    // allow to ask for an instance of a certain category
    // regardless of the real class of the returned object.
    // The configured default for the current platform (if defined)
    // will be returned.
    static T *new_instance(const char *category)
    {
        FTagMap::iterator pair = defaults_map->find(category);
        if (pair != defaults_map->end())
            return new_instance(category, pair->second);
        return NULL;
    }
    
    // Create (and return) a new instance of the class which matches 'id' within a certain category.
    static T *new_instance(const char *category, const char *id)
    {
        char tag[FACTORY_ID_MAXLEN];
        if (!category || !id) // safety belts
            return NULL;

        func("(new_instance) Looking for %s::%s \n", category, id);

        if (strlen(category)+strlen(id)+3 > sizeof(tag)) { // check the size of the requested id
            error("Factory::new_instance : requested ID (%s::%s) exceedes maximum size", category, id);
            return NULL;
        }
        snprintf(tag, sizeof(tag), "%s::%s", category, id);
        func("Looking for %s in instantiators_map (%d)\n", tag, instantiators_map->size());
        FInstantiatorsMap::iterator instantiators_pair = instantiators_map->find(tag);
        if (instantiators_pair != instantiators_map->end()) { // check if we have a match
            func("id %s found\n", id);
            Instantiator create_instance = instantiators_pair->second;
            if (create_instance)
                return (T*)create_instance();
        }
        return NULL;
    };

    static T*get_instance(const char *category)
    {
        FTagMap::iterator pair = defaults_map->find(category);
        if (pair != defaults_map->end())
            return get_instance(category, pair->second);
        return NULL;        
    }
    
    static T *get_instance(const char *category, const char *id)
    {
        char tag[FACTORY_ID_MAXLEN];
        if (!category || !id) // safety belts
            return NULL;
        
        func("(get_instance) Looking for %s::%s \n", category, id);
        
        if (strlen(category)+strlen(id)+3 > sizeof(tag)) { // check the size of the requested id
            error("Factory::new_instance : requested ID (%s::%s) exceedes maximum size", category, id);
            return NULL;
        }
        snprintf(tag, sizeof(tag), "%s::%s", category, id);
        if (instances_map) {
            func("Looking for %s in instantiators_map (%d)\n", tag, instances_map->size());
            FInstancesMap::iterator instance_pair = instances_map->find(tag);
            if (instance_pair != instances_map->end()) {
                void *instance = instance_pair->second;
                func("Returning instance of %s at address %p", tag, instance);
                return (T *)instance;
            }
        } else {
            instances_map = new FInstancesMap();
        }
        T *instance = new_instance(category, id);
        instances_map->insert(FInstancePair(tag, (void *)instance));
        func("Created instance of %s at address %p", tag, (void *)instance);
        return instance;
    }

    // register a new class instantiator
    // tag is : <category>::<id>
    // where 'category' could be for instance "VideoLayer"
    //          (when using a Factory<Layer>)
    //       'id' could be any of 'ffmpeg' or 'qt' or 'opencv' and so on
    //          (referencing a specific VideoLayer implementation)
    static int register_instantiator(const char *tag, Instantiator func)
    {
        if (!instantiators_map) { // create on first use
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
template <class T> FDefaultClassesMap *Factory<T>::defaults_map = NULL;
template <class T> FInstancesMap *Factory<T>::instances_map = NULL;


#endif
