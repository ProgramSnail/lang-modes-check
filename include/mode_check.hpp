#pragma once

#include "parsing_tree.hpp"

#include <map>
#include <source_location>

namespace mode_check {

using namespace types;

struct VarState {
  VarState(Mode mode, size_t count = 0) : mode(mode), count(count) {}

  Mode mode;
  size_t count = 0;
};

struct State {
  friend struct Context;

  State() { vars_stack.emplace_back(); }

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

    utils::throw_error("NO_VAR");
    return std::nullopt;
  }

  void add_var(std::string name, Mode mode = Mode()) {
    vars_stack.back().insert({std::move(name), VarState{mode}});
    // TODO: check existance
  }

private:
  void enter_context() { vars_stack.emplace_back(); }

  void exit_context() { vars_stack.pop_back(); }

private:
  vector<map<string, VarState>> vars_stack;
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

} // mode_check
