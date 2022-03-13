#include "ModTeam.h"

#include "Mod.h"
#include "ModTeamMember.h"

#include <Utilities/RapidJSONUtilities.h>
#include <Utilities/StringUtilities.h>

#include <fmt/core.h>
#include <tinyxml2.h>

#include <string_view>

static const std::string XML_MOD_TEAM_ELEMENT_NAME("team");
static const std::string XML_MOD_TEAM_NAME_ATTRIBUTE_NAME("name");
static const std::string XML_MOD_TEAM_COUNTY_ATTRIBUTE_NAME("county");
static const std::string XML_MOD_TEAM_CITY_ATTRIBUTE_NAME("city");
static const std::string XML_MOD_TEAM_PROVINCE_ATTRIBUTE_NAME("province");
static const std::string XML_MOD_TEAM_STATE_ATTRIBUTE_NAME("state");
static const std::string XML_MOD_TEAM_COUNTRY_ATTRIBUTE_NAME("country");
static const std::string XML_MOD_TEAM_EMAIL_ATTRIBUTE_NAME("email");
static const std::string XML_MOD_TEAM_WEBSITE_ATTRIBUTE_NAME("website");
static const std::vector<std::string> XML_MOD_TEAM_ATTRIBUTE_NAMES = {
	XML_MOD_TEAM_NAME_ATTRIBUTE_NAME,
	XML_MOD_TEAM_COUNTY_ATTRIBUTE_NAME,
	XML_MOD_TEAM_CITY_ATTRIBUTE_NAME,
	XML_MOD_TEAM_PROVINCE_ATTRIBUTE_NAME,
	XML_MOD_TEAM_STATE_ATTRIBUTE_NAME,
	XML_MOD_TEAM_COUNTRY_ATTRIBUTE_NAME,
	XML_MOD_TEAM_EMAIL_ATTRIBUTE_NAME,
	XML_MOD_TEAM_WEBSITE_ATTRIBUTE_NAME
};

static constexpr const char * JSON_MOD_TEAM_NAME_PROPERTY_NAME = "name";
static constexpr const char * JSON_MOD_TEAM_WEBSITE_PROPERTY_NAME = "website";
static constexpr const char * JSON_MOD_TEAM_EMAIL_PROPERTY_NAME = "email";
static constexpr const char * JSON_MOD_TEAM_COUNTY_PROPERTY_NAME = "county";
static constexpr const char * JSON_MOD_TEAM_CITY_PROPERTY_NAME = "city";
static constexpr const char * JSON_MOD_TEAM_PROVINCE_PROPERTY_NAME = "province";
static constexpr const char * JSON_MOD_TEAM_STATE_PROPERTY_NAME = "state";
static constexpr const char * JSON_MOD_TEAM_COUNTRY_PROPERTY_NAME = "country";
static constexpr const char * JSON_MOD_TEAM_MEMBERS_PROPERTY_NAME = "members";
static const std::vector<std::string_view> JSON_MOD_TEAM_PROPERTY_NAMES = {
	JSON_MOD_TEAM_NAME_PROPERTY_NAME,
	JSON_MOD_TEAM_WEBSITE_PROPERTY_NAME,
	JSON_MOD_TEAM_EMAIL_PROPERTY_NAME,
	JSON_MOD_TEAM_COUNTY_PROPERTY_NAME,
	JSON_MOD_TEAM_CITY_PROPERTY_NAME,
	JSON_MOD_TEAM_PROVINCE_PROPERTY_NAME,
	JSON_MOD_TEAM_STATE_PROPERTY_NAME,
	JSON_MOD_TEAM_COUNTRY_PROPERTY_NAME,
	JSON_MOD_TEAM_MEMBERS_PROPERTY_NAME,
};

ModTeam::ModTeam(const std::string & name, const std::string & website, const std::string & email)
	: m_name(Utilities::trimString(name))
	, m_website(Utilities::trimString(website))
	, m_email(Utilities::trimString(email))
	, m_parentMod(nullptr) { }

