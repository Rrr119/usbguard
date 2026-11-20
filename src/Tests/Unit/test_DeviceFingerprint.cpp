#include "usbguard/DeviceFingerprint.hpp"
#include "usbguard/Rule.hpp"

#include <stdexcept>
#include <string>
#include <vector>

#ifdef HAVE_CATCH2_V3
  #include <catch_test_macros.hpp>
#else
  #include <catch.hpp>
#endif

using namespace usbguard;

TEST_CASE("Device fingerprint is independent of interface order and duplicates", "[DeviceFingerprint]")
{
  const DeviceFingerprint first("SN-123", { "03:01:01", "08:06:50", "03:01:01" });
  const DeviceFingerprint second("SN-123", { "08:06:50", "03:01:01" });
  REQUIRE(first == second);
  REQUIRE(first.digest() == second.digest());
  REQUIRE(first.getInterfaceTypes().size() == 2);
}

TEST_CASE("Device fingerprint changes with the serial number", "[DeviceFingerprint]")
{
  const DeviceFingerprint first("SN-123", { "08:*:*" });
  const DeviceFingerprint second("SN-124", { "08:*:*" });
  REQUIRE(first != second);
  REQUIRE(first.digest() != second.digest());
}

TEST_CASE("Device fingerprint changes with the device type", "[DeviceFingerprint]")
{
  const DeviceFingerprint first("SN-123", { "08:*:*" });
  const DeviceFingerprint second("SN-123", { "03:*:*" });
  REQUIRE(first != second);
  REQUIRE(first.digest() != second.digest());
}

TEST_CASE("Device fingerprint round-trips through a native rule", "[DeviceFingerprint]")
{
  const DeviceFingerprint fingerprint("SN-123", { "mass-storage", "03:01:01" });
  const Rule rule = fingerprint.toRule();
  const DeviceFingerprint reparsed = DeviceFingerprint::fromRule(Rule::fromString(rule.toString()));
  REQUIRE(fingerprint == reparsed);
  REQUIRE(reparsed.digest() == fingerprint.digest());
}

/* vim: set ts=2 sw=2 et */
