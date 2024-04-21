#include "type_check.hpp"

#include "parsing_tree.hpp"

// TODO

namespace type_check {

using namespace types;

// C++ 23
[[noreturn]] inline void unreachable() {
  // Uses compiler specific extensions if possible.
  // Even if no extension is used, undefined behavior is still raised by
  // an empty function body and the noreturn attribute.
#if defined(_MSC_VER) && !defined(__clang__) // MSVC
  __assume(false);
#else // GCC, Clang
  __builtin_unreachable();
#endif
}

// bool eq(Type x, Type y) {
//   if (x.type.index() != y.type.index()) {
//     return false;
//   }

//   if (x.type.index() != Type::ARROW_TYPE_INDEX) {
//     return true;
//   }

//   const auto &x_types = std::get<Type::ARROW_TYPE_INDEX>(x.type);
//   const auto &y_types = std::get<Type::ARROW_TYPE_INDEX>(y.type);

//   if (x_types.size() != )
// }

// -----------------

template <typename T>
using Typechecked = std::pair<T /*expr*/, types::Type /*type*/>;

Typechecked<nodes::ExprPtr> typecheck_expr(nodes::ExprPtr expr);

Typechecked<nodes::Const> typecheck_const(nodes::Const expr) {
  // TODO
  return {std::move(expr), make_type<AnyType>()};
}

Typechecked<nodes::Var> typecheck_var(nodes::Var expr) {
  // TODO
  return {std::move(expr), make_type<AnyType>()};
}

Typechecked<nodes::Let> typecheck_let(nodes::Let expr) {
  // TODO
  return {std::move(expr), make_type<AnyType>()};
}

Typechecked<nodes::Lambda> typecheck_lambda(nodes::Lambda expr) {
  // TODO
  return {std::move(expr), make_type<AnyType>()};
}

Typechecked<nodes::Call> typecheck_call(nodes::Call expr) {
  // TODO
  return {std::move(expr), make_type<AnyType>()};
}

Typechecked<nodes::Condition> typecheck_condition(nodes::Condition expr) {
  // const auto [condition_expr, condition_type] = typecheck_expr(expr.condition);
  // expr.condition = std::move(condition_expr);

  // const auto [then_case_expr, then_case_type] = typecheck_expr(expr.then_case);
  // expr.then_case = std::move(then_case_expr);

  // const auto [else_case_expr, else_type_type] = typecheck_expr(expr.else_case);
  // expr.else_case = std::move(else_case_expr);

  return {std::move(expr), make_type<AnyType>()};
}

Typechecked<nodes::ExprPtr> typecheck_expr(nodes::ExprPtr expr) {
  types::Type type;

  switch (expr->value.index()) {
  case 0: // Const
    std::tie(expr->value, type) = typecheck_const(std::get<0>(expr->value));
    break;
  case 1: // Var
    std::tie(expr->value, type) = typecheck_var(std::get<1>(expr->value));
    break;
  case 2: // Let
    std::tie(expr->value, type) = typecheck_let(std::get<2>(expr->value));
    break;
  case 3: // Lambda
    std::tie(expr->value, type) = typecheck_lambda(std::get<3>(expr->value));
    break;
  case 4: // Call
    std::tie(expr->value, type) = typecheck_call(std::get<4>(expr->value));
    break;
  case 5: // Condition
    std::tie(expr->value, type) = typecheck_condition(std::get<5>(expr->value));
    break;
  default:
    unreachable();
  }

  return {std::move(expr), std::move(type)};
}

} // namespace type_check