ModTeam::ModTeam(ModTeam && m) noexcept
	: m_name(std::move(m.m_name))
	, m_website(std::move(m.m_website))
	, m_email(std::move(m.m_email))
	, m_county(std::move(m.m_county))
	, m_city(std::move(m.m_city))
	, m_state(std::move(m.m_state))
	, m_country(std::move(m.m_country))
	, m_members(std::move(m.m_members))
	, m_parentMod(nullptr) {
	updateParent();
}

ModTeam::ModTeam(const ModTeam & m)
	: m_name(m.m_name)
	, m_website(m.m_website)
	, m_email(m.m_email)
	, m_county(m.m_county)
	, m_city(m.m_city)
	, m_state(m.m_state)
	, m_country(m.m_country)
	, m_parentMod(nullptr) {
	for(std::vector<std::shared_ptr<ModTeamMember>>::const_iterator i = m.m_members.begin(); i != m.m_members.end(); ++i) {
		m_members.push_back(std::make_shared<ModTeamMember>(**i));
	}

	updateParent();
}

ModTeam & ModTeam::operator = (ModTeam && m) noexcept {
	if(this != &m) {
		m_name = std::move(m.m_name);
		m_website = std::move(m.m_website);
		m_email = std::move(m.m_email);
		m_county = std::move(m.m_county);
		m_city = std::move(m.m_city);
		m_state = std::move(m.m_state);
		m_country = std::move(m.m_country);
		m_members = std::move(m.m_members);

		updateParent();
	}

	return *this;
}

ModTeam & ModTeam::operator = (const ModTeam & m) {
	m_members.clear();

	m_name = m.m_name;
	m_website = m.m_website;
	m_email = m.m_email;
	m_county = m.m_county;
	m_city = m.m_city;
	m_state = m.m_state;
	m_country = m.m_country;

	for(std::vector<std::shared_ptr<ModTeamMember>>::const_iterator i = m.m_members.begin(); i != m.m_members.end(); ++i) {
		m_members.push_back(std::make_shared<ModTeamMember>(**i));
	}

	updateParent();

	return *this;
}

ModTeam::~ModTeam() {
	m_parentMod = nullptr;
}

bool ModTeam::hasName() const {
	return !m_name.empty();
}

const std::string & ModTeam::getName() const {
	return m_name;
}

const std::string & ModTeam::getWebsite() const {
	return m_website;
}

const std::string & ModTeam::getEmail() const {
	return m_email;
}

const std::string & ModTeam::getCounty() const {
	return m_county;
}

const std::string & ModTeam::getCity() const {
	return m_city;
}

const std::string & ModTeam::getProvince() const {
	return m_province;
}

const std::string & ModTeam::getState() const {
	return m_state;
}

const std::string & ModTeam::getProvinceOrState() const {
	return m_province.empty() ? m_state : m_province;
}

const std::string & ModTeam::getCountry() const {
	return m_country;
}

const Mod * ModTeam::getParentMod() const {
	return m_parentMod;
}

void ModTeam::setName(const std::string & name) {
	m_name = Utilities::trimString(name);
}

void ModTeam::setWebsite(const std::string & website) {
	m_website = Utilities::trimString(website);
}

void ModTeam::setEmail(const std::string & email) {
	m_email = Utilities::trimString(email);
}

void ModTeam::setCounty(const std::string & county) {
	m_county = Utilities::trimString(county);
}

void ModTeam::setCity(const std::string & city) {
	m_city = Utilities::trimString(city);
}

void ModTeam::setProvince(const std::string & province) {
	m_province = Utilities::trimString(province);
}

void ModTeam::setState(const std::string & state) {
	m_state = Utilities::trimString(state);
}

void ModTeam::setCountry(const std::string & country) {
	m_country = Utilities::trimString(country);
}

void ModTeam::setParentMod(const Mod * mod) {
	m_parentMod = mod;
}

size_t ModTeam::numberOfMembers() const {
	return m_members.size();
}

bool ModTeam::hasMember(const ModTeamMember & member) const {
	for(std::vector<std::shared_ptr<ModTeamMember>>::const_iterator i = m_members.begin(); i != m_members.end(); i++) {
		if(Utilities::compareStringsIgnoreCase((*i)->getName(), member.getName()) == 0) {
			return true;
		}
	}

	return false;
}

