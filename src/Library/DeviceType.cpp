#ifdef HAVE_BUILD_CONFIG_H
  #include <build-config.h>
#endif

#include "usbguard/DeviceType.hpp"
#include "usbguard/USB.hpp"

#include <algorithm>
#include <cctype>
#include <stdexcept>
#include <unordered_map>

namespace usbguard
{
  namespace
  {
    std::string normalizeName(const std::string& value)
    {
      const auto first = value.find_first_not_of(" \t\r\n");

      if (first == std::string::npos) {
        return {};
      }

      const auto last = value.find_last_not_of(" \t\r\n");
      std::string normalized = value.substr(first, last - first + 1);

      std::transform(normalized.begin(), normalized.end(), normalized.begin(), [](unsigned char character) {
        if (character == '_' || character == ' ') {
          return '-';
        }

        return static_cast<char>(std::tolower(character));
      });

      return normalized;
    }

    const std::unordered_map<std::string, std::string>& deviceTypeAliases()
    {
      static const std::unordered_map<std::string, std::string> aliases = {
        { "audio", "01:*:*" },
        { "communications", "02:*:*" },
        { "cdc", "02:*:*" },
        { "hid", "03:*:*" },
        { "keyboard", "03:01:01" },
        { "mouse", "03:01:02" },
        { "physical", "05:*:*" },
        { "image", "06:*:*" },
        { "printer", "07:*:*" },
        { "mass-storage", "08:*:*" },
        { "storage", "08:*:*" },
        { "usb-storage", "08:*:*" },
        { "hub", "09:*:*" },
        { "cdc-data", "0a:*:*" },
        { "smart-card", "0b:*:*" },
        { "content-security", "0d:*:*" },
        { "video", "0e:*:*" },
        { "camera", "0e:*:*" },
        { "webcam", "0e:*:*" },
        { "personal-healthcare", "0f:*:*" },
        { "audio-video", "10:*:*" },
        { "billboard", "11:*:*" },
        { "type-c-bridge", "12:*:*" },
        { "diagnostic", "dc:*:*" },
        { "wireless", "e0:*:*" },
        { "miscellaneous", "ef:*:*" },
        { "misc", "ef:*:*" },
        { "application-specific", "fe:*:*" },
        { "vendor-specific", "ff:*:*" }
      };

      return aliases;
    }
  } /* namespace */

  std::string DeviceType::toInterfaceType(const std::string& value)
  {
    const std::string normalized = normalizeName(value);

    if (normalized.empty()) {
      throw std::runtime_error("USB device type must not be empty");
    }

    const auto alias = deviceTypeAliases().find(normalized);

    if (alias != deviceTypeAliases().end()) {
      return alias->second;
    }

    try {
      return USBInterfaceType(normalized).typeString();
    } catch (const std::exception&) {
      throw std::runtime_error("Unknown USB device type: " + value);
    }
  }

  bool DeviceType::isValid(const std::string& value) noexcept
  {
    try {
      (void)toInterfaceType(value);
      return true;
    } catch (...) {
      return false;
    }
  }
} /* namespace usbguard */

/* vim: set ts=2 sw=2 et */
