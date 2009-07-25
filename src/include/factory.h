#ifndef __FACTORY_H__
#define __FACTORY_H__

#include <layer.h>
#include <map> // for std::map
#include <string> // for std::string

#define FACTORY_ALLOWED \
    static int isRegistered;

#define __QUOTE__(__s) #__s
#define __CONCAT__(__a, __b) __a##__b

#define FACTORY_REGISTER_INSTANTIATOR(__name, __category, __tag) \
    void * get##__name() \
    { \
        return (void *)new __name(); \
    } \
    int __name::isRegistered = Context::register_layer_instantiator( \
        __QUOTE__( \
            __CONCAT__( \
                __CONCAT__(__category, ::), \
                __tag \
            ) \
        ), get##__name \
    );

typedef void *(*Instantiator)();
typedef std::map<std::string, Instantiator> InstantiatorsMap;
typedef std::pair<std::string, Instantiator> TInstantiatorPair;

template <class T>
class Factory 
{
  private:
    static InstantiatorsMap instantiators_map;
  public:
    static T *new_instance(const char *category, const char *tag)
    {
        char id[1024];
        snprintf(id, sizeof(id), "%s::%s", category, tag);
        Instantiator create_instance = instantiators_map.find(id)->second;
        return (T *)create_instance();
    };
    static int register_instantiator(const char *tag, Instantiator func)
    {
        instantiators_map.insert(TInstantiatorPair(tag, func));
        return 1;
    };
};

template <class T> InstantiatorsMap Factory<T>::instantiators_map;

#endif
