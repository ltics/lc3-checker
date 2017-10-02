#pragma once

#include <string>
#ifndef FORMAT_HEADER
#define FORMAT_HEADER
#include <fmt/format.h>
#include <fmt/format.cc>
#endif

using namespace std;
using namespace fmt;

namespace ast {
  enum class NodeType : size_t {
    LAMBDA,
    IDENTIFIER,
    APPLY,
    LET,
    LETREC
  };


  class Node {
  public:
    virtual NodeType type() = 0;
    virtual string to_string() = 0;
  };

  class Lambda : public Node {
  public:
    string param;
    shared_ptr<Node> body;

    Lambda(string p,
           shared_ptr<Node> b)
      : param(p), body(b) {};

    NodeType type() {
      return NodeType::LAMBDA;
    }

    string to_string() {
      return format("(λ{0}. {1})", param, body->to_string());
    }
  };

  class Identifier : public Node {
  public:
    string name;

    Identifier(string n): name(n) {};

    NodeType type() {
      return NodeType::IDENTIFIER;
    }

    string to_string() {
      return this->name;
    }
  };

  class Apply : public Node {
  public:
    shared_ptr<Node> func;
    shared_ptr<Node> arg;

    Apply(shared_ptr<Node> f,
          shared_ptr<Node> a)
      : func(f), arg(a) {};

    NodeType type() {
      return NodeType::APPLY;
    }

    string to_string() {
      return format("({0} {1})", func->to_string(), arg->to_string());
    }
  };

  // let ... in ...
  class Let : public Node {
  public:
    string name;
    shared_ptr<Node> defn;
    shared_ptr<Node> body;

    Let(string name,
        shared_ptr<Node> defn,
        shared_ptr<Node> body)
      : name(name), defn(defn), body(body) {};

    NodeType type() {
      return NodeType::LET;
    }

    string to_string() {
      return format("(let {0} = {1} in {2})", name, defn->to_string(), body->to_string());
    }
  };

  class Letrec : public Node {
  public:
    string name;
    shared_ptr<Node> defn;
    shared_ptr<Node> body;

    Letrec(string name,
        shared_ptr<Node> defn,
        shared_ptr<Node> body)
      : name(name), defn(defn), body(body) {};

    NodeType type() {
      return NodeType::LETREC;
    }

    string to_string() {
      return format("(letrec {0} = {1} in {2})", name, defn->to_string(), body->to_string());
    }
  };
}
