#include "safeMap.h"



SafeMap& SafeMap::operator=(SafeMap&& other) noexcept {
      sv_ = other.sv_;
      other.sv_ = {};
      return *this;
    }

SafeMap::SafeMap(SafeMap&& other) noexcept {
  sv_ = other.sv_;
  other.sv_ = {};
}

SafeMap::~SafeMap() {
  if (!sv_.empty()) {
    munmap(const_cast<char*>(sv_.data()), sv_.size());
  }
}

std::string_view SafeMap::view() const {
  return sv_;
}