/*  FreeJ OSC controller
 *  (c) Copyright 2008 Denis Rojo <jaromil@dyne.org>
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
 * "$Id: $"
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include <config.h>

#include <osc_ctrl.h>

#include <context.h>
#include <jutils.h>

#include <callbacks_js.h>

void osc_error_handler(int num, const char *msg, const char *path) {
  error("OSC server error %d in path %s: %s\n", num, path, msg);
}

int osc_command_handler(const char *path, const char *types,
			 lo_arg **argv, int argc,
			 void *data, void *user_data) {

  OscController *osc = (OscController*)user_data;
  OscCommand *cmd;

  func("OSC call path %s type %s", path, types);

  cmd = (OscCommand*) osc->commands_handled.search((char*)path,NULL);

  // check that path is handled
  if(cmd) func("OSC path handled by %s",cmd->js_cmd);
  else {
    warning("OSC path %s called, but no method is handling it", path);
    return -1;
  }

  // check that types are matching
  if(strcmp(types, cmd->proto_cmd) != 0) {
    error("OSC path %s called with wrong types: \"%s\" instead of \"%s\"",
	  cmd->name, types, cmd->proto_cmd);
    return -1;
  }

  jsval fval = JSVAL_VOID;

//   JS_CallFunctionValue(JSContext *cx, JSObject *obj, jsval fval, uintN argc,
//                      jsval *argv, jsval *rval);
  JS_GetProperty(osc->jsenv, osc->jsobj, cmd->js_cmd, &fval);
  if(JSVAL_IS_VOID(fval)) {
    error("OSC path %s has method but no javascript function", cmd->js_cmd);
    return -1;
  }
  func("OSC call to %s with argc %u",cmd->js_cmd, argc);

      // TODO: arguments are not supported
      // the code below correctly parses them, but then
      // the jsval is not valid as such in JS_CallFunction

  jsval *jsargv;
  jsargv = (jsval*)calloc(argc+1, sizeof(jsval));
  int c;
  for(c=0;c<argc;c++) {
    switch(types[c]) {
    case 'i':
      func("arg %u is int",c);
      jsargv[c] = INT_TO_JSVAL(argv[c]->i);
      break;
    case 'f':
      func("arg %u is float",c);
      jsargv[c] = DOUBLE_TO_JSVAL((double)argv[c]->f);
      break;
    case 's':
      func("arg %u is string: %s",c, argv[c]);
      jsargv[c] = STRING_TO_JSVAL(argv[c]->s);
      break;
    default:
      error("OSC unrecognized type '%c' in arg %u of path %s",
	    types[c], c, cmd->name);
    }
  }

  
  JsCommand *jscmd = new JsCommand();
  jscmd->set_name(cmd->js_cmd);
  jscmd->function = fval;
  jscmd->argc = argc;
  jscmd->argv = jsargv;
  osc->commands_pending.append(jscmd);

  return 1;
}

/////// Javascript OscController
JS(js_osc_ctrl_constructor);

DECLARE_CLASS("OscController", js_osc_ctrl_class, js_osc_ctrl_constructor);

JS(js_osc_ctrl_start);
JS(js_osc_ctrl_stop);
JS(js_osc_ctrl_add_method);
//JS(js_osc_ctrl_rem_method);

JSFunctionSpec js_osc_ctrl_methods[] = {
  {"start",      js_osc_ctrl_start,      0},
  {"stop",       js_osc_ctrl_stop,       0},
  {"add_method", js_osc_ctrl_add_method, 3},
  //  {"rem_method", js_osc_ctrl_rem_method, 2},
  {0}
};

JS(js_osc_ctrl_constructor) {
  func("%u:%s:%s",__LINE__,__FILE__,__FUNCTION__);
  char excp_msg[MAX_ERR_MSG + 1];

  OscController *osc = new OscController();
  // assign instance into javascript object
  if( ! JS_SetPrivate(cx, obj, (void*)osc) ) {
    sprintf(excp_msg, "failed assigning OSC controller to javascript");
    goto error;
  }
  // initialize with javascript context
  if(! osc->init(cx, obj) ) {
    sprintf(excp_msg, "failed initializing OSC controller");
    goto error;
  }

  char *port;
  JS_ARG_STRING(port,0);
  strncpy(osc->port, port, 64);

  osc->srv = lo_server_thread_new(osc->port, osc_error_handler);

  // register method handler
  // here we register only one method handler
  // as we use our own marshaller instead of liblo's
  lo_server_thread_add_method(osc->srv, NULL, NULL, osc_command_handler, osc);

  notice("OSC controller created: %s",lo_server_thread_get_url(osc->srv));

  *rval = OBJECT_TO_JSVAL(obj);
  return JS_TRUE;

 error:
  JS_ReportErrorNumber(cx, JSFreej_GetErrorMessage, NULL,
		       JSSMSG_FJ_CANT_CREATE, __func__, excp_msg);
  //  cx->newborn[GCX_OBJECT] = NULL;
  delete osc; return JS_FALSE;
}

JS(js_osc_ctrl_start) {
    func("%u:%s:%s argc: %u",__LINE__,__FILE__,__FUNCTION__, argc);
    OscController *osc = (OscController *)JS_GetPrivate(cx, obj);
    if(!osc) JS_ERROR("OSC core data is NULL");

    lo_server_thread_start(osc->srv);

    act("OSC controller listening on port %s",osc->port);

    return JS_TRUE;
}

JS(js_osc_ctrl_stop) {
    func("%u:%s:%s argc: %u",__LINE__,__FILE__,__FUNCTION__, argc);
    OscController *osc = (OscController *)JS_GetPrivate(cx, obj);
    if(!osc) JS_ERROR("OSC core data is NULL");

    lo_server_thread_stop(osc->srv);

    act("OSC controller stopped");

    return JS_TRUE;
}

JS(js_osc_ctrl_add_method) {
    func("%u:%s:%s argc: %u",__LINE__,__FILE__,__FUNCTION__, argc);

    JS_CHECK_ARGC(3);

    OscController *osc = (OscController *)JS_GetPrivate(cx, obj);
    if(!osc) JS_ERROR("OSC core data is NULL");

    char *osc_cmd;
    JS_ARG_STRING(osc_cmd,0);
    char *proto_cmd;
    JS_ARG_STRING(proto_cmd,1);
    char *js_cmd;
    JS_ARG_STRING(js_cmd,2);

    // queue metods in commands_handled linklist
    OscCommand *cmd = new OscCommand();
    cmd->set_name(osc_cmd);
    strncpy(cmd->proto_cmd, proto_cmd, 128);
    strncpy(cmd->js_cmd, js_cmd, 512);
    osc->commands_handled.append(cmd);

    act("OSC method \"%s\" with args \"%s\" binded to %s",
	osc_cmd, proto_cmd, js_cmd);

    return JS_TRUE;
}

//JS(js_osc_ctrl_rem_method) {

  // remove methods from commands_pending linklist

//}


OscController::OscController()
  :Controller() {

  srv = NULL;

  set_name("OscCtrl");
}

OscController::~OscController() {

  if(srv)
    lo_server_thread_free(srv);
  
}

bool OscController::init(JSContext *env, JSObject *obj) {

  jsenv = env;
  jsobj = obj;

  initialized = true;
  return(true);
  
}

int OscController::poll() {
  // check if there are pending commands
  if(commands_pending.len() > 0)
    return dispatch();
  else
    return 0;
}

int OscController::dispatch() {
  jsval ret = JSVAL_VOID;
  int c = 0;
  // execute pending comamnds (javascript calls)
  JsCommand *jscmd = (JsCommand*) commands_pending.begin();
  while(jscmd) {
    
    int res = JS_CallFunctionValue
      (jsenv, jsobj, jscmd->function, jscmd->argc, jscmd->argv, &ret);
    
    if (res)
      if(!JSVAL_IS_VOID(ret)) {
	JSBool ok;
	JS_ValueToBoolean(jsenv, ret, &ok);
	if (ok) func("OSC executed call to %s",jscmd->name);
      }
    free(jscmd->argv);
    commands_pending.rem(1);
    delete jscmd;
    jscmd = (JsCommand*)commands_pending.begin();
    c++;
  }
  return c;
}

