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


#ifndef __FREEJ_XMLRPC_H__
#define __FREEJ_XMLRPC_H__


#include <controller.h>

#include <XmlRpc.h>

#include <iostream>
#include <stdlib.h>


using namespace XmlRpc;


class Context;

// XMLRPC METHODS
class ExecMethod;


class FreejDaemon: public Controller {
 public:
  FreejDaemon();
  ~FreejDaemon();

  bool init(JSContext *env, JSObject *obj);
  int peep(Context *env);
  int poll(Context *env);

  Context *freej;

 private:

  XmlRpc::XmlRpcServer *xmlrpc;

  ExecMethod *exec;

};

class ExecMethod: public XmlRpc::XmlRpcServerMethod {
 public:
  
  ExecMethod(XmlRpc::XmlRpcServer *srv, FreejDaemon *freej);
  ~ExecMethod();

  void execute(XmlRpcValue &params, XmlRpcValue &result);

  std::string help() { 
    return std::string("Execute a script in FreeJ"); }

  FreejDaemon *daemon;

};


#endif
