#include "mode_check.hpp"
#include "parsing_tree.hpp"
#include "type_check.hpp"

#include <iostream>

auto make_program(bool uniq) {
  using namespace nodes;
  return make_expr<Let>(
      Arg("f"),
      lambda1(Arg("x", uniq ? types::Mode(types::Mode::Uniq::UNIQUE) : types::Mode()),
              operator_call("+", make_expr<Var>("x"), make_expr<Var>("x"))),
      make_expr<Var>("f"));
}

void add_builtin_functions_types(type_check::State &state, bool uniq) {
  auto sum_type = state.type_storage.add(types::make_operator(
      state.type_storage.get_int_type(uniq ? types::Mode(types::Mode::Uniq::UNIQUE) : types::Mode()),
      state.type_storage.get_int_type(uniq ? types::Mode(types::Mode::Uniq::UNIQUE) : types::Mode()),
      state.type_storage.get_int_type()));
  state.manager.add_var("+", sum_type);
}

void add_builtin_functions_modes(mode_check::State &state) {
  state.add_var("+"); // mode ??
}

void print_error(const std::string &general_message,
                 const utils::Error &error) {
  std::cerr << general_message << " "
            << "file: " << error.location.file_name() << "("
            << error.location.line() << ":" << error.location.column() << ") `"
            << error.location.function_name() << "`: " << error.message << std::endl;
}

int run_example(bool arg_uniq, bool sum_uniq) {
  const auto program = make_program(arg_uniq);

  try {
    type_check::State state;

    add_builtin_functions_types(state, sum_uniq);

    type_check::check_expr(program, state);

  } catch (utils::Error error) {
    print_error("TYPE CHECK ERROR:", error);
    return 1;
  }

  try {
    mode_check::State state;

    add_builtin_functions_modes(state);

    mode_check::check_expr(program, state);
  } catch (utils::Error error) {
    print_error("MODE CHECK ERROR:", error);
    return 1;
  }

  std::cout << "CHECK DONE\n";

  return 0;
}

int main() {
  int n = 0;

  while(true) {
    std::cout << "--- START TEST ---\n";
    std::cout << "TEST ID (0 - 3): ";
    std::cin >> n;
    if (n < 0 or n >= 4) {
      break;
    }
    run_example(n % 2 == 1, n / 4 == 1);
    std::cout << "--- END TEST ---\n";
  }
}
