#ifdef HAVE_BUILD_CONFIG_H
  #include <build-config.h>
#endif

#include "usbguard/DeviceFingerprint.hpp"

#include "Hash.hpp"
#include "usbguard/Device.hpp"
#include "usbguard/DeviceType.hpp"
#include "usbguard/USB.hpp"

#include <algorithm>
#include <stdexcept>
#include <utility>

namespace usbguard
{
  namespace
  {
    std::vector<std::string> canonicalizeInterfaceTypes(const std::vector<std::string>& interface_types)
    {
      std::vector<std::string> canonical;
      canonical.reserve(interface_types.size());

      for (const auto& interface_type : interface_types) {
        canonical.push_back(DeviceType::toInterfaceType(interface_type));
      }

      std::sort(canonical.begin(), canonical.end());
      canonical.erase(std::unique(canonical.begin(), canonical.end()), canonical.end());
      return canonical;
    }
  } /* namespace */

  DeviceFingerprint::DeviceFingerprint(std::string serial_number, std::vector<std::string> interface_types)
    : _serial(std::move(serial_number)),
      _interface_types(canonicalizeInterfaceTypes(interface_types))
  {
    if (_serial.empty()) {
      throw std::runtime_error("Cannot create a device fingerprint without a serial number");
    }

    if (_interface_types.empty()) {
      throw std::runtime_error("Cannot create a device fingerprint without interface types");
    }
  }

  DeviceFingerprint DeviceFingerprint::fromDevice(const Device& device)
  {
    std::vector<std::string> interface_types;
    interface_types.reserve(device.getInterfaceTypes().size());

    for (const auto& interface_type : device.getInterfaceTypes()) {
      interface_types.push_back(interface_type.typeString());
    }

    return DeviceFingerprint(device.getSerial(), std::move(interface_types));
  }

  DeviceFingerprint DeviceFingerprint::fromRule(const Rule& rule)
  {
    if (rule.attributeSerial().count() != 1) {
      throw std::runtime_error("Device fingerprint requires exactly one serial attribute");
    }

    if (rule.attributeWithInterface().empty()) {
      throw std::runtime_error("Device fingerprint requires at least one with-interface attribute");
    }

    std::vector<std::string> interface_types;
    interface_types.reserve(rule.attributeWithInterface().count());

    for (const auto& interface_type : rule.attributeWithInterface().values()) {
      interface_types.push_back(interface_type.typeString());
    }

    return DeviceFingerprint(rule.getSerial(), std::move(interface_types));
  }

  const std::string& DeviceFingerprint::getSerial() const
  {
    return _serial;
  }

  const std::vector<std::string>& DeviceFingerprint::getInterfaceTypes() const
  {
    return _interface_types;
  }

  std::string DeviceFingerprint::canonicalString() const
  {
    std::string canonical = "usbguard-device-fingerprint-v1\n";
    canonical.append("serial:");
    canonical.append(std::to_string(_serial.size()));
    canonical.append(":");
    canonical.append(_serial);
    canonical.append("\ninterfaces:");
    canonical.append(std::to_string(_interface_types.size()));

    for (const auto& interface_type : _interface_types) {
      canonical.append("\n");
      canonical.append(std::to_string(interface_type.size()));
      canonical.append(":");
      canonical.append(interface_type);
    }

    return canonical;
  }

  std::string DeviceFingerprint::digest() const
  {
    Hash hash;
    hash.update(canonicalString());
    return hash.getBase64();
  }

  std::string DeviceFingerprint::toString() const
  {
    return digest();
  }

  Rule DeviceFingerprint::toRule() const
  {
    Rule rule;
    std::vector<USBInterfaceType> interface_types;
    interface_types.reserve(_interface_types.size());

    for (const auto& interface_type : _interface_types) {
      interface_types.emplace_back(interface_type);
    }

    rule.setTarget(Rule::Target::Allow);
    rule.setSerial(_serial);
    rule.attributeWithInterface().set(interface_types, Rule::SetOperator::Equals);
    return rule;
  }

  bool DeviceFingerprint::operator==(const DeviceFingerprint& rhs) const
  {
    return _serial == rhs._serial && _interface_types == rhs._interface_types;
  }

  bool DeviceFingerprint::operator!=(const DeviceFingerprint& rhs) const
  {
    return !(*this == rhs);
  }
} /* namespace usbguard */

/* vim: set ts=2 sw=2 et */
