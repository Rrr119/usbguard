#pragma once

#include "Typedefs.hpp"

#include <string>

namespace usbguard
{
  /**
   * @brief Translates human readable USB device types into interface types.
   *
   * Device types are authoring aliases for USB interface class triplets.
   * For example, "mass-storage" is translated to "08:*:*" and "keyboard"
   * is translated to "03:01:01". Numeric interface types are validated and
   * returned in canonical form.
   */
  class DLL_PUBLIC DeviceType final
  {
  public:
    /**
     * @brief Converts a device type name or interface type to "cc:ss:pp".
     *
     * @param value Device type alias or numeric USB interface type.
     * @return Canonical USB interface type.
     * @throw std::runtime_error If the value is empty or unknown.
     */
    static std::string toInterfaceType(const std::string& value);

    /**
     * @brief Checks whether a device type name or interface type is valid.
     *
     * @param value Device type alias or numeric USB interface type.
     * @return True if the value can be translated, false otherwise.
     */
    static bool isValid(const std::string& value) noexcept;
  };
} /* namespace usbguard */

/* vim: set ts=2 sw=2 et */
