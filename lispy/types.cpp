#include "types.hpp"
#include "utils.hpp"

namespace lispy {

bool Object::gFlag;
std::unordered_set<Object*> Object::gRoots;
std::forward_list<Object*> Object::gAll;

void
Object::gc()
{
  gFlag = !gFlag;

  for (auto root : gRoots)
    gc_mark(root);

  auto iter = gAll.before_begin();
  while (true) {
    auto next = std::next(iter);
    if (next == gAll.end())
      break;

    if ((*next)->__flag__ != gFlag) {
      delete *next;
      gAll.erase_after(iter);
    } else {
      iter = next;
    }
  }
}

void
String::__repr__(std::ostream& out, __Went__& went) const
{
  out << '"';
  for (auto&& i : *this) {
    if (i == '\\')
      out << "\\\\";

    else if (i == '"')
      out << "\\\"";

    else if (i == '\t')
      out << "\\t";

    else if (i == '\n')
      out << "\\n";

    else if (i >= 32 && i < 127)
      out << i;

    else {
      out << "\\x";
      auto hex = (i >> 4) & 0xf;
      out << char((hex >= 10) ? ('A' + hex) : ('0' + hex));
      hex = i & 0xf;
      out << char((hex >= 10) ? ('A' + hex) : ('0' + hex));
    }
  }
  out << '"';
}

std::size_t
String::__hash__() const
{
  static const std::hash<std::string> hasher;
  return hasher(*this);
}

bool
String::__equal__(Object* obj) const
{
  auto lhs = dynamic_cast<String*>(obj);
  return lhs != nullptr && *this == *lhs;
}

String Null::gName("!");
Null Null::g;

void
Null::__repr__(std::ostream& out, __Went__& went) const
{
  out << gName;
}

String False::gName("!f");
False False::g;

void
False::__repr__(std::ostream& out, __Went__& went) const
{
  out << gName;
}

String True::gName("!t");
True True::g;

void
True::__repr__(std::ostream& out, __Went__& went) const
{
  out << gName;
}

void
Number::__repr__(std::ostream& out, __Went__& went) const
{
  out << _;
}

std::size_t
Number::__hash__() const
{
  return reinterpret_cast<const std::size_t&>(_);
}

bool
Number::__equal__(Object* obj) const
{
  auto lhs = dynamic_cast<Number*>(obj);
  return lhs != nullptr && _ == lhs->_;
}

void
Vector::__repr__(std::ostream& out, __Went__& went) const
{
  out << '[';

  auto iter = begin();
  if (iter != end()) {
    print(*iter, out, went);

    while (++iter != end()) {
      out << ' ';
      print(*iter, out, went);
    }
  }

  out << ']';
}

void
Vector::__mark__()
{
  for (auto&& i : *this)
    gc_mark(i);
}

void
HashMap::__mark__()
{
  for (auto&& i : *this) {
    gc_mark(i.first);
    gc_mark(i.second);
  }
}

void
HashMap::__repr__(std::ostream& out, __Went__& went) const
{
  out << '{';

  auto iter = begin();
  if (iter != end()) {
    out << "( ";
    print(iter->first, out, went);
    out << ' ';
    print(iter->second, out, went);
    out << " )";

    while (++iter != end()) {
      out << "  ( ";
      print(iter->first, out, went);
      out << ' ';
      print(iter->second, out, went);
      out << " )";
    }
  }

  out << '}';
}

Object*
Funco::eval(HashMap* ctx,
               Vector::iterator begin,
               Vector::iterator const& end)
{
  auto args = gc_make<Vector>();
  args->reserve(end - begin);
  while (begin != end)
    args->push_back(evaluate(ctx, *begin++));
  return call(args);
}

}
