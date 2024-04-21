#pragma once

#include <memory>
#include <optional>
#include <string>
#include <variant>
#include <vector>

namespace types {

using namespace std;

struct Type;
using TypePtr = shared_ptr<Type>;

struct ArrowType {
  vector<TypePtr> types;
};

struct BoolType {};
struct IntType {};
// struct UnitType {};

struct AnyType {};

struct Type {
  static constexpr size_t ARROW_TYPE_INDEX = 0;
  variant<ArrowType, BoolType, IntType, AnyType> type;

  enum class Loc { GLOBAL, LOCAL } loc = Loc::GLOBAL;
  enum class Uniq { SHARED, UNIQUE, EXCL } uniq = Uniq::SHARED;
  enum class Lin { MANY, ONCE, SEP } lin = Lin::MANY;
};

template<typename T, typename... Args>
Type make_type(Args&&... args) {
  return Type{T{std::forward<Args>(args)...}};
}

} // namespace types

namespace nodes {

using namespace std;

struct Node {
  optional<types::Type> type = std::nullopt;
};

struct Expr;
using ExprPtr = shared_ptr<Expr>;
using ExprPtrV = std::vector<ExprPtr>;

struct Arg : public Node {
  string name;
};

struct Const : public Node {
  int value;
};

struct Var : public Node {
  string name;
};

struct Let : public Node {
  Arg name;
  ExprPtr body;
  ExprPtr where;
};

struct Lambda : public Node {

  vector<Arg> args;
  ExprPtr expr;
};

struct Call : public Node {
  ExprPtr func;
  vector<ExprPtr> args;
};

struct Condition : public Node {
  ExprPtr condition;
  ExprPtr then_case;
  ExprPtr else_case;
};

// struct FunctionDecl {
//   string name;
//   vector<Arg> args;
//   ExprPtr expr;
// };

struct Expr {
  variant<Const, Var, Let, Lambda, Call, Condition> value;
};

template<typename T, typename... Args>
ExprPtr make_expr(Args&&... args) {
  return std::make_shared<Expr>(T{std::forward<Args>(args)...});
}

static ExprPtr lambda1(string name, ExprPtr expr) {
  return make_expr<Lambda>(vector<Arg>{{name}}, std::move(expr));
}

ExprPtr operator_call(string name, ExprPtr left, ExprPtr right) {
  return make_expr<Call>(make_expr<Var>(name), ExprPtrV{left, right});
}

// TODO: all constructors

} // namespace nodes
