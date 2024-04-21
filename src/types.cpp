#include "types.hpp"

namespace types {

const Type& TypeID::get() const {
  return storage->get_type(id); 
}

Type& TypeID::get() {
  return storage->get_type(id); 
}

TypeID TypeID::with_mode(Mode new_mode) const {
  return storage->add(get().with_mode(new_mode)); 
}

} // namespace types
