#include "lispy.hpp"
#include "common.hpp"
#include "hashmap.hpp"
#include "number.hpp"
#include "program.hpp"
#include "string.hpp"
#include "utils.hpp"
#include "vector.hpp"

namespace lispy {

Program*
compile(std::string::iterator& begin, std::string::iterator& end)
{
  spirit::ast::Expr expr;
  if (!spirit::parse(begin, end, expr))
    return nullptr;
  return compile_ast(expr);
}

template<typename T>
static inline void
add_std(HashMap* map)
{
  map->set(&T::gName, &T::g);
}

HashMap*
setup_stdctx()
{
  auto map = Object::gc_make<HashMap>();

  add_std<IsNull>(map);
  add_std<IsFalse>(map);
  add_std<IsTrue>(map);
  add_std<IsNumber>(map);
  add_std<IsString>(map);
  add_std<IsVector>(map);
  add_std<IsHashMap>(map);
  add_std<IsProgram>(map);
  add_std<IsMacro>(map);

  add_std<Null>(map);
  add_std<False>(map);
  add_std<True>(map);
  add_std<And>(map);
  add_std<Or>(map);
  add_std<Not>(map);
  add_std<Xor>(map);
  add_std<Eq>(map);

  add_std<Exec>(map);
  add_std<Eval>(map);
  add_std<Let>(map);
  add_std<Ref>(map);
  add_std<Attr>(map);
  add_std<If>(map);
  add_std<Loop>(map);
  add_std<Throw>(map);
  add_std<Catch>(map);
  add_std<MakeMacro>(map);
  add_std<MakeFunco>(map);

  add_std<number::Add>(map);
  add_std<number::Sub>(map);
  add_std<number::Mul>(map);
  add_std<number::Div>(map);
  add_std<number::Mod>(map);
  add_std<number::Fdv>(map);
  add_std<number::Pow>(map);
  add_std<number::Gt>(map);
  add_std<number::Ge>(map);
  add_std<number::Lt>(map);
  add_std<number::Le>(map);

  add_std<hashmap::Get>(map);
  add_std<hashmap::Set>(map);
  add_std<hashmap::Del>(map);
  add_std<hashmap::Size>(map);
  add_std<hashmap::Iter>(map);

  add_std<vector::Get>(map);
  add_std<vector::Set>(map);
  add_std<vector::Size>(map);
  add_std<vector::Insert>(map);
  add_std<vector::Remove>(map);

  add_std<string::Substr>(map);
  add_std<string::Concat>(map);
  add_std<string::Print>(map);
  add_std<string::Input>(map);
  add_std<string::Repr>(map);
  add_std<string::Compile>(map);

  return map;
}

}
