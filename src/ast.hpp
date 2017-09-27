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
}
