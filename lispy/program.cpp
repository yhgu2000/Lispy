#include "program.hpp"
#include "utils.hpp"

namespace lispy {

static String sObjStr("obj");

void
PImmd::__repr__(std::ostream& out, __Went__& went) const
{
  out << "(PImmd ";
  print(_obj, out, went);
  out << ')';
}

Object*
PImmd::__attr__(Object* obj)
{
  if (sObjStr.__equal__(obj))
    return _obj;
  return nullptr;
}

void
PImmd::__mark__()
{
  gc_mark(_obj);
}

Object*
PSref::exec(HashMap* ctx)
{
  return ctx_get(ctx, _obj);
}

void
PSref::__repr__(std::ostream& out, __Went__& went) const
{
  out << "(PSref ";
  print(_obj, out, went);
  out << ')';
}

Object*
PSref::__attr__(Object* obj)
{
  if (sObjStr.__equal__(obj))
    return _obj;
  return nullptr;
}

void
PSref::__mark__()
{
  gc_mark(_obj);
}

Object*
PEval::exec(HashMap* ctx)
{
  if (_vec->empty())
    return &Null::g;

  auto iter = _vec->begin();

  auto first = Macro::evaluate(ctx, *iter++);
  auto macro = dynamic_cast<Macro*>(first);
  if (macro != nullptr)
    return macro->eval(ctx, iter, _vec->end());

  std::ostringstream sout;
  sout << first << " is not a macro";
  throw gc_make<String>(sout.str());
}

void
PEval::__repr__(std::ostream& out, __Went__& went) const
{
  out << "(PEval ";
  print(_vec, out, went);
  out << ')';
}

static String sVecStr("vec");

Object*
PEval::__attr__(Object* obj)
{
  if (sVecStr.__equal__(obj))
    return _vec;
  return nullptr;
}

void
PEval::__mark__()
{
  gc_mark(_vec);
}

Object*
PMakeVec::exec(HashMap* ctx)
{
  auto vec = gc_make<Vector>();
  vec->reserve(_vec->size());
  // 不能用 resize + 下标赋值，因为求值过程中可能出现异常

  for (auto i : *_vec)
    vec->push_back(Macro::evaluate(ctx, i));
  return vec;
}

void
PMakeVec::__repr__(std::ostream& out, __Went__& went) const
{
  out << "(PMakeVec ";
  print(_vec, out, went);
  out << ')';
}

Object*
PMakeVec::__attr__(Object* obj)
{
  if (sVecStr.__equal__(obj))
    return _vec;
  return nullptr;
}

void
PMakeVec::__mark__()
{
  gc_mark(_vec);
}

Object*
PMakeMap::exec(HashMap* ctx)
{
  auto map = gc_make<HashMap>();

  if (_keys->size() < _vals->size()) {
    auto ki = _keys->begin();
    auto vi = _vals->begin();

    while (ki != _keys->end()) {
      map->set(Macro::evaluate(ctx, *ki), Macro::evaluate(ctx, *vi));
      ++ki, ++vi;
    }

    while (vi != _vals->end()) {
      map->set(&Null::g, Macro::evaluate(ctx, *vi));
      ++vi;
    }

  } else {
    auto ki = _keys->begin();
    auto vi = _vals->begin();

    while (vi != _vals->end()) {
      map->set(Macro::evaluate(ctx, *ki), Macro::evaluate(ctx, *vi));
      ++ki, ++vi;
    }

    while (ki != _keys->end()) {
      map->set(Macro::evaluate(ctx, *ki), &Null::g);
      ++ki;
    }
  }

  return map;
}

void
PMakeMap::__repr__(std::ostream& out, __Went__& went) const
{
  out << "(PMakeMap ";
  print(_keys, out, went);
  out << ' ';
  print(_vals, out, went);
  out << ')';
}

Object*
PMakeMap::__attr__(Object* obj)
{
  static String sKeys("keys");
  static String sVals("vals");
  if (sKeys.__equal__(obj))
    return _keys;
  if (sVals.__equal__(obj))
    return _vals;
  return nullptr;
}

void
PMakeMap::__mark__()
{
  gc_mark(_keys), gc_mark(_vals);
}

CompileVisitor CompileVisitor::g;

Program*
CompileVisitor::operator()(spirit::ast::Number& ast)
{
  return Object::gc_make<PImmd>(Object::gc_make<Number>(ast));
}

Program*
CompileVisitor::operator()(spirit::ast::String& ast)
{
  return Object::gc_make<PImmd>(Object::gc_make<String>(std::move(ast)));
}

Program*
CompileVisitor::operator()(spirit::ast::Symbol& ast)
{
  return Object::gc_make<PSref>(Object::gc_make<String>(std::move(ast)));
}

Program*
CompileVisitor::operator()(spirit::ast::Eval& ast)
{
  auto vec = Object::gc_make<Vector>();
  vec->reserve(ast.size());
  for (auto i : ast)
    vec->push_back((*this)(i));
  return Object::gc_make<PEval>(vec);
}

Program*
CompileVisitor::operator()(spirit::ast::MakeVec& ast)
{
  auto vec = Object::gc_make<Vector>();
  vec->reserve(ast.size());
  for (auto i : ast)
    vec->push_back((*this)(i));
  return Object::gc_make<PMakeVec>(vec);
}

Program*
CompileVisitor::operator()(spirit::ast::MakeMap& ast)
{
  auto keys = Object::gc_make<Vector>();
  auto vals = Object::gc_make<Vector>();

  auto iter = ast.begin();
  while (iter != ast.end()) {
    keys->push_back((*this)(*iter));
    assert(++iter != ast.end());
    vals->push_back((*this)(*iter));
    ++iter;
  }

  return Object::gc_make<PMakeMap>(keys, vals);
}

}
