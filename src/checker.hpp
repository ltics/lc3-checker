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
  typedef set<shared_ptr<Type>> typevars;

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
      throw runtime_error(format("Undefined symbol {}", name));
    }
  }

  auto unify(shared_ptr<Type> t1, shared_ptr<Type> t2) -> void {
    auto pruned1 = prune(t1);
    auto pruned2 = prune(t2);

    if (pruned1->type() == TypeType::VARIABLE) {
      if (pruned1 != pruned2) {
        if (occurs_in_type(pruned1, pruned2)) {
          throw runtime_error("Recursive unification");
        }
        static_pointer_cast<TypeVariable>(pruned1)->instance = pruned2;
      }
    } else if (pruned1->type() == TypeType::OPERATOR && pruned2->type() == TypeType::VARIABLE) {
      unify(pruned2, pruned1);
    } else if (pruned1->type() == TypeType::OPERATOR && pruned2->type() == TypeType::OPERATOR) {
      auto oper1 = static_pointer_cast<TypeOperator>(pruned1);
      auto oper2 = static_pointer_cast<TypeOperator>(pruned2);
      if (oper1->name != oper2->name || oper1->types.size() != oper2->types.size()) {
        throw runtime_error(format("Type mismatch: {0} != {1}", pruned1->to_string(), pruned2->to_string()));
      }
      auto types1 = oper1->types;
      auto types2 = oper2->types;
      for (auto entry1 = types1.begin(), entry2 = types2.begin();
           entry1 != types1.end();
           ++entry1 , ++entry2) {
        unify(*entry1, *entry2);
      }
    } else {
      throw runtime_error(format("Can not unify: {0}, {1}", pruned1->to_string(), pruned2->to_string()));
    }
  }

  auto analyse(shared_ptr<Node> node, environment env, typevars non_generic) -> shared_ptr<Type> {
    switch (node->type()) {
    case NodeType::IDENTIFIER:
      return get_type(static_pointer_cast<Identifier>(node)->name, env, non_generic);
    case NodeType::APPLY: {
      auto func_node = static_pointer_cast<Apply>(node);
      auto func_type = analyse(func_node->func, env, non_generic);
      auto arg_type = analyse(func_node->arg, env, non_generic);
      auto return_type = make_shared<TypeVariable>();
      unify(FunctionType(arg_type, return_type), func_type);
      return return_type;
    }
    case NodeType::LAMBDA: {
      auto lambda_node = static_pointer_cast<Lambda>(node);
      auto param_type = make_shared<TypeVariable>();
      environment new_env = env;
      typevars new_non_generic = non_generic;
      new_env[lambda_node->param] = param_type;
      new_non_generic.insert(param_type);
      auto return_type = analyse(lambda_node->body, new_env, new_non_generic);
      return FunctionType(param_type, return_type);
    }
    case NodeType::LET: {
      auto let_node = static_pointer_cast<Let>(node);
      auto defn_type = analyse(let_node->defn, env, non_generic);
      environment new_env = env;
      new_env[let_node->name] = defn_type;
      return analyse(let_node->body, new_env, non_generic);
    }
    case NodeType::LETREC: {
      auto letrec_node = static_pointer_cast<Letrec>(node);
      auto new_type = make_shared<TypeVariable>();
      environment new_env = env;
      typevars new_non_generic = non_generic;
      new_env[letrec_node->name] = new_type;
      new_non_generic.insert(new_type);
      auto defn_type = analyse(letrec_node->defn, new_env, new_non_generic);
      unify(new_type, defn_type);
      return analyse(letrec_node->body, new_env, non_generic);
    }
    default:
      throw runtime_error(format("Unhandled syntax node {}", node->to_string()));
    }
  }

  auto analyse(shared_ptr<Node> node, environment env) -> shared_ptr<Type> {
    return analyse(node, env, typevars({}));
  }
}
