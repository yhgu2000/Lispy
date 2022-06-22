#pragma once

#include "utils.hpp"

namespace lispy {

// ==========================================================================
// 类型判别函数
// ==========================================================================

#define LISPY_DEC_ISTYPE(t) LISPY_DEC_STATIC_FUNCO(Is##t);
#define LISPY_DEF_ISTYPE(t)                                                    \
  String Is##t::gName(#t "?");                                                 \
  Is##t Is##t::g;                                                              \
                                                                               \
  Object* Is##t::call(Vector* args)                                            \
  {                                                                            \
    for (auto i : *args)                                                       \
      if (dynamic_cast<t*>(i) == nullptr)                                      \
        return &False::g;                                                      \
    return &True::g;                                                           \
  }

LISPY_DEC_ISTYPE(Null);
LISPY_DEC_ISTYPE(False);
LISPY_DEC_ISTYPE(True);
LISPY_DEC_ISTYPE(Number);
LISPY_DEC_ISTYPE(String);
LISPY_DEC_ISTYPE(Vector);
LISPY_DEC_ISTYPE(HashMap);
LISPY_DEC_ISTYPE(Program);
LISPY_DEC_ISTYPE(Macro);

LISPY_DEC_STATIC_FUNCO(And);

LISPY_DEC_STATIC_FUNCO(Or);

LISPY_DEC_STATIC_FUNCO(Not);

LISPY_DEC_STATIC_FUNCO(Xor);

LISPY_DEC_STATIC_FUNCO(Eq);

/**
 * @brief 在指定上下文中依次运行一组程序，返回最后一个结果
 */
LISPY_DEC_STATIC_FUNCO(Exec);

/**
 * @brief 在指定上下文中对宏求值，传入参数
 */
LISPY_DEC_STATIC_FUNCO(Eval);

/**
 * @brief 赋值语句（=）
 */
LISPY_DEC_STATIC_MACRO(Let);

/**
 * @brief 引用语句（@）
 */
LISPY_DEC_STATIC_MACRO(Ref);

/**
 * @brief 属性获取语句（.）
 */
LISPY_DEC_STATIC_FUNCO(Attr);

/**
 * @brief 求值第一项，如果为真，返回第二项，否则从第三项开始执行
 */
LISPY_DEC_STATIC_MACRO(If);

LISPY_DEC_STATIC_MACRO(Loop);

LISPY_DEC_STATIC_MACRO(Throw);

LISPY_DEC_STATIC_MACRO(Catch);

/**
 * @brief 用户宏创建宏
 *
 * 用法：
 *
 * ```
 *    (`
 *      "ctx"   # 第一个参数指明了上下文形参名，实参是一个 HashMap
 *      "args"  # 第二个参数指明了参数形参名，实参是一个 Vector
 *
 *      # 从第三个参数开始，为在宏上下文中执行的程序
 *      (set . "n" (+ 1 2))
 *      (get ctx n)
 *     )
 * ```
 */
class LISPY_EXPORT MakeMacro : public Macro
{
public:
  static String gName;
  static MakeMacro g;

public:
  virtual Object* eval(HashMap* ctx,
                       Vector::iterator begin,
                       Vector::iterator const& end) override;

  virtual void __repr__(std::ostream& out, __Went__& went) const override;

private:
  class UserMacro;
};

/**
 * @brief 用户函数创建宏
 *
 * 用法：
 *
 * ```
 *    (`
 *      "args"  # 第二个参数指明了参数形参名，实参是一个 Vector
 *
 *      # 从第二个参数开始，为函数体
 *      (set . "n" (+ 1 2))
 *      (get ctx n)
 *     )
 * ```
 */
class LISPY_EXPORT MakeFunco : public Macro
{
public:
  static String gName;
  static MakeFunco g;

public:
  virtual Object* eval(HashMap* ctx,
                       Vector::iterator begin,
                       Vector::iterator const& end) override;

  virtual void __repr__(std::ostream& out, __Went__& went) const override;

private:
  class UserFunco;
};

}
