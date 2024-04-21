#pragma once

#include <map>
#include <memory>
#include <variant>
#include <vector>

#include <iostream>

namespace types {

using namespace std;

struct Mode {
  enum class Loc { LOCAL = 0, GLOBAL = 1 } loc = Loc::GLOBAL;
  enum class Uniq { UNIQUE = 0, EXCL = 1, SHARED = 2 } uniq = Uniq::SHARED;
  enum class Lin { ONCE = 0, SEP = 1, MANY = 2 } lin = Lin::MANY;

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

  auto operator<=>(const Mode &other) const {
    return tie(loc, uniq, lin) <=> tie(other.loc, other.uniq, other.lin);
  }

  bool is_submode(const Mode &other) const {
    return loc <= other.loc and uniq <= other.uniq and lin <= other.lin;
  }

  static Mode choose_min(const Mode &left, const Mode &right) {
    Mode ans;
    ans.loc = static_cast<Loc>(std::min(static_cast<size_t>(left.loc),
                                        static_cast<size_t>(right.loc)));
    ans.uniq = static_cast<Uniq>(std::min(static_cast<size_t>(left.uniq),
                                          static_cast<size_t>(right.uniq)));
    ans.lin = static_cast<Lin>(std::min(static_cast<size_t>(left.lin),
                                        static_cast<size_t>(right.lin)));
    return ans;
  }
};
using ModePtr = shared_ptr<Mode>;

struct Storage;
struct Type;
struct TypeID {
  TypeID(size_t id, Storage *storage) : id(id), storage(storage) {}

  const Type &get() const;
  Type &get();

  TypeID with_mode(Mode new_mode) const;

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
  std::string name;
};

struct Type {
  template <typename T> Type(T type, Mode mode = {}) : type(type), mode(mode) {}

  Type with_mode(Mode new_mode) const {
    Type copy = *this;
    copy.mode = new_mode;
    return copy;
  }

  variant<ArrowType, BoolType, IntType, GenericType> type;
  Mode mode;
};

template <typename T, typename... Args> Type make_type(Args &&...args) {
  return Type{T{std::forward<Args>(args)...}};
}

template <typename T, typename... Args>
Type make_moded_type(Mode mode, Args &&...args) {
  return make_type<T>(std::forward<Args>(args)...).with_mode(mode);
}

template <typename T, typename... Args>
Type make_func1(TypeID in, TypeID ret, Mode mode = {}) {
  return make_moded_type<ArrowType>(mode, TypeIDV{in, ret});
}

inline Type make_operator(TypeID left, TypeID right, TypeID ret,
                          Mode mode = {}) {
  return make_moded_type<ArrowType>(mode, TypeIDV{left, right, ret});
}

enum class UnifyModePolicy {
  Ignore,             // all mode differences ignored
  ApplyStrongest,     // unique > shared, modes changed
  CheckLeftIsSubmode, // only check is performed
};

struct Storage {
  Storage() {}

  TypeID get_int_type(Mode mode = {}) {
    auto it = int_types.find(mode);
    if (it != int_types.end()) {
      return it->second;
    }
    return int_types.insert({mode, add(make_moded_type<IntType>(mode))})
        .first->second;
  }

  TypeID get_bool_type(Mode mode = {}) {
    auto it = bool_types.find(mode);
    if (it != bool_types.end()) {
      return it->second;
    }
    return bool_types.insert({mode, add(make_moded_type<BoolType>(mode))})
        .first->second;
  }

  Type &get_type(size_t id) { return types[id]; }

  const Type &get_type(size_t id) const { return types[id]; }

  TypeID introduce_new_generic(std::string name, Mode mode = {}) {
    return add(make_moded_type<GenericType>(mode, first_unused_generic_id++,
                                            std::move(name)));
  }

  TypeID add(Type type) {
    types.push_back(std::move(type));
    return TypeID(types.size() - 1, this);
  }

  bool unify(TypeID left_id, TypeID right_id, UnifyModePolicy policy) {
    Type &left = left_id.get();
    Type &right = right_id.get();

    switch (policy) {
    case UnifyModePolicy::Ignore:
      break;
    case UnifyModePolicy::ApplyStrongest:
      left.mode = Mode::choose_min(left.mode, right.mode);
      right.mode = left.mode;
      break;
    case UnifyModePolicy::CheckLeftIsSubmode:
      if (not left.mode.is_submode(right.mode)) {
        return false;
      }
      break;
    }

    if (const auto *left_generic = get_if<GenericType>(&left.type);
        left_generic != nullptr) {
      // TODO: check if other type contains generic
      std::clog << "left is resolved with policy <" << static_cast<size_t>(policy) << ">\n";
      resolve(*left_generic, right, left.mode);
      return true;
    }

    if (const auto *right_generic = get_if<GenericType>(&right.type);
        right_generic != nullptr) {
      // TODO: check if other type contains generic
      std::clog << "right is resolved with policy <" << static_cast<size_t>(policy) << ">\n";
      resolve(*right_generic, left, right.mode);
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
        if (not unify(left_types[i], right_types[i], policy)) {
          all_unify_passed = false;
        }
      }

      return all_unify_passed;
    }

    return true;
  }

  void resolve(GenericType generic, const Type &replacement, Mode mode = {}) {
    std::clog << "generic type " << generic.name << " is resolved with mode==UNIQUE: <"
              << (replacement.mode.uniq == Mode::Uniq::UNIQUE) << ">\n";
    for (auto &type : types) {
      if (const auto *generic_type = get_if<GenericType>(&type.type);
          generic_type != nullptr and generic_type->id == generic.id) {
        type = replacement;
        type.mode = mode;
      }
    }
  }

private:
  size_t first_unused_generic_id = 0;

  vector<Type> types;

  map<Mode, TypeID> int_types;
  map<Mode, TypeID> bool_types;
};

} // namespace types
