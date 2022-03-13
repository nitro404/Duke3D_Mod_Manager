#include "ModTeamMember.h"

#include "Mod.h"
#include "ModTeam.h"

#include <Utilities/RapidJSONUtilities.h>
#include <Utilities/StringUtilities.h>

#include <fmt/core.h>
#include <tinyxml2.h>

#include <string_view>
#include <vector>

static const std::string XML_MOD_TEAM_MEMBER_ELEMENT_NAME("member");
static const std::string XML_MOD_TEAM_MEMBER_NAME_ATTRIBUTE_NAME("name");
static const std::string XML_MOD_TEAM_MEMBER_ALIAS_ATTRIBUTE_NAME("alias");
static const std::string XML_MOD_TEAM_MEMBER_EMAIL_ATTRIBUTE_NAME("email");
static const std::string XML_MOD_TEAM_MEMBER_WEBSITE_ATTRIBUTE_NAME("website");
static const std::string XML_MOD_TEAM_MEMBER_AIM_ATTRIBUTE_NAME("aim");
static const std::string XML_MOD_TEAM_MEMBER_ICQ_ATTRIBUTE_NAME("icq");
static const std::string XML_MOD_TEAM_MEMBER_PHONE_NUMBER_ATTRIBUTE_NAME("phone_number");
static const std::vector<std::string> XML_MOD_TEAM_MEMBER_ATTRIBUTE_NAMES = {
	XML_MOD_TEAM_MEMBER_NAME_ATTRIBUTE_NAME,
	XML_MOD_TEAM_MEMBER_ALIAS_ATTRIBUTE_NAME,
	XML_MOD_TEAM_MEMBER_EMAIL_ATTRIBUTE_NAME,
	XML_MOD_TEAM_MEMBER_WEBSITE_ATTRIBUTE_NAME,
	XML_MOD_TEAM_MEMBER_AIM_ATTRIBUTE_NAME,
	XML_MOD_TEAM_MEMBER_ICQ_ATTRIBUTE_NAME,
	XML_MOD_TEAM_MEMBER_PHONE_NUMBER_ATTRIBUTE_NAME
};

static constexpr const char * JSON_MOD_TEAM_MEMBER_NAME_PROPERTY_NAME = "name";
static constexpr const char * JSON_MOD_TEAM_MEMBER_ALIAS_PROPERTY_NAME = "alias";
static constexpr const char * JSON_MOD_TEAM_MEMBER_EMAIL_PROPERTY_NAME = "email";
static constexpr const char * JSON_MOD_TEAM_MEMBER_WEBSITE_PROPERTY_NAME = "website";
static constexpr const char * JSON_MOD_TEAM_MEMBER_AIM_PROPERTY_NAME = "aim";
static constexpr const char * JSON_MOD_TEAM_MEMBER_ICQ_PROPERTY_NAME = "icq";
static constexpr const char * JSON_MOD_TEAM_MEMBER_PHONE_NUMBER_PROPERTY_NAME = "phoneNumber";
static const std::vector<std::string> JSON_MOD_TEAM_MEMBER_PROPERTY_NAMES = {
	JSON_MOD_TEAM_MEMBER_NAME_PROPERTY_NAME,
	JSON_MOD_TEAM_MEMBER_ALIAS_PROPERTY_NAME,
	JSON_MOD_TEAM_MEMBER_EMAIL_PROPERTY_NAME,
	JSON_MOD_TEAM_MEMBER_WEBSITE_PROPERTY_NAME,
	JSON_MOD_TEAM_MEMBER_AIM_PROPERTY_NAME,
	JSON_MOD_TEAM_MEMBER_ICQ_PROPERTY_NAME,
	JSON_MOD_TEAM_MEMBER_PHONE_NUMBER_PROPERTY_NAME,
};

ModTeamMember::ModTeamMember(const std::string & name, const std::string & alias, const std::string & email, const std::string & website)
	: m_name(Utilities::trimString(name))
	, m_alias(Utilities::trimString(alias))
	, m_email(Utilities::trimString(email))
	, m_website(Utilities::trimString(website))
	, m_parentModTeam(nullptr) { }

