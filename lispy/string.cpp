#pragma once

#include "string.hpp"
#include "program.hpp"

namespace lispy::string {

LISPY_DEF_STATIC_FUNCO(Substr, "string-substr")
{
  if (args->size() < 2)
    throw gc_make<String>("string-substr needs 2 arguments at least");

  auto iter = args->begin();
  auto str = dcast_throw<String>(*iter);

  auto ans = gc_make<String>();
  while (++iter != args->end()) {
    auto lft = intlize(*iter);
    if (lft < 0)
      lft += str->size();
    if (lft < 0 || lft >= str->size())
      throw gc_make<String>("string index out of range");

    if (++iter == args->end()) {
      for (auto i = lft; i < str->size(); ++i)
        ans->push_back((*str)[i]);
      break;
    }

    auto rht = intlize(*iter);
    if (rht < 0)
      rht += str->size();
    if (rht < 0 || rht >= str->size())
      throw gc_make<String>("string index out of range");

    if (rht < lft) {
      for (auto i = lft; i > rht; --i)
        ans->push_back((*str)[i]); // 若右界小于左界，则取逆序子串
    } else {
      for (auto i = lft; i < rht; ++i)
        ans->push_back((*str)[i]);
    }
  }

  return ans;
}

LISPY_DEF_STATIC_FUNCO(Concat, "string-concat")
{
  auto ans = gc_make<String>();
  for (auto i : *args)
    ans->append(*dcast_throw<String>(i));
  return ans;
}

LISPY_DEF_STATIC_FUNCO(Print, "<<<")
{
  Object* obj = &Null::g;
  Object::__Went__ went;
  for (std::size_t i = 0; i < args->size(); ++i) {
    obj = (*args)[i];
    obj->__repr__(std::cout, went);
    std::cout << '\n';
  }
  std::cout << std::flush;
  return obj;
}

LISPY_DEF_STATIC_MACRO(Input, ">>>")
{
  Object* obj = &Null::g;

  while (begin != end) {
    auto ref = evaluate(ctx, *begin);

    std::string line;
    if (std::getline(std::cin, line)) {
      auto iter = line.begin();
      spirit::ast::Expr expr;
      if (spirit::parse(iter, line.end(), expr))
        obj = evaluate(ctx, compile_ast(expr));
    }

    ctx->set(ref, obj);
    ++begin;
  }

  return obj;
}

LISPY_DEF_STATIC_FUNCO(Repr, "string-repr")
{
  std::ostringstream sout;

  Object::__Went__ went;
  for (std::size_t i = 0; i < args->size(); ++i) {
    (*args)[i]->__repr__(sout, went);
    sout << '\n';
  }
  sout << std::flush;

  return gc_make<String>(sout.str());
}

LISPY_DEF_STATIC_MACRO(Compile, "string-compile")
{
  if (end - begin < 1)
    throw gc_make<String>("string-compile needs 1 arguments at least");

  auto obj = evaluate(ctx, *begin);
  while (++begin != end)
    obj = evaluate(ctx, *begin);

  auto str = dcast_throw<String>(obj);
  auto iter = str->begin();
  spirit::ast::Expr expr;
  if (!spirit::parse(iter, str->end(), expr))
    throw gc_make<String>("fail to compile");
  return compile_ast(expr);
}

}
