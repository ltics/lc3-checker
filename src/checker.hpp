#include <set>
#include <map>
#include <string>
#include <vector>
#include <memory>
#include <functional>
#include <range/v3/all.hpp>
#ifndef FORMAT_HEADER
#define FORMAT_HEADER
#include <fmt/format.h>
#include <fmt/format.cc>
#endif
#include "ast.hpp"
#include "type.hpp"

using namespace std;
using namespace ast;
using namespace fmt;
using namespace type;
using namespace ranges;

namespace checker {
  typedef map<string, shared_ptr<Type>> environment;
  typedef map<shared_ptr<Type>, shared_ptr<Type>> typevar_mapping;
  typedef set<Type> typevars;

  template<typename Base, typename T>
  inline bool instanceof(const shared_ptr<T> ptr) {
    return dynamic_pointer_cast<Base>(ptr) != nullptr;
  }

  auto prune(shared_ptr<Type> t) -> shared_ptr<Type> {
    if (t->type() == TypeType::VARIABLE) {
      auto var = static_pointer_cast<TypeVariable>(t);
      if (var->instance == nullptr) {
        return t;
      } else {
        auto new_instance = prune(var->instance);
        var->instance = new_instance;
        return new_instance;
      }
    } else {
      return t;
    }
  }


  auto occurs_in_type(shared_ptr<Type> var, shared_ptr<Type> t) -> bool;

  auto occurs_in(shared_ptr<Type> var, vector<shared_ptr<Type>> types) -> bool {
    for (auto iter = types.begin(); iter != types.end(); iter++) {
      if (occurs_in_type(var, *iter)) {
        return true;
      }
    }
    return false;
  }

  auto occurs_in_type(shared_ptr<Type> var, shared_ptr<Type> t) -> bool {
    auto pruned = prune(t);
    if (var == pruned) {
      return true;
    } else if (pruned->type() == TypeType::OPERATOR) {
      return occurs_in(var, static_pointer_cast<TypeOperator>(pruned)->types);
    } else {
      return false;
    }
  }

  auto is_generic(shared_ptr<Type> var, typevars non_generic) -> bool {
    vector<shared_ptr<Type>> non_generic_vector = {};
    std::copy(non_generic.begin(), non_generic.end(), std::back_inserter(non_generic_vector));
    return !occurs_in(var, non_generic_vector);
  }

  auto fresh(shared_ptr<Type> t, typevars non_generic) -> shared_ptr<Type> {
    typevar_mapping mapping = {};

    function<shared_ptr<Type>(shared_ptr<Type>)> fresh_rec = [&](shared_ptr<Type> tp) -> shared_ptr<Type> {
      auto pruned = prune(tp);
      if (pruned->type() == TypeType::VARIABLE) {
        if (is_generic(pruned, non_generic)) {
          auto result = mapping.find(pruned);
          if (result != mapping.end()) {
            mapping[pruned] = make_shared<TypeVariable>();
          }
          return mapping[pruned];
        } else {
          return pruned;
        }
      } else if (pruned->type() == TypeType::OPERATOR) {
        auto oper = static_pointer_cast<TypeOperator>(pruned);
        auto freshs = oper->types | view::transform([&](shared_ptr<Type> ta) {
            return fresh_rec(ta);
          });
        return make_shared<TypeOperator>(oper->name, freshs);
      } else {
        return nullptr;
      }
    };

    return fresh_rec(t);
  }

  auto get_type(string name, environment env, typevars non_generic) -> shared_ptr<Type> {
    auto result = env.find(name);
    if (result != env.end()) {
      return fresh(env[name], non_generic);
    } else {
      throw std::runtime_error(format("Undefined symbol {}", name));
    }
  }
}
