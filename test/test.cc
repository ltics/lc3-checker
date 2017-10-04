#define CATCH_CONFIG_MAIN  // This tells Catch to provide a main() - only do this in one cpp file
#include "catch.hpp"
#include "../src/ast.hpp"
#include "../src/type.hpp"
#include "../src/checker.hpp"
#include <vector>

using namespace std;
using namespace ast;
using namespace type;
using namespace checker;

TEST_CASE("basic type inference") {
  struct GoodTestCase {
    shared_ptr<Node> input;
    shared_ptr<Type> expected;
  };

  struct BadTestCase {
    shared_ptr<Node> input;
    string error_msg;
  };

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

  auto compose_type = FunctionType(FunctionType(var2, var3), FunctionType(FunctionType(var1, var2), FunctionType(var1, var3)));

  vector<GoodTestCase> good_tests = {
    { recursion_expr, IntegerType },
    { let_poly_expr, make_shared<TypeOperator>("*", vector<shared_ptr<Type>>({ IntegerType, BooleanType })) },
    { lazy_expr, IntegerType },
    { compose_expr, compose_type }
  };

  vector<BadTestCase> bad_tests = {
    { fail_expr, "Type mismatch: bool != int" },
    { pair_expr, "Undefined symbol f" },
    { infinite_expr, "Recursive unification" }
  };

  std::for_each(good_tests.cbegin(), good_tests.cend(), [=](const GoodTestCase &c) {
      auto type = normalize(analyse(c.input, env));
      REQUIRE(type->to_string() == c.expected->to_string());
    });

  auto get_error_msg = [](shared_ptr<Node> expr, environment env) -> string {
    try {
      analyse(expr, env);
      return "";
    } catch (std::runtime_error &e) {
      return e.what();
    }
  };

  std::for_each(bad_tests.cbegin(), bad_tests.cend(), [=](const BadTestCase &c) {
      auto msg = get_error_msg(c.input, env);
      REQUIRE(msg == c.error_msg);
    });
}
