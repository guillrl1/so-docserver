#ifndef SAFE_MAP_H
#define SAFE_MAP_H

#include <string_view>
#include <sys/mman.h>
class SafeMap {

  private:
    std::string_view sv_;
  public:
    SafeMap(std::string_view sv) : sv_(sv) {}

    // Prohibir copia para evitar problemas con múltiples llamadas a munmap
    SafeMap(const SafeMap&) = delete;

    SafeMap& operator=(const SafeMap&) = delete;

    // Permitir movimiento
    SafeMap(SafeMap&& other) noexcept;

    SafeMap& operator=(SafeMap&& other) noexcept;

    // Desmapear la región
    ~SafeMap();

    // Obtener la vista
    std::string_view view() const;
};

#endif // SAFE_MAP_H