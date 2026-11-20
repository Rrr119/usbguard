#pragma once

#include "Rule.hpp"
#include "Typedefs.hpp"

#include <string>

#include <cstddef>

namespace usbguard
{
  /**
   * @brief Compiles portable rule authoring syntax into native USBGuard rules.
   *
   * The compiler supports the device-type authoring attribute:
   *
   * @code
   * allow device-type "mass-storage" serial "SN-123"
   * allow device-type one-of { keyboard mouse } serial "SN-456"
   * @endcode
   *
   * Device types are translated to with-interface attributes. The resulting
   * rules contain only attributes understood by existing USBGuard daemons.
   */
  class DLL_PUBLIC RuleCompiler final
  {
  public:
    /**
     * @brief Compiles an authoring rule into native rule text.
     *
     * @param rule_spec Rule using native attributes, device-type aliases or
     * both.
     * @return Native rule text.
     * @throw RuleParserError If the input or generated rule is invalid.
     */
    static std::string compileToString(const std::string& rule_spec);

    /**
     * @brief Compiles and parses an authoring rule.
     *
     * @param rule_spec Rule using native attributes, device-type aliases or
     * both.
     * @param file Source file used in parser diagnostics.
     * @param line Source line used in parser diagnostics.
     * @param trace Enable parser tracing.
     * @return Parsed native USBGuard rule.
     */
    static Rule compile(
      const std::string& rule_spec, const std::string& file = std::string(), size_t line = 0, bool trace = false);

    /**
     * @brief Compiles an allow/block rule matching a device type and serial.
     *
     * @param device_type Device type alias or "cc:ss:pp" interface type.
     * @param serial_number Non-empty exact device serial number.
     * @param target Rule target, Allow by default.
     * @return Native USBGuard rule.
     * @throw std::runtime_error If the device type or serial is invalid.
     */
    static Rule compileDeviceRule(
      const std::string& device_type, const std::string& serial_number, Rule::Target target = Rule::Target::Allow);
  };
} /* namespace usbguard */

/* vim: set ts=2 sw=2 et */
