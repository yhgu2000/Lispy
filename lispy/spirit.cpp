#include "spirit.hpp"
#include <boost/phoenix.hpp>
#include <boost/spirit/include/lex_lexertl.hpp>
#include <boost/spirit/include/qi.hpp>

namespace fu = boost::fusion;
namespace qi = boost::spirit::qi;
namespace lex = boost::spirit::lex;

namespace lispy::spirit {

namespace {

// ==========================================================================
// 词法分析
// ==========================================================================

using lex::lexertl::actor_lexer;
using lex::lexertl::token;

using TokenTypes = boost::mpl::vector<ast::Number, ast::String, ast::Symbol>;

class Lexer : public lex::lexer<actor_lexer<token<Iterator, TokenTypes>>>
{
public:
  lex::token_def<> comments;
  lex::token_def<> spaces;
  lex::token_def<ast::Number> number;
  lex::token_def<ast::String> string;
  lex::token_def<ast::Symbol> symbol;

public:
  Lexer();

private:
  static bool match_string(Iterator& iter, Iterator& eoi, std::string& str);
};

Lexer::Lexer()
{
  comments = R"(#)";
  self +=
    comments[([](auto& start, auto& end, auto& pass, auto& id, auto& ctx) {
      pass = lex::pass_flags::pass_ignore;

      auto eoi = ctx.get_eoi();
      if (end == eoi)
        return;

      if (*end == '\'' || *end == '"') {
        std::string str;
        if (!match_string(end, eoi, str))
          pass = lex::pass_flags::pass_fail;
        return;
      }

      while (end != eoi && *end++ != '\n')
        ;
    })];

  spaces = R"([\s]+)";
  self += spaces[lex::_pass = lex::pass_flags::pass_ignore];

  static const std::string kLits = R"([]{}())";
  for (auto i : kLits)
    self += i;

  number = R"([\+\-]?\d+(\.\d+)?)";
  self += number;

  string = R"(["'])";
  self += string[([](auto& start, auto& end, auto& pass, auto& id, auto& ctx) {
    auto eoi = ctx.get_eoi();
    ast::String str;

    end = start;
    if (!match_string(end, eoi, str)) {
      pass = lex::pass_flags::pass_fail;
      return;
    }

    ctx.set_value(std::move(str));
  })];

  // symbol = R"([^\s]*)";
  //
  // 不能这么写正则表达式，会报错：
  //
  // terminate called after throwing an instance of
  // 'boost::lexer::runtime_error' what():  Mismatch in charset negation
  // preceding index 4.
  // Aborted

  symbol = R"([^\t\n\r\f\v ()\[\]{}"'#]+)";
  self += symbol;
}

bool
Lexer::match_string(Iterator& iter, Iterator& eoi, std::string& str)
{
  if (*iter == '"') {
    while (true) {
      if (++iter == eoi)
        return false;

      if (*iter == '"') {
        ++iter;
        return true;
      }

      if (*iter == '\\') {
        if (++iter == eoi)
          return false;

        switch (*iter) {
          case '"':
          case '\\':
            str.push_back(*iter);
            break;

          case 't':
            str.push_back('\t');
            break;

          case 'n':
            str.push_back('\n');
            break;

          case 'x': {
            if (++iter == eoi)
              return false;
            auto hi = *iter;

            char c;
            if (hi >= '0' && hi <= '9')
              c = hi - '0';
            else if (hi >= 'A' && hi <= 'F')
              c = hi - 'A' + 10;
            else if (hi >= 'a' && hi <= 'f')
              c = hi - 'a' + 10;
            else
              return false;
            c <<= 4;

            if (++iter == eoi)
              return false;
            auto lo = *iter;

            if (lo >= '0' && lo <= '9')
              c |= lo - '0';
            else if (lo >= 'A' && lo <= 'F')
              c |= lo - 'A' + 10;
            else if (lo >= 'a' && lo <= 'f')
              c |= lo - 'a' + 10;
            else
              return false;

            str.push_back(c);
          } break;

          default:
            return false;
        }

        continue;
      }

      str.push_back(*iter);
    }
  }

  if (*iter == '\'') {
    std::string tag;
    while (true) {
      if (++iter == eoi)
        return false;

      if (*iter == '"') {
        tag.push_back('\''); // 方便匹配结束标签
        break;
      }

      tag.push_back(*iter);
    }

    while (true) {
      if (++iter == eoi) // 第一次进入循环时 iter 一定指向双引号
        return false;

      if (*iter == '"') {
        auto tagi = tag.begin();
        while (tagi != tag.end()) {
          if (++iter == eoi)
            return false;

          if (*iter != *tagi) {
            str.push_back('"');
            for (auto i = tag.begin(); i != tagi; ++i)
              str.push_back(*i);
            goto CONTINUE;
          }

          ++tagi;
        }

        return true;
      }

    CONTINUE:
      str.push_back(*iter);
    }
  }

  return false;
}

Lexer const lexer;

// ==========================================================================
// 语法分析
// ==========================================================================

using LexerIter = Lexer::iterator_type;

template<typename... Ts>
using Rule = qi::rule<LexerIter, Ts...>;

class Parser : public qi::grammar<LexerIter, ast::Expr>
{
public:
  Rule<ast::Expr> expr;
  Rule<ast::Eval> eval;
  Rule<ast::MakeVec> makeVec;
  Rule<ast::MakeMap> makeMap;

public:
  Parser()
    : base_type(expr)
  {
    expr %=
      eval | makeVec | makeMap | lexer.number | lexer.string | lexer.symbol;
    eval %= '(' >> *expr >> ')';
    makeVec = '[' >> *expr >> ']';
    makeMap = '{' >> *('(' >> expr >> expr >> ')') >> '}';
  }
};

Parser const parser;

}

bool
parse(Iterator& begin, Iterator const& end, ast::Expr& expr)
{
  return lex::tokenize_and_parse(begin, end, lexer, parser, expr);
}

}
