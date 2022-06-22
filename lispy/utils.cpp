#include "utils.hpp"

namespace lispy {

void
print(Object* obj, std::ostream& out, Object::__Went__& went)
{
  if (went.find(obj) == went.end())
    went.insert(obj), obj->__repr__(out, went);
  else
    out << "(@ \"" << typeid(*obj).name() << " 0x" << std::hex
        << std::size_t(obj) << "\")";
}

Object*
ctx_get(HashMap* ctx, Object* key)
{
  auto iter = ctx->find(key);
  while (iter == ctx->end()) {
    auto sup = ctx->find(ctx);
    if (sup != ctx->end()) {
      ctx = dynamic_cast<HashMap*>(sup->second);
      if (ctx != nullptr) {
        iter = ctx->find(key);
        continue;
      }
    }

    std::ostringstream sout;
    sout << "invalid reference: " << key;
    throw Object::gc_make<String>(sout.str());
  }
  return iter->second;
}

Object*
attr_get(Object* obj, Object* attr)
{
  auto retobj = obj->__attr__(attr);
  if (retobj == nullptr) {
    std::ostringstream sout;
    sout << obj << ": invalid attribute: " << attr;
    throw Object::gc_make<String>(sout.str());
  }
  return retobj;
}

bool
boolize(Object* obj)
{
  auto now = obj;

  while (true) {
    if (now == &True::g)
      return true;

    if (now == &False::g)
      return false;

    auto bobj = obj->__attr__(&attr::__bool__);
    if (bobj == nullptr)
      break;
  }

  std::ostringstream sout;
  sout << "fail to boolize: " << obj;
  throw Object::gc_make<String>(sout.str());
}

int64_t
intlize(Object* obj)
{
  auto now = obj;

  while (true) {
    auto num = dynamic_cast<Number*>(now);
    if (num != nullptr) {
      auto dbl = **num;
      if (dbl == 0)
        return 0;

      auto exp = ((reinterpret_cast<std::uint64_t&>(dbl) >> 52) & 0x7FF) - 1023;
      if (exp <= 52) {
        auto i = static_cast<std::int64_t>(dbl);
        if (i == dbl)
          return i;
        throw Object::gc_make<String>("not integer");
      }

      throw Object::gc_make<String>("not precise");
    }

    auto bobj = obj->__attr__(&attr::__number__);
    if (bobj == nullptr)
      break;
  }

  std::ostringstream sout;
  sout << "fail to intlize: " << obj;
  throw Object::gc_make<String>(sout.str());
}

namespace attr {

String __bool__("__bool__");

String __number__("__number__");

}

}
