#include <string>
#ifndef FORMAT_HEADER
#define FORMAT_HEADER
#include <fmt/format.h>
#include <fmt/format.cc>
#endif

using namespace std;
using namespace fmt;

namespace ast {
  class Node {
  public:
    virtual string to_string() = 0;
  };

  class Lambda : public Node {
  public:
    string param;
    shared_ptr<Node> body;

    Lambda(string p,
           shared_ptr<Node> b)
      : param(p), body(b) {};

    string to_string() {
      return format("(λ{0}. {1})", param, body->to_string());
    }
  };

  class Identifier : public Node {
  public:
    string name;

    Identifier(string n): name(n) {};

    string to_string() {
      return this->name;
    }
  };

  class Apply : public Node {
  public:
    shared_ptr<Node> func;
    shared_ptr<Node> arg;

    Apply(f, a): func(f), arg(a) {};

    string to_string() {
      return format("({0} {1})", func->to_string(), arg->to_string());
    }
  };

  // let ... in ...
  class Let : public Node {
  public:
    string name;
    shared_ptr<Node> func;
    shared_ptr<Node> body;

    Let(string name,
        shared_ptr<Node> func,
        shared_ptr<Node> body)
      : name(name), func(func), body(body) {};

    string to_string() {
      return format("(let {0} = {1} in {2})", name, func->to_string(), body->to_string());
    }
  };

  class Letrec : public Node {
  public:
    string name;
    shared_ptr<Node> func;
    shared_ptr<Node> body;

    Letrec(string name,
        shared_ptr<Node> func,
        shared_ptr<Node> body)
      : name(name), func(func), body(body) {};

    string to_string() {
      return format("(letrec {0} = {1} in {2})", name, func->to_string(), body->to_string());
    }
  };
}
