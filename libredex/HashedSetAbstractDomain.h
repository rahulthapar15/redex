/**
 * Copyright (c) 2016-present, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the root directory of this source tree. An additional grant
 * of patent rights can be found in the PATENTS file in the same directory.
 */

#pragma once

#include <cstddef>
#include <initializer_list>
#include <unordered_set>

#include "Debug.h"
#include "PowersetAbstractDomain.h"

template <typename Element, typename Hash, typename Equal>
class HashedSetAbstractDomain;

namespace hsad_impl {

/*
 * An abstract value from a powerset is implemented as a hash table.
 */
template <typename Element, typename Hash, typename Equal>
class SetValue final : public PowersetImplementation<
                           Element,
                           const std::unordered_set<Element, Hash, Equal>&,
                           SetValue<Element, Hash, Equal>> {
 public:
  using Kind = typename AbstractValue<SetValue<Element, Hash, Equal>>::Kind;

  SetValue() = default;

  SetValue(const Element& e) { m_set.insert(e); }

  SetValue(std::initializer_list<Element> l) : m_set(l.begin(), l.end()) {}

  const std::unordered_set<Element, Hash, Equal>& elements() const override {
    return m_set;
  }

  size_t size() const override { return m_set.size(); }

  bool contains(const Element& e) const override { return m_set.count(e) > 0; }

  void add(const Element& e) override { m_set.insert(e); }

  void remove(const Element& e) override { m_set.erase(e); }

  void clear() override { m_set.clear(); }

  Kind kind() const override { return Kind::Value; }

  bool leq(const SetValue& other) const override {
    if (m_set.size() > other.m_set.size()) {
      return false;
    }
    for (const Element& e : m_set) {
      if (other.m_set.count(e) == 0) {
        return false;
      }
    }
    return true;
  }

  bool equals(const SetValue& other) const override {
    return (m_set.size() == other.m_set.size()) && leq(other);
  }

  Kind join_with(const SetValue& other) override {
    for (const Element& e : other.m_set) {
      m_set.insert(e);
    }
    return Kind::Value;
  }

  Kind meet_with(const SetValue& other) override {
    for (auto it = m_set.begin(); it != m_set.end();) {
      if (other.m_set.count(*it) == 0) {
        it = m_set.erase(it);
      } else {
        ++it;
      }
    }
    return Kind::Value;
  }

 private:
  std::unordered_set<Element, Hash, Equal> m_set;

  template <typename T1, typename T2, typename T3>
  friend class ::HashedSetAbstractDomain;
};

} // namespace hsad_impl

/*
 * An implementation of powerset abstract domains using hash tables.
 */
template <typename Element,
          typename Hash = std::hash<Element>,
          typename Equal = std::equal_to<Element>>
class HashedSetAbstractDomain final
    : public PowersetAbstractDomain<
          Element,
          hsad_impl::SetValue<Element, Hash, Equal>,
          const std::unordered_set<Element, Hash, Equal>&,
          HashedSetAbstractDomain<Element, Hash, Equal>> {
 public:
  using Value = hsad_impl::SetValue<Element, Hash, Equal>;

  using AbstractValueKind = typename Value::Kind;

  HashedSetAbstractDomain()
      : PowersetAbstractDomain<Element,
                               Value,
                               const std::unordered_set<Element, Hash, Equal>&,
                               HashedSetAbstractDomain>() {}

  HashedSetAbstractDomain(AbstractValueKind kind)
      : PowersetAbstractDomain<Element,
                               Value,
                               const std::unordered_set<Element, Hash, Equal>&,
                               HashedSetAbstractDomain>(kind) {}

  explicit HashedSetAbstractDomain(const Element& e) {
    this->set_to_value(Value(e));
  }

  explicit HashedSetAbstractDomain(std::initializer_list<Element> l) {
    this->set_to_value(Value(l));
  }

  static HashedSetAbstractDomain bottom() {
    return HashedSetAbstractDomain(AbstractValueKind::Bottom);
  }

  static HashedSetAbstractDomain top() {
    return HashedSetAbstractDomain(AbstractValueKind::Top);
  }
};
