#ifdef HAVE_BUILD_CONFIG_H
  #include <build-config.h>
#endif

#include "usbguard/RuleCompiler.hpp"

#include "Utility.hpp"
#include "usbguard/DeviceType.hpp"
#include "usbguard/RuleParser.hpp"
#include "usbguard/USB.hpp"

#include <algorithm>
#include <cctype>
#include <stdexcept>
#include <vector>

namespace usbguard
{
  namespace
  {
    enum class TokenKind {
      Word,
      Quoted,
      Symbol,
      Space,
      Comment
    };

    struct Token {
      TokenKind kind;
      std::string text;
    };

    std::vector<Token> tokenize(const std::string& rule_spec)
    {
      std::vector<Token> tokens;
      size_t offset = 0;

      while (offset < rule_spec.size()) {
        const char character = rule_spec[offset];

        if (std::isspace(static_cast<unsigned char>(character))) {
          const size_t begin = offset;

          while (offset < rule_spec.size() && std::isspace(static_cast<unsigned char>(rule_spec[offset]))) {
            ++offset;
          }

          tokens.push_back({ TokenKind::Space, rule_spec.substr(begin, offset - begin) });
          continue;
        }

        if (character == '#') {
          const size_t begin = offset;

          while (offset < rule_spec.size() && rule_spec[offset] != '\n') {
            ++offset;
          }

          tokens.push_back({ TokenKind::Comment, rule_spec.substr(begin, offset - begin) });
          continue;
        }

        if (character == '"') {
          const size_t begin = offset++;
          bool escaped = false;
          bool closed = false;

          while (offset < rule_spec.size()) {
            const char current = rule_spec[offset++];

            if (escaped) {
              escaped = false;
            }
            else if (current == '\\') {
              escaped = true;
            }
            else if (current == '"') {
              closed = true;
              break;
            }
          }

          if (!closed) {
            throw RuleParserError(rule_spec, "unterminated quoted string", "", 0, static_cast<unsigned int>(begin));
          }

          tokens.push_back({ TokenKind::Quoted, rule_spec.substr(begin, offset - begin) });
          continue;
        }

        if (character == '{' || character == '}') {
          tokens.push_back({ TokenKind::Symbol, std::string(1, character) });
          ++offset;
          continue;
        }

        const size_t begin = offset;

        while (offset < rule_spec.size()) {
          const char current = rule_spec[offset];

          if (std::isspace(static_cast<unsigned char>(current)) || current == '"' || current == '{' || current == '}'
              || current == '#') {
            break;
          }

          ++offset;
        }

        tokens.push_back({ TokenKind::Word, rule_spec.substr(begin, offset - begin) });
      }

      return tokens;
    }

    bool isSetOperator(const std::string& value)
    {
      return value == "all-of" || value == "one-of" || value == "none-of" || value == "equals"
        || value == "equals-ordered" || value == "match-all";
    }

    std::string unquote(const std::string& value)
    {
      if (value.size() >= 2 && value.front() == '"' && value.back() == '"') {
        return Utility::unescapeString(value.substr(1, value.size() - 2));
      }

      return value;
    }

    size_t skipSpaces(const std::vector<Token>& tokens, size_t offset)
    {
      while (offset < tokens.size() && tokens[offset].kind == TokenKind::Space) {
        ++offset;
      }

      return offset;
    }
  } /* namespace */

  std::string RuleCompiler::compileToString(const std::string& rule_spec)
  {
    const std::vector<Token> tokens = tokenize(rule_spec);
    std::string compiled;
    size_t offset = 0;

    while (offset < tokens.size()) {
      const Token& token = tokens[offset];

      if (token.kind != TokenKind::Word || token.text != "device-type") {
        compiled.append(token.text);
        ++offset;
        continue;
      }

      size_t next = skipSpaces(tokens, offset + 1);
      std::string set_operator = "one-of";

      if (next < tokens.size() && tokens[next].kind == TokenKind::Word && isSetOperator(tokens[next].text)) {
        set_operator = tokens[next].text;
        next = skipSpaces(tokens, next + 1);
      }

      bool braced = false;

      if (next < tokens.size() && tokens[next].kind == TokenKind::Symbol && tokens[next].text == "{") {
        braced = true;
        ++next;
      }

      std::vector<std::string> values;

      if (braced) {
        while (true) {
          next = skipSpaces(tokens, next);

          if (next >= tokens.size()) {
            throw RuleParserError(rule_spec, "unterminated device-type set", "", 0, 0);
          }

          if (tokens[next].kind == TokenKind::Symbol && tokens[next].text == "}") {
            ++next;
            break;
          }

          if (tokens[next].kind != TokenKind::Word && tokens[next].kind != TokenKind::Quoted) {
            throw RuleParserError(rule_spec, "invalid device-type value", "", 0, 0);
          }

          values.push_back(unquote(tokens[next].text));
          ++next;
        }
      }
      else {
        next = skipSpaces(tokens, next);

        if (next >= tokens.size() || (tokens[next].kind != TokenKind::Word && tokens[next].kind != TokenKind::Quoted)) {
          throw RuleParserError(rule_spec, "missing device-type value", "", 0, 0);
        }

        values.push_back(unquote(tokens[next].text));
        ++next;
      }

      if (values.empty()) {
        throw RuleParserError(rule_spec, "device-type set must not be empty", "", 0, 0);
      }

      compiled.append("with-interface ");
      compiled.append(set_operator);
      compiled.append(" { ");

      for (const auto& value : values) {
        compiled.append(DeviceType::toInterfaceType(value));
        compiled.append(" ");
      }

      compiled.append("}");
      offset = next;
    }

    return compiled;
  }

  Rule RuleCompiler::compile(const std::string& rule_spec, const std::string& file, const size_t line, const bool trace)
  {
    return parseRuleFromString(compileToString(rule_spec), file, line, trace);
  }

  Rule RuleCompiler::compileDeviceRule(
    const std::string& device_type, const std::string& serial_number, const Rule::Target target)
  {
    if (serial_number.empty()) {
      throw std::runtime_error("Cannot compile a device rule without a serial number");
    }

    const std::string interface_type = DeviceType::toInterfaceType(device_type);
    Rule rule;
    rule.setTarget(target);
    rule.setSerial(serial_number);
    rule.attributeWithInterface().set({ USBInterfaceType(interface_type) }, Rule::SetOperator::OneOf);
    return rule;
  }
} /* namespace usbguard */

/* vim: set ts=2 sw=2 et */
