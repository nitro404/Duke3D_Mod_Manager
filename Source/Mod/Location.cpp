#include "Location.h"

#include <Utilities/RapidJSONUtilities.h>
#include <Utilities/StringUtilities.h>

#include <spdlog/spdlog.h>
#include <tinyxml2.h>

#include <sstream>

static const std::string XML_LOCATION_TOWN_ATTRIBUTE_NAME("town");
static const std::string XML_LOCATION_CITY_ATTRIBUTE_NAME("city");
static const std::string XML_LOCATION_PROVINCE_ATTRIBUTE_NAME("province");
static const std::string XML_LOCATION_STATE_ATTRIBUTE_NAME("state");
static const std::string XML_LOCATION_COUNTRY_ATTRIBUTE_NAME("country");

static constexpr const char * JSON_LOCATION_TOWN_PROPERTY_NAME = "town";
static constexpr const char * JSON_LOCATION_CITY_PROPERTY_NAME = "city";
static constexpr const char * JSON_LOCATION_PROVINCE_PROPERTY_NAME = "province";
static constexpr const char * JSON_LOCATION_STATE_PROPERTY_NAME = "state";
static constexpr const char * JSON_LOCATION_COUNTRY_PROPERTY_NAME = "country";

Location::Location() { }

Location::Location(Location && location) noexcept
	: m_town(std::move(location.m_town))
	, m_city(std::move(location.m_city))
	, m_province(std::move(location.m_province))
	, m_state(std::move(location.m_state))
	, m_country(std::move(location.m_country)) { }

Location::Location(const Location & location)
	: m_town(location.m_town)
	, m_city(location.m_city)
	, m_province(location.m_province)
	, m_state(location.m_state)
	, m_country(location.m_country) { }

Location & Location::operator = (Location && location) noexcept {
	if(this != &location) {
		m_town = std::move(location.m_town);
		m_city = std::move(location.m_city);
		m_province = std::move(location.m_province);
		m_state = std::move(location.m_state);
		m_country = std::move(location.m_country);
	}

	return *this;
}

Location & Location::operator = (const Location & location) {
	m_town = location.m_town;
	m_city = location.m_city;
	m_province = location.m_province;
	m_state = location.m_state;
	m_country = location.m_country;

	return *this;
}

Location::~Location() = default;

const std::string & Location::getTown() const {
	return m_town;
}

void Location::setTown(const std::string & town) {
	m_town = Utilities::trimString(town);
}

const std::string & Location::getCity() const {
	return m_city;
}

void Location::setCity(const std::string & city) {
	m_city = Utilities::trimString(city);
}

const std::string & Location::getProvince() const {
	return m_province;
}

void Location::setProvince(const std::string & province) {
	m_province = Utilities::trimString(province);
}

const std::string & Location::getState() const {
	return m_state;
}

void Location::setState(const std::string & state) {
	m_state = Utilities::trimString(state);
}

const std::string & Location::getProvinceOrState() const {
	return m_province.empty() ? m_state : m_province;
}

const std::string & Location::getCountry() const {
	return m_country;
}

void Location::setCountry(const std::string & country) {
	m_country = Utilities::trimString(country);
}

bool Location::hasValue() const {
	return !m_town.empty() ||
		   !m_city.empty() ||
		   !m_province.empty() ||
		   !m_state.empty() ||
		   !m_country.empty();
}

std::string Location::getDetails() const {
	std::stringstream locationStream;

	if(!m_town.empty()) {
		locationStream << m_town;
	}

	if(!m_city.empty()) {
		if(locationStream.tellp() != 0) {
			locationStream << ", ";
		}

		locationStream << m_city;
	}

	const std::string & provinceOrState = getProvinceOrState();

	if(!provinceOrState.empty()) {
		if(locationStream.tellp() != 0) {
			locationStream << ", ";
		}

		locationStream << provinceOrState;
	}

	if(!m_country.empty()) {
		if(locationStream.tellp() != 0) {
			locationStream << ", ";
		}

		locationStream << m_country;
	}

	return locationStream.str();
}

