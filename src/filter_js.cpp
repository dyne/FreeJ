/*  FreeJ - New Freior based Filter class
 *
 *  Copyright (C) 2007-2010 Denis Roio <jaromil@dyne.org>
 *  Copyright (C) 2010    Andrea Guzzo <xant@xant.net>
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

$Id: $

 */


#include <callbacks_js.h>
#include <jsparser_data.h>
#include <filter.h>


DECLARE_CLASS("Filter",filter_class,filter_constructor);

JSFunctionSpec filter_methods[] = {
  {"set_parameter",           filter_set_parameter,             4},
  {"activate",                filter_activate,                  1},
  ENTRY_METHODS   ,
  {0}
};

JSPropertySpec filter_properties[] = {
  // ro
  { "parameters", 0,  JSPROP_ENUMERATE | JSPROP_PERMANENT | JSPROP_READONLY, filter_list_parameters, NULL },
  { "description", 0,  JSPROP_ENUMERATE | JSPROP_PERMANENT | JSPROP_READONLY, filter_get_description, NULL },
  { "author", 0,  JSPROP_ENUMERATE | JSPROP_PERMANENT | JSPROP_READONLY, filter_get_author, NULL },
  {0}
};

JS(filter_constructor) {
  func("%u:%s:%s",__LINE__,__FILE__,__FUNCTION__);

  int idx;
  char *name;
  FilterInstance *filter_instance = NULL;
  if(argc < 1)
      JS_ERROR("missing argument");

  //JS_SetContextThread(cx);
  JS_BeginRequest(cx);
  name = js_get_string(argv[0]);

  Filter *filter = (Filter*) global_environment->filters.search(name, &idx);
  
  if(!filter) {
    error("filter not found: %s",name);
    *rval = JSVAL_FALSE;
    return JS_TRUE;
  } else {
    filter_instance = filter->new_instance();
    // fill with class description
    filter_instance->jsclass = &filter_class;
  }

  if(!JS_SetPrivate(cx, obj, (void*)filter_instance))
    JS_ERROR("internal error setting private value");
  else
    filter_instance->jsobj = obj;

  *rval = OBJECT_TO_JSVAL(obj);
  JS_EndRequest(cx);
  //JS_ClearContextThread(cx);
  return JS_TRUE;
}

JS(filter_activate) {
  func("%u:%s:%s",__LINE__,__FILE__,__FUNCTION__);

  //JS_SetContextThread(cx);
  JS_BeginRequest(cx);
  FilterInstance *filter_instance = (FilterInstance *) JS_GetPrivate(cx, obj);
  if(!filter_instance) {
    error("%u:%s:%s :: Filter core data is NULL",
	  __LINE__,__FILE__,__FUNCTION__);
    JS_EndRequest(cx);
    //JS_ClearContextThread(cx);
    return JS_FALSE;
  }
  *rval = BOOLEAN_TO_JSVAL(filter_instance->active);
  if (argc == 1) {
    jsint var = js_get_int(argv[0]);
    filter_instance->active = (bool)var;
  }
  JS_EndRequest(cx);
  //JS_ClearContextThread(cx);
  return JS_TRUE;
}


