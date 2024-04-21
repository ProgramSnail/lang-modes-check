#include "mode_check.hpp"
#include "parsing_tree.hpp"
#include "type_check.hpp"

#include <iostream>

auto make_program() {
  using namespace nodes;
  return make_expr<Let>(Arg("f"),
                        lambda1(with_unique(Arg("x")),
                                operator_call("+", make_var("x", types::Mode::Uniq::UNIQUE),
                                              make_var("x", types::Mode::Uniq::UNIQUE))),
                        make_var("f"));
}

void add_builtin_functions_types(type_check::State &state) {
  auto sum_type = state.type_storage.add(types::make_operator(
      state.type_storage.get_int_type(), state.type_storage.get_int_type(),
      state.type_storage.get_int_type()));
  state.manager.add_var("+", sum_type);
}

void add_builtin_functions_modes(mode_check::State &state) {
  state.add_var("+");
}

void print_error(const std::string &general_message,
                 const utils::Error &error) {
  std::cerr << general_message << " "
            << "file: " << error.location.file_name() << "("
            << error.location.line() << ":" << error.location.column() << ") `"
            << error.location.function_name() << "`: " << error.message;
}

int main() {
  const auto program = make_program();

  try {
    type_check::State state;

    add_builtin_functions_types(state);

    type_check::check_expr(program, state);

  } catch (utils::Error error) {
    print_error("TYPE CHECK ERROR:", error);
  }

  try {
    mode_check::State state;

    add_builtin_functions_modes(state);

    mode_check::check_expr(program, state);
  } catch (utils::Error error) {
    print_error("MODE CHECK ERROR:", error);
  }
}
