/*  FreeJ - Parameter javascript class implementation
 *  (c) Copyright 2010 Denis Roio <jaromil@dyne.org>
 *
 * This source code  is free software; you can  redistribute it and/or
 * modify it under the terms of the GNU Public License as published by
 * the Free Software  Foundation; either version 3 of  the License, or
 * (at your option) any later version.
 *
 * This source code is distributed in the hope that it will be useful,
 * but  WITHOUT ANY  WARRANTY; without  even the  implied  warranty of
 * MERCHANTABILITY or FITNESS FOR  A PARTICULAR PURPOSE.  Please refer
 * to the GNU Public License for more details.
 *
 * You should  have received  a copy of  the GNU Public  License along
 * with this source code; if  not, write to: Free Software Foundation,
 * Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 *
 */

#include <callbacks_js.h>
#include <jsparser_data.h>

#include <parameter.h>

DECLARE_CLASS("Parameter", parameter_class, parameter_constructor);

JSFunctionSpec parameter_methods[] = {
  // TODO
  {0}
};

JSPropertySpec parameter_properties[] = {
  { "name",        0,  JSPROP_ENUMERATE | JSPROP_PERMANENT | JSPROP_READONLY, parameter_get_name, NULL },
  { "description", 0,  JSPROP_ENUMERATE | JSPROP_PERMANENT | JSPROP_READONLY, parameter_get_desc, NULL },
  { "type",        0,  JSPROP_ENUMERATE | JSPROP_PERMANENT | JSPROP_READONLY, parameter_get_type, NULL },
  {0}
};

JS(parameter_constructor) {
  func("%u:%s:%s",__LINE__,__FILE__,__FUNCTION__);

  // nop

  *rval = OBJECT_TO_JSVAL(obj);
  return JS_TRUE;
}

// TODO
JSP(parameter_get_name) { 
  Parameter *param = (Parameter*)JS_GetPrivate(cx, obj);
  JSString *str;

  if(!param) { JS_ERROR("Parameter used has no internal data"); }
  else {
    str  = JS_NewStringCopyZ(cx, param->name);
    *vp = STRING_TO_JSVAL(str);
  }
  return JS_TRUE;
}

JSP(parameter_get_desc) { 
  Parameter *param = (Parameter*)JS_GetPrivate(cx, obj);
  JSString *str;

  if(!param) { JS_ERROR("Parameter used has no internal data"); }
  else {
    str  = JS_NewStringCopyZ(cx, param->description);
    *vp = STRING_TO_JSVAL(str);
  }
  return JS_TRUE;
}

JSP(parameter_get_type) { 
  Parameter *param = (Parameter*)JS_GetPrivate(cx, obj);
  JSString *str;

  if(!param) { JS_ERROR("Parameter used has no internal data"); }
  else {
    switch(param->type) {
    case Parameter::BOOL:
      str  = JS_NewStringCopyZ(cx,"Boolean");
      break;
    case Parameter::NUMBER:
      str  = JS_NewStringCopyZ(cx,"Number");
      break;
    case Parameter::COLOR:
      str  = JS_NewStringCopyZ(cx,"Color");
      break;
    case Parameter::POSITION: 
      str  = JS_NewStringCopyZ(cx, "Position");
      break;
    case Parameter::STRING:
      str  = JS_NewStringCopyZ(cx, "String");
      break;
    default:
      str  = JS_NewStringCopyZ(cx, "Unknown");
      break;
    }
    *vp = STRING_TO_JSVAL(str);
  }
  return JS_TRUE;
}