bool ModTeam::hasMember(const std::string & memberName) const {
	if(memberName.empty()) {
		return false;
	}

	for(std::vector<std::shared_ptr<ModTeamMember>>::const_iterator i = m_members.begin(); i != m_members.end(); ++i) {
		if(Utilities::compareStringsIgnoreCase((*i)->getName(), memberName) == 0) {
			return true;
		}
	}

	return false;
}

size_t ModTeam::indexOfMember(const ModTeamMember & member) const {
	for(size_t i = 0; i < m_members.size(); i++) {
		if(Utilities::compareStringsIgnoreCase(m_members[i]->getName(), member.getName()) == 0) {
			return i;
		}
	}

	return std::numeric_limits<size_t>::max();
}

size_t ModTeam::indexOfMember(const std::string & memberName) const {
	if(memberName.empty()) {
		return std::numeric_limits<size_t>::max();
	}

	for(size_t i = 0; i < m_members.size(); i++) {
		if(Utilities::compareStringsIgnoreCase(m_members[i]->getName(), memberName) == 0) {
			return i;
		}
	}

	return std::numeric_limits<size_t>::max();
}

std::shared_ptr<ModTeamMember> ModTeam::getMember(size_t index) const {
	if(index >= m_members.size()) {
		return nullptr;
	}

	return m_members[index];
}

std::shared_ptr<ModTeamMember> ModTeam::getMember(const std::string & memberName) const {
	if(memberName.empty()) {
		return nullptr;
	}

	for(std::vector<std::shared_ptr<ModTeamMember>>::const_iterator i = m_members.begin(); i != m_members.end(); ++i) {
		if(Utilities::compareStringsIgnoreCase((*i)->getName(), memberName) == 0) {
			return *i;
		}
	}

	return nullptr;
}

const std::vector<std::shared_ptr<ModTeamMember>> & ModTeam::getMembers() const {
	return m_members;
}

bool ModTeam::addMember(const ModTeamMember & member) {
	if(!member.isValid() || hasMember(member)) {
		return false;
	}

	std::shared_ptr<ModTeamMember> newModTeamMember = std::make_shared<ModTeamMember>(member);
	newModTeamMember->setParentModTeam(this);

	m_members.push_back(newModTeamMember);

	return true;
}

bool ModTeam::removeMember(size_t index) {
	if(index >= m_members.size()) {
		return false;
	}

	m_members[index]->setParentModTeam(nullptr);
	m_members.erase(m_members.begin() + index);

	return true;
}

bool ModTeam::removeMember(const ModTeamMember & member) {
	for(std::vector<std::shared_ptr<ModTeamMember>>::const_iterator i = m_members.begin(); i != m_members.end(); ++i) {
		if(Utilities::compareStringsIgnoreCase((*i)->getName(), member.getName()) == 0) {
			(*i)->setParentModTeam(nullptr);
			m_members.erase(i);

			return true;
		}
	}

	return false;
}

bool ModTeam::removeMember(const std::string & memberName) {
	if(memberName.empty()) {
		return false;
	}

	for(std::vector<std::shared_ptr<ModTeamMember>>::const_iterator i = m_members.begin(); i != m_members.end(); ++i) {
		if(Utilities::compareStringsIgnoreCase((*i)->getName(), memberName) == 0) {
			(*i)->setParentModTeam(nullptr);
			m_members.erase(i);

			return true;
		}
	}

	return false;
}

void ModTeam::clearMembers() {
	m_members.clear();
}

void ModTeam::updateParent() {
	for(std::vector<std::shared_ptr<ModTeamMember>>::const_iterator i = m_members.begin(); i != m_members.end(); ++i) {
		(*i)->setParentModTeam(this);
	}
}

