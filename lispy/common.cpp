#include "common.hpp"
#include "program.hpp"
#include "utils.hpp"

namespace lispy {

LISPY_DEF_STATIC_FUNCO(And, "&")
{
  for (auto i : *args)
    if (!boolize(i))
      return &False::g;
  return &True::g;
}

LISPY_DEF_STATIC_FUNCO(Or, "|")
{
  for (auto i : *args)
    if (boolize(i))
      return &True::g;
  return &False::g;
}

LISPY_DEF_STATIC_FUNCO(Not, "~")
{
  bool ans = true;
  for (auto i : *args)
    ans = !boolize(i);
  if (ans)
    return &True::g;
  return &False::g;
}

LISPY_DEF_STATIC_FUNCO(Xor, "^")
{
  bool ans = false;
  for (auto i : *args)
    ans = (ans != boolize(i));
  if (ans)
    return &True::g;
  return &False::g;
}

LISPY_DEF_STATIC_FUNCO(Eq, "==")
{
  if (args->empty())
    throw gc_make<String>("= needs 1 arguments at least");

  auto obj = (*args)[0];
  for (std::size_t i = 0; i < args->size(); ++i)
    if (!obj->__equal__((*args)[i]))
      return &False::g;
  return &True::g;
}

LISPY_DEF_ISTYPE(Null);
LISPY_DEF_ISTYPE(False);
LISPY_DEF_ISTYPE(True);
LISPY_DEF_ISTYPE(Number);
LISPY_DEF_ISTYPE(String);
LISPY_DEF_ISTYPE(Vector);
LISPY_DEF_ISTYPE(HashMap);
LISPY_DEF_ISTYPE(Program);
LISPY_DEF_ISTYPE(Macro);

LISPY_DEF_STATIC_FUNCO(Exec, "$")
{
  if (args->empty())
    throw gc_make<String>("$ needs 1 arguments at least");

  auto iter = args->begin();
  auto local = dcast_throw<HashMap>(*iter);

  Object* ans = &Null::g;
  while (++iter != args->end())
    ans = evaluate(local, *iter);

  return ans;
}

LISPY_DEF_STATIC_FUNCO(Eval, "$$")
{
  if (args->size() < 2)
    throw gc_make<String>("$$ needs 2 arguments at least");

  auto iter = args->begin();
  auto local = dcast_throw<HashMap>(*iter++);
  auto macro = dcast_throw<Macro>(*iter++);

  return macro->eval(local, iter, args->end());
}

LISPY_DEF_STATIC_MACRO(Let, "=")
{
  Object* obj = &Null::g;
  while (begin != end) {
    auto key = evaluate(ctx, *begin++);
    if (begin == end) {
      ctx->set(key, &Null::g);
      break;
    }
    obj = evaluate(ctx, *begin++);
    ctx->set(key, obj);
  }
  return obj;
}

LISPY_DEF_STATIC_MACRO(Ref, "@")
{
  if (begin == end)
    return ctx;

  Object* key;
  do {
    key = evaluate(ctx, *begin);
  } while (++begin != end);

  return ctx_get(ctx, key);
}

LISPY_DEF_STATIC_FUNCO(Attr, ".")
{
  if (args->empty())
    return &Null::g;

  auto iter = args->begin();
  Object* obj = *iter;
  while (++iter != args->end())
    obj = attr_get(obj, *iter);

  return obj;
}

LISPY_DEF_STATIC_MACRO(If, "?")
{
  if (end - begin < 2)
    throw gc_make<String>("? needs 2 arguments at least");

  auto cond = evaluate(ctx, *begin++);
  auto then = *begin++;
  if (boolize(cond))
    return evaluate(ctx, then);

  Object* ans = &Null::g;
  while (begin != end)
    ans = evaluate(ctx, *begin++);
  return ans;
}

LISPY_DEF_STATIC_MACRO(Loop, ":")
{
  while (true)
    for (auto iter = begin; iter != end; ++iter)
      evaluate(ctx, *iter);
}

LISPY_DEF_STATIC_MACRO(Throw, ",")
{
  Object* obj = &Null::g;
  while (begin != end)
    obj = evaluate(ctx, *begin++);
  throw obj;
}

LISPY_DEF_STATIC_MACRO(Catch, ";")
{
  if (end - begin < 2)
    throw gc_make<String>("; needs 2 arguments at least");

  auto errKey = evaluate(ctx, *begin++);
  auto prg = *begin++;

  try {
    Object* ans = &Null::g;
    while (begin != end)
      ans = evaluate(ctx, *begin++);
    return ans;

  } catch (Object* err) {
    auto ectx = gc_make<HashMap>();
    ectx->set(ectx, ctx);
    ectx->set(errKey, err);
    return evaluate(ectx, prg);
  }
}

// ==========================================================================
// MakeMacro
// ==========================================================================

String MakeMacro::gName("`");
MakeMacro MakeMacro::g;

class MakeMacro::UserMacro : public Macro
{
public:
  HashMap* _sup;
  Object* _ctxKey;
  Object* _argsKey;
  std::vector<Program*> _prgs;

public:
  UserMacro(HashMap* sup,
            Object* ctxKey,
            Object* argsKey,
            std::vector<Program*> prgs = {})
    : _sup(sup)
    , _ctxKey(ctxKey)
    , _argsKey(argsKey)
    , _prgs(std::move(prgs))
  {
  }

public:
  virtual Object* eval(HashMap* ctx,
                       Vector::iterator begin,
                       Vector::iterator const& end) override;