ModTeamMember::ModTeamMember(ModTeamMember && m) noexcept
	: m_name(std::move(m.m_name))
	, m_alias(std::move(m.m_alias))
	, m_email(std::move(m.m_email))
	, m_website(std::move(m.m_website))
	, m_aim(std::move(m.m_aim))
	, m_icq(std::move(m.m_icq))
	, m_phoneNumber(std::move(m.m_phoneNumber))
	, m_parentModTeam(nullptr) { }

ModTeamMember::ModTeamMember(const ModTeamMember & m)
	: m_name(m.m_name)
	, m_alias(m.m_alias)
	, m_email(m.m_email)
	, m_website(m.m_website)
	, m_aim(m.m_aim)
	, m_icq(m.m_icq)
	, m_phoneNumber(m.m_phoneNumber)
	, m_parentModTeam(nullptr) { }

ModTeamMember & ModTeamMember::operator = (ModTeamMember && m) noexcept {
	if(this != &m) {
		m_name = std::move(m.m_name);
		m_alias = std::move(m.m_alias);
		m_email = std::move(m.m_email);
		m_website = std::move(m.m_website);
		m_aim = std::move(m.m_aim);
		m_icq = std::move(m.m_icq);
		m_phoneNumber = std::move(m.m_phoneNumber);
	}

	return *this;
}

ModTeamMember & ModTeamMember::operator = (const ModTeamMember & m) {
	m_name = m.m_name;
	m_alias = m.m_alias;
	m_email = m.m_email;
	m_website = m.m_website;
	m_aim = m.m_aim;
	m_icq = m.m_icq;
	m_phoneNumber = m.m_phoneNumber;

	return *this;
}

ModTeamMember::~ModTeamMember() {
	m_parentModTeam = nullptr;
}

const std::string & ModTeamMember::getName() const {
	return m_name;
}

const std::string & ModTeamMember::getAlias() const {
	return m_alias;
}

const std::string & ModTeamMember::getEmail() const {
	return m_email;
}

const std::string & ModTeamMember::getWebsite() const {
	return m_website;
}

const std::string & ModTeamMember::getAIM() const {
	return m_aim;
}

const std::string & ModTeamMember::getICQ() const {
	return m_icq;
}

const std::string & ModTeamMember::getPhoneNumber() const {
	return m_phoneNumber;
}

const Mod * ModTeamMember::getParentMod() const {
	if(m_parentModTeam == nullptr) {
		return nullptr;
	}

	return m_parentModTeam->getParentMod();
}

const ModTeam * ModTeamMember::getParentModTeam() const {
	return m_parentModTeam;
}

void ModTeamMember::setName(const std::string & name) {
	m_name = Utilities::trimString(name);
}

void ModTeamMember::setAlias(const std::string& alias) {
	m_alias = Utilities::trimString(alias);
}

void ModTeamMember::setEmail(const std::string& email) {
	m_email = Utilities::trimString(email);
}

void ModTeamMember::setWebsite(const std::string & website) {
	m_website = Utilities::trimString(website);
}

void ModTeamMember::setAIM(const std::string & aim) {
	m_aim = Utilities::trimString(aim);
}

void ModTeamMember::setICQ(const std::string & icq) {
	m_icq = Utilities::trimString(icq);
}

void ModTeamMember::setPhoneNumber(const std::string & phoneNumber) {
	m_phoneNumber = Utilities::trimString(phoneNumber);
}

void ModTeamMember::setParentModTeam(const ModTeam * modTeam) {
	m_parentModTeam = modTeam;
}

