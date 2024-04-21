#pragma once

#include <memory>
#include <optional>
#include <string>
#include <variant>
#include <vector>

#include "utils.hpp"
#include "types.hpp"

namespace nodes {

using namespace std;

struct NodeInfo {
  optional<types::TypeID> type = std::nullopt;
};

struct Expr;
using ExprPtr = shared_ptr<Expr>;
using ExprPtrV = std::vector<ExprPtr>;

struct Arg : public NodeInfo {
  Arg(string name, types::Mode mode_hint = {}) : name(std::move(name)), mode_hint(mode_hint) {}

  string name;

  types::Mode mode_hint;
};

struct Const : public NodeInfo {
  Const(int value) : value(value) {}

  int value;
};

struct Var : public NodeInfo {
  Var(string name) : name(std::move(name)) {}

  string name;
};

struct Let : public NodeInfo {
  Let(Arg name, ExprPtr body, ExprPtr where)
      : name(std::move(name)), body(body), where(where) {}

  Arg name;
  ExprPtr body;
  ExprPtr where;
};

struct Lambda : public NodeInfo {
  Lambda(vector<Arg> args, ExprPtr expr) : args(std::move(args)), expr(expr) {}

  vector<Arg> args;
  ExprPtr expr;
};

struct Call : public NodeInfo {
  Call(ExprPtr func, vector<ExprPtr> args)
      : func(func), args(std::move(args)) {}

  ExprPtr func;
  vector<ExprPtr> args;
};

struct Condition : public NodeInfo {
  Condition(ExprPtr condition, ExprPtr then_case, ExprPtr else_case)
      : condition(condition), then_case(then_case), else_case(else_case) {}

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

template <typename T, typename... Args> ExprPtr make_expr(Args &&...args) {
  return std::make_shared<Expr>(T(std::forward<Args>(args)...));
}

template <typename T> inline T with_type(T node, types::Type type) {
  node.type = std::move(type);
  return node;
}

inline Arg with_mode_hint(Arg arg, types::Mode mode) {
  arg.mode_hint = mode;
  return arg;
}

inline Arg with_unique_hint(Arg arg) {
  return with_mode_hint(std::move(arg), types::Mode(types::Mode::Uniq::UNIQUE));
}

inline ExprPtr lambda1(string var, ExprPtr expr) {
  return make_expr<Lambda>(vector<Arg>{Arg(var)}, std::move(expr));
}

inline ExprPtr lambda1(Arg var, ExprPtr expr) {
  return make_expr<Lambda>(vector<Arg>{var}, std::move(expr));
}

inline ExprPtr operator_call(string name, ExprPtr left, ExprPtr right) {
  return make_expr<Call>(make_expr<Var>(name), ExprPtrV{left, right});
}

// TODO: all constructors

} // namespace nodes