rapidjson::Value ModTeam::toJSON(rapidjson::MemoryPoolAllocator<rapidjson::CrtAllocator> & allocator) const {
	rapidjson::Value modTeamValue(rapidjson::kObjectType);

	if(!m_name.empty()) {
		rapidjson::Value nameValue(m_name.c_str(), allocator);
		modTeamValue.AddMember(rapidjson::StringRef(JSON_MOD_TEAM_NAME_PROPERTY_NAME), nameValue, allocator);
	}

	if(!m_website.empty()) {
		rapidjson::Value websiteValue(m_website.c_str(), allocator);
		modTeamValue.AddMember(rapidjson::StringRef(JSON_MOD_TEAM_WEBSITE_PROPERTY_NAME), websiteValue, allocator);
	}

	if(!m_email.empty()) {
		rapidjson::Value emailValue(m_email.c_str(), allocator);
		modTeamValue.AddMember(rapidjson::StringRef(JSON_MOD_TEAM_EMAIL_PROPERTY_NAME), emailValue, allocator);
	}

	if(!m_county.empty()) {
		rapidjson::Value countyValue(m_county.c_str(), allocator);
		modTeamValue.AddMember(rapidjson::StringRef(JSON_MOD_TEAM_COUNTY_PROPERTY_NAME), countyValue, allocator);
	}

	if(!m_city.empty()) {
		rapidjson::Value stateValue(m_city.c_str(), allocator);
		modTeamValue.AddMember(rapidjson::StringRef(JSON_MOD_TEAM_CITY_PROPERTY_NAME), stateValue, allocator);
	}

	if(!m_province.empty()) {
		rapidjson::Value provinceValue(m_province.c_str(), allocator);
		modTeamValue.AddMember(rapidjson::StringRef(JSON_MOD_TEAM_PROVINCE_PROPERTY_NAME), provinceValue, allocator);
	}

	if(!m_state.empty()) {
		rapidjson::Value countryValue(m_state.c_str(), allocator);
		modTeamValue.AddMember(rapidjson::StringRef(JSON_MOD_TEAM_STATE_PROPERTY_NAME), countryValue, allocator);
	}

	if(!m_country.empty()) {
		rapidjson::Value countryValue(m_country.c_str(), allocator);
		modTeamValue.AddMember(rapidjson::StringRef(JSON_MOD_TEAM_COUNTRY_PROPERTY_NAME), countryValue, allocator);
	}

	if(!m_members.empty()) {
		rapidjson::Value membersValue(rapidjson::kArrayType);

		for(std::vector<std::shared_ptr<ModTeamMember>>::const_iterator i = m_members.begin(); i != m_members.end(); ++i) {
			membersValue.PushBack((*i)->toJSON(allocator), allocator);
		}

		modTeamValue.AddMember(rapidjson::StringRef(JSON_MOD_TEAM_MEMBERS_PROPERTY_NAME), membersValue, allocator);
	}

	return modTeamValue;
}

tinyxml2::XMLElement * ModTeam::toXML(tinyxml2::XMLDocument * document) const {
	if(document == nullptr) {
		return nullptr;
	}

	tinyxml2::XMLElement * modTeamElement = document->NewElement(XML_MOD_TEAM_ELEMENT_NAME.c_str());

	if(!m_name.empty()) {
		modTeamElement->SetAttribute(XML_MOD_TEAM_NAME_ATTRIBUTE_NAME.c_str(), m_name.c_str());
	}

	if(!m_website.empty()) {
		modTeamElement->SetAttribute(XML_MOD_TEAM_WEBSITE_ATTRIBUTE_NAME.c_str(), m_website.c_str());
	}

	if(!m_email.empty()) {
		modTeamElement->SetAttribute(XML_MOD_TEAM_EMAIL_ATTRIBUTE_NAME.c_str(), m_email.c_str());
	}

	if(!m_county.empty()) {
		modTeamElement->SetAttribute(XML_MOD_TEAM_COUNTY_ATTRIBUTE_NAME.c_str(), m_county.c_str());
	}

	if(!m_city.empty()) {
		modTeamElement->SetAttribute(XML_MOD_TEAM_CITY_ATTRIBUTE_NAME.c_str(), m_city.c_str());
	}

	if(!m_province.empty()) {
		modTeamElement->SetAttribute(XML_MOD_TEAM_PROVINCE_ATTRIBUTE_NAME.c_str(), m_province.c_str());
	}

	if(!m_state.empty()) {
		modTeamElement->SetAttribute(XML_MOD_TEAM_STATE_ATTRIBUTE_NAME.c_str(), m_state.c_str());
	}

	if(!m_country.empty()) {
		modTeamElement->SetAttribute(XML_MOD_TEAM_COUNTRY_ATTRIBUTE_NAME.c_str(), m_country.c_str());
	}

	if(!m_members.empty()) {
		for(std::vector<std::shared_ptr<ModTeamMember>>::const_iterator i = m_members.begin(); i != m_members.end(); ++i) {
			modTeamElement->InsertEndChild((*i)->toXML(document));
		}
	}

	return modTeamElement;
}

