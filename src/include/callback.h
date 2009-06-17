#ifndef __CALLBACK_H__
#define __CALLBACK_H__

/*
 * Copyright (C) 2009 - Luca Bigliardi
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 */

#include <list>
#include <string>
#include <typeinfo>
#include <pthread.h>

#include <closure.h>

#if 0
/*
 * CallbackHandler - pseudocode/draft/will_it_compile?
 *
 * The idea is to have an handler which has an account of various types of
 * callbacks and, for each type, the listeners (something like a poor man's
 * SIGNAL/SLOT mechanism). Callbacks are performed in a separate thread by a
 * ThreadedClosure, so the main flow won't block.
 *
 * We'd like to provide automatically _static_ type checking for listeners and
 * notifications, hence there's a lot of quasi-duplicated code (still not sure
 * about this hierarchy/checking-approach anyway).
 *
 * Example:
 *
 * CallbackHandler *h = new CallbackHandler();
 *
 * // an object who wants to provide a type of signal has to register first
 * h->register_callback("foo", &function_prototype);
 * // and then, each time the signal has to be raised it has to call
 * h->notify("foo");
 *
 * // an object who wants to be informed about a signal
 * h->add_call("foo", &my_callback);
 *
 */

namespace callbacks {

  class CbackData {
    public:
      CbackData(std::string name) : name_(name) {}
      virtual ~CbackData() { }
      const std::string& name() const { return name_; }

    protected:
      std::string name_;
  };

  class CbackDataFun0 : public CbackData {
    public:
      typedef void (*FunctionType)();

      CbackDataFun0(std::string name, FunctionType function)
      : CbackData(name), function_(function) {}
      ~CbackDataFun0() { calls_.clear(); }

      bool add_call(FunctionType call) {
        if (typeid(call) == typeid(function_)) {
          if (get_fun_(call)) {
            warning("%s %s, callback already added",
                    __PRETTY_FUNCTION__, name_.c_str());
            return false;
          }
          calls_.push_back(call);
          return true;
        } else {
          warning("%s %s, signature mismatch",
                  __PRETTY_FUNCTION__, name_.c_str());
          return false;
        }
      }

      bool rem_call(FunctionType call) {
        if (!get_fun_(call)) {
          warning("%s %s, callback not present",
                  __PRETTY_FUNCTION__, name_.c_str());
          return false;
        }
        calls_.remove(call);
        return true;
      }

      void notify(ThreadedClosing *dispatcher) {
        std::list<FunctionType>::iterator i;
        for (i=calls_.begin() ; i!=calls_.end() ; i++)
          dispatcher->add_job(NewClosure(*i));
      }

    private:
      FunctionType get_fun_(FunctionType call) {
        FunctionType fun = NULL;
        std::list<FunctionType>::iterator i;
        for (i=calls_.begin() ; i!=calls_.end() ; i++)
          if (*i == call) fun = call;
        return fun;
      }

      FunctionType function_;
      std::list<FunctionType> calls_;
  };

  template <typename Arg1>
  class CbackDataFun1 : public CbackData {
    public:
      typedef void (*FunctionType)(Arg1);

      CbackDataFun1(std::string name, FunctionType function)
      : CbackData(name), function_(function) {}
      ~CbackDataFun1() { calls_.clear(); }

      bool add_call(FunctionType call) {
        if (typeid(call) == typeid(function_)) {
          if (get_fun_(call)) {
            warning("%s %s, callback already added",
                    __PRETTY_FUNCTION__, name_.c_str());
            return false;
          }
          calls_.push_back(call);
          return true;
        } else {
          warning("%s %s, signature mismatch",
                  __PRETTY_FUNCTION__, name_.c_str());
          return false;
        }
      }

      bool rem_call(FunctionType call) {
        if (!get_fun_(call)) {
          warning("%s %s, callback not present",
                  __PRETTY_FUNCTION__, name_.c_str());
          return false;
        }
        calls_.remove(call);
        return true;
      }

      void notify(ThreadedClosing *dispatcher, Arg1 arg1) {
        typename std::list<FunctionType>::iterator i;
        for (i=calls_.begin() ; i!=calls_.end() ; i++)
          dispatcher->add_job(NewClosure(*i, arg1));
      }

    private:
      FunctionType get_fun_(FunctionType call) {
        FunctionType fun = NULL;
        typename std::list<FunctionType>::iterator i;
        for (i=calls_.begin() ; i!=calls_.end() ; i++)
          if (*i == call) fun = call;
        return fun;
      }

      FunctionType function_;
      std::list<FunctionType> calls_;
  };

}  // namespace callbacks


class CallbackHandler {
  public:
    CallbackHandler() { dispatcher_ = new ThreadedClosing(); }
    ~CallbackHandler() {
      cbacks_.clear();
      delete dispatcher_;
    }

