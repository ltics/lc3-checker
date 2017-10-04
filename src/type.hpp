#pragma once

#include <string>
#include <vector>
#include <memory>
#include <algorithm>
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
    shared_ptr<Type> instance;

    static int next_id;

    TypeVariable() {
      if (TypeVariable::next_id == 25) {
        TypeVariable::next_id = 0;
      }
      this->id = TypeVariable::next_id;
      TypeVariable::next_id += 1;
      this->instance = nullptr;
    }

    TypeType type() {
      return TypeType::VARIABLE;
    }

    string to_name() {
      return format("{}", (char)(this->id + 97));
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

  auto get_var_ids(shared_ptr<Type> t) -> vector<int> {
    switch (t->type()) {
    case TypeType::VARIABLE: {
      auto var = static_pointer_cast<TypeVariable>(t);
      if (var->instance == nullptr) {
        return vector<int>({ var->id });
      } else {
        return get_var_ids(var->instance);
      }
    }
    case TypeType::OPERATOR: {
      auto oper = static_pointer_cast<TypeOperator>(t);
      vector<int> ids = oper->types
        | view::transform(get_var_ids)
        | action::join;

      return ids;
    }
    default:
      return vector<int>({});
    }
  }

  auto minus_base(shared_ptr<Type> t, int base) -> shared_ptr<Type> {
    switch (t->type()) {
    case TypeType::VARIABLE: {
      auto var = static_pointer_cast<TypeVariable>(t);
      if (var->instance == nullptr) {
        if (var->id >= base) {
          var->id -= base;
        }
      } else {
        auto new_instance = minus_base(var->instance, base);
        var->instance = new_instance;
      }
      return var;
    }
    case TypeType::OPERATOR: {
      auto oper = static_pointer_cast<TypeOperator>(t);
      auto new_types = oper->types
        | view::transform([=](shared_ptr<Type> tt) {
            return minus_base(tt, base);
          });
          oper->types = new_types;
      return oper;
    }
    default:
      return t;
    }
  }

  auto normalize(shared_ptr<Type> t) -> shared_ptr<Type> {
    auto ids = get_var_ids(t);
    auto min_iter = std::min_element(ids.cbegin(), ids.cend());
    if (min_iter != ids.end()) {
      auto min_id = *min_iter;
      return minus_base(t, min_id);
    } else {
      return t;
    }
  }
}
