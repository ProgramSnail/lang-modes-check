#include "mode_check.hpp"

#include "parsing_tree.hpp"

#include <map>
#include <source_location>

namespace mode_check {

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

using namespace types;

struct VarState {
  Type type;
  size_t count = 0;
};

struct ModeError {
  enum Error {
    UNKNOWN,
    NO_VAR,
    NO_VAR_TYPE,
    NO_TYPE,
    WRONG_TYPE,
    LOCAL,
    UNIQUE,
    EXCL,
    ONCE,
    SEP
  } error = UNKNOWN;

  source_location location;

  ModeError(Error error, source_location location)
      : type(type), location(location) {}
};

struct State {
  friend struct Context;

  State() { vars_stack.emplace_back(); }

  void set_error(ModeError::Error error,
                 source_location location = source_location::current()) {
    if (first_error.has_value()) {
      return;
    }

    first_error = ModeError(std::move(error), location);
  }

  std::optional<VarState *> get_var_state(const std::string &name,
                                          bool last_context_only = false) {
    for (auto vars_it = vars_stack.rbegin(); vars_it != vars_stack.rend();
         ++vars_it) {
      auto var_it = vars_it->find(name);
      if (var_it == vars_it->end()) {
        if (last_context_only) {
          break;
        }

        continue;
      }
      return &var_it->second;
    }

    set_error(ModeError::NO_VAR);
    return std::nullopt;
  }

  void add_var(std::string name, Type type) {
    vars_stack.back().insert({std::move(name), VarState{std::move(type)}});
    // TODO: check existance
  }

  std::optional<ModeError> get_first_error() { return first_error; }

private:
  void enter_context() { vars_stack.emplace_back(); }

  void exit_context() { vars_stack.pop_back(); }

private:
  vector<map<string, VarState>> vars_stack;
  std::optional<ModeError> first_error;
};

struct Context {
  Context(State &state) : state_(state) { state_.enter_context(); }

  ~Context() { state_.exit_context(); }

private:
  State &state_;
};

// struct ExclVarScope {
//   ExclVarScope(std::string name, State &state)
//       : name_(std::move(name)), state_(state) {}

//   ~ExclVarScope() {}

// private:
//   std::string name_;
//   State &state_;
// };

void check_expr(nodes::ExprPtr expr, State &state);

void check_const(const nodes::Const &, State &) {}

void check_var(const nodes::Var &expr, State &state) {
  if (not expr.type.has_value()) {
    state.set_error(ModeError::NO_TYPE);
    return;
  }
  auto type = expr.type.value();

  if (auto maybe_var_state = state.get_var_state(expr.name);
      maybe_var_state.has_value()) {
    auto &var_state = *maybe_var_state.value();

    if (var_state.type.uniq == Type::Uniq::UNIQUE) {
      ++var_state.count;
      if (var_state.count > 1 || type.uniq != Type::Uniq::UNIQUE) {
        state.set_error(ModeError::UNIQUE);
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

    if (not expr.name.type.has_value()) {
      state.set_error(ModeError::NO_VAR_TYPE);
    }
    state.add_var(expr.name.name, expr.name.type.value());

    check_expr(expr.where, state);
  }
}

void check_lambda(const nodes::Lambda &expr, State &state) {
  Context context(state);

  for (const auto &arg : expr.args) {
    if (not arg.type.has_value()) {
      state.set_error(ModeError::NO_VAR_TYPE);
      continue;
    }
    state.add_var(arg.name, arg.type.value());
  }

  check_expr(expr.expr, state);
}

void check_call(const nodes::Call &expr, State &state) {
  // if (not expr.type.has_value()) {
  //   state.set_error(ModeError::NO_TYPE);
  //   return;
  // }
  // auto type = expr.type.value();

  // if (not holds_alternative<types::ArrowType>(type.type)) {
  //   state.set_error(ModeError::WRONG_TYPE);
  //   return;
  // }

  // const auto &arrow_type = get<types::ArrowType>(type.type);

  // if (arrow_type.types.size() != expr.args.size() + 1) {
  //   state.set_error(ModeError::WRONG_TYPE);
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
    unreachable();
  }
}

} // namespace mode_check