void Location::addToJSONObject(rapidjson::Value & locationValue, rapidjson::MemoryPoolAllocator<rapidjson::CrtAllocator> & allocator) const {
	if(!locationValue.IsObject()) {
		return;
	}

	if(!m_town.empty()) {
		rapidjson::Value townValue(m_town.c_str(), allocator);
		locationValue.AddMember(rapidjson::StringRef(JSON_LOCATION_TOWN_PROPERTY_NAME), townValue, allocator);
	}

	if(!m_city.empty()) {
		rapidjson::Value stateValue(m_city.c_str(), allocator);
		locationValue.AddMember(rapidjson::StringRef(JSON_LOCATION_CITY_PROPERTY_NAME), stateValue, allocator);
	}

	if(!m_province.empty()) {
		rapidjson::Value provinceValue(m_province.c_str(), allocator);
		locationValue.AddMember(rapidjson::StringRef(JSON_LOCATION_PROVINCE_PROPERTY_NAME), provinceValue, allocator);
	}

	if(!m_state.empty()) {
		rapidjson::Value countryValue(m_state.c_str(), allocator);
		locationValue.AddMember(rapidjson::StringRef(JSON_LOCATION_STATE_PROPERTY_NAME), countryValue, allocator);
	}

	if(!m_country.empty()) {
		rapidjson::Value countryValue(m_country.c_str(), allocator);
		locationValue.AddMember(rapidjson::StringRef(JSON_LOCATION_COUNTRY_PROPERTY_NAME), countryValue, allocator);
	}
}

void Location::addToXMLElement(tinyxml2::XMLElement * locationElement) const {
	if(locationElement == nullptr) {
		return;
	}

	if(!m_town.empty()) {
		locationElement->SetAttribute(XML_LOCATION_TOWN_ATTRIBUTE_NAME.c_str(), m_town.c_str());
	}

	if(!m_city.empty()) {
		locationElement->SetAttribute(XML_LOCATION_CITY_ATTRIBUTE_NAME.c_str(), m_city.c_str());
	}

	if(!m_province.empty()) {
		locationElement->SetAttribute(XML_LOCATION_PROVINCE_ATTRIBUTE_NAME.c_str(), m_province.c_str());
	}

	if(!m_state.empty()) {
		locationElement->SetAttribute(XML_LOCATION_STATE_ATTRIBUTE_NAME.c_str(), m_state.c_str());
	}

	if(!m_country.empty()) {
		locationElement->SetAttribute(XML_LOCATION_COUNTRY_ATTRIBUTE_NAME.c_str(), m_country.c_str());
	}
}

