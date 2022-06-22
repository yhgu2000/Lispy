#include "number.hpp"
#include "utils.hpp"

namespace lispy::number {

LISPY_DEF_STATIC_FUNCO(Add, "+")
{
  if (args->empty())
    throw gc_make<String>("+ needs 1 argument at least");

  auto iter = args->begin();
  auto n = gc_make<Number>(**dcast_throw<Number>(*iter));
  while (++iter != args->end())
    **n += **dcast_throw<Number>(*iter);

  return n;
}

LISPY_DEF_STATIC_FUNCO(Sub, "-")
{
  if (args->empty())
    throw gc_make<String>("- needs 1 argument at least");

  auto iter = args->begin();
  auto n = gc_make<Number>(**dcast_throw<Number>(*iter));
  while (++iter != args->end())
    **n -= **dcast_throw<Number>(*iter);

  return n;
}

LISPY_DEF_STATIC_FUNCO(Mul, "*")
{
  if (args->empty())
    throw gc_make<String>("* needs 1 argument at least");

  auto iter = args->begin();
  auto n = gc_make<Number>(**dcast_throw<Number>(*iter));
  while (++iter != args->end())
    **n *= **dcast_throw<Number>(*iter);

  return n;
}

LISPY_DEF_STATIC_FUNCO(Div, "/")
{
  if (args->empty())
    throw gc_make<String>("/ needs 1 argument at least");

  auto iter = args->begin();
  auto n = gc_make<Number>(**dcast_throw<Number>(*iter));
  while (++iter != args->end())
    **n /= **dcast_throw<Number>(*iter);

  return n;
}

LISPY_DEF_STATIC_FUNCO(Mod, "%")
{
  if (args->empty())
    throw gc_make<String>("% needs 1 argument at least");

  auto iter = args->begin();
  auto n = gc_make<Number>(**dcast_throw<Number>(*iter));
  while (++iter != args->end()) {
    auto rhs = **dcast_throw<Number>(*iter);
    **n -= rhs * std::floor(**n / rhs);
  }

  return n;
}

LISPY_DEF_STATIC_FUNCO(Fdv, "//")
{
  if (args->empty())
    throw gc_make<String>("// needs 1 argument at least");

  auto iter = args->begin();
  auto n = gc_make<Number>(**dcast_throw<Number>(*iter));
  while (++iter != args->end())
    **n = std::floor(**n / **dcast_throw<Number>(*iter));

  return n;
}

LISPY_DEF_STATIC_FUNCO(Pow, "**")
{
  if (args->empty())
    throw gc_make<String>("** needs 1 argument at least");

  auto iter = args->begin();
  auto n = gc_make<Number>(**dcast_throw<Number>(*iter));
  while (++iter != args->end())
    **n = std::pow(**n, **dcast_throw<Number>(*iter));

  return n;
}

LISPY_DEF_STATIC_FUNCO(Gt, ">")
{
  if (args->empty())
    throw gc_make<String>("** needs 1 argument at least");

  auto iter = args->begin();
  auto n = gc_make<Number>(**dcast_throw<Number>(*iter));
  while (++iter != args->end())
    if (!(**n > **dcast_throw<Number>(*iter)))
      return &False::g;

  return &True::g;
}

LISPY_DEF_STATIC_FUNCO(Ge, ">=")
{
  if (args->empty())
    throw gc_make<String>("** needs 1 argument at least");

  auto iter = args->begin();
  auto n = gc_make<Number>(**dcast_throw<Number>(*iter));
  while (++iter != args->end())
    if (!(**n >= **dcast_throw<Number>(*iter)))
      return &False::g;

  return &True::g;
}

LISPY_DEF_STATIC_FUNCO(Lt, "<")
{
  if (args->empty())
    throw gc_make<String>("** needs 1 argument at least");

  auto iter = args->begin();
  auto n = gc_make<Number>(**dcast_throw<Number>(*iter));
  while (++iter != args->end())
    if (!(**n < **dcast_throw<Number>(*iter)))
      return &False::g;

  return &True::g;
}

LISPY_DEF_STATIC_FUNCO(Le, "<=")
{
  if (args->empty())
    throw gc_make<String>("** needs 1 argument at least");

  auto iter = args->begin();
  auto n = gc_make<Number>(**dcast_throw<Number>(*iter));
  while (++iter != args->end())
    if (!(**n <= **dcast_throw<Number>(*iter)))
      return &False::g;

  return &True::g;
}

}