rapidjson::Value ModTeamMember::toJSON(rapidjson::MemoryPoolAllocator<rapidjson::CrtAllocator> & allocator) const {
	rapidjson::Value modTeamMemberValue(rapidjson::kObjectType);

	rapidjson::Value nameValue(m_name.c_str(), allocator);
	modTeamMemberValue.AddMember(rapidjson::StringRef(JSON_MOD_TEAM_MEMBER_NAME_PROPERTY_NAME), nameValue, allocator);

	if(!m_alias.empty()) {
		rapidjson::Value aliasValue(m_alias.c_str(), allocator);
		modTeamMemberValue.AddMember(rapidjson::StringRef(JSON_MOD_TEAM_MEMBER_ALIAS_PROPERTY_NAME), aliasValue, allocator);
	}

	if(!m_email.empty()) {
		rapidjson::Value emailValue(m_email.c_str(), allocator);
		modTeamMemberValue.AddMember(rapidjson::StringRef(JSON_MOD_TEAM_MEMBER_EMAIL_PROPERTY_NAME), emailValue, allocator);
	}

	if(!m_website.empty()) {
		rapidjson::Value websiteValue(m_website.c_str(), allocator);
		modTeamMemberValue.AddMember(rapidjson::StringRef(JSON_MOD_TEAM_MEMBER_WEBSITE_PROPERTY_NAME), websiteValue, allocator);
	}

	if(!m_aim.empty()) {
		rapidjson::Value aimValue(m_aim.c_str(), allocator);
		modTeamMemberValue.AddMember(rapidjson::StringRef(JSON_MOD_TEAM_MEMBER_AIM_PROPERTY_NAME), aimValue, allocator);
	}

	if(!m_icq.empty()) {
		rapidjson::Value icqValue(m_icq.c_str(), allocator);
		modTeamMemberValue.AddMember(rapidjson::StringRef(JSON_MOD_TEAM_MEMBER_ICQ_PROPERTY_NAME), icqValue, allocator);
	}

	if(!m_phoneNumber.empty()) {
		rapidjson::Value phoneNumberValue(m_phoneNumber.c_str(), allocator);
		modTeamMemberValue.AddMember(rapidjson::StringRef(JSON_MOD_TEAM_MEMBER_PHONE_NUMBER_PROPERTY_NAME), phoneNumberValue, allocator);
	}

	return modTeamMemberValue;
}

tinyxml2::XMLElement * ModTeamMember::toXML(tinyxml2::XMLDocument * document) const {
	if(document == nullptr) {
		return nullptr;
	}

	tinyxml2::XMLElement * modTeamMemberElement = document->NewElement(XML_MOD_TEAM_MEMBER_ELEMENT_NAME.c_str());

	modTeamMemberElement->SetAttribute(XML_MOD_TEAM_MEMBER_NAME_ATTRIBUTE_NAME.c_str(), m_name.c_str());

	if(!m_alias.empty()) {
		modTeamMemberElement->SetAttribute(XML_MOD_TEAM_MEMBER_ALIAS_ATTRIBUTE_NAME.c_str(), m_alias.c_str());
	}

	if(!m_email.empty()) {
		modTeamMemberElement->SetAttribute(XML_MOD_TEAM_MEMBER_EMAIL_ATTRIBUTE_NAME.c_str(), m_email.c_str());
	}

	if(!m_website.empty()) {
		modTeamMemberElement->SetAttribute(XML_MOD_TEAM_MEMBER_WEBSITE_ATTRIBUTE_NAME.c_str(), m_website.c_str());
	}

	if(!m_aim.empty()) {
		modTeamMemberElement->SetAttribute(XML_MOD_TEAM_MEMBER_AIM_ATTRIBUTE_NAME.c_str(), m_aim.c_str());
	}

	if(!m_icq.empty()) {
		modTeamMemberElement->SetAttribute(XML_MOD_TEAM_MEMBER_ICQ_ATTRIBUTE_NAME.c_str(), m_icq.c_str());
	}

	if(!m_phoneNumber.empty()) {
		modTeamMemberElement->SetAttribute(XML_MOD_TEAM_MEMBER_PHONE_NUMBER_ATTRIBUTE_NAME.c_str(), m_phoneNumber.c_str());
	}

	return modTeamMemberElement;
}

