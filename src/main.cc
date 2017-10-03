#include <string>
#include <iostream>
#include "type.hpp"
#include "checker.hpp"

using namespace std;
using namespace ast;
using namespace type;
using namespace checker;

auto try_analyse(shared_ptr<Node> expr, environment env) -> shared_ptr<Type> {
  try {
    return analyse(expr, env);
  } catch (std::runtime_error &e) {
    cout << "runtime error: " << e.what () << endl;
    return nullptr;
  }
}

auto get_type(shared_ptr<Node> expr, environment env) -> void {
  auto type = try_analyse(expr, env);
  cout << (type == nullptr) << endl;
  if (type != nullptr)
    cout << type->to_string() << endl;
}

int main(int argc, char** argv) {
  auto var1 = make_shared<TypeVariable>();
  auto var2 = make_shared<TypeVariable>();
  auto var3 = make_shared<TypeVariable>();

  auto pair_type = make_shared<TypeOperator>("*", vector<shared_ptr<Type>>({ var1, var2 }));
  environment env = {
    { "true", BooleanType },
    { "pair", FunctionType(var1, FunctionType(var2, pair_type)) },
    { "cond", FunctionType(BooleanType, FunctionType(var3, FunctionType(var3, var3))) },
    { "pred", FunctionType(IntegerType, IntegerType) },
    { "zero?", FunctionType(IntegerType, BooleanType) },
    { "times", FunctionType(IntegerType, FunctionType(IntegerType, IntegerType)) }
  };

  auto pair = make_shared<Apply>(make_shared<Apply>(make_shared<Identifier>("pair"), make_shared<Apply>(make_shared<Identifier>("f"), make_shared<Identifier>("3"))),
                                 make_shared<Apply>(make_shared<Identifier>("f"), make_shared<Identifier>("true")));

  // recursion
  auto recursion_expr = make_shared<Letrec>("factorial",
                                            make_shared<Lambda>("n",
                                                                make_shared<Apply>(make_shared<Apply>(make_shared<Apply>(make_shared<Identifier>("cond"),
                                                                                                                         make_shared<Apply>(make_shared<Identifier>("zero?"),
                                                                                                                                            make_shared<Identifier>("n"))),
                                                                                                      make_shared<Identifier>("1")),
                                                                                   make_shared<Apply>(make_shared<Apply>(make_shared<Identifier>("times"),
                                                                                                                         make_shared<Identifier>("n")),
                                                                                                      make_shared<Apply>(make_shared<Identifier>("factorial"),
                                                                                                                         make_shared<Apply>(make_shared<Identifier>("pred"),
                                                                                                                                            make_shared<Identifier>("n")))))),
                                            make_shared<Apply>(make_shared<Identifier>("factorial"), make_shared<Identifier>("5")));

  // let polymorphism
  auto let_poly_expr = make_shared<Let>("f", make_shared<Lambda>("x", make_shared<Identifier>("x")), pair);

  // lambda param is non generic
  auto fail_expr = make_shared<Lambda>("x",
                                       make_shared<Apply>(make_shared<Apply>(make_shared<Identifier>("pair"),
                                                                             make_shared<Apply>(make_shared<Identifier>("x"),
                                                                                                make_shared<Identifier>("3"))),
                                                          make_shared<Apply>(make_shared<Identifier>("x"),
                                                                             make_shared<Identifier>("true"))));

  auto pair_expr = make_shared<Apply>(make_shared<Apply>(make_shared<Identifier>("pair"),
                                                         make_shared<Apply>(make_shared<Identifier>("f"),
                                                                            make_shared<Identifier>("3"))),
                                      make_shared<Apply>(make_shared<Identifier>("f"),
                                                         make_shared<Identifier>("true")));

  // recursive unification
  auto infinite_expr = make_shared<Lambda>("f",
                                           make_shared<Apply>(make_shared<Identifier>("f"),
                                                              make_shared<Identifier>("f")));

  auto lazy_expr = make_shared<Let>("g",
                                    make_shared<Lambda>("f", make_shared<Identifier>("5")),
                                    make_shared<Apply>(make_shared<Identifier>("g"),
                                                       make_shared<Identifier>("g")));

  // funciton composition
  auto compose_expr = make_shared<Lambda>("f",
                                          make_shared<Lambda>("g",
                                                              make_shared<Lambda>("arg",
                                                                                  make_shared<Apply>(make_shared<Identifier>("f"),
                                                                                                     make_shared<Apply>(make_shared<Identifier>("g"),
                                                                                                                        make_shared<Identifier>("arg"))))));

  cout << compose_expr->to_string() << endl;

  get_type(recursion_expr, env);
  get_type(let_poly_expr, env);
  get_type(fail_expr, env);
  get_type(pair_expr, env);
  get_type(infinite_expr, env);
  get_type(lazy_expr, env);
  get_type(compose_expr, env);

  return 0;
}
