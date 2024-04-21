#include "parsing_tree.hpp"

using namespace nodes;

int main() {
  const auto program =
      Expr{Let{Arg{"f", {}},
               lambda1("x", operator_call("+", make_expr<Var>("x"),
                                          make_expr<Var>("x"))),
               make_expr<Var>("f")}};
}