    bool add_call(const char *name, void (*prototype)()) {
      callbacks::CbackData *c = get_data_(name);
      if (!c) {
          warning("%s name %s not found", __PRETTY_FUNCTION__, name);
          return false;
      }
      callbacks::CbackDataFun0* c0 = static_cast<callbacks::CbackDataFun0 *>(c);
      return c0->add_call(prototype);
    }

    template <typename Arg1>
    bool add_call(const char *name, void (*prototype)(Arg1)) {
      callbacks::CbackData *c = get_data_(name);
      if (!c) {
          warning("%s name %s not found", __PRETTY_FUNCTION__, name);
          return false;
      }
      callbacks::CbackDataFun1<Arg1>* c1 = static_cast<callbacks::CbackDataFun1<Arg1>*>(c);
      return c1->add_call(prototype);
    }

    bool rem_call(const char *name, void (*prototype)()) {
      callbacks::CbackData *c = get_data_(name);
      if (!c) {
          warning("%s name %s not found", __PRETTY_FUNCTION__, name);
          return false;
      }
      // here we have a static checking for callback signature
      callbacks::CbackDataFun0* c0 = static_cast<callbacks::CbackDataFun0 *>(c);
      return c0->rem_call(prototype);
    }

    template <typename Arg1>
    bool rem_call(const char *name, void (*prototype)(Arg1)) {
      callbacks::CbackData *c = get_data_(name);
      if (!c) {
          warning("%s name %s not found", __PRETTY_FUNCTION__, name);
          return false;
      }
      callbacks::CbackDataFun1<Arg1>* c1 = static_cast<callbacks::CbackDataFun1<Arg1>*>(c);
      return c1->rem_call(prototype);
    }

    bool unregister_callback(const char *name) {
      callbacks::CbackData *c = get_data_(name);
      if (!c) {
          warning("%s name %s not found", __PRETTY_FUNCTION__, name);
          return false;
      }
      cbacks_.remove(c); // this performs 'delete c' as well
      return true;
    }

    bool register_callback(const char *name, void (*prototype)()) {
      callbacks::CbackData *c = get_data_(name);
      if (c) {
          warning("%s name %s already registered", __PRETTY_FUNCTION__, name);
          return false;
      }
      c = new callbacks::CbackDataFun0(name, prototype);
      cbacks_.push_back(c);
      return true;
    }

    template <typename Arg1>
    bool register_callback(const char *name, void (*prototype)(Arg1)) {
      callbacks::CbackData *c = get_data_(name);
      if (c) {
          warning("%s name %s already registered", __PRETTY_FUNCTION__, name);
          return false;
      }
      c = new callbacks::CbackDataFun1<Arg1>(name, prototype);
      cbacks_.push_back(c);
      return true;
    }

    bool notify(const char *name) {
      callbacks::CbackData *c = get_data_(name);
      if (!c) {
          warning("%s name %s not found", __PRETTY_FUNCTION__, name);
          return false;
      }
      // here we have a static checking for callback signature
      callbacks::CbackDataFun0* c0 = static_cast<callbacks::CbackDataFun0 *>(c);
      c0->notify(dispatcher_);
      return true;
    }

    template <typename Arg1>
    bool notify(const char *name, Arg1 arg1) {
      callbacks::CbackData *c = get_data_(name);
      if (!c) {
          warning("%s name %s not found", __PRETTY_FUNCTION__, name);
          return false;
      }
      callbacks::CbackDataFun1<Arg1>* c1 = static_cast<callbacks::CbackDataFun1<Arg1>*>(c);
      c1->notify(dispatcher_, arg1);
      return true;
    }


  private:
    callbacks::CbackData *get_data_(const std::string& name) {
      callbacks::CbackData *c = NULL;
      std::list<callbacks::CbackData*>::iterator i;
      for (i=cbacks_.begin() ; i!=cbacks_.end() ; i++)
          if ((*i)->name() == name) c = *i;
      return c;
    }

    std::list<callbacks::CbackData*> cbacks_;
    ThreadedClosing *dispatcher_;
};
#endif

// TODO(shammash): quick hack to provide EOS callback immediately
class DumbCall {
  public:
    DumbCall();
    virtual ~DumbCall();

    void notify();
    void enqueue();
    void dequeue();
    
    virtual void callback() {};

  private:
    pthread_mutex_t pending_; // used as a flag to block destructor if a call
                              // is in progress
    pthread_mutexattr_t pendingattr_;
    int refcount_; // reference counter for the number of calls in progress
    pthread_mutex_t refcount_mutex_;
    pthread_mutexattr_t refcount_mutexattr_;
};

class DumbCallback {
  public:
    DumbCallback();
    ~DumbCallback();

    bool add_call(DumbCall *call);
    bool rem_call(DumbCall *call);
    void notify();

  private:
    DumbCall *get_call_(DumbCall *call);

    std::list<DumbCall *> calls_;
    ThreadedClosing *dispatcher_;
};

#endif // __CALLBACK_H__

// vim:tabstop=2:expandtab:shiftwidth=2

