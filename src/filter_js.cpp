/*  FreeJ - New Freior based Filter class
 *  (c) Copyright 2007 Denis Rojo <jaromil@dyne.org>
 *
 * This source code is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Public License as published 
 * by the Free Software Foundation; either version 2 of the License,
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

$Id: $

 */


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

  int idx;
  char *name;
  
  if(argc < 1) JS_ERROR("missing argument");

  JS_ARG_STRING(name,0);

  FilterDuo *duo = new FilterDuo();
  
  duo->proto = (Filter*) env->filters.search(name, &idx);

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

JS(filter_activate) {
  func("%u:%s:%s",__LINE__,__FILE__,__FUNCTION__);

  FilterDuo *duo = (FilterDuo *) JS_GetPrivate(cx, obj);
  if(!duo) {
    error("%u:%s:%s :: Filter core data is NULL",
	  __LINE__,__FILE__,__FUNCTION__);
    return JS_TRUE;
  }
  
  *rval = BOOLEAN_TO_JSVAL(duo->instance->active);
  if (argc == 1) {
    JS_ARG_NUMBER(var,0);
    duo->instance->active = (bool)var;
  }

  return JS_TRUE;
}


JS(filter_set_parameter) {
  func("%u:%s:%s",__LINE__,__FILE__,__FUNCTION__);
  int idx;
  char *name;
  Parameter *param;
  jsdouble val[3];

  if(argc < 2) JS_ERROR("missing arguments: name, values");


  FilterDuo *duo = (FilterDuo*)JS_GetPrivate(cx, obj);
  if(!duo) {
    error("%u:%s:%s :: Filter core data is NULL",
	  __LINE__,__FILE__,__FUNCTION__);
    return JS_FALSE;
  }
  
  if(JSVAL_IS_DOUBLE(argv[0])) {
    
    double *argidx = JSVAL_TO_DOUBLE(argv[0]);
    param = (Parameter*) duo->proto->parameters.pick((int)*argidx);
    
  } else { // get it by the param name
    
    JS_ARG_STRING(name,0);
    param = (Parameter*) duo->proto->parameters.search(name, &idx);
    
  }
  
  if(!param) { 
    error("parameter %s not found in filter %s", name, duo->proto->name);
    return JS_TRUE;
  }
  
  switch(param->type) {
    
  case Parameter::BOOL:
  case Parameter::NUMBER:
    {
      if(!JS_ValueToNumber(cx, argv[1], &val[0])) {
	error("set parameter called with an invalid value for filter %s",
	      duo->proto->name);
	return JS_TRUE;
      }
      func("javascript %s->%s to [%.5f]",
	   duo->proto->name, param->name, val[0]);
      //  duo->proto->set_parameter_value( duo->instance, &val, it->second );
      
      param->set(&val);
      duo->instance->set_parameter(idx);
      break; 
    }
  case Parameter::POSITION:
    if(!JS_ValueToNumber(cx, argv[1], &val[0])) {
      error("set parameter called with an invalid value for filter %s",
	    duo->proto->name);
      return JS_TRUE;
    }
    if(!JS_ValueToNumber(cx, argv[2], &val[1])) {
      error("set parameter called with an invalid value for filter %s",
	    duo->proto->name);
      return JS_TRUE;
    }
    func("javascript %s->%s to x[%.1f] y[%.1f]",
	 duo->proto->name, param->name, val[0], val[1]);
    //  duo->proto->set_parameter_value( duo->instance, &val, it->second );
    
    param->set(&val[0]);
    duo->instance->set_parameter(idx);
    break; 
    
  default:
    error("parameter of unknown type: %s->%s", duo->proto->name, param->name);
    break;
  }
  
  return JS_TRUE;

}

JS(filter_list_parameters) {
  func("%u:%s:%s",__LINE__,__FILE__,__FUNCTION__);
  JSObject *arr;
  JSString *str;
  jsval val;

  arr = JS_NewArrayObject(cx, 0, NULL); //create void array
  if(!arr) return JS_FALSE;

  FilterDuo *duo = (FilterDuo*)JS_GetPrivate(cx, obj);
  if(!duo) {
    error("%u:%s:%s :: Layer core data is NULL",
	  __LINE__,__FILE__,__FUNCTION__);
    return JS_FALSE;
  }

  Parameter *parm = (Parameter*)duo->proto->parameters.begin();
  int c = 0;
  while(parm) {
    str = JS_NewStringCopyZ(cx, parm->name);
    val = STRING_TO_JSVAL(str);
    JS_SetElement(cx, arr, c, &val);
    c++;
    parm = (Parameter*)parm->next;
  }

  *rval = OBJECT_TO_JSVAL( arr );
  return JS_TRUE;
}
  

////////////////////////////////////
/// TODO:
// see filter.h and filter.cpp
// methods to be implemented:
// set/get parameter value
// list parameters (return an array)
// description and info
// activate and deactivate filter
