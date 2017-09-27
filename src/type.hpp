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

  class TypeVariable {
  private:
    int id;
    char name;
    static int next_id;
    static char next_name;
    shared_ptr<TypeOperator> instance;

  public:
    TypeVariable() {
      this->id = TypeVariable::next_id;
      TypeVariable::next_id += 1;
      this->instance = nullptr;
      this->name = ' ';
    }

    string to_name() {
      if (this->name == ' ') {
        this->name = next_name;
        TypeVariable::next_name = (char)((int)TypeVariable::next_name + 1);
      }
      return format("{}", this->name);
    }

    string to_string() {
      if (this->instance != nullptr) {
        return this->instance->to_string();
      } else {
        return this->to_name();
      }
    }

    string to_repr() {
      return format("TypeVariable(id = {})", this->id);
    }
  };

  int TypeVariable::next_id = 0;
  char TypeVariable::next_name = 'a';
}
