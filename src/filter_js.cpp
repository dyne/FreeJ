

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
  
  if(argc < 2) JS_ERROR("missing arguments: name, values");


  FilterDuo *duo = (FilterDuo*)JS_GetPrivate(cx, obj);
  if(!duo) {
    error("%u:%s:%s :: Filter core data is NULL",
	  __LINE__,__FILE__,__FUNCTION__);
    return JS_FALSE;
  }

  JS_ARG_STRING(name,0);

  // get it by the param name
  Parameter *param = (Parameter*)duo->proto->parameters.search(name);
  if(!param) { 
    error("parameter %s not found in filter %s", param->name, duo->proto->name);
    return JS_TRUE;
  }

  switch(param->type) {

  case PARAM_BOOL: {
    uint16_t val;
    if(!js_ValueToUint16(cx, argv[1], &val)) {
      error("set parameter called with an invalid bool value for filter %s",
	    duo->proto->name);
      return JS_TRUE;
    }
    act("%s->%s (bool) to %s",
	duo->proto->name, param->name, (val)?"true":"false");
  //  duo->proto->set_parameter_value( duo->instance, &val, it->second );
    param->set_value((void*)&val);
    break; }
    
  case PARAM_NUMBER: {
    jsdouble val;
    if(!js_ValueToNumber(cx, argv[1], &val)) {
      error("set parameter called with an invalid numeric value for filter %s",
	    duo->proto->name);
      return JS_TRUE;
    }
    act("%s->%s (number) to %.2f", duo->proto->name, param->name, val);
    param->set_value((void*)&val);
    break; }

  default:
    error("parameter of unknown type: %s->%s", duo->proto->name, param->name);
    break;
  }

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