bool Location::parseFrom(const rapidjson::Value & locationValue) {
	if(!locationValue.IsObject()) {
		spdlog::error("Invalid location type: '{}', expected 'object'.", Utilities::typeToString(locationValue.GetType()));
		return false;
	}

	// parse the location town property
	if(locationValue.HasMember(JSON_LOCATION_TOWN_PROPERTY_NAME)) {
		const rapidjson::Value & townValue = locationValue[JSON_LOCATION_TOWN_PROPERTY_NAME];

		if(!townValue.IsString()) {
			spdlog::error("Location '{}' property has invalid type: '{}', expected 'string'.", JSON_LOCATION_TOWN_PROPERTY_NAME, Utilities::typeToString(townValue.GetType()));
			return false;
		}

		m_town = townValue.GetString();
	}

	// parse the location city property
	if(locationValue.HasMember(JSON_LOCATION_CITY_PROPERTY_NAME)) {
		const rapidjson::Value & cityValue = locationValue[JSON_LOCATION_CITY_PROPERTY_NAME];

		if(!cityValue.IsString()) {
			spdlog::error("Location '{}' property has invalid type: '{}', expected 'string'.", JSON_LOCATION_CITY_PROPERTY_NAME, Utilities::typeToString(cityValue.GetType()));
			return false;
		}

		m_city = cityValue.GetString();
	}

	// parse the location province property
	if(locationValue.HasMember(JSON_LOCATION_PROVINCE_PROPERTY_NAME)) {
		const rapidjson::Value & provinceValue = locationValue[JSON_LOCATION_PROVINCE_PROPERTY_NAME];

		if(!provinceValue.IsString()) {
			spdlog::error("Location '{}' property has invalid type: '{}', expected 'string'.", JSON_LOCATION_PROVINCE_PROPERTY_NAME, Utilities::typeToString(provinceValue.GetType()));
			return false;
		}

		m_province = provinceValue.GetString();
	}

	// parse the location state property
	if(locationValue.HasMember(JSON_LOCATION_STATE_PROPERTY_NAME)) {
		const rapidjson::Value & stateValue = locationValue[JSON_LOCATION_STATE_PROPERTY_NAME];

		if(!stateValue.IsString()) {
			spdlog::error("Location '{}' property has invalid type: '{}', expected 'string'.", JSON_LOCATION_STATE_PROPERTY_NAME, Utilities::typeToString(stateValue.GetType()));
			return false;
		}

		m_state = stateValue.GetString();
	}

	// parse the location country property
	if(locationValue.HasMember(JSON_LOCATION_COUNTRY_PROPERTY_NAME)) {
		const rapidjson::Value & countryValue = locationValue[JSON_LOCATION_COUNTRY_PROPERTY_NAME];

		if(!countryValue.IsString()) {
			spdlog::error("Location '{}' property has invalid type: '{}', expected 'string'.", JSON_LOCATION_COUNTRY_PROPERTY_NAME, Utilities::typeToString(countryValue.GetType()));
			return false;
		}

		m_country = countryValue.GetString();
	}

	return true;
}

bool Location::parseFrom(const tinyxml2::XMLElement * locationElement) {
	if(locationElement == nullptr) {
		return nullptr;
	}

	const char * townRaw = locationElement->Attribute(XML_LOCATION_TOWN_ATTRIBUTE_NAME.c_str());
	const char * cityRaw = locationElement->Attribute(XML_LOCATION_CITY_ATTRIBUTE_NAME.c_str());
	const char * provinceRaw = locationElement->Attribute(XML_LOCATION_PROVINCE_ATTRIBUTE_NAME.c_str());
	const char * stateRaw = locationElement->Attribute(XML_LOCATION_STATE_ATTRIBUTE_NAME.c_str());
	const char * countryRaw = locationElement->Attribute(XML_LOCATION_COUNTRY_ATTRIBUTE_NAME.c_str());

	if(townRaw != nullptr) {
		m_town = Utilities::trimString(townRaw);
	}

	if(cityRaw != nullptr) {
		m_city = Utilities::trimString(cityRaw);
	}

	if(provinceRaw != nullptr) {
		m_province = Utilities::trimString(provinceRaw);
	}

	if(stateRaw != nullptr) {
		m_state = Utilities::trimString(stateRaw);
	}

	if(countryRaw != nullptr) {
		m_country = Utilities::trimString(countryRaw);
	}

	return true;
}

bool Location::isValid() const {
	if(!m_province.empty() && !m_state.empty()) {
		return false;
	}

	if((!m_province.empty() || !m_state.empty()) && m_country.empty()) {
		return false;
	}

	if(!m_city.empty() && m_country.empty()) {
		return false;
	}

	if(!m_town.empty() && m_city.empty()) {
		return false;
	}

	return true;
}

bool Location::isValid(const Location * location) {
	return location != nullptr && location->isValid();
}

bool Location::operator == (const Location & location) const {
	return Utilities::areStringsEqual(m_town, location.m_town) &&
		   Utilities::areStringsEqual(m_city, location.m_city) &&
		   Utilities::areStringsEqual(m_province, location.m_province) &&
		   Utilities::areStringsEqual(m_state, location.m_state) &&
		   Utilities::areStringsEqual(m_country, location.m_country);
}

bool Location::operator != (const Location & location) const {
	return !operator == (location);
}
