#include "types.hpp"

namespace types {

const Type& TypeID::get() const {
  return storage->get_type(id); 
}

Type& TypeID::get() {
  return storage->get_type(id); 
}

} // namespace types
