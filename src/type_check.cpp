#include "type_check.hpp"

namespace type_check {

types::TypeID check_const(nodes::Const &expr, State &state) {
  return (expr.type = state.type_storage.get_int_type()).value();
}

types::TypeID check_var(nodes::Var &expr, State &state) {
  if (auto maybe_var_type = state.manager.get_var_type(expr.name);
      maybe_var_type.has_value()) {
    return (expr.type = maybe_var_type).value();
  }
  utils::unreachable();
}

types::TypeID check_let(nodes::Let &expr, State &state) {
  Context context(state.manager);

  types::TypeID new_type = state.type_storage.introduce_new_generic();

  state.manager.add_var(expr.name.name, new_type);

  types::TypeID body_type = check_expr(expr.body, state);

  if (not state.type_storage.unify(new_type, body_type)) {
    utils::throw_error("DIFFERENT_TYPES");
  }

  types::TypeID where_type = check_expr(expr.where, state);
  return (expr.type = where_type).value();
}

types::TypeID check_lambda(nodes::Lambda &expr, State &state) {
  Context context(state.manager);

  for (const auto &arg : expr.args) {
    types::TypeID new_type = state.type_storage.introduce_new_generic();
    state.manager.add_var(arg.name, new_type);
  }

  types::TypeID lambda_type = check_expr(expr.expr, state);
  return (expr.type = lambda_type).value();
}

types::TypeID check_call(nodes::Call &expr, State &state) {
  types::TypeID func_type = check_expr(expr.func, state);

  if (auto *arrow_func_type = get_if<types::ArrowType>(&func_type.get().type);
      arrow_func_type != nullptr) {

    if (arrow_func_type->types.size() != expr.args.size() + 1) {
      utils::throw_error("ARG_COUNT_MISMATCH");
    }

    for (size_t i = 0; i < expr.args.size(); ++i) {
      types::TypeID arg_type = check_expr(expr.args[i], state);
      if (not state.type_storage.unify(arrow_func_type->types[i], arg_type)) {
        utils::throw_error("DIFFERENT_TYPES");
      }
    }

    return (expr.type = arrow_func_type->types.back()).value();
  }

  utils::throw_error("FUNC_IS_NOT_ARROW_TYPE");
  utils::unreachable();
}

types::TypeID check_condition(nodes::Condition &expr, State &state) {
  types::TypeID condition_type = check_expr(expr.condition, state);

  if (not state.type_storage.unify(condition_type,
                                   state.type_storage.get_bool_type())) {
    utils::throw_error("DIFFERENT_TYPES");
  }

  types::TypeID then_type = check_expr(expr.then_case, state);
  types::TypeID else_type = check_expr(expr.else_case, state);

  if (not state.type_storage.unify(then_type, else_type)) {
    utils::throw_error("DIFFERENT_TYPES");
  }

  return (expr.type = then_type).value();
}

types::TypeID check_expr(nodes::ExprPtr expr, State &state) {
  switch (expr->value.index()) {
  case 0: // Const
    return check_const(std::get<0>(expr->value), state);
  case 1: // Var
    return check_var(std::get<1>(expr->value), state);
  case 2: // Let
    return check_let(std::get<2>(expr->value), state);
  case 3: // Lambda
    return check_lambda(std::get<3>(expr->value), state);
  case 4: // Call
    return check_call(std::get<4>(expr->value), state);
  case 5: // Condition
    return check_condition(std::get<5>(expr->value), state);
  default:
    utils::unreachable();
  }
}

} // namespace type_check
