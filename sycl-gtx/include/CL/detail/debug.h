#pragma once

#include <iostream>
#include <sstream>

// Visual Studio 2013 still lacks some support for modern C++
#ifdef _MSC_VER
#if _MSC_VER <= 1800
#define MSVC_LOW 1
#endif
#endif

#if MSVC_LOW
#define __func__ __FUNCTION__
#define constexpr const
#endif

#define DSELF() debug(__func__)

#ifndef NDEBUG
#define SYCL_ENABLE_DEBUG 1
#endif

#if SYCL_ENABLE_DEBUG
class debug {
protected:
  static constexpr bool DEBUG_ACTIVE = true;
  std::stringstream stream;
  std::stringstream before;

  template<typename T>
  void AddToStream(T add) {
    if(DEBUG_ACTIVE) {
      stream << add << ' ';
    }
  }

  template<class T>
  void AddToStream(std::basic_string<T> string) {
    if(DEBUG_ACTIVE) {
      stream << string << ' ';
    }
  }

  template<template<class, class...> class Container, class First, class... Others>
  void AddToStream(Container<First, Others...> list) {
    for(auto&& element : list) {
      AddToStream(element);
    }
  }

public:
  debug() = default;
#if MSVC_LOW
  debug(debug&& move)
    : stream(std::move(move.stream)), before(std::move(move.before)) {}
#else
  debug(debug&& move) = default;
#endif
  debug(const debug& copy) = delete;

  template<typename T>
  debug(T add)
    : debug() {
    before << "Debug: ";
    AddToStream(add);
  }

  template<typename U, typename T>
  debug(U before, T add)
    : debug() {
    this->before << before;
    AddToStream(add);
  }

  template<typename T>
  debug& operator<<(T add) {
    AddToStream(add);
    return *this;
  }

  ~debug() {
    if(DEBUG_ACTIVE) {
      std::cout << before.str() << stream.str() << std::endl;
    }
  }

  template<typename T>
  static debug warning(T message) {
    return debug("SYCL warning: ", message);
  }
};
#else
class debug {
public:
  debug() {}

  template <typename T>
  debug(T) {}

  template<typename U, typename T>
  debug(U before, T add) {}

  template<typename T>
  debug& operator<<(T add) {
    return *this;
  }

  template<typename T>
  static debug warning(T message) {
    return debug();
  }

  ~debug() {}
};
#endif
