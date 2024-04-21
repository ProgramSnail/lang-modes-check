#include "mode_check.hpp"
#include "parsing_tree.hpp"
#include "printers.hpp"
#include "type_check.hpp"

#include <iostream>

auto make_program_1(bool uniq) {
  using namespace nodes;
  return make_expr<Let>(
      Arg("f"),
      lambda1(Arg("x", uniq ? types::Mode(types::Mode::Uniq::UNIQUE)
                            : types::Mode()),
              operator_call("+", make_expr<Var>("x"), make_expr<Var>("x"))),
      make_expr<Var>("f"));
}

auto make_program_2(bool uniq) {
  using namespace nodes;
  return make_expr<Let>(
      Arg("f"),
      lambda2(Arg("x", uniq ? types::Mode(types::Mode::Uniq::UNIQUE)
                            : types::Mode()),
              Arg("y"),
              operator_call("+", make_expr<Var>("x"), make_expr<Var>("y"))),
      make_expr<Var>("f"));
}

auto make_program_3() {
  using namespace nodes;
  return make_expr<Let>(Arg("f"), lambda1(Arg("x"), make_expr<Var>("x")),
                        make_expr<Var>("f"));
}

void add_builtin_functions_types(type_check::State &state, bool uniq) {
  auto sum_type = state.type_storage.add(types::make_operator(
      state.type_storage.get_int_type(
          uniq ? types::Mode(types::Mode::Uniq::UNIQUE) : types::Mode()),
      state.type_storage.get_int_type(
          uniq ? types::Mode(types::Mode::Uniq::UNIQUE) : types::Mode()),
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
            << error.location.function_name() << "`: " << error.message
            << std::endl;
}

void run_example(const auto &make_program, bool arg_uniq, bool sum_uniq) {
  const auto program = make_program(arg_uniq);

  std::cout << "\x1b[1;34mPROGRAM:\x1b[0m \x1b[1;90m" << *program
            << "\x1b[0m\n";
  if (sum_uniq) {
    std::cout << "+: int<unique> -> int<unique> -> int\n";
  } else {
    std::cout << "+: int -> int -> int\n";
  }
  std::cout << "\n";

  try {
    type_check::State state;

    add_builtin_functions_types(state, sum_uniq);

    type_check::check_expr(program, state);

  } catch (utils::Error error) {
    print_error("\x1b[1;31mTYPE CHECK ERROR:\x1b[0m", error);
    return;
  }

  try {
    mode_check::State state;

    add_builtin_functions_modes(state);

    mode_check::check_expr(program, state);
  } catch (utils::Error error) {
    print_error("\x1b[1;31mMODE CHECK ERROR:\x1b[0m", error);
    return;
  }

  std::cout << "\x1b[1;92mPROGRAM IS CORRECT\x1b[0m\n";
}

void run_example_2() {
  const auto program = make_program_3();

  std::cout << "\n\x1b[1;34m--- TYPE CHECK ---\x1b[0m\n";
  std::cout << "\x1b[1;34mPROGRAM:\x1b[0m \x1b[1;90m" << *program
            << "\x1b[0m\n\n";

  try {
    type_check::State state;

    std::cout << "expression type is " << type_check::check_expr(program, state).get().type.index()
              << "\n";

    for (auto &type : state.type_storage.types) {
      std::cout << type.type.index();
      if (auto *arrow_type = get_if<types::ArrowType>(&type.type);
          arrow_type != nullptr) {
        std::cout << "[-";
        for (const auto &arg : arrow_type->types) {
          std::cout << arg.get().type.index() << "-";
        }
        std::cout << "]";
      }
      std::cout << ' ';
    }
  } catch (utils::Error error) {
    print_error("\x1b[1;31mTYPE CHECK ERROR:\x1b[0m", error);
    return;
  }

  std::cout << "\n\n\x1b[1;34m--- END ---\x1b[0m\n";
}

int main() {
  for (size_t n = 0; n < 8; ++n) {
    std::cout << "\n\x1b[1;34m--- TEST ---\x1b[0m\n";
    run_example(n / 4 == 0 ? &make_program_1 : &make_program_2, n % 2 == 1,
                (n / 2) % 2 == 1);
    std::cout << "\n\x1b[1;34m--- END ---\x1b[0m\n";
  }
  run_example_2();
}
