#pragma once

#include <boost/variant.hpp>

namespace lispy::spirit {

namespace ast {

using boost::recursive_wrapper;
using boost::variant;

using Number = double;

struct LISPY_EXPORT String : std::string
{
  using std::string::string;
  using std::string::operator=;
};

struct LISPY_EXPORT Symbol : std::string
{
  using std::string::string;
  using std::string::operator=;
};

struct Eval;
struct MakeVec;
struct MakeMap;

using Expr = variant<Number,
                     String,
                     Symbol,
                     recursive_wrapper<Eval>,
                     recursive_wrapper<MakeVec>,
                     recursive_wrapper<MakeMap>>;

struct LISPY_EXPORT Eval : std::vector<Expr>
{};

struct LISPY_EXPORT MakeVec : std::vector<Expr>
{};

struct LISPY_EXPORT MakeMap : std::vector<Expr>
{};

}

using Iterator = std::string::iterator;

LISPY_EXPORT bool
parse(Iterator& begin, Iterator const& end, ast::Expr& expr);

}
