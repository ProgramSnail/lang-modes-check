#include "mode_check.hpp"

namespace mode_check {

void check_const(const nodes::Const &, State &) {}

void check_var(const nodes::Var &expr, State &state) {
  if (not expr.mode.has_value()) {
    utils::throw_error("NO_MODE for " + expr.name);
    return;
  }
  auto mode = expr.mode.value();

  if (auto maybe_var_state = state.get_var_state(expr.name);
      maybe_var_state.has_value()) {
    auto &var_state = *maybe_var_state.value();

    if (var_state.mode.uniq == Mode::Uniq::UNIQUE) {
      ++var_state.count;
      if (var_state.count > 1 || mode.uniq != Mode::Uniq::UNIQUE) {
        utils::throw_error("UNIQUE for " + expr.name);
        return;
      }
    }
  }
  // TODO: other modes
}

void check_let(const nodes::Let &expr, State &state) {
  {
    Context context(state);
    check_expr(expr.body, state);
  }

  {
    Context context(state);

    if (not expr.name.mode.has_value()) {
      utils::throw_error("NO_VAR_MODE");
    }
    state.add_var(expr.name.name, expr.name.mode.value());

    check_expr(expr.where, state);
  }
}

void check_lambda(const nodes::Lambda &expr, State &state) {
  Context context(state);

  for (const auto &arg : expr.args) {
    if (not arg.mode.has_value()) {
      utils::throw_error("NO_VAR_MODE");
      continue;
    }
    state.add_var(arg.name, arg.mode.value());
  }

  check_expr(expr.expr, state);
}

void check_call(const nodes::Call &expr, State &state) {
  // if (not expr.type.has_value()) {
  //   utils::throw_error("NO_TYPE");
  //   return;
  // }
  // auto type = expr.type.value();

  // if (not holds_alternative<types::ArrowType>(type.type)) {
  //   utils::throw_error("WRONG_TYPE");
  //   return;
  // }

  // const auto &arrow_type = get<types::ArrowType>(type.type);

  // if (arrow_type.types.size() != expr.args.size() + 1) {
  //   utils::throw_error("WRONG_TYPE");
  //   return;
  // }

  // not required, because types are propogated to Vars in type check

  check_expr(expr.func, state);

  for (const auto &arg : expr.args) {
    check_expr(arg, state);
  }
}

void check_condition(const nodes::Condition &expr, State &state) {
  check_expr(expr.condition, state);
  check_expr(expr.then_case, state);
  check_expr(expr.else_case, state);
}

void check_expr(nodes::ExprPtr expr, State &state) {
  switch (expr->value.index()) {
  case 0: // Const
    check_const(std::get<0>(expr->value), state);
    break;
  case 1: // Var
    check_var(std::get<1>(expr->value), state);
    break;
  case 2: // Let
    check_let(std::get<2>(expr->value), state);
    break;
  case 3: // Lambda
    check_lambda(std::get<3>(expr->value), state);
    break;
  case 4: // Call
    check_call(std::get<4>(expr->value), state);
    break;
  case 5: // Condition
    check_condition(std::get<5>(expr->value), state);
    break;
  default:
    utils::unreachable();
  }
}

} // namespace mode_check
