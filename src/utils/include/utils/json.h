#pragma once

#include <nlohmann/json.hpp>

#include <unordered_set>

namespace utils {

using json = nlohmann::json;

/** @brief Check if the given json object has the given key. */
bool hasKey(const json& j, const std::string& key);

/** @brief Check if the given json object has the given key with the given type. */
bool validateKey(const json& j, const std::string& key, const json::value_t& type);

/** @brief Check if the given json object has the given key, with a type in the given set of types. */
bool validateKey(const json& j, const std::string& key, const std::unordered_set<json::value_t>& types);

/**
 * @brief Check if the value in the given json object at the given key is a string in the given set of allowed values.
 */
bool validateOneOf(const json& j, const std::string& key, const std::unordered_set<std::string>& vals);

/**
 * @brief Check if the value in the given json object at the given key is a floating-point number between min and max, inclusive.
 */
bool validateRange(const json& j, const std::string& key, double min, double max);

} // namespace utils