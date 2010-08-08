/*  FreeJ
 *  (c) Copyright 2009 Xant <xant@dyne.org>
 *
 * This source code is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Public License as published 
 * by the Free Software Foundation; either version 3 of the License,
 * or (at your option) any later version.
 *
 * This source code is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * Please refer to the GNU Public License for more details.
 *
 * You should have received a copy of the GNU Public License along with
 * this source code; if not, write to:
 * Free Software Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 *
 */

#ifndef __FACTORY_H__
#define __FACTORY_H__

/* Factory implementation
 * This draft implementation aims to provide factory functionalities
 * within Freej environment.
 * It can be used to provide instances of any class with no extraeffort to integrate the factory in your class.
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
 *  >  Factory<Layer>::new_instance("GeometryLayer", "MyImplementation");
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
 * If the purpose is to reuse instances (implementing, for instance, singletons) the get_instance() method 
 * can be used instead of new_instance().
 * for example: 
 *  > MyScreen *screenInstance = Factory<Screen>::get_instance("Screen"); 
 * 
 * will return always the same instance as long as only 'get_instance()' is called.
 * It will take care of creating a new instance when called the first time so 
 * mixing calls to new_instance() and get_instance() can lead to unexpected behaviours.
 *
 */
#include <map>      // std::map
#include <string>   // std::string
#include <stdio.h>
#include <jutils.h> // func() and error()

#define FACTORY_ID_MAXLEN 64

#define FACTORY_ALLOWED \
    static int isRegistered;

#define FACTORY_REGISTER_INSTANTIATOR(__base_class, __class_name, __category, __id) \
    static __class_name * get##__class_name() \
    { \
        func("Creating %s -- %s", #__base_class, #__class_name);\
        return new __class_name(); \
    } \
    int __class_name::isRegistered = Factory<__base_class>::register_instantiator( \
        #__category, #__id, (Instantiator)get##__class_name \
    );

typedef void *(*Instantiator)();
typedef std::map<std::string, Instantiator> FInstantiatorsMap;
typedef std::pair<std::string, Instantiator> FInstantiatorPair;
typedef std::pair<std::string, const char *> FTagPair;
typedef std::map<std::string, const char *> FTagMap;
typedef std::map<std::string, FInstantiatorsMap *> FMapsMap;
typedef std::pair<std::string, FInstantiatorsMap *> FMapPair;
typedef std::map<std::string, void *> FInstancesMap;
typedef std::pair<std::string, void *> FInstancePair;
typedef std::map<std::string, const char *> FDefaultClassesMap;

template <class T>
class Factory
{

  private:

    static FInstantiatorsMap *instantiators_map;
    static FDefaultClassesMap *defaults_map;
    static FInstancesMap *instances_map;

    static inline bool make_tag(const char *category, const char *id, char *out, unsigned int outlen)
    {
        if (strlen(category)+strlen(id)+3 > outlen) { // check the size of the requested id
            error("Factory::new_instance : requested ID (%s::%s) exceedes maximum size", category, id);
            return false;
        }
        snprintf(out, outlen, "%s::%s", category, id);
        return true;
    }
    
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
    static inline T *new_instance(const char *category)
    {
        FTagMap::iterator pair = defaults_map->find(category);
        if (pair != defaults_map->end())
            return new_instance(category, pair->second);
        return NULL;
    }
    
    // Create (and return) a new instance of the class which matches 'id' within a certain category.
    static inline T *new_instance(const char *category, const char *id)
    {
        char tag[FACTORY_ID_MAXLEN];
        if (!category || !id) // safety belts
            return NULL;

        func("(new_instance) Looking for %s::%s", category, id);
        if (!make_tag(category, id, tag, sizeof(tag)))
            return NULL;
        func("Looking for %s in instantiators_map (%d)", tag, instantiators_map->size());
        FInstantiatorsMap::iterator instantiators_pair = instantiators_map->find(tag);
        if (instantiators_pair != instantiators_map->end()) { // check if we have a match
            func("id %s found", id);
            Instantiator create_instance = instantiators_pair->second;
            if (create_instance)
                return (T*)create_instance();
        }
        return NULL;
    };

    static inline T*get_instance(const char *category)
    {
        FTagMap::iterator pair = defaults_map->find(category);
        if (pair != defaults_map->end())
            return get_instance(category, pair->second);
        return NULL;        
    }

    // TODO - remove duplicated code across get_instance() and new_instance
    //        (move common code in a private method used by both (get|new)_instance()
    static inline T *get_instance(const char *category, const char *id)
    {
        char tag[FACTORY_ID_MAXLEN];
        if (!category || !id) // safety belts
            return NULL;
        
        func("(get_instance) Looking for %s::%s", category, id);
        
        if (!make_tag(category, id, tag, sizeof(tag)))
            return NULL;
        
        if (instances_map) {
            func("Looking for %s in instantiators_map (%d)", tag, instances_map->size());
            FInstancesMap::iterator instance_pair = instances_map->find(tag);
            if (instance_pair != instances_map->end()) {
                void *instance = instance_pair->second;
                func("Returning instance of %s at address %p", tag, instance);
                return (T *)instance;
            }
        } else {
            instances_map = new FInstancesMap();
        }
        // create on first use
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
    static int register_instantiator(const char *category, const char *id, Instantiator func)
    {
        char tag[FACTORY_ID_MAXLEN];

        if (!make_tag(category, id, tag, sizeof(tag)))
            return 0;

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
            // check if we already have a default_map
            if (!defaults_map)
                defaults_map = new FDefaultClassesMap();

            FTagMap::iterator pair = defaults_map->find(category);
            if (pair != defaults_map->end())
                set_default_classtype(category, id);
            return 1;
        }
        return 0;
    };
};

template <class T> FInstantiatorsMap *Factory<T>::instantiators_map = NULL;
template <class T> FDefaultClassesMap *Factory<T>::defaults_map = NULL;
template <class T> FInstancesMap *Factory<T>::instances_map = NULL;

#endif
