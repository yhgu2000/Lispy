#include "hashmap.hpp"

namespace lispy::hashmap {

LISPY_DEF_STATIC_MACRO(Get, "hashmap-get")
{
  if (end - begin < 1)
    throw gc_make<String>("hashmap-get needs 1 arguments at least");

  auto map = dcast_throw<HashMap>(evaluate(ctx, *begin++));

  Object* key = ctx;
  while (begin != end)
    key = evaluate(ctx, *begin++);

  auto val = ctx->get(key);
  if (val == nullptr) {
    std::ostringstream sout;
    sout << "invalid key: Object 0x" << std::hex << std::size_t(key);
    throw gc_make<String>(sout.str());
  }

  return val;
}

LISPY_DEF_STATIC_FUNCO(Set, "hashmap-set")
{
  if (args->size() < 1)
    throw gc_make<String>("hashmap-set needs 1 arguments at least");

  auto iter = args->begin();
  auto map = dcast_throw<HashMap>(*iter);

  while (++iter != args->end()) {
    auto key = *iter;
    if (++iter == args->end()) {
      map->set(key, &Null::g);
      break;
    }
    map->set(key, *iter);
  }

  return map;
}

LISPY_DEF_STATIC_FUNCO(Del, "hashmap-del")
{
  if (args->size() < 1)
    throw gc_make<String>("hashmap-del needs 1 arguments at least");

  auto iter = args->begin();
  auto map = dcast_throw<HashMap>(*iter);

  while (++iter != args->end())
    map->del(*iter);

  return map;
}

LISPY_DEF_STATIC_FUNCO(Size, "hashmap-size")
{
  auto num = gc_make<Number>();

  for (auto i : *args) {
    auto map = dcast_throw<HashMap>(i);
    **num += map->size();
  }

  return num;
}

/**
 * @brief 用法：
 *
 * (set . iter (iter-map .))
 *
 * (; ! ! (:
 *   (? (iter ! "val1" "key2" ! "key3") (,))
 *   (<<< val1 key2 key3)
 *  ))
 *
 * 遍历结束返回 true，否则返回 false
 * 若最后一遍时元素不够，剩下的参数将被关联到 null，返回 false
 */
class MapIter : public Macro
{
public:
  std::vector<HashMap*> _maps;
  std::size_t _which;
  Object* _where;

public:
  MapIter(std::vector<HashMap*> map);

public:
  virtual Object* eval(HashMap* ctx,
                       Vector::iterator begin,
                       Vector::iterator const& end) override;

  virtual void __repr__(std::ostream& out, __Went__& went) const override;

protected:
  virtual void __mark__() override;
};

LISPY_DEF_STATIC_FUNCO(Iter, "hashmap-iter")
{
  std::vector<HashMap*> maps;
  maps.reserve(args->size());
  for (auto i : *args)
    maps.push_back(dcast_throw<HashMap>(i));
  return gc_make<MapIter>(std::move(maps));
}

MapIter::MapIter(std::vector<HashMap*> map)
  : _maps(std::move(map))
  , _which(0)
  , _where(&Null::g)
{
  while (_which < map.size()) {
    if (!_maps[_which]->empty()) {
      _where = _maps[_which]->begin()->first;
      break;
    }
    ++_which;
  }
}

Object*
MapIter::eval(HashMap* ctx, Vector::iterator begin, Vector::iterator const& end)
{
  if (_which == _maps.size())
    return &True::g;

  // _where 记录这次的遍历开始位置
  auto& which = _maps[_which];
  auto iter = which->find(_where);
  if (iter == which->end())
    throw gc_make<String>("iteration break off");

  while (begin != end) {
    auto kref = evaluate(ctx, *begin++);
    ctx->set(kref, iter->first);

    if (begin != end) {
      auto vref = evaluate(ctx, *begin++);
      ctx->set(vref, iter->second);
    }

    if (++iter == which->end()) {
      do {
        if (++_which == _maps.size())
          goto LAST_PASS;
        which = _maps[_which];
        iter = which->begin();
      } while (iter == which->end());
    }
  }

  return &False::g;

LAST_PASS:
  while (begin != end)
    ctx->set(evaluate(ctx, *begin++), &Null::g);
  return &False::g;
}

void
MapIter::__repr__(std::ostream& out, __Went__& went) const
{
  out << "(@ \"MapIter at 0x" << std::hex << std::size_t(this) << "\")";
}

void
MapIter::__mark__()
{
  for (auto i : _maps)
    gc_mark(i);
  gc_mark(_where);
}

}
