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
  optional<types::Mode> mode = std::nullopt;
};

struct Expr;
using ExprPtr = shared_ptr<Expr>;
using ExprPtrV = std::vector<ExprPtr>;

struct Arg : public NodeInfo {
  Arg(string name) : name(std::move(name)) {}

  string name;
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

template <typename T> inline T with_mode(T node, types::Mode mode) {
  node.mode = std::move(mode);
  return node;
}

template <typename T> inline T with_unique(T node) {
  return with_mode(node, types::Mode(types::Mode::Uniq::UNIQUE));
}

inline ExprPtr make_var(std::string name, types::Mode mode = types::Mode()) {
  return make_expr<Var>(with_mode(Var(std::move(name)), mode));
}

inline ExprPtr lambda1(string var, ExprPtr expr) {
  return make_expr<Lambda>(vector<Arg>{Arg(var)}, std::move(expr));
}

inline ExprPtr lambda1(Arg var, ExprPtr expr) {
  return make_expr<Lambda>(vector<Arg>{var}, std::move(expr));
}

inline ExprPtr operator_call(string name, ExprPtr left, ExprPtr right,
                             types::Mode mode = types::Mode()) {
  return make_expr<Call>(make_var(name, mode), ExprPtrV{left, right});
}

// TODO: all constructors

} // namespace nodes
