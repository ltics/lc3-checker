#pragma once

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
  // OK, in Haskell, we call it kind, the type of a type, hmm... not really...
  enum class TypeType : size_t {
    OPERATOR,
    VARIABLE
  };

  class Type {
  public:
    virtual TypeType type() = 0;
    virtual string to_string() = 0;
  };

  class TypeVariable : public Type {
  public:
    int id;
    char name;
    static int next_id;
    static char next_name;
    shared_ptr<Type> instance;

    TypeVariable() {
      this->id = TypeVariable::next_id;
      TypeVariable::next_id += 1;
      this->instance = nullptr;
      this->name = ' ';
    }

    TypeType type() {
      return TypeType::VARIABLE;
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

  class TypeOperator : public Type {
  public:
    string name;
    vector<shared_ptr<Type>> types;

    TypeOperator(string name,
                 vector<shared_ptr<Type>> types)
      : name(name), types(types) {};

    TypeType type() {
      return TypeType::OPERATOR;
    }

    string to_string() {
      if (this->types.size() == 0) {
        return this->name;
      } else if (this->types.size() == 2) {
        return format("({0} {1} {2})", this->types[0]->to_string(), this->name, this->types[1]->to_string());
      } else {
        vector<string> literals = this->types | view::transform([](shared_ptr<Type> t) -> string {
            return t->to_string();
          });
        string literal = literals | view::join(' ');
        return format("({0} {1})", this->name, literal);
      }
    }
  };

  auto IntegerType = make_shared<TypeOperator>("int", vector<shared_ptr<Type>>({}));
  auto BooleanType = make_shared<TypeOperator>("bool", vector<shared_ptr<Type>>({}));
  auto StringType = make_shared<TypeOperator>("string", vector<shared_ptr<Type>>({}));
  auto FunctionType(shared_ptr<Type> from_type, shared_ptr<Type> to_type) -> shared_ptr<Type> {
    return make_shared<TypeOperator>("->", vector<shared_ptr<Type>>({ from_type, to_type }));
  }

  bool operator==(shared_ptr<Type> t1, shared_ptr<Type> t2) {
    if (t1 == nullptr && t2 == nullptr) {
      return true;
    } else if (t1 == nullptr || t2 == nullptr || t1->type() != t2->type()) {
      return false;
    } else {
      if (t1->type() == TypeType::VARIABLE) {
        auto var1 = static_pointer_cast<TypeVariable>(t1);
        auto var2 = static_pointer_cast<TypeVariable>(t2);

        return
          var1->id == var2->id &&
          var1->name == var2->name &&
          var1->instance == var2->instance;
      } else if (t1->type() == TypeType::OPERATOR) {
        auto oper1 = static_pointer_cast<TypeOperator>(t1);
        auto oper2 = static_pointer_cast<TypeOperator>(t2);

        return
          oper1->name == oper2->name &&
          oper1->types == oper2->types;
      } else {
        return false;
      }
    }
  }
}
