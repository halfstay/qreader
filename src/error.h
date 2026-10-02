#pragma once
#include <stdexcept>

namespace qr
{
// Carry a stable message key across worker threads; translate on the UI thread.
class Error : public std::runtime_error
{
  public:
    explicit Error(const char *key) : std::runtime_error(key) {}
};
} // namespace qr
