#pragma once

#include "parsing_tree.hpp"

#include <map>
#include <source_location>

namespace type_check {

using namespace types;

struct VarManager {
  friend struct Context;

  VarManager() { vars_stack.emplace_back(); }

  optional<TypeID> get_var_type(const std::string &name,
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
      return var_it->second;
    }

    utils::throw_error("NO_VAR for " + name);
    return std::nullopt;
  }

  void add_var(std::string name, TypeID type) {
    vars_stack.back().insert({std::move(name), type});
    // TODO: check existance
  }

private:
  void enter_context() { vars_stack.emplace_back(); }

  void exit_context() { vars_stack.pop_back(); }

private:
  vector<map<string, TypeID>> vars_stack;
};

struct Context {
  Context(VarManager &manager) : manager_(manager) { manager_.enter_context(); }

  ~Context() { manager_.exit_context(); }

private:
  VarManager &manager_;
};

// ---------------

struct State {
  types::Storage type_storage;
  VarManager manager;
};

// struct GenericVarContext {
//     GenericVarContext() { /*introduce generic*/ }
//     ~GenericVarContext() { /*resolve generic (two ways: as let, or as func
//     arg)*/ }
// };

types::TypeID check_expr(nodes::ExprPtr expr, State &state);

} // namespace type_check
