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
 * "$Id:$"
 *
 */

#include <config.h>

#include <algorithm>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <memory>
#include <new>
#include <strings.h>
#include <type_traits>
#include <utility>

#include <callbacks_js.h>
#include <context.h>
#include <jsparser.h>
#include <osc_ctrl.h>

FACTORY_REGISTER_INSTANTIATOR(Controller, OscController, OscController, core);

namespace {

// Four batches drain the stress fixture's burst while leaving a presentation
// between batches. Inputs arriving during dispatch stay in the same FIFO.
constexpr int messages_per_poll = 32;

const char *string_argument(jsval value) {
  return JSVAL_IS_STRING(value) ? JS_GetStringBytes(JSVAL_TO_STRING(value)) : nullptr;
}

void osc_error_handler(int num, const char *msg, const char *path) {
  error("OSC server error %d in path %s: %s", num, path ? path : "", msg);
}

}

int osc_command_handler(const char *path, const char *types, lo_arg **argv, int argc,
                        lo_message, void *user_data) {
  auto *osc = static_cast<OscController *>(user_data);
  try {
    return osc->receive(path, types, argv, argc);
  } catch (const std::bad_alloc &) {
    error("OSC could not queue message for %s: out of memory", path);
    return 1;
  }
}

OscController::OscController()
  : freej(nullptr), srv(nullptr), sendto(nullptr), port{} {
  set_name("OscCtrl");
}

OscController::~OscController() {
  if (srv)
    lo_server_thread_free(srv); // stops and joins the receiver before its state is destroyed
  if (sendto)
    lo_address_free(sendto);
  unbind_receiver();
}

bool OscController::open(const char *requested_port) {
  if (srv || std::strlen(requested_port) >= sizeof(port))
    return false;

  srv = lo_server_thread_new(requested_port, osc_error_handler);
  if (!srv)
    return false;

  // liblo's wildcard route preserves the literal signature for our own
  // validation instead of coercing arguments to a registered signature.
  if (!lo_server_thread_add_method(srv, nullptr, nullptr, osc_command_handler, this)) {
    lo_server_thread_free(srv);
    srv = nullptr;
    return false;
  }
  std::snprintf(port, sizeof(port), "%s", requested_port);
  return true;
}

bool OscController::start() {
  return srv && lo_server_thread_start(srv) == 0;
}

bool OscController::start(int requested_port) {
  char number[sizeof(port)];
  std::snprintf(number, sizeof(number), "%d", requested_port);
  return (srv ? std::strcmp(port, number) == 0 : open(number)) && start();
}

void OscController::stop() {
  // Stop waits for the current receiver callback. Keep accepted messages for
  // later frames, including across a subsequent start on the same socket.
  if (srv && lo_server_thread_stop(srv) != 0)
    error("OSC controller failed to stop on port %s", port);
}

bool OscController::bind_receiver(JSContext *cx, JSObject *object) {
  auto *owner = static_cast<JsExecutionContext *>(JS_GetRuntimePrivate(JS_GetRuntime(cx)));
  if (!owner)
    return false;
  owner->osc_receivers_.push_back(this);
  jsenv = cx;
  jsobj = object;
  if (!JS_AddNamedRoot(cx, &jsobj, "OSC receiver")) {
    owner->osc_receivers_.pop_back();
    return false;
  }
  receiver_owner_ = owner;
  add_listener(cx, object);
  return JS_SetPrivate(cx, object, this);
}

void OscController::unbind_receiver() {
  if (!receiver_owner_)
    return;
  auto &receivers = receiver_owner_->osc_receivers_;
  receivers.erase(std::find(receivers.begin(), receivers.end(), this));
  receiver_owner_ = nullptr;

  const bool attach = JS_GetContextThread(jsenv) == 0;
  if (attach)
    JS_SetContextThread(jsenv);
  JS_BeginRequest(jsenv);
  if (JS_GetPrivate(jsenv, jsobj) == this)
    JS_SetPrivate(jsenv, jsobj, nullptr);
  JS_RemoveRoot(jsenv, &jsobj);
  JS_EndRequest(jsenv);
  if (attach)
    JS_ClearContextThread(jsenv);
  jsenv = nullptr;
  jsobj = nullptr;
}

void OscController::add_method(const char *path, const char *signature, const char *callback) {
  std::lock_guard<std::mutex> lock(messages_mutex_);
  routes_.push_back({path, signature, callback});
}