JS(filter_set_parameter) {
  func("%u:%s:%s",__LINE__,__FILE__,__FUNCTION__);
  int idx;
  char *name;
  Parameter *param;
  jsdouble val[3];

  //JS_SetContextThread(cx);
  JS_BeginRequest(cx);
  
  if(argc < 2)
    JS_ERROR("missing arguments: name, values");


  FilterInstance *filter_instance = (FilterInstance*)JS_GetPrivate(cx, obj);
  if(!filter_instance) {
    error("%u:%s:%s :: Filter core data is NULL",
	  __LINE__,__FILE__,__FUNCTION__);
    return JS_FALSE;
  }
  
  if(JSVAL_IS_DOUBLE(argv[0])) {
    
    double *argidx = JSVAL_TO_DOUBLE(argv[0]);
    param = (Parameter*) filter_instance->proto->parameters.pick((int)*argidx);
    
  } else { // get it by the param name
    
    name = js_get_string(argv[0]);
    param = (Parameter*) filter_instance->proto->parameters.search(name, &idx);
    
  }
  
  if(!param) { 
    JS_EndRequest(cx);
    //JS_ClearContextThread(cx);
    error("parameter %s not found in filter %s", name, filter_instance->proto->name);
    return JS_TRUE;
  }
  
  switch(param->type) {
    
  case Parameter::BOOL:
  case Parameter::NUMBER:
    {
      if(!JS_ValueToNumber(cx, argv[1], &val[0])) {
        error("set parameter called with an invalid value for filter %s",
	      filter_instance->proto->name);
        break;
      }
      func("javascript %s->%s to [%.5f]",
	   filter_instance->proto->name, param->name, val[0]);
      //  filter_instance->proto->set_parameter_value( filter_instance->instance, &val, it->second );
      
      param->set(&val);
      filter_instance->set_parameter(idx);
      break; 
    }
  case Parameter::POSITION:
    if(!JS_ValueToNumber(cx, argv[1], &val[0])) {
      error("set parameter called with an invalid value for filter %s",
	    filter_instance->proto->name);
      break;
    }
    if(!JS_ValueToNumber(cx, argv[2], &val[1])) {
      error("set parameter called with an invalid value for filter %s",
	    filter_instance->proto->name);
      break;
    }
    func("javascript %s->%s to x[%.1f] y[%.1f]",
	 filter_instance->proto->name, param->name, val[0], val[1]);
    //  filter_instance->proto->set_parameter_value( filter_instance->instance, &val, it->second );
    
    param->set(&val[0]);
    filter_instance->set_parameter(idx);
    break; 
    
  default:
    error("parameter of unknown type: %s->%s", filter_instance->proto->name, param->name);
    break;
  }
  JS_EndRequest(cx);
  //JS_ClearContextThread(cx);
  return JS_TRUE;

}

JSP(filter_list_parameters) {
  func("%u:%s:%s",__LINE__,__FILE__,__FUNCTION__);
  JSObject *arr, *otmp;
  JSString *str;
  jsval val;

  //JS_SetContextThread(cx);
  JS_BeginRequest(cx);
  arr = JS_NewArrayObject(cx, 0, NULL); //create void array
  if(!arr) {
    JS_EndRequest(cx);
    //JS_ClearContextThread(cx);  
    return JS_FALSE;
  }

  FilterInstance *filter_instance = (FilterInstance*)JS_GetPrivate(cx, obj);
  if(!filter_instance) {
    error("%u:%s:%s :: Layer core data is NULL",
	  __LINE__,__FILE__,__FUNCTION__);
    JS_EndRequest(cx);
    //JS_ClearContextThread(cx);
    return JS_FALSE;
  }

  // take the prototype (descriptive) and create an array
  Parameter *parm = (Parameter *)filter_instance->proto->parameters.begin();
  int c = 0;
  while(parm) {
    otmp = JS_NewObject(cx, &parameter_class, NULL, obj);
    JS_SetPrivate(cx,otmp, (void*)parm);
    parm->jsclass = &parameter_class;
    parm->jsobj = otmp;
    val = OBJECT_TO_JSVAL(otmp);
    JS_SetElement(cx, arr, c, &val);
    c++;
    parm = (Parameter*)parm->next;
  }

  *vp = OBJECT_TO_JSVAL( arr );
  JS_EndRequest(cx);
  //JS_ClearContextThread(cx);
  return JS_TRUE;
}

JSP(filter_get_description) {
  func("%u:%s:%s",__LINE__,__FILE__,__FUNCTION__);

  JSString *str;

  JS_BeginRequest(cx);

  FilterInstance *instance = (FilterInstance *)JS_GetPrivate(cx, obj);
  if(!instance) {
    error("%u:%s:%s :: Filter core data is NULL", __LINE__, __FILE__, __FUNCTION__);
    JS_EndRequest(cx);
    //JS_ClearContextThread(cx);
    return JS_FALSE;
  }

  str = JS_NewStringCopyZ(cx, instance->proto->description());
  *vp = STRING_TO_JSVAL(str);
  
  JS_EndRequest(cx);
  return JS_TRUE;
}

JSP(filter_get_author) {
  func("%u:%s:%s", __LINE__, __FILE__, __FUNCTION__);

  JSString *str;

  JS_BeginRequest(cx);

  FilterInstance *instance = (FilterInstance *)JS_GetPrivate(cx, obj);
  if(!instance) {
    error("%u:%s:%s :: Filter core data is NULL", __LINE__, __FILE__, __FUNCTION__);
    JS_EndRequest(cx);
    //JS_ClearContextThread(cx);
    return JS_FALSE;
  }

  str = JS_NewStringCopyZ(cx, instance->proto->author());
  *vp = STRING_TO_JSVAL(str);
  
  JS_EndRequest(cx);
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
