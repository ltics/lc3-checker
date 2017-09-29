#include <set>
#include <map>
#include <string>
#include <memory>
#include "ast.hpp"
#include "type.hpp"

using namespace std;
using namespace ast;
using namespace type;

namespace checker {
  typedef map<string, shared_ptr<Node>> environment;
  typedef set<TypeVariable> type_vars;

  template<typename Base, typename T>
  inline bool instanceof(const shared_ptr<T> ptr) {
    return dynamic_pointer_cast<Base>(ptr) != nullptr;
  }

  auto get_type(string name, environment env, type_vars non_generic) -> shared_ptr<TypeOperator> {
    return nullptr;
  }
}
