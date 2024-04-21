#pragma once

#include "parsing_tree.hpp"

using namespace nodes;

inline std::ostream &operator<<(std::ostream &, const Expr &);

inline std::ostream &operator<<(std::ostream &out, const Arg &expr) {
  out << expr.name << (expr.mode_hint.uniq == types::Mode::Uniq::UNIQUE ? "<unique>" : ""); // TODO: all modes
  return out;
}

inline std::ostream &operator<<(std::ostream &out, const Const &expr) {
  out << expr.value;
  return out;
}

inline std::ostream &operator<<(std::ostream &out, const Var &expr) {
  out << expr.name;
  return out;
}

inline std::ostream &operator<<(std::ostream &out, const Let &expr) {
  out << "let " << expr.name << " = " << *expr.body << " in " << *expr.where;
  return out;
}

inline std::ostream &operator<<(std::ostream &out, const Lambda &expr) {
  out << "\\";
  for (const auto &arg : expr.args) {
    out << arg << " ";
  }
  out << "-> " << *expr.expr;
  return out;
}

inline std::ostream &operator<<(std::ostream &out, const Call &expr) {
  out << *expr.func;
  for (const auto &arg : expr.args) {
    out << " " << *arg;
  }
  return out;
}

inline std::ostream &operator<<(std::ostream &out, const Condition &expr) {
  out << "if " << *expr.condition << " then " << *expr.then_case << " else "
      << *expr.else_case;
  return out;
}

inline std::ostream &operator<<(std::ostream &out, const Expr &expr) {
  switch (expr.value.index()) {
  case 0: // Const
    out << std::get<0>(expr.value);
    break;
  case 1: // Var
    out << std::get<1>(expr.value);
    break;
  case 2: // Let
    out << std::get<2>(expr.value);
    break;
  case 3: // Lambda
    out << std::get<3>(expr.value);
    break;
  case 4: // Call
    out << std::get<4>(expr.value);
    break;
  case 5: // Condition
    out << std::get<5>(expr.value);
    break;
  default:
    utils::unreachable();
  }
  return out;
}