  virtual void __repr__(std::ostream& out, __Went__& went) const override;

protected:
  virtual void __mark__() override;
};

Object*
MakeMacro::eval(HashMap* ctx,
                Vector::iterator begin,
                Vector::iterator const& end)
{
  if (end - begin < 2)
    throw gc_make<String>("` needs 2 arguments at least");

  auto ctxKey = evaluate(ctx, *begin++);
  auto argsKey = evaluate(ctx, *begin++);

  auto macro = gc_make<UserMacro>(ctx, ctxKey, argsKey);
  macro->_prgs.reserve(end - begin);

  while (begin != end) {
    auto obj = *begin;
    auto prg = dynamic_cast<Program*>(obj);
    if (prg)
      macro->_prgs.push_back(prg);
    else
      macro->_prgs.push_back(gc_make<PImmd>(obj));
    ++begin;
  }

  return macro;
}

Object*
MakeMacro::UserMacro::eval(HashMap* ctx,
                           Vector::iterator begin,
                           const Vector::iterator& end)
{
  auto local = gc_make<HashMap>();
  local->set(local, _sup);
  local->set(_ctxKey, ctx);

  auto args = gc_make<Vector>();
  while (begin != end)
    args->push_back(*begin++);
  local->set(_argsKey, args);

  Object* ans = &Null::g;
  for (auto i : _prgs)
    ans = i->exec(local);
  return ans;
}

void
MakeMacro::UserMacro::__repr__(std::ostream& out, __Went__& went) const
{
  out << "(@ \"UserMacro at 0x" << std::hex << std::size_t(this) << "\")";
}

void
MakeMacro::UserMacro::__mark__()
{
  gc_mark(_sup), gc_mark(_ctxKey), gc_mark(_argsKey);
  for (auto i : _prgs)
    gc_mark(i);
}

void
MakeMacro::__repr__(std::ostream& out, __Went__& went) const
{
  out << gName;
}

// ==========================================================================
// MakeFunco
// ==========================================================================

String MakeFunco::gName("``");
MakeFunco MakeFunco::g;

class MakeFunco::UserFunco : public Funco
{
public:
  HashMap* _sup;
  Object* _argsKey;
  std::vector<Program*> _prgs;

public:
  UserFunco(HashMap* sup, Object* argsKey, std::vector<Program*> prgs = {})
    : _sup(sup)
    , _argsKey(argsKey)
    , _prgs(std::move(prgs))
  {
  }

public:
  virtual Object* call(Vector* args) override;

public:
  virtual void __repr__(std::ostream& out, __Went__& went) const override;

protected:
  virtual void __mark__() override;
};

Object*
MakeFunco::eval(HashMap* ctx,
                Vector::iterator begin,
                Vector::iterator const& end)
{
  if (end - begin < 1)
    throw gc_make<String>("`` needs 1 arguments at least");

  auto argsKey = evaluate(ctx, *begin++);
  auto funco = gc_make<UserFunco>(ctx, argsKey);
  funco->_prgs.reserve(end - begin);

  while (begin != end) {
    auto obj = *begin;
    auto prg = dynamic_cast<Program*>(obj);
    if (prg)
      funco->_prgs.push_back(prg);
    else
      funco->_prgs.push_back(gc_make<PImmd>(obj));
    ++begin;
  }

  return funco;
}

Object*
MakeFunco::UserFunco::call(Vector* args)
{
  auto local = gc_make<HashMap>();
  local->set(local, _sup);
  local->set(_argsKey, args);

  Object* ans = &Null::g;
  for (auto i : _prgs)
    ans = i->exec(local);
  return ans;
}

void
MakeFunco::UserFunco::__repr__(std::ostream& out, __Went__& went) const
{
  out << "(@ \"UserFunco at 0x" << std::hex << std::size_t(this) << "\")";
}

void
MakeFunco::UserFunco::__mark__()
{
  gc_mark(_sup), gc_mark(_argsKey);
  for (auto i : _prgs)
    gc_mark(i);
}

void
MakeFunco::__repr__(std::ostream& out, __Went__& went) const
{
  out << gName;
}

}