int OscController::receive(const char *path, const char *types, lo_arg **argv, int argc) {
  Route route;
  {
    std::lock_guard<std::mutex> lock(messages_mutex_);
    auto found = std::find_if(routes_.begin(), routes_.end(), [path](const Route &registered) {
      return strcasecmp(registered.path.c_str(), path) == 0;
    });
    if (found != routes_.end())
      route = *found;
  }

  if (route.path.empty()) {
    warning("OSC path %s called, but no method is handling it", path);
    return 1;
  }
  if (route.signature != types || argc < 0 ||
      static_cast<size_t>(argc) != route.signature.size()) {
    error("OSC path %s called with wrong types: \"%s\" instead of \"%s\"",
          path, types, route.signature.c_str());
    return 1;
  }

  Message message;
  message.callback = std::move(route.callback);
  message.arguments.reserve(argc);
  for (int i = 0; i < argc; ++i) {
    Argument value;
    value.type = types[i];
    // Liblo presents host-order values on OSC's four-byte boundaries.
    // Its union may have stricter alignment than a packet argument.
    switch (value.type) {
      case 'i':
        std::memcpy(&value.integer, argv[i], sizeof(value.integer));
        break;
      case 'f':
        std::memcpy(&value.real, argv[i], sizeof(value.real));
        break;
      case 's':
        value.text = reinterpret_cast<const char *>(argv[i]);
        break;
      default:
        error("OSC unrecognized type '%c' in arg %d of path %s", value.type, i, path);
        return 1;
    }
    message.arguments.push_back(std::move(value));
  }

  {
    std::lock_guard<std::mutex> lock(messages_mutex_);
    pending_.push_back(std::move(message));
  }
  func("OSC queued %s (%s)", path, types);
  return 0;
}

bool OscController::invoke(const Message &message) {
  std::vector<jsval> argv(message.arguments.size());
  const bool attach = JS_GetContextThread(jsenv) == 0;
  if (attach && JS_SetContextThread(jsenv) == -1)
    return false;
  JS_BeginRequest(jsenv);

  bool called = false;
  if (JS_EnterLocalRootScope(jsenv)) {
    bool converted = true;
    for (size_t i = 0; i < argv.size() && converted; ++i) {
      const auto &argument = message.arguments[i];
      if (argument.type == 's') {
        JSString *text = JS_NewStringCopyZ(jsenv, argument.text.c_str());
        converted = text != nullptr;
        if (text)
          argv[i] = STRING_TO_JSVAL(text);
      } else {
        const double number = argument.type == 'i' ? static_cast<double>(argument.integer)
                                                   : static_cast<double>(argument.real);
        converted = JS_NewNumberValue(jsenv, number, &argv[i]);
      }
    }
    if (converted)
      called = JSCall(message.callback.c_str(), argv.size(), argv.data()) != 0;
    JS_LeaveLocalRootScope(jsenv);
  }
  JS_EndRequest(jsenv);
  if (attach)
    JS_ClearContextThread(jsenv);
  return called;
}

int OscController::poll() {
  return dispatch();
}

int OscController::dispatch() {
  int dispatched = 0;
  while (dispatched < messages_per_poll && receiver_owner_ && env->can_poll_controller(*this)) {
    Message message;
    {
      std::lock_guard<std::mutex> lock(messages_mutex_);
      if (pending_.empty())
        break;
      message = std::move(pending_.front());
      pending_.pop_front();
    }
    // User code may stop the server, unregister this receiver, or reset the
    // context. The registry keeps us alive until poll returns.
    if (invoke(message))
      func("OSC dispatched call to %s", message.callback.c_str());
    else
      error("OSC failed JSCall to %s", message.callback.c_str());
    ++dispatched;
  }
  return dispatched;
}

JS(js_osc_ctrl_constructor);
DECLARE_CLASS("OscController", js_osc_ctrl_class, js_osc_ctrl_constructor);

JS(js_osc_ctrl_start);
JS(js_osc_ctrl_stop);
JS(js_osc_ctrl_add_method);
JS(js_osc_ctrl_send_to);
JS(js_osc_ctrl_send);

JSFunctionSpec js_osc_ctrl_methods[] = {
  {"start",      js_osc_ctrl_start,      0},
  {"stop",       js_osc_ctrl_stop,       0},
  {"add_method", js_osc_ctrl_add_method, 3},
  {"send_to",    js_osc_ctrl_send_to,    2},
  {"send",       js_osc_ctrl_send,       8},
  {0}
};

