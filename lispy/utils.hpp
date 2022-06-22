#pragma once

#include "types.hpp"
#include <cassert>
#include <iostream>
#include <sstream>

namespace lispy {

void
print(Object* obj, std::ostream& out, Object::__Went__& went);

template<typename T>
LISPY_EXPORT T*
dcast_throw(Object* p, std::string what)
{
  auto t = dynamic_cast<T*>(p);
  if (t == nullptr)
    throw p->gc_make<String>(std::move(what));
  return t;
}

template<typename T>
LISPY_EXPORT T*
dcast_throw(Object* p)
{
  auto t = dynamic_cast<T*>(p);
  if (t == nullptr) {
    std::ostringstream sout;
    sout << "#! fail to cast " << p << " to " << typeid(T).name();
    throw Object::gc_make<String>(sout.str());
  }
  return t;
}

/**
 * @brief 搜索 key 映射到的对象，若没有找到，则继续在 &Null::g 指向的对象中
 * 寻找（如果指向的对象也为 HashMap 的话），如果还没找到，返回空指针。
 */
LISPY_EXPORT Object*
ctx_get(HashMap* ctx, Object* key);

LISPY_EXPORT Object*
attr_get(Object* obj, Object* attr);

/**
 * @brief 对象布尔化，如果对象是布尔值，则之间转化，否则递归查找对象上的
 * __bool__ 属性。
 */
LISPY_EXPORT bool
boolize(Object* obj);

/**
 * @brief 对象数值化，如果对象是数值（double），并且是整数，则返回对应值，
 * 否则递归查找对象上的 __number__ 属性。
 */
LISPY_EXPORT std::int64_t
intlize(Object* obj);

namespace attr {

LISPY_EXPORT extern String __bool__;

LISPY_EXPORT extern String __number__;

}
}

#define LISPY_DEC_STATIC_MACRO(cls)                                            \
  class LISPY_EXPORT cls : public Macro                                        \
  {                                                                            \
  public:                                                                      \
    static String gName;                                                       \
    static cls g;                                                              \
                                                                               \
  public:                                                                      \
    virtual Object* eval(HashMap* ctx,                                         \
                         Vector::iterator begin,                               \
                         Vector::iterator const& end) override;                \
                                                                               \
    virtual void __repr__(std::ostream& out, __Went__& went) const override    \
    {                                                                          \
      out << gName;                                                            \
    }                                                                          \
  };

#define LISPY_DEF_STATIC_MACRO(cls, name)                                      \
  String cls::gName(name);                                                     \
  cls cls::g;                                                                  \
                                                                               \
  Object* cls::eval(                                                           \
    HashMap* ctx, Vector::iterator begin, Vector::iterator const& end)

#define LISPY_DEC_STATIC_FUNCO(cls)                                            \
  class LISPY_EXPORT cls : public Funco                                        \
  {                                                                            \
  public:                                                                      \
    static String gName;                                                       \
    static cls g;                                                              \
                                                                               \
  public:                                                                      \
    virtual Object* call(Vector* args) override;                               \
                                                                               \
    virtual void __repr__(std::ostream& out, __Went__& went) const override    \
    {                                                                          \
      out << gName;                                                            \
    }                                                                          \
                                                                               \
  private:                                                                     \
    cls() = default;                                                           \
  };

#define LISPY_DEF_STATIC_FUNCO(cls, name)                                      \
  String cls::gName(name);                                                     \
  cls cls::g;                                                                  \
                                                                               \
  Object* cls::call(Vector* args)