std::unique_ptr<ModTeam> ModTeam::parseFrom(const rapidjson::Value & modTeamValue) {
	if(!modTeamValue.IsObject()) {
		fmt::print("Invalid mod team type: '{}', expected 'object'.\n", Utilities::typeToString(modTeamValue.GetType()));
		return nullptr;
	}

	// check for unhandled mod team properties
	bool propertyHandled = false;

	for(rapidjson::Value::ConstMemberIterator i = modTeamValue.MemberBegin(); i != modTeamValue.MemberEnd(); ++i) {
		propertyHandled = false;

		for(const std::string_view propertyName : JSON_MOD_TEAM_PROPERTY_NAMES) {
			if(i->name.GetString() == propertyName) {
				propertyHandled = true;
				break;
			}
		}

		if(!propertyHandled) {
			fmt::print("Mod team has unexpected property '{}'.\n", i->name.GetString());
			return nullptr;
		}
	}

	// parse the mod team province property
	std::string modTeamProvince;

	if(modTeamValue.HasMember(JSON_MOD_TEAM_PROVINCE_PROPERTY_NAME)) {
		const rapidjson::Value & modTeamProvinceValue = modTeamValue[JSON_MOD_TEAM_PROVINCE_PROPERTY_NAME];

		if(!modTeamProvinceValue.IsString()) {
			fmt::print("Mod team '{}' property has invalid type: '{}', expected 'string'.\n", JSON_MOD_TEAM_PROVINCE_PROPERTY_NAME, Utilities::typeToString(modTeamProvinceValue.GetType()));
			return nullptr;
		}

		modTeamProvince = modTeamProvinceValue.GetString();
	}

	// parse the mod team state property
	std::string modTeamState;

	if(modTeamValue.HasMember(JSON_MOD_TEAM_STATE_PROPERTY_NAME)) {
		const rapidjson::Value & modTeamStateValue = modTeamValue[JSON_MOD_TEAM_STATE_PROPERTY_NAME];

		if(!modTeamStateValue.IsString()) {
			fmt::print("Mod team '{}' property has invalid type: '{}', expected 'string'.\n", JSON_MOD_TEAM_STATE_PROPERTY_NAME, Utilities::typeToString(modTeamStateValue.GetType()));
			return nullptr;
		}

		modTeamState = modTeamStateValue.GetString();
	}

	if(!modTeamProvince.empty() && !modTeamState.empty()) {
		fmt::print("Mod team has both '{}' and '{}' attributes set to '{}' and '{}'' respectively, expected one or the other.\n", JSON_MOD_TEAM_PROVINCE_PROPERTY_NAME, JSON_MOD_TEAM_STATE_PROPERTY_NAME, modTeamProvince, modTeamState);
		return nullptr;
	}

	// initialize the mod team
	std::unique_ptr<ModTeam> newModTeam = std::make_unique<ModTeam>();

	if(!modTeamProvince.empty()) {
		newModTeam->setProvince(modTeamProvince);
	}

	if(!modTeamState.empty()) {
		newModTeam->setState(modTeamState);
	}

	// parse the mod team name property
	if(modTeamValue.HasMember(JSON_MOD_TEAM_NAME_PROPERTY_NAME)) {
		const rapidjson::Value & modTeamNameValue = modTeamValue[JSON_MOD_TEAM_NAME_PROPERTY_NAME];

		if(!modTeamNameValue.IsString()) {
			fmt::print("Mod team '{}' property has invalid type: '{}', expected 'string'.\n", JSON_MOD_TEAM_NAME_PROPERTY_NAME, Utilities::typeToString(modTeamNameValue.GetType()));
			return nullptr;
		}

		newModTeam->setName(modTeamNameValue.GetString());
	}

	// parse the mod team website property
	if(modTeamValue.HasMember(JSON_MOD_TEAM_WEBSITE_PROPERTY_NAME)) {
		const rapidjson::Value & modTeamWebsiteValue = modTeamValue[JSON_MOD_TEAM_WEBSITE_PROPERTY_NAME];

		if(!modTeamWebsiteValue.IsString()) {
			fmt::print("Mod team '{}' property has invalid type: '{}', expected 'string'.\n", JSON_MOD_TEAM_WEBSITE_PROPERTY_NAME, Utilities::typeToString(modTeamWebsiteValue.GetType()));
			return nullptr;
		}

		newModTeam->setWebsite(modTeamWebsiteValue.GetString());
	}

	// parse the mod team email property
	if(modTeamValue.HasMember(JSON_MOD_TEAM_EMAIL_PROPERTY_NAME)) {
		const rapidjson::Value & modTeamEmailValue = modTeamValue[JSON_MOD_TEAM_EMAIL_PROPERTY_NAME];

		if(!modTeamEmailValue.IsString()) {
			fmt::print("Mod team '{}' property has invalid type: '{}', expected 'string'.\n", JSON_MOD_TEAM_EMAIL_PROPERTY_NAME, Utilities::typeToString(modTeamEmailValue.GetType()));
			return nullptr;
		}

		newModTeam->setEmail(modTeamEmailValue.GetString());
	}

	// parse the mod team county property
	if(modTeamValue.HasMember(JSON_MOD_TEAM_COUNTY_PROPERTY_NAME)) {
		const rapidjson::Value & modTeamCountyValue = modTeamValue[JSON_MOD_TEAM_COUNTY_PROPERTY_NAME];

		if(!modTeamCountyValue.IsString()) {
			fmt::print("Mod team '{}' property has invalid type: '{}', expected 'string'.\n", JSON_MOD_TEAM_COUNTY_PROPERTY_NAME, Utilities::typeToString(modTeamCountyValue.GetType()));
			return nullptr;
		}

		newModTeam->setCounty(modTeamCountyValue.GetString());
	}

	// parse the mod team city property
	if(modTeamValue.HasMember(JSON_MOD_TEAM_CITY_PROPERTY_NAME)) {
		const rapidjson::Value & modTeamCityValue = modTeamValue[JSON_MOD_TEAM_CITY_PROPERTY_NAME];

		if(!modTeamCityValue.IsString()) {
			fmt::print("Mod team '{}' property has invalid type: '{}', expected 'string'.\n", JSON_MOD_TEAM_CITY_PROPERTY_NAME, Utilities::typeToString(modTeamCityValue.GetType()));
			return nullptr;
		}

		newModTeam->setCity(modTeamCityValue.GetString());
	}

	// parse the mod team country property
	if(modTeamValue.HasMember(JSON_MOD_TEAM_COUNTRY_PROPERTY_NAME)) {
		const rapidjson::Value & modTeamCountryValue = modTeamValue[JSON_MOD_TEAM_COUNTRY_PROPERTY_NAME];

		if(!modTeamCountryValue.IsString()) {
			fmt::print("Mod team '{}' property has invalid type: '{}', expected 'string'.\n", JSON_MOD_TEAM_COUNTRY_PROPERTY_NAME, Utilities::typeToString(modTeamCountryValue.GetType()));
			return nullptr;
		}

		newModTeam->setCountry(modTeamCountryValue.GetString());
	}

	// parse the mod team members property
	if(modTeamValue.HasMember(JSON_MOD_TEAM_MEMBERS_PROPERTY_NAME)) {
		const rapidjson::Value & modTeamMembersValue = modTeamValue[JSON_MOD_TEAM_MEMBERS_PROPERTY_NAME];

		if(!modTeamMembersValue.IsArray()) {
			fmt::print("Mod team '{}' property has invalid type: '{}', expected 'array'.\n", JSON_MOD_TEAM_MEMBERS_PROPERTY_NAME, Utilities::typeToString(modTeamMembersValue.GetType()));
			return nullptr;
		}

		std::shared_ptr<ModTeamMember> newModTeamMember;

		for(rapidjson::Value::ConstValueIterator i = modTeamMembersValue.Begin(); i != modTeamMembersValue.End(); ++i) {
			newModTeamMember = std::shared_ptr<ModTeamMember>(std::move(ModTeamMember::parseFrom(*i)).release());

			if(!ModTeamMember::isValid(newModTeamMember.get())) {
				fmt::print("Failed to parse mod team member #{}.\n", newModTeam->m_members.size() + 1);
				return nullptr;
			}

			newModTeamMember->setParentModTeam(newModTeam.get());

			if(newModTeam->hasMember(*newModTeamMember.get())) {
				fmt::print("Encountered duplicate mod team member #{}.\n", newModTeam->m_members.size() + 1);
				return nullptr;
			}

			newModTeam->m_members.push_back(newModTeamMember);
		}
	}

	return newModTeam;
}

