#include <string>
#include <vector>
#include <memory>
#include <range/v3/all.hpp>
#ifndef FORMAT_HEADER
#define FORMAT_HEADER
#include <fmt/format.h>
#include <fmt/format.cc>
#endif

using namespace std;
using namespace fmt;
using namespace ranges;

namespace type {
  class TypeOperator {
  private:
    string name;
    vector<shared_ptr<TypeOperator>> types;
  public:
    TypeOperator(string name,
                 vector<shared_ptr<TypeOperator>> types)
      : name(name), types(types) {};

    virtual string to_string() {
      if (this->types.size() == 0) {
        return this->name;
      } else if (this->types.size() == 2) {
        return format("{0} {1} {2}", this->types[0]->to_string(), this->name, this->types[2]->to_string());
      } else {
        auto literal = this->types
          | view::transform([](shared_ptr<TypeOperator> t) -> string {
            return t->to_string();
          })
          | view::join(' ');
        return format("{0} {1}", this->name, literal);
      }
    }
  };

  class FunctionType : public TypeOperator {
  public:
    FunctionType(shared_ptr<TypeOperator> from_type,
                 shared_ptr<TypeOperator> to_type)
      : TypeOperator("->", vector<shared_ptr<TypeOperator>>({ from_type, to_type })) {};
  };

  auto IntegerType = make_shared<TypeOperator>("int", vector<shared_ptr<TypeOperator>>({}));
  auto BooleanType = make_shared<TypeOperator>("bool", vector<shared_ptr<TypeOperator>>({}));
  auto StringType = make_shared<TypeOperator>("string", vector<shared_ptr<TypeOperator>>({}));
}
