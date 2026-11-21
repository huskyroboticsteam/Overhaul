#include "../include/utils/json.h"

namespace utils {

using json = nlohmann::json;

bool hasKey(const json& j, const std::string& key) {
	return j.contains(key);
}

bool validateKey(const json& j, const std::string& key, const json::value_t& type) {
	return hasKey(j, key) && j.at(key).type() == type;
}

bool validateKey(const json& j, const std::string& key, const std::unordered_set<json::value_t>& types) {
	return hasKey(j, key) && types.find(j.at(key).type()) != types.end();
}

bool validateOneOf(const json& j, const std::string& key, const std::unordered_set<std::string>& vals) {
	// TODO convert this to use Frozen sets
	return validateKey(j, key, json::value_t::string) &&
		   vals.find(static_cast<std::string>(j[key])) != vals.end();
}

bool validateRange(const json& j, const std::string& key, double min, double max) {
	if (validateKey(j, key, {json::value_t::number_float,
                             json::value_t::number_unsigned,
                             json::value_t::number_integer})) {
		double d = j[key];
		return min <= d && d <= max;
	}
	return false;
}

} // namespace utils