std::unique_ptr<ModTeamMember> ModTeamMember::parseFrom(const rapidjson::Value & modTeamMemberValue) {
	if(!modTeamMemberValue.IsObject()) {
		fmt::print("Invalid mod team member type: '{}', expected 'object'.\n", Utilities::typeToString(modTeamMemberValue.GetType()));
		return nullptr;
	}

	// check for unhandled mod team member properties
	bool propertyHandled = false;

	for(rapidjson::Value::ConstMemberIterator i = modTeamMemberValue.MemberBegin(); i != modTeamMemberValue.MemberEnd(); ++i) {
		propertyHandled = false;

		for(const std::string_view propertyName : JSON_MOD_TEAM_MEMBER_PROPERTY_NAMES) {
			if(i->name.GetString() == propertyName) {
				propertyHandled = true;
				break;
			}
		}

		if(!propertyHandled) {
			fmt::print("Mod team member has unexpected property '{}'.\n", i->name.GetString());
			return nullptr;
		}
	}

	// parse mod team member name
	if(!modTeamMemberValue.HasMember(JSON_MOD_TEAM_MEMBER_NAME_PROPERTY_NAME)) {
		fmt::print("Mod team member is missing '{}' property'.\n", JSON_MOD_TEAM_MEMBER_NAME_PROPERTY_NAME);
		return nullptr;
	}

	const rapidjson::Value & modTeamMemberNameValue = modTeamMemberValue[JSON_MOD_TEAM_MEMBER_NAME_PROPERTY_NAME];

	if(!modTeamMemberNameValue.IsString()) {
		fmt::print("Mod team member has an invalid '{}' property type: '{}', expected 'string'.\n", JSON_MOD_TEAM_MEMBER_NAME_PROPERTY_NAME, Utilities::typeToString(modTeamMemberNameValue.GetType()));
		return nullptr;
	}

	std::string modTeamMemberName(Utilities::trimString(modTeamMemberNameValue.GetString()));

	if(modTeamMemberName.empty()) {
		fmt::print("Mod team member '{}' property cannot be empty.\n", JSON_MOD_TEAM_MEMBER_NAME_PROPERTY_NAME);
		return nullptr;
	}

	// initialize the mod team member
	std::unique_ptr<ModTeamMember> newModTeamMember = std::make_unique<ModTeamMember>(modTeamMemberName);

	// parse the mod team member alias property
	if(modTeamMemberValue.HasMember(JSON_MOD_TEAM_MEMBER_ALIAS_PROPERTY_NAME)) {
		const rapidjson::Value & modTeamMemberAliasValue = modTeamMemberValue[JSON_MOD_TEAM_MEMBER_ALIAS_PROPERTY_NAME];

		if(!modTeamMemberAliasValue.IsString()) {
			fmt::print("Mod team member '{}' property has invalid type: '{}', expected 'string'.\n", JSON_MOD_TEAM_MEMBER_ALIAS_PROPERTY_NAME, Utilities::typeToString(modTeamMemberAliasValue.GetType()));
			return nullptr;
		}

		newModTeamMember->setAlias(modTeamMemberAliasValue.GetString());
	}

	// parse the mod team member email property
	if(modTeamMemberValue.HasMember(JSON_MOD_TEAM_MEMBER_EMAIL_PROPERTY_NAME)) {
		const rapidjson::Value & modTeamMemberEmailValue = modTeamMemberValue[JSON_MOD_TEAM_MEMBER_EMAIL_PROPERTY_NAME];

		if(!modTeamMemberEmailValue.IsString()) {
			fmt::print("Mod team member '{}' property has invalid type: '{}', expected 'string'.\n", JSON_MOD_TEAM_MEMBER_EMAIL_PROPERTY_NAME, Utilities::typeToString(modTeamMemberEmailValue.GetType()));
			return nullptr;
		}

		newModTeamMember->setEmail(modTeamMemberEmailValue.GetString());
	}

	// parse the mod team member website property
	if(modTeamMemberValue.HasMember(JSON_MOD_TEAM_MEMBER_WEBSITE_PROPERTY_NAME)) {
		const rapidjson::Value & modTeamMemberWebsiteValue = modTeamMemberValue[JSON_MOD_TEAM_MEMBER_WEBSITE_PROPERTY_NAME];

		if(!modTeamMemberWebsiteValue.IsString()) {
			fmt::print("Mod team member '{}' property has invalid type: '{}', expected 'string'.\n", JSON_MOD_TEAM_MEMBER_WEBSITE_PROPERTY_NAME, Utilities::typeToString(modTeamMemberWebsiteValue.GetType()));
			return nullptr;
		}

		newModTeamMember->setWebsite(modTeamMemberWebsiteValue.GetString());
	}

	// parse the mod team member aim property
	if(modTeamMemberValue.HasMember(JSON_MOD_TEAM_MEMBER_AIM_PROPERTY_NAME)) {
		const rapidjson::Value & modTeamMemberAIMValue = modTeamMemberValue[JSON_MOD_TEAM_MEMBER_AIM_PROPERTY_NAME];

		if(!modTeamMemberAIMValue.IsString()) {
			fmt::print("Mod team member '{}' property has invalid type: '{}', expected 'string'.\n", JSON_MOD_TEAM_MEMBER_AIM_PROPERTY_NAME, Utilities::typeToString(modTeamMemberAIMValue.GetType()));
			return nullptr;
		}

		newModTeamMember->setAIM(modTeamMemberAIMValue.GetString());
	}

	// parse the mod team member icq property
	if(modTeamMemberValue.HasMember(JSON_MOD_TEAM_MEMBER_ICQ_PROPERTY_NAME)) {
		const rapidjson::Value & modTeamMemberICQValue = modTeamMemberValue[JSON_MOD_TEAM_MEMBER_ICQ_PROPERTY_NAME];

		if(!modTeamMemberICQValue.IsString()) {
			fmt::print("Mod team member '{}' property has invalid type: '{}', expected 'string'.\n", JSON_MOD_TEAM_MEMBER_ICQ_PROPERTY_NAME, Utilities::typeToString(modTeamMemberICQValue.GetType()));
			return nullptr;
		}

		newModTeamMember->setICQ(modTeamMemberICQValue.GetString());
	}

	// parse the mod team member phone number property
	if(modTeamMemberValue.HasMember(JSON_MOD_TEAM_MEMBER_PHONE_NUMBER_PROPERTY_NAME)) {
		const rapidjson::Value & modTeamMemberPhoneNumberValue = modTeamMemberValue[JSON_MOD_TEAM_MEMBER_PHONE_NUMBER_PROPERTY_NAME];

		if(!modTeamMemberPhoneNumberValue.IsString()) {
			fmt::print("Mod team member '{}' property has invalid type: '{}', expected 'string'.\n", JSON_MOD_TEAM_MEMBER_PHONE_NUMBER_PROPERTY_NAME, Utilities::typeToString(modTeamMemberPhoneNumberValue.GetType()));
			return nullptr;
		}

		newModTeamMember->setPhoneNumber(modTeamMemberPhoneNumberValue.GetString());
	}

	return newModTeamMember;
}

