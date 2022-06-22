#pragma once

#include "spirit.hpp"
#include "types.hpp"

namespace lispy {

/**
 * @brief 立即数程序对象
 */
class LISPY_EXPORT PImmd : public Program
{
public:
  Object* _obj;

public:
  PImmd(Object* obj)
    : _obj(obj)
  {
  }

public:
  /**
   * @brief 在任何上下文中的求职结果都是立即数 _obj
   */
  virtual Object* exec(HashMap* ctx) override { return _obj; }

public:
  virtual void __repr__(std::ostream& out, __Went__& went) const override;
  virtual Object* __attr__(Object* obj) override;

protected:
  virtual void __mark__() override;
};

/**
 * @brief 上下文引用程序对象
 */
class LISPY_EXPORT PSref : public Program
{
public:
  Object* _obj;

public:
  PSref(Object* obj)
    : _obj(obj)
  {
  }

public:
  /**
   * @brief 引用上下文中的 _obj 对应对象
   */
  virtual Object* exec(HashMap* ctx) override;

public:
  virtual void __repr__(std::ostream& out, __Went__& went) const override;
  virtual Object* __attr__(Object* obj) override;

protected:
  virtual void __mark__() override;
};

/**
 * @brief 宏调用程序对象
 */
class LISPY_EXPORT PEval : public Program
{
public:
  Vector* _vec;

public:
  PEval(Vector* vec)
    : _vec(vec)
  {
  }

public:
  /**
   * @brief 其执行语义是，在当前上下文中对首项求值，若结果是宏，
   * 则将其余项传入，返回宏的求值结果。
   */
  virtual Object* exec(HashMap* ctx) override;

public:
  virtual void __repr__(std::ostream& out, __Went__& went) const override;
  virtual Object* __attr__(Object* obj) override;

protected:
  virtual void __mark__() override;
};

class LISPY_EXPORT PMakeVec : public Program
{
public:
  Vector* _vec;

public:
  PMakeVec(Vector* vec)
    : _vec(vec)
  {
  }

public:
  virtual Object* exec(HashMap* ctx) override;

public:
  virtual void __repr__(std::ostream& out, __Went__& went) const override;
  virtual Object* __attr__(Object* obj) override;

protected:
  virtual void __mark__() override;
};

class LISPY_EXPORT PMakeMap : public Program
{
public:
  Vector* _keys; // 以较长的一个为准，另一个不足的视为 &Null::g
  Vector* _vals;

public:
  PMakeMap(Vector* keys, Vector* vals)
    : _keys(keys)
    , _vals(vals)
  {
  }

public:
  virtual Object* exec(HashMap* ctx) override;

public:
  virtual void __repr__(std::ostream& out, __Went__& went) const override;
  virtual Object* __attr__(Object* obj) override;

protected:
  virtual void __mark__() override;
};

struct CompileVisitor
{
  using result_type = Program*;

  static CompileVisitor g;

  Program* operator()(spirit::ast::Number& ast);

  Program* operator()(spirit::ast::String& ast);

  Program* operator()(spirit::ast::Symbol& ast);

  Program* operator()(spirit::ast::Eval& ast);

  Program* operator()(spirit::ast::MakeVec& ast);

  Program* operator()(spirit::ast::MakeMap& ast);

  Program* operator()(spirit::ast::Expr& ast)
  {
    return boost::apply_visitor(*this, ast);
  }
};

inline Program*
compile_ast(spirit::ast::Expr& ast)
{
  return CompileVisitor::g(ast);
}

}
