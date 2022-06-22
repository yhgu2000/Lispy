#include "vector.hpp"
#include <algorithm>

namespace lispy::vector {

LISPY_DEF_STATIC_MACRO(Get, "vector-get")
{
  if (end - begin < 2)
    throw gc_make<String>("vector-get needs 2 arguments at least");

  auto vec = dcast_throw<Vector>(evaluate(ctx, *begin++));

  Object* key;
  while (begin != end)
    key = evaluate(ctx, *begin++);

  auto index = intlize(key);
  if (index < 0)
    index += vec->size();
  if (index < 0 || index >= vec->size())
    throw gc_make<String>("vector index out of range");
  return (*vec)[index];
}

LISPY_DEF_STATIC_FUNCO(Set, "vector-set")
{
  if (args->size() < 1)
    throw gc_make<String>("hashmap-set needs 1 arguments at least");

  auto iter = args->begin();
  auto vec = dcast_throw<Vector>(*iter);

  while (++iter != args->end()) {
    auto index = intlize(*iter);
    if (index < 0)
      index += vec->size();
    if (index < 0 || index >= vec->size())
      throw gc_make<String>("vector index out of range");

    if (++iter == args->end()) {
      (*vec)[index] = &Null::g;
      break;
    }

    (*vec)[index] = *iter;
  }

  return vec;
}

LISPY_DEF_STATIC_FUNCO(Size, "vector-size")
{
  auto num = gc_make<Number>();

  for (auto i : *args) {
    auto vec = dcast_throw<Vector>(i);
    **num += vec->size();
  }

  return num;
}

LISPY_DEF_STATIC_FUNCO(Insert, "vector-insert")
{
  if (args->size() < 2)
    throw gc_make<String>("vector-insert needs 2 arguments at least");

  auto iter = args->begin();
  auto vec = dcast_throw<Vector>(*iter++);

  auto index = intlize(*iter++);
  if (index < 0)
    index += vec->size();
  if (index < 0 || index > vec->size())
    throw gc_make<String>("vector index out of range");

  vec->insert(vec->begin() + index, args->begin() + 2, args->end());
  return vec;
}

LISPY_DEF_STATIC_FUNCO(Remove, "vector-remove")
{
  if (args->size() < 2)
    throw gc_make<String>("hashmap-remove needs 2 arguments at least");

  auto iter = args->begin();
  auto vec = dcast_throw<Vector>(*iter);

  std::vector<std::int64_t> indexes;
  while (++iter != args->end()) {
    auto index = intlize(*iter);
    if (index < 0)
      index += vec->size();
    if (index < 0 || index >= vec->size())
      throw gc_make<String>("vector index out of range");
    indexes.push_back(index);
  }

  std::sort(indexes.begin(), indexes.end());
  indexes.push_back(vec->size());

  std::size_t lft = 0, rht = 1, shift = 0;
  do {
    while (indexes[lft] == indexes[rht])
      ++rht;
    ++shift;

    for (std::size_t i = indexes[lft] + 1; i < indexes[rht]; ++i)
      (*vec)[i - shift] = (*vec)[i];

    lft = rht;
  } while (indexes[rht] < vec->size());

  vec->resize(vec->size() - shift);
  return vec;
}

}