JS(js_osc_ctrl_constructor) {
  JS_CHECK_ARGC(1);
  const char *port = string_argument(argv[0]);
  if (!port)
    JS_ERROR("OSC port must be a string");

  std::unique_ptr<OscController> osc(static_cast<OscController *>(
      Factory<Controller>::new_instance("OscController")));
  if (!osc || !osc->init(global_environment) || !osc->open(port))
    JS_ERROR("could not open OSC server");
  if (!osc->bind_receiver(cx, obj))
    JS_ERROR("could not bind OSC receiver");

  osc->javascript = true;
  char *url = lo_server_thread_get_url(osc->srv);
  if (url) {
    notice("OSC controller created: %s", url);
    std::free(url);
  }

  *rval = OBJECT_TO_JSVAL(obj);
  osc.release();
  return JS_TRUE;
}

JS(js_osc_ctrl_start) {
  auto *osc = static_cast<OscController *>(JS_GetPrivate(cx, obj));
  if (!osc)
    JS_ERROR("OSC core data is NULL");
  if (!osc->start())
    JS_ERROR("could not start OSC server");

  act("OSC controller listening on port %s", osc->port);
  return JS_TRUE;
}

JS(js_osc_ctrl_stop) {
  auto *osc = static_cast<OscController *>(JS_GetPrivate(cx, obj));
  if (!osc)
    JS_ERROR("OSC core data is NULL");
  osc->stop();
  act("OSC controller stopped");
  return JS_TRUE;
}

JS(js_osc_ctrl_add_method) {
  JS_CHECK_ARGC(3);
  auto *osc = static_cast<OscController *>(JS_GetPrivate(cx, obj));
  if (!osc)
    JS_ERROR("OSC core data is NULL");
  const char *path = string_argument(argv[0]);
  const char *signature = string_argument(argv[1]);
  const char *callback = string_argument(argv[2]);
  if (!path || !signature || !callback)
    JS_ERROR("OSC method path, signature and callback must be strings");

  osc->add_method(path, signature, callback);
  act("OSC method \"%s\" with args \"%s\" bound to %s", path, signature, callback);
  return JS_TRUE;
}

JS(js_osc_ctrl_send_to) {
  JS_CHECK_ARGC(2);
  auto *osc = static_cast<OscController *>(JS_GetPrivate(cx, obj));
  if (!osc)
    JS_ERROR("OSC core data is NULL");
  const char *host = string_argument(argv[0]);
  const char *port = string_argument(argv[1]);
  if (!host || !port)
    JS_ERROR("OSC destination host and port must be strings");

  lo_address destination = lo_address_new(host, port);
  if (!destination)
    JS_ERROR("could not create OSC destination");
  if (osc->sendto)
    lo_address_free(osc->sendto);
  osc->sendto = destination;
  act("OSC controller sends messages to %s port %s", host, port);
  return JS_TRUE;
}

JS(js_osc_ctrl_send) {
  JS_CHECK_ARGC(2);
  auto *osc = static_cast<OscController *>(JS_GetPrivate(cx, obj));
  if (!osc)
    JS_ERROR("OSC core data is NULL");
  if (!osc->sendto)
    JS_ERROR("OSC destination must be configured with send_to");

  const char *path = string_argument(argv[0]);
  const char *signature = string_argument(argv[1]);
  if (!path || !signature || std::strlen(signature) != argc - 2)
    JS_ERROR("OSC path and signature must match the supplied arguments");
  std::unique_ptr<std::remove_pointer<lo_message>::type, decltype(&lo_message_free)>
      message(lo_message_new(), lo_message_free);
  if (!message)
    JS_ERROR("could not allocate OSC message");

  for (uintN i = 0; i < argc - 2; ++i) {
    const jsval argument = argv[i + 2];
    int status;
    switch (signature[i]) {
      case 'i': {
        int32 value;
        if (!JSVAL_IS_NUMBER(argument) || !JS_ValueToInt32(cx, argument, &value))
          JS_ERROR("OSC integer argument must be a number");
        status = lo_message_add_int32(message.get(), value);
        break;
      }
      case 'f': {
        jsdouble value;
        if (!JSVAL_IS_NUMBER(argument) || !JS_ValueToNumber(cx, argument, &value))
          JS_ERROR("OSC float argument must be a number");
        status = lo_message_add_float(message.get(), static_cast<float>(value));
        break;
      }
      case 's': {
        const char *value = string_argument(argument);
        if (!value)
          JS_ERROR("OSC string argument must be a string");
        status = lo_message_add_string(message.get(), value);
        break;
      }
      default:
        JS_ERROR("unrecognized OSC argument type");
    }
    if (status != 0)
      JS_ERROR("could not append OSC argument");
  }

  lo_server server = osc->srv ? lo_server_thread_get_server(osc->srv) : nullptr;
  if (lo_send_message_from(osc->sendto, server, path, message.get()) < 0)
    JS_ERROR("could not send OSC message");
  return JS_TRUE;
}
