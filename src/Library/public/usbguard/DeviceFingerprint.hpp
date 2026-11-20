#pragma once

#include "Rule.hpp"
#include "Typedefs.hpp"

#include <string>
#include <vector>

namespace usbguard
{
  class Device;

  /**
   * @brief Stable identity of a USB device based on interface types and serial.
   *
   * Interface types are canonicalized, deduplicated and sorted before hashing,
   * so descriptor order and duplicate interfaces do not change the fingerprint.
   * The digest is a Base64 encoded SHA-256 value and can be stored as an
   * out-of-band device identity without changing the USBGuard rule language.
   */
  class DLL_PUBLIC DeviceFingerprint final
  {
  public:
    /**
     * @brief Constructs a fingerprint from a serial number and interface types.
     *
     * @param serial_number Non-empty device serial number.
     * @param interface_types Interface types in alias or "cc:ss:pp" form.
     * @throw std::runtime_error If the serial is empty, no interface type is
     * provided or an interface type is invalid.
     */
    DeviceFingerprint(std::string serial_number, std::vector<std::string> interface_types);

    /**
     * @brief Builds a fingerprint from a live USB device.
     */
    static DeviceFingerprint fromDevice(const Device& device);

    /**
     * @brief Builds a fingerprint from a parsed rule.
     *
     * The rule must contain exactly one non-empty serial and at least one
     * with-interface value.
     */
    static DeviceFingerprint fromRule(const Rule& rule);

    /**
     * @brief Returns the serial number used by this fingerprint.
     */
    const std::string& getSerial() const;

    /**
     * @brief Returns sorted, unique canonical interface types.
     */
    const std::vector<std::string>& getInterfaceTypes() const;

    /**
     * @brief Returns the unambiguous canonical fingerprint input.
     */
    std::string canonicalString() const;

    /**
     * @brief Returns the Base64 encoded SHA-256 fingerprint digest.
     */
    std::string digest() const;

    /**
     * @brief Alias for digest().
     */
    std::string toString() const;

    /**
     * @brief Returns a native rule representing the fingerprinted identity.
     *
     * The returned rule uses only serial and with-interface attributes and is
     * therefore accepted by existing USBGuard daemons.
     */
    Rule toRule() const;

    bool operator==(const DeviceFingerprint& rhs) const;
    bool operator!=(const DeviceFingerprint& rhs) const;

  private:
    std::string _serial;
    std::vector<std::string> _interface_types;
  };
} /* namespace usbguard */

/* vim: set ts=2 sw=2 et */
