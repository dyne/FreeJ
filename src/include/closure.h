#ifndef __CLOSURE_H__
#define __CLOSURE_H__

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
 *
 *
 * Closure classes, useful for callbacks and delayed executions of functions
 * or methods (profoundly inspired by ProtocolBuffers' callbacks).
 *
 * Examples:
 *
 * function(arg1, arg2);
 * Closure *closure = NewClosure(&function, arg1, arg2);
 * ...
 * closure.run();
 *
 * class Class {
 *   ...
 *   method(arg1, arg2);
 *   ...
 * }
 * Class *obj = new Class();
 * Closure *closure = NewClosure(obj, &Class::method, arg1, arg2);
 * ...
 * closure.run();
 *
 *
 * After a run() closure object will be automatically deleted.
 * If you want to delete it by hand use NewPermanentClosure() instead.
 *
 *
 * A different number of arguments leads to a different internal representation
 * of a closure to enforce a safe type-checking at compile time. If you need a
 * Closure for a number of arguments not yet covered simply add the appropriate
 * FunctionClosureN MethodClosureN classes, NewClosure() and NewPermanentClosure().
 *
 */


class Closure {
  public:
    Closure() {}
    virtual ~Closure() {}
    virtual void run() = 0;
};


namespace closures {

  class FunctionClosure0 : public Closure {
    public:
      typedef void (*FunctionType)();

      FunctionClosure0(FunctionType function, bool self_deleting)
        : function_(function), self_deleting_(self_deleting) {}
      ~FunctionClosure0();

      void run() {
        function_();
        if (self_deleting_) delete this;
      }

    private:
      FunctionType function_;
      bool self_deleting_;
  };


  template <typename Class>
  class MethodClosure0 : public Closure {
    public:
      typedef void (Class::*MethodType)();

      MethodClosure0(Class* object, MethodType method, bool self_deleting)
        : object_(object), method_(method), self_deleting_(self_deleting) {}
      ~MethodClosure0() {}

      void run() {
        (object_->*method_)();
        if (self_deleting_) delete this;
      }

    private:
      Class* object_;
      MethodType method_;
      bool self_deleting_;
  };


  template <typename Arg1>
  class FunctionClosure1 : public Closure {
    public:
      typedef void (*FunctionType)(Arg1 arg1);

      FunctionClosure1(FunctionType function, bool self_deleting,
          Arg1 arg1)
        : function_(function), self_deleting_(self_deleting),
        arg1_(arg1) {}
      ~FunctionClosure1() {}


      void run() {
        function_(arg1_);
        if (self_deleting_) delete this;
      }

    private:
      FunctionType function_;
      bool self_deleting_;
      Arg1 arg1_;
  };


  template <typename Class, typename Arg1>
  class MethodClosure1 : public Closure {
    public:
      typedef void (Class::*MethodType)(Arg1 arg1);

      MethodClosure1(Class* object, MethodType method, bool self_deleting,
          Arg1 arg1)
        : object_(object), method_(method), self_deleting_(self_deleting),
        arg1_(arg1) {}
      ~MethodClosure1() {}

      void run() {
        (object_->*method_)(arg1_);
        if (self_deleting_) delete this;
      }

    private:
      Class* object_;
      MethodType method_;
      bool self_deleting_;
      Arg1 arg1_;
  };


  template <typename Arg1, typename Arg2>
  class FunctionClosure2 : public Closure {
    public:
      typedef void (*FunctionType)(Arg1 arg1, Arg2 arg2);

      FunctionClosure2(FunctionType function, bool self_deleting,
          Arg1 arg1, Arg2 arg2)
        : function_(function), self_deleting_(self_deleting),
        arg1_(arg1), arg2_(arg2) {}
      ~FunctionClosure2() {}

      void run() {
        function_(arg1_, arg2_);
        if (self_deleting_) delete this;
      }

    private:
      FunctionType function_;
      bool self_deleting_;
      Arg1 arg1_;
      Arg2 arg2_;
  };


  template <typename Class, typename Arg1, typename Arg2>
  class MethodClosure2 : public Closure {
    public:
      typedef void (Class::*MethodType)(Arg1 arg1, Arg2 arg2);

      MethodClosure2(Class* object, MethodType method, bool self_deleting,
          Arg1 arg1, Arg2 arg2)
        : object_(object), method_(method), self_deleting_(self_deleting),
        arg1_(arg1), arg2_(arg2) {}
      ~MethodClosure2() {}

      void Run() {
        (object_->*method_)(arg1_, arg2_);
        if (self_deleting_) delete this;
      }

    private:
      Class* object_;
      MethodType method_;
      bool self_deleting_;
      Arg1 arg1_;
      Arg2 arg2_;
  };

}


inline Closure* NewClosure(void (*function)()) {
  return new closures::FunctionClosure0(function, true);
}

inline Closure* NewPermanentClosure(void (*function)()) {
  return new closures::FunctionClosure0(function, false);
}

template <typename Class>
inline Closure* NewClosure(Class* object, void (Class::*method)()) {
  return new closures::MethodClosure0<Class>(object, method, true);
}

template <typename Class>
inline Closure* NewPermanentClosure(Class* object, void (Class::*method)()) {
  return new closures::MethodClosure0<Class>(object, method, false);
}

template <typename Arg1>
inline Closure* NewClosure(void (*function)(Arg1),
                            Arg1 arg1) {
  return new closures::FunctionClosure1<Arg1>(function, true, arg1);
}

template <typename Arg1>
inline Closure* NewPermanentClosure(void (*function)(Arg1),
                                     Arg1 arg1) {
  return new closures::FunctionClosure1<Arg1>(function, false, arg1);
}

template <typename Class, typename Arg1>
inline Closure* NewClosure(Class* object, void (Class::*method)(Arg1),
                            Arg1 arg1) {
  return new closures::MethodClosure1<Class, Arg1>(object, method, true, arg1);
}

template <typename Class, typename Arg1>
inline Closure* NewPermanentClosure(Class* object, void (Class::*method)(Arg1),
                                     Arg1 arg1) {
  return new closures::MethodClosure1<Class, Arg1>(object, method, false, arg1);
}

template <typename Arg1, typename Arg2>
inline Closure* NewClosure(void (*function)(Arg1, Arg2),
                            Arg1 arg1, Arg2 arg2) {
  return new closures::FunctionClosure2<Arg1, Arg2>(
    function, true, arg1, arg2);
}

template <typename Arg1, typename Arg2>
inline Closure* NewPermanentClosure(void (*function)(Arg1, Arg2),
                                     Arg1 arg1, Arg2 arg2) {
  return new closures::FunctionClosure2<Arg1, Arg2>(
    function, false, arg1, arg2);
}

template <typename Class, typename Arg1, typename Arg2>
inline Closure* NewClosure(Class* object, void (Class::*method)(Arg1, Arg2),
                            Arg1 arg1, Arg2 arg2) {
  return new closures::MethodClosure2<Class, Arg1, Arg2>(
    object, method, true, arg1, arg2);
}

template <typename Class, typename Arg1, typename Arg2>
inline Closure* NewPermanentClosure(
    Class* object, void (Class::*method)(Arg1, Arg2),
    Arg1 arg1, Arg2 arg2) {
  return new closures::MethodClosure2<Class, Arg1, Arg2>(
    object, method, false, arg1, arg2);
}

#endif