std::unique_ptr<ModTeam> ModTeam::parseFrom(const tinyxml2::XMLElement * modTeamElement) {
	if(modTeamElement == nullptr) {
		return nullptr;
	}

	// verify element name
	if(modTeamElement->Name() != XML_MOD_TEAM_ELEMENT_NAME) {
		fmt::print("Invalid mod team element name: '{}', expected '{}'.\n", modTeamElement->Name(), XML_MOD_TEAM_ELEMENT_NAME);
		return nullptr;
	}

	// check for unhandled mod team element attributes
	bool attributeHandled = false;
	const tinyxml2::XMLAttribute * modTeamAttribute = modTeamElement->FirstAttribute();

	while(true) {
		if(modTeamAttribute == nullptr) {
			break;
		}

		attributeHandled = false;

		for(const std::string & attributeName : XML_MOD_TEAM_ATTRIBUTE_NAMES) {
			if(modTeamAttribute->Name() == attributeName) {
				attributeHandled = true;
				break;
			}
		}

		if(!attributeHandled) {
			fmt::print("Element '{}' has unexpected attribute '{}'.\n", XML_MOD_TEAM_ELEMENT_NAME, modTeamAttribute->Name());
			return nullptr;
		}

		modTeamAttribute = modTeamAttribute->Next();
	}

	// read the mod team attributes
	const char * teamName = modTeamElement->Attribute(XML_MOD_TEAM_NAME_ATTRIBUTE_NAME.c_str());
	const char * teamCounty = modTeamElement->Attribute(XML_MOD_TEAM_COUNTY_ATTRIBUTE_NAME.c_str());
	const char * teamCity = modTeamElement->Attribute(XML_MOD_TEAM_CITY_ATTRIBUTE_NAME.c_str());
	const char * teamProvince = modTeamElement->Attribute(XML_MOD_TEAM_PROVINCE_ATTRIBUTE_NAME.c_str());
	const char * teamState = modTeamElement->Attribute(XML_MOD_TEAM_STATE_ATTRIBUTE_NAME.c_str());
	const char * teamCountry = modTeamElement->Attribute(XML_MOD_TEAM_COUNTRY_ATTRIBUTE_NAME.c_str());
	const char * teamEmail = modTeamElement->Attribute(XML_MOD_TEAM_EMAIL_ATTRIBUTE_NAME.c_str());
	const char * teamWebsite = modTeamElement->Attribute(XML_MOD_TEAM_WEBSITE_ATTRIBUTE_NAME.c_str());

	if(Utilities::stringLength(teamProvince) != 0 && Utilities::stringLength(teamState) != 0) {
		fmt::print("Element '{}' has both '{}' and '{}' attributes set to '{}' and '{}'' respectively, expected one or the other.\n", XML_MOD_TEAM_ELEMENT_NAME, XML_MOD_TEAM_PROVINCE_ATTRIBUTE_NAME, XML_MOD_TEAM_STATE_ATTRIBUTE_NAME, teamProvince, teamState);
		return nullptr;
	}

	// initialize the mod team
	std::unique_ptr<ModTeam> newModTeam = std::make_unique<ModTeam>(teamName == nullptr ? "" : teamName, teamWebsite == nullptr ? "" : teamWebsite, teamEmail == nullptr ? "" : teamEmail);

	if(teamCounty != nullptr) {
		newModTeam->setCounty(teamCounty);
	}

	if(teamCity != nullptr) {
		newModTeam->setCity(teamCity);
	}

	if(teamProvince != nullptr) {
		newModTeam->setProvince(teamProvince);
	}

	if(teamState != nullptr) {
		newModTeam->setState(teamState);
	}

	if(teamCountry != nullptr) {
		newModTeam->setCountry(teamCountry);
	}

	// iterate over all of the mod team member elements
	const tinyxml2::XMLElement * modTeamMemberElement = modTeamElement->FirstChildElement();

	if(modTeamMemberElement != nullptr) {
		std::shared_ptr<ModTeamMember> newModTeamMember;

		while(true) {
			if(modTeamMemberElement == nullptr) {
				break;
			}

			newModTeamMember = std::shared_ptr<ModTeamMember>(std::move(ModTeamMember::parseFrom(modTeamMemberElement)).release());

			if(!ModTeamMember::isValid(newModTeamMember.get())) {
				fmt::print("Failed to parse mod team member #{}.\n", newModTeam->m_members.size() + 1);
				return nullptr;
			}

			newModTeamMember->setParentModTeam(newModTeam.get());

			if(newModTeam->hasMember(*newModTeamMember.get())) {
				fmt::print("Encountered duplicate mod team member #{}.\n", newModTeam->m_members.size() + 1);
				return nullptr;
			}

			newModTeam->m_members.push_back(newModTeamMember);

			modTeamMemberElement = modTeamMemberElement->NextSiblingElement();
		}
	}

	return newModTeam;
}

