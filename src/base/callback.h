// Copyright 2011 Google Inc. All Rights Reserved.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

// Modeled after protobuf: http://code.google.com/p/protobuf/

#ifndef XPAF_BASE_CALLBACK_H_
#define XPAF_BASE_CALLBACK_H_

#include "base/macros.h"

namespace xpaf {

class Closure {
 public:
  Closure() {}
  virtual ~Closure() {}

  virtual void Run() = 0;

 private:
  DISALLOW_COPY_AND_ASSIGN(Closure);
};

namespace internal {

class FunctionClosure0 : public Closure {
 public:
  typedef void (*FunctionType)();

  FunctionClosure0(FunctionType function, bool self_deleting)
      : function_(function), self_deleting_(self_deleting) {}
  ~FunctionClosure0() {}

  void Run() {
    bool needs_delete = self_deleting_;  // read in case callback deletes
    function_();
    if (needs_delete) delete this;
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

  void Run() {
    bool needs_delete = self_deleting_;  // read in case callback deletes
    (object_->*method_)();
    if (needs_delete) delete this;
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

  void Run() {
    bool needs_delete = self_deleting_;  // read in case callback deletes
    function_(arg1_);
    if (needs_delete) delete this;
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

  void Run() {
    bool needs_delete = self_deleting_;  // read in case callback deletes
    (object_->*method_)(arg1_);
    if (needs_delete) delete this;
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

  void Run() {
    bool needs_delete = self_deleting_;  // read in case callback deletes
    function_(arg1_, arg2_);
    if (needs_delete) delete this;
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
    bool needs_delete = self_deleting_;  // read in case callback deletes
    (object_->*method_)(arg1_, arg2_);
    if (needs_delete) delete this;
  }

 private:
  Class* object_;
  MethodType method_;
  bool self_deleting_;
  Arg1 arg1_;
  Arg2 arg2_;
};

}  // namespace internal

inline Closure* NewCallback(void (*function)()) {
  return new internal::FunctionClosure0(function, true);
}

inline Closure* NewPermanentCallback(void (*function)()) {
  return new internal::FunctionClosure0(function, false);
}

template <typename Class>
inline Closure* NewCallback(Class* object, void (Class::*method)()) {
  return new internal::MethodClosure0<Class>(object, method, true);
}

template <typename Class>
inline Closure* NewPermanentCallback(Class* object, void (Class::*method)()) {
  return new internal::MethodClosure0<Class>(object, method, false);
}

template <typename Arg1>
inline Closure* NewCallback(void (*function)(Arg1), Arg1 arg1) {
  return new internal::FunctionClosure1<Arg1>(function, true, arg1);
}

template <typename Arg1>
inline Closure* NewPermanentCallback(void (*function)(Arg1), Arg1 arg1) {
  return new internal::FunctionClosure1<Arg1>(function, false, arg1);
}

template <typename Class, typename Arg1>
inline Closure* NewCallback(
    Class* object, void (Class::*method)(Arg1), Arg1 arg1) {
  return new internal::MethodClosure1<Class, Arg1>(object, method, true, arg1);
}

template <typename Class, typename Arg1>
inline Closure* NewPermanentCallback(
    Class* object, void (Class::*method)(Arg1), Arg1 arg1) {
  return new internal::MethodClosure1<Class, Arg1>(object, method, false, arg1);
}

template <typename Arg1, typename Arg2>
inline Closure* NewCallback(
    void (*function)(Arg1, Arg2), Arg1 arg1, Arg2 arg2) {
  return new internal::FunctionClosure2<Arg1, Arg2>(
      function, true, arg1, arg2);
}

template <typename Arg1, typename Arg2>
inline Closure* NewPermanentCallback(
    void (*function)(Arg1, Arg2), Arg1 arg1, Arg2 arg2) {
  return new internal::FunctionClosure2<Arg1, Arg2>(
      function, false, arg1, arg2);
}

template <typename Class, typename Arg1, typename Arg2>
inline Closure* NewCallback(
    Class* object, void (Class::*method)(Arg1, Arg2), Arg1 arg1, Arg2 arg2) {
  return new internal::MethodClosure2<Class, Arg1, Arg2>(
      object, method, true, arg1, arg2);
}

template <typename Class, typename Arg1, typename Arg2>
inline Closure* NewPermanentCallback(
    Class* object, void (Class::*method)(Arg1, Arg2), Arg1 arg1, Arg2 arg2) {
  return new internal::MethodClosure2<Class, Arg1, Arg2>(
      object, method, false, arg1, arg2);
}


// A function which does nothing.
// Useful for creating no-op callbacks, e.g. NewCallback(&DoNothing).
void DoNothing() {}


// Executes a Closure upon deletion. Similar to scoped_ptr.
class AutoClosureRunner {
 public:
  explicit AutoClosureRunner(Closure* c) : closure_(c) {}

  ~AutoClosureRunner() {
    if (closure_) closure_->Run();
  }

  void Reset(Closure *c) {
    delete closure_;
    closure_ = c;
  }

  Closure* Release() {
    Closure* c = closure_;
    closure_ = NULL;
    return c;
  }

 private:
  Closure* closure_;

  DISALLOW_COPY_AND_ASSIGN(AutoClosureRunner);
};

}  // namespace xpaf

#endif  // XPAF_BASE_CALLBACK_H_