std::unique_ptr<ModTeamMember> ModTeamMember::parseFrom(const tinyxml2::XMLElement * modTeamMemberElement) {
	if(modTeamMemberElement == nullptr) {
		return nullptr;
	}

	// verify element name
	if(modTeamMemberElement->Name() != XML_MOD_TEAM_MEMBER_ELEMENT_NAME) {
		fmt::print("Invalid mod team element name: '{}', expected '{}'.\n", modTeamMemberElement->Name(), XML_MOD_TEAM_MEMBER_ELEMENT_NAME);
		return nullptr;
	}

	// check for unhandled mod team member element attributes
	bool attributeHandled = false;
	const tinyxml2::XMLAttribute * modTeamMemberAttribute = modTeamMemberElement->FirstAttribute();

	while(true) {
		if(modTeamMemberAttribute == nullptr) {
			break;
		}

		attributeHandled = false;

		for(const std::string & attributeName : XML_MOD_TEAM_MEMBER_ATTRIBUTE_NAMES) {
			if(modTeamMemberAttribute->Name() == attributeName) {
				attributeHandled = true;
				break;
			}
		}

		if(!attributeHandled) {
			fmt::print("Element '{}' has unexpected attribute '{}'.\n", XML_MOD_TEAM_MEMBER_ELEMENT_NAME, modTeamMemberAttribute->Name());
			return nullptr;
		}

		modTeamMemberAttribute = modTeamMemberAttribute->Next();
	}

	// check for unexpected mod team member element child elements
	if(modTeamMemberElement->FirstChildElement() != nullptr) {
		fmt::print("Element '{}' has an unexpected child element.\n", XML_MOD_TEAM_MEMBER_ELEMENT_NAME);
		return nullptr;
	}

	// read the mod team attributes
	const char * teamMemberName = modTeamMemberElement->Attribute(XML_MOD_TEAM_MEMBER_NAME_ATTRIBUTE_NAME.c_str());
	const char * teamMemberAlias = modTeamMemberElement->Attribute(XML_MOD_TEAM_MEMBER_ALIAS_ATTRIBUTE_NAME.c_str());
	const char * teamMemberEmail = modTeamMemberElement->Attribute(XML_MOD_TEAM_MEMBER_EMAIL_ATTRIBUTE_NAME.c_str());
	const char * teamMemberWebsite = modTeamMemberElement->Attribute(XML_MOD_TEAM_MEMBER_WEBSITE_ATTRIBUTE_NAME.c_str());
	const char * teamMemberAIM = modTeamMemberElement->Attribute(XML_MOD_TEAM_MEMBER_AIM_ATTRIBUTE_NAME.c_str());
	const char * teamMemberICQ = modTeamMemberElement->Attribute(XML_MOD_TEAM_MEMBER_ICQ_ATTRIBUTE_NAME.c_str());
	const char * teamMemberPhoneNumber = modTeamMemberElement->Attribute(XML_MOD_TEAM_MEMBER_PHONE_NUMBER_ATTRIBUTE_NAME.c_str());

	if(teamMemberName == nullptr || Utilities::stringLength(teamMemberName) == 0) {
		fmt::print("Attribute '{}' is missing from '{}' element.\n", XML_MOD_TEAM_MEMBER_NAME_ATTRIBUTE_NAME, XML_MOD_TEAM_MEMBER_ELEMENT_NAME);
		return nullptr;
	}

	// initialize the mod team member
	std::unique_ptr<ModTeamMember> newModTeamMember = std::make_unique<ModTeamMember>(teamMemberName, teamMemberAlias == nullptr ? "" : teamMemberAlias, teamMemberEmail == nullptr ? "" : teamMemberEmail, teamMemberWebsite == nullptr ? "" : teamMemberWebsite);

	if(teamMemberAIM != nullptr) {
		newModTeamMember->setAIM(teamMemberAIM);
	}

	if(teamMemberICQ != nullptr) {
		newModTeamMember->setICQ(teamMemberICQ);
	}

	if(teamMemberPhoneNumber != nullptr) {
		newModTeamMember->setPhoneNumber(teamMemberPhoneNumber);
	}

	return newModTeamMember;
}

bool ModTeamMember::isValid() const {
	return !m_name.empty();
}

bool ModTeamMember::isValid(const ModTeamMember * m) {
	return m != nullptr && m->isValid();
}

bool ModTeamMember::operator == (const ModTeamMember & m) const {
	return Utilities::compareStringsIgnoreCase(m_name, m.m_name) == 0 &&
		   Utilities::compareStringsIgnoreCase(m_alias, m.m_alias) == 0 &&
		   Utilities::compareStringsIgnoreCase(m_email, m.m_email) == 0 &&
		   Utilities::compareStringsIgnoreCase(m_website, m.m_website) == 0 &&
		   Utilities::compareStringsIgnoreCase(m_aim, m.m_aim) == 0 &&
		   m_icq == m.m_icq &&
		   m_phoneNumber == m.m_phoneNumber;
}

bool ModTeamMember::operator != (const ModTeamMember & m) const {
	return !operator == (m);
}
