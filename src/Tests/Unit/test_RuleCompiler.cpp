#include "usbguard/Rule.hpp"
#include "usbguard/RuleCompiler.hpp"
#include "usbguard/RuleParser.hpp"

#include <stdexcept>

#ifdef HAVE_CATCH2_V3
  #include <catch_test_macros.hpp>
#else
  #include <catch.hpp>
#endif

using namespace usbguard;

TEST_CASE("Rule compiler translates a named device type and serial", "[RuleCompiler]")
{
  const Rule rule = RuleCompiler::compile("allow device-type mass-storage serial \"SN-123\"");
  REQUIRE(rule.getTarget() == Rule::Target::Allow);
  REQUIRE(rule.getSerial() == "SN-123");
  REQUIRE(rule.attributeWithInterface().count() == 1);
  REQUIRE(rule.attributeWithInterface().get().typeString() == "08:*:*");
  REQUIRE(rule.attributeWithInterface().setOperator() == Rule::SetOperator::OneOf);
}

TEST_CASE("Rule compiler accepts a numeric interface triplet", "[RuleCompiler]")
{
  const Rule rule = RuleCompiler::compile("allow device-type 08:06:50 serial \"SN-123\"");
  REQUIRE(rule.attributeWithInterface().get().typeString() == "08:06:50");
  REQUIRE(rule.getSerial() == "SN-123");
}

TEST_CASE("Rule compiler accepts a quoted device type alias", "[RuleCompiler]")
{
  const Rule rule = RuleCompiler::compile("allow device-type \"smart-card\" serial \"SN-123\"");
  REQUIRE(rule.attributeWithInterface().get().typeString() == "0b:*:*");
}

TEST_CASE("Rule compiler expands multiple device types", "[RuleCompiler]")
{
  const Rule rule = RuleCompiler::compile("allow device-type one-of { keyboard mouse } serial \"SN-123\"");
  REQUIRE(rule.attributeWithInterface().count() == 2);
  REQUIRE(rule.attributeWithInterface().get(0).typeString() == "03:01:01");
  REQUIRE(rule.attributeWithInterface().get(1).typeString() == "03:01:02");
  REQUIRE(rule.attributeWithInterface().setOperator() == Rule::SetOperator::OneOf);
}

TEST_CASE("Rule compiler preserves an explicit device type set operator", "[RuleCompiler]")
{
  const Rule rule = RuleCompiler::compile("allow device-type equals { mass-storage } serial \"SN-123\"");
  REQUIRE(rule.attributeWithInterface().setOperator() == Rule::SetOperator::Equals);
  REQUIRE(rule.attributeWithInterface().get().typeString() == "08:*:*");
}

TEST_CASE("Rule compiler leaves native rules unchanged", "[RuleCompiler]")
{
  const std::string native = "block id 1234:5678 name \"Native rule\"";
  REQUIRE(RuleCompiler::compileToString(native) == native);
  const Rule rule = RuleCompiler::compile(native);
  REQUIRE(rule.getTarget() == Rule::Target::Block);
  REQUIRE(rule.getName() == "Native rule");
}

TEST_CASE("Rule compiler rejects an unknown device type", "[RuleCompiler]")
{
  REQUIRE_THROWS_AS(RuleCompiler::compile("allow device-type teleporter serial \"SN-123\""), std::runtime_error);
}

TEST_CASE("Rule compiler rejects duplicate with-interface attributes", "[RuleCompiler]")
{
  REQUIRE_THROWS_AS(
    RuleCompiler::compile("allow device-type mass-storage with-interface 03:*:* serial \"SN-123\""), RuleParserError);
}

TEST_CASE("Rule compiler rejects a missing device type value", "[RuleCompiler]")
{
  REQUIRE_THROWS_AS(RuleCompiler::compile("allow device-type"), RuleParserError);
}

TEST_CASE("Rule compiler convenience API creates a legacy-compatible rule", "[RuleCompiler]")
{
  const Rule rule = RuleCompiler::compileDeviceRule("keyboard", "SN-\"123\"");
  REQUIRE(rule.getTarget() == Rule::Target::Allow);
  REQUIRE(rule.getSerial() == "SN-\"123\"");
  REQUIRE(rule.attributeWithInterface().get().typeString() == "03:01:01");
  REQUIRE(rule.attributeWithInterface().setOperator() == Rule::SetOperator::OneOf);

  const Rule reparsed = Rule::fromString(rule.toString());
  REQUIRE(reparsed.getSerial() == "SN-\"123\"");
  REQUIRE(reparsed.attributeWithInterface().get().typeString() == "03:01:01");
  REQUIRE(RuleCompiler::compileToString(rule.toString()) == rule.toString());
}

/* vim: set ts=2 sw=2 et */