bool ModTeam::isValid() const {
	for(std::vector<std::shared_ptr<ModTeamMember>>::const_iterator i = m_members.begin(); i != m_members.end(); i++) {
		if(!(*i)->isValid()) {
			return false;
		}

		if((*i)->getParentModTeam() != this) {
			return false;
		}

		for(std::vector<std::shared_ptr<ModTeamMember>>::const_iterator j = i + 1; j != m_members.end(); j++) {
			if(Utilities::compareStringsIgnoreCase((*i)->getName(), (*j)->getName()) == 0) {
				return false;
			}
		}
	}

	return true;
}

bool ModTeam::isValid(const ModTeam * modTeam) {
	return modTeam != nullptr && modTeam->isValid();
}

bool ModTeam::operator == (const ModTeam & m) const {
	if(Utilities::compareStringsIgnoreCase(m_name, m.m_name) != 0 ||
	   Utilities::compareStringsIgnoreCase(m_website, m.m_website) != 0 ||
	   Utilities::compareStringsIgnoreCase(m_email, m.m_email) != 0 ||
	   Utilities::compareStringsIgnoreCase(m_county, m.m_county) != 0 ||
	   Utilities::compareStringsIgnoreCase(m_city, m.m_city) != 0 ||
	   Utilities::compareStringsIgnoreCase(m_province, m.m_province) != 0 ||
	   Utilities::compareStringsIgnoreCase(m_state, m.m_state) != 0 ||
	   Utilities::compareStringsIgnoreCase(m_country, m.m_country) != 0 ||
	   (!m_province.empty() && !m_state.empty()) ||
	   m_members.size() != m.m_members.size()) {
		return false;
	}

	for(size_t i = 0; i < m_members.size(); i++) {
		if(*m_members[i] != *m.m_members[i]) {
			return false;
		}
	}

	return true;
}

bool ModTeam::operator != (const ModTeam & m) const {
	return !operator == (m);
}
