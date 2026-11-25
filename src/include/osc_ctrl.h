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

#ifndef __OSC_CTRL_H__
#define __OSC_CTRL_H__

#include <lo/lo.h>
#include <deque>
#include <mutex>
#include <string>
#include <vector>
#include <controller.h>
#include <factory.h>

class JsExecutionContext;

class OscController: public Controller {

 public:
  OscController();
  ~OscController();

  virtual int dispatch();
  int poll();

  bool open(const char *port);
  bool start();
  bool start(int port);
  void stop();
  bool bind_receiver(JSContext *cx, JSObject *object);
  void add_method(const char *path, const char *signature, const char *callback);

  Context *freej;

  lo_server_thread srv;

  lo_address sendto;

  char port[64];

 private:
  friend class JsExecutionContext;
  friend int osc_command_handler(const char *, const char *, lo_arg **, int,
                                 lo_message, void *);

  struct Route {
    std::string path;
    std::string signature;
    std::string callback;
  };

  struct Argument {
    char type;
    int32_t integer = 0;
    float real = 0;
    std::string text;
  };

  struct Message {
    std::string callback;
    std::vector<Argument> arguments;
  };

  int receive(const char *path, const char *types, lo_arg **argv, int argc);
  bool invoke(const Message &message);
  void unbind_receiver();

  JsExecutionContext *receiver_owner_ = nullptr;
  std::mutex messages_mutex_;
  std::vector<Route> routes_;
  std::deque<Message> pending_;

 public:
  FACTORY_ALLOWED;
};

#endif
