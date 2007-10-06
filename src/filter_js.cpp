

#include <callbacks_js.h>
#include <jsparser_data.h>
#include <filter.h>


DECLARE_CLASS("Filter",filter_class,filter_constructor);

JSFunctionSpec filter_methods[] = {
  FILTER_METHODS  ,
  ENTRY_METHODS   ,
  {0}
};

JS(filter_constructor) {
  func("%u:%s:%s",__LINE__,__FILE__,__FUNCTION__);

  char *name;
  
  if(argc < 1) JS_ERROR("missing argument");

  JS_ARG_STRING(name,0);

  FilterDuo *duo = new FilterDuo();
  
  duo->proto = (Filter*)env->filters.search(name);
  if(!duo->proto) {
    error("filter not found: %s",name);
    delete duo;
    *rval = JSVAL_FALSE;
    return JS_TRUE;
  }

  if(!JS_SetPrivate(cx, obj, (void*)duo))
    JS_ERROR("internal error setting private value");
  
  *rval = OBJECT_TO_JSVAL(obj);
  return JS_TRUE;
}



JS(filter_set_parameter) {
  func("%u:%s:%s",__LINE__,__FILE__,__FUNCTION__);

  char *name;
  jsdouble val;
  
  if(argc < 2) JS_ERROR("missing arguments: name, value");

  JS_ARG_STRING(name,0);
  js_ValueToNumber(cx, argv[1], &val);

  FilterDuo *duo = (FilterDuo*)JS_GetPrivate(cx, obj);
  if(!duo) {
    error("%u:%s:%s :: Filter core data is NULL",
	  __LINE__,__FILE__,__FUNCTION__);
    return JS_FALSE;
  }
  
  // get it by the param name
  map<string, int>::iterator it = duo->proto->parameters.find(name);
  if(it == duo->proto->parameters.end()) { // not found
    error("parameter %s not found in filter %s", name, duo->proto->name);
    return JS_TRUE;
  }

  act("set parameter %s->%s to %.2f", duo->proto->name, name, val);
  duo->proto->set_parameter_value( duo->instance, &val, it->second );
  return JS_TRUE;
}
//////
/// TODO:
/// see filter.h and filter.cpp
// methods to be implemented:
// set/get parameter value
// list parameters (return an array)
// description and info
// activate and deactivate filter
