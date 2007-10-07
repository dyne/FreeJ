/*  XmlRpc daemon code - originally from IvySync - using xmlrpc++
 *
 *  (c) Copyright 2004 - 2007 Denis Rojo <jaromil@dyne.org>
 *                     Nederlands Instituut voor Mediakunst
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
 */

#include <config.h>

#include <xmlrpc_ctrl.h>

#include <context.h>
#include <jutils.h>

#include <callbacks_js.h> // javascript

/////// Javascript XmlRpcController
JS(js_xmlrpc_ctrl_constructor);

DECLARE_CLASS("XmlRpcController", js_xmlrpc_ctrl_class, js_xmlrpc_ctrl_constructor);

JSFunctionSpec js_xmlrpc_ctrl_methods[] = { {0} }; // TODO


FreejDaemon::FreejDaemon()
  :Controller() {
  
  xmlrpc = new XmlRpc::XmlRpcServer();
  xmlrpc->_ssl = false;
  xmlrpc->_ssl_ssl = NULL;

  exec = NULL;

  set_name("XmlRpc");
}

FreejDaemon::~FreejDaemon() {
  if(exec) free(exec);
}

bool FreejDaemon::init(JSContext *env, JSObject *obj) {

  if(!exec)
    exec = new ExecMethod(xmlrpc, this);

  if( ! xmlrpc->bindAndListen(2640) ) {
    error("XmlRpc cannot bind and listen");
    return false;
  }

  jsenv = env;
  jsobj = obj;

  initialized = true;
  //  to be introspective we can list our own methods
  //  xmlrpc->enableIntrospection(true);
  return true;
}

int FreejDaemon::peep(Context *env) {
  func("XmlRpcDaemon::peep");
  // run for amount of milliseconds (-1.0 for infinite)
  xmlrpc->work( 1.0 );

  return 1;
}

int FreejDaemon::poll(Context *env) {
  return 1;
}

ExecMethod::ExecMethod(XmlRpcServer *srv, FreejDaemon *freej)
  : XmlRpcServerMethod("Exec", srv)
{
  daemon = freej;
}

ExecMethod::~ExecMethod()
{ }

void ExecMethod::execute(XmlRpcValue &params, XmlRpcValue &result) {
  func("XMLRPC: Exec method called");

  char *script;

  if( params.size() < 1) {
    error("XMLRPC: Exec called without any script", params.size());
    result = 0.0;
    return;
  }

  //  int t = params.getType();
  //  if(t!=TypeString) error("parameter is not a string");
  script = (char*)string(params[0]).c_str();
  act("XMLRPC: received script of size %u", params[0].size());

  daemon->freej->open_script( script );

  act("XMLRPC: script executed");

}

JS(js_xmlrpc_ctrl_constructor) {
  func("%u:%s:%s",__LINE__,__FILE__,__FUNCTION__);

  FreejDaemon *daemon = new FreejDaemon();
  // assign instance into javascript object
  if( ! JS_SetPrivate(cx, obj, (void*)daemon) ) {
    error("failed assigning xmlrpc daemon controller to javascript");
    delete daemon; return JS_FALSE;
  }

  // initialize with javascript context
  if(! daemon->init(cx, obj) ) {
    error("failed initializing xmlrpc daemon controller");
    delete daemon; return JS_FALSE;
  }
  
  *rval = OBJECT_TO_JSVAL(obj);
  return JS_TRUE;
}

