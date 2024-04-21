#pragma once

#include <memory>
#include <vector>
#include <variant>

namespace types {

using namespace std;

struct Mode {
  enum class Loc { GLOBAL, LOCAL } loc = Loc::GLOBAL;
  enum class Uniq { SHARED, UNIQUE, EXCL } uniq = Uniq::SHARED;
  enum class Lin { MANY, ONCE, SEP } lin = Lin::MANY;

  Mode with(Loc mode) const {
    Mode copy = *this;
    copy.loc = mode;
    return copy;
  }
  Mode with(Uniq mode) const {
    Mode copy = *this;
    copy.uniq = mode;
    return copy;
  }
  Mode with(Lin mode) const {
    Mode copy = *this;
    copy.lin = mode;
    return copy;
  }

  Mode() = default;
  Mode(Loc mode) : loc(mode) {}
  Mode(Uniq mode) : uniq(mode) {}
  Mode(Lin mode) : lin(mode) {}
};
using ModePtr = shared_ptr<Mode>;

struct Storage;
struct Type;
struct TypeID {
  TypeID(size_t id, Storage *storage) : id(id), storage(storage) {}

  const Type &get() const;
  Type &get();

private:
  size_t id;
  Storage *storage = nullptr;
};
using TypeIDV = vector<TypeID>;

struct ArrowType {
  vector<TypeID> types;
};

struct BoolType {};
struct IntType {};
// struct UnitType {};

struct GenericType {
  size_t id;
};

struct Type {
  variant<ArrowType, BoolType, IntType, GenericType> type;
};

template <typename T, typename... Args> Type make_type(Args &&...args) {
  return Type{T{std::forward<Args>(args)...}};
}

template <typename T, typename... Args> Type make_func1(TypeID in, TypeID ret) {
  return make_type<ArrowType>(TypeIDV{in, ret});
}

inline Type make_operator(TypeID left, TypeID right, TypeID ret) {
  return make_type<ArrowType>(TypeIDV{left, right, ret});
}

struct Storage {
  Storage()
      : int_type(add(make_type<IntType>())),
        bool_type(add(make_type<BoolType>())) {}

  TypeID get_int_type() { return int_type; }
  TypeID get_bool_type() { return bool_type; }

  Type &get_type(size_t id) { return types[id]; }

  const Type &get_type(size_t id) const { return types[id]; }

  TypeID introduce_new_generic() {
    return add(make_type<GenericType>(first_unused_generic_id++));
  }

  TypeID add(Type type) {
    types.push_back(std::move(type));
    return TypeID(types.size() - 1, this);
  }

  // TODO: add modes ??
  bool unify(TypeID left_id, TypeID right_id) {
    Type &left = left_id.get();
    Type &right = right_id.get();

    if (const auto *left_generic = get_if<GenericType>(&left.type);
        left_generic != nullptr) {
      // TODO: check if other type contins generic
      resolve(*left_generic, right);
      return true;
    }

    if (const auto *right_generic = get_if<GenericType>(&right.type);
        right_generic != nullptr) {
      // TODO: check if other type contins generic
      resolve(*right_generic, left);
      return true;
    }

    if (left.type.index() != right.type.index()) {
      return false;
    }

    if (holds_alternative<ArrowType>(left.type)) {
      const auto &left_types = std::get<ArrowType>(left.type).types;
      const auto &right_types = std::get<ArrowType>(right.type).types;

      if (left_types.size() != right_types.size()) {
        return false;
      }

      bool all_unify_passed = true;
      for (size_t i = 0; i < left_types.size(); ++i) {
        if (not unify(left_types[i], right_types[i])) {
          all_unify_passed = false;
        }
      }

      return all_unify_passed;
    }

    return true;
  }

  void resolve(GenericType generic, const Type &replacement) {
    for (auto &type : types) {
      if (const auto *generic_type = get_if<GenericType>(&type.type);
          generic_type != nullptr and generic_type->id == generic.id) {
        type = replacement;
      }
    }
  }

private:
  size_t first_unused_generic_id = 0;

  vector<Type> types;

  TypeID int_type;
  TypeID bool_type;
};

} // namespace types

