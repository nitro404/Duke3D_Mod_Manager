#include "ModTeam.h"

#include "Mod.h"

#include <Utilities/RapidJSONUtilities.h>
#include <Utilities/StringUtilities.h>

#include <spdlog/spdlog.h>
#include <tinyxml2.h>

#include <array>
#include <sstream>
#include <string_view>

static const std::string XML_MOD_TEAM_ELEMENT_NAME("team");
static const std::string XML_MOD_TEAM_NAME_ATTRIBUTE_NAME("name");
static const std::string XML_MOD_TEAM_WEBSITE_ATTRIBUTE_NAME("website");
static const std::string XML_MOD_TEAM_EMAIL_ATTRIBUTE_NAME("email");
static const std::string XML_MOD_TEAM_TWITTER_ATTRIBUTE_NAME("twitter");
static const std::string XML_MOD_TEAM_DISCORD_ATTRIBUTE_NAME("discord");
static const std::string XML_MOD_TEAM_COUNTY_ATTRIBUTE_NAME("county");
static const std::string XML_MOD_TEAM_CITY_ATTRIBUTE_NAME("city");
static const std::string XML_MOD_TEAM_PROVINCE_ATTRIBUTE_NAME("province");
static const std::string XML_MOD_TEAM_STATE_ATTRIBUTE_NAME("state");
static const std::string XML_MOD_TEAM_COUNTRY_ATTRIBUTE_NAME("country");
static const std::array<std::string_view, 10> XML_MOD_TEAM_ATTRIBUTE_NAMES = {
	XML_MOD_TEAM_NAME_ATTRIBUTE_NAME,
	XML_MOD_TEAM_WEBSITE_ATTRIBUTE_NAME,
	XML_MOD_TEAM_EMAIL_ATTRIBUTE_NAME,
	XML_MOD_TEAM_TWITTER_ATTRIBUTE_NAME,
	XML_MOD_TEAM_DISCORD_ATTRIBUTE_NAME,
	XML_MOD_TEAM_COUNTY_ATTRIBUTE_NAME,
	XML_MOD_TEAM_CITY_ATTRIBUTE_NAME,
	XML_MOD_TEAM_PROVINCE_ATTRIBUTE_NAME,
	XML_MOD_TEAM_STATE_ATTRIBUTE_NAME,
	XML_MOD_TEAM_COUNTRY_ATTRIBUTE_NAME
};

static constexpr const char * JSON_MOD_TEAM_NAME_PROPERTY_NAME = "name";
static constexpr const char * JSON_MOD_TEAM_WEBSITE_PROPERTY_NAME = "website";
static constexpr const char * JSON_MOD_TEAM_EMAIL_PROPERTY_NAME = "email";
static constexpr const char * JSON_MOD_TEAM_TWITTER_PROPERTY_NAME = "twitter";
static constexpr const char * JSON_MOD_TEAM_DISCORD_PROPERTY_NAME = "discord";
static constexpr const char * JSON_MOD_TEAM_COUNTY_PROPERTY_NAME = "county";
static constexpr const char * JSON_MOD_TEAM_CITY_PROPERTY_NAME = "city";
static constexpr const char * JSON_MOD_TEAM_PROVINCE_PROPERTY_NAME = "province";
static constexpr const char * JSON_MOD_TEAM_STATE_PROPERTY_NAME = "state";
static constexpr const char * JSON_MOD_TEAM_COUNTRY_PROPERTY_NAME = "country";
static constexpr const char * JSON_MOD_TEAM_MEMBERS_PROPERTY_NAME = "members";
static const std::array<std::string_view, 11> JSON_MOD_TEAM_PROPERTY_NAMES = {
	JSON_MOD_TEAM_NAME_PROPERTY_NAME,
	JSON_MOD_TEAM_WEBSITE_PROPERTY_NAME,
	JSON_MOD_TEAM_EMAIL_PROPERTY_NAME,
	JSON_MOD_TEAM_TWITTER_PROPERTY_NAME,
	JSON_MOD_TEAM_DISCORD_PROPERTY_NAME,
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
	, m_twitter(std::move(m.m_twitter))
	, m_discord(std::move(m.m_discord))
	, m_location(std::move(m.m_location))
	, m_members(std::move(m.m_members))
	, m_parentMod(nullptr) {
	updateParent();
}

ModTeam::ModTeam(const ModTeam & m)
	: m_name(m.m_name)
	, m_website(m.m_website)
	, m_email(m.m_email)
	, m_twitter(m.m_twitter)
	, m_discord(m.m_discord)
	, m_location(m.m_location)
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
		m_twitter = std::move(m.m_twitter);
		m_discord = std::move(m.m_discord);
		m_members = std::move(m.m_members);
		m_location = std::move(m.m_location);

		updateParent();
	}

	return *this;
}

ModTeam & ModTeam::operator = (const ModTeam & m) {
	m_members.clear();

	m_name = m.m_name;
	m_website = m.m_website;
	m_email = m.m_email;
	m_twitter = m.m_twitter;
	m_discord = m.m_discord;
	m_location = m.m_location;

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

const std::string & ModTeam::getTwitter() const {
	return m_twitter;
}

const std::string & ModTeam::getDiscord() const {
	return m_discord;
}

const Location & ModTeam::getLocation() const {
	return m_location;
}

Location & ModTeam::getLocation() {
	return m_location;
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

void ModTeam::setTwitter(const std::string & twitter) {
	m_twitter = Utilities::trimString(twitter);
}

void ModTeam::setDiscord(const std::string & discord) {
	m_discord = Utilities::trimString(discord);
}

void ModTeam::setLocation(const Location & location) {
	m_location = location;
}

void ModTeam::setParentMod(const Mod * mod) {
	m_parentMod = mod;
}

size_t ModTeam::numberOfMembers() const {
	return m_members.size();
}

bool ModTeam::hasMember(const ModTeamMember & member) const {
	for(std::vector<std::shared_ptr<ModTeamMember>>::const_iterator i = m_members.begin(); i != m_members.end(); i++) {
		if(Utilities::areStringsEqualIgnoreCase((*i)->getName(), member.getName())) {
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
		if(Utilities::areStringsEqualIgnoreCase((*i)->getName(), memberName)) {
			return true;
		}
	}

	return false;
}

size_t ModTeam::indexOfMember(const ModTeamMember & member) const {
	for(size_t i = 0; i < m_members.size(); i++) {
		if(Utilities::areStringsEqualIgnoreCase(m_members[i]->getName(), member.getName())) {
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
		if(Utilities::areStringsEqualIgnoreCase(m_members[i]->getName(), memberName)) {
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
		if(Utilities::areStringsEqualIgnoreCase((*i)->getName(), memberName)) {
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

	std::shared_ptr<ModTeamMember> newModTeamMember(std::make_shared<ModTeamMember>(member));
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
		if(Utilities::areStringsEqualIgnoreCase((*i)->getName(), member.getName())) {
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
		if(Utilities::areStringsEqualIgnoreCase((*i)->getName(), memberName)) {
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

	if(!m_twitter.empty()) {
		rapidjson::Value twitterValue(m_twitter.c_str(), allocator);
		modTeamValue.AddMember(rapidjson::StringRef(JSON_MOD_TEAM_TWITTER_PROPERTY_NAME), twitterValue, allocator);
	}

	if(!m_discord.empty()) {
		rapidjson::Value discordValue(m_discord.c_str(), allocator);
		modTeamValue.AddMember(rapidjson::StringRef(JSON_MOD_TEAM_DISCORD_PROPERTY_NAME), discordValue, allocator);
	}

	m_location.addToJSONObject(modTeamValue, allocator);

	if(!m_members.empty()) {
		rapidjson::Value membersValue(rapidjson::kArrayType);
		membersValue.Reserve(m_members.size(), allocator);

		for(const std::shared_ptr<ModTeamMember> & member : m_members) {
			membersValue.PushBack(member->toJSON(allocator), allocator);
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

	if(!m_twitter.empty()) {
		modTeamElement->SetAttribute(XML_MOD_TEAM_TWITTER_ATTRIBUTE_NAME.c_str(), m_twitter.c_str());
	}

	if(!m_discord.empty()) {
		modTeamElement->SetAttribute(XML_MOD_TEAM_DISCORD_ATTRIBUTE_NAME.c_str(), m_discord.c_str());
	}

	m_location.addToXMLElement(modTeamElement);

	if(!m_members.empty()) {
		for(std::vector<std::shared_ptr<ModTeamMember>>::const_iterator i = m_members.begin(); i != m_members.end(); ++i) {
			modTeamElement->InsertEndChild((*i)->toXML(document));
		}
	}

	return modTeamElement;
}

std::unique_ptr<ModTeam> ModTeam::parseFrom(const rapidjson::Value & modTeamValue) {
	if(!modTeamValue.IsObject()) {
		spdlog::error("Invalid mod team type: '{}', expected 'object'.", Utilities::typeToString(modTeamValue.GetType()));
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
			spdlog::warn("Mod team has unexpected property '{}'.", i->name.GetString());
		}
	}

	// initialize the mod team
	std::unique_ptr<ModTeam> newModTeam(std::make_unique<ModTeam>());

	// parse the mod team name property
	if(modTeamValue.HasMember(JSON_MOD_TEAM_NAME_PROPERTY_NAME)) {
		const rapidjson::Value & modTeamNameValue = modTeamValue[JSON_MOD_TEAM_NAME_PROPERTY_NAME];

		if(!modTeamNameValue.IsString()) {
			spdlog::error("Mod team '{}' property has invalid type: '{}', expected 'string'.", JSON_MOD_TEAM_NAME_PROPERTY_NAME, Utilities::typeToString(modTeamNameValue.GetType()));
			return nullptr;
		}

		newModTeam->setName(modTeamNameValue.GetString());
	}

	// parse the mod team website property
	if(modTeamValue.HasMember(JSON_MOD_TEAM_WEBSITE_PROPERTY_NAME)) {
		const rapidjson::Value & modTeamWebsiteValue = modTeamValue[JSON_MOD_TEAM_WEBSITE_PROPERTY_NAME];

		if(!modTeamWebsiteValue.IsString()) {
			spdlog::error("Mod team '{}' property has invalid type: '{}', expected 'string'.", JSON_MOD_TEAM_WEBSITE_PROPERTY_NAME, Utilities::typeToString(modTeamWebsiteValue.GetType()));
			return nullptr;
		}

		newModTeam->setWebsite(modTeamWebsiteValue.GetString());
	}

	// parse the mod team email property
	if(modTeamValue.HasMember(JSON_MOD_TEAM_EMAIL_PROPERTY_NAME)) {
		const rapidjson::Value & modTeamEmailValue = modTeamValue[JSON_MOD_TEAM_EMAIL_PROPERTY_NAME];

		if(!modTeamEmailValue.IsString()) {
			spdlog::error("Mod team '{}' property has invalid type: '{}', expected 'string'.", JSON_MOD_TEAM_EMAIL_PROPERTY_NAME, Utilities::typeToString(modTeamEmailValue.GetType()));
			return nullptr;
		}

		newModTeam->setEmail(modTeamEmailValue.GetString());
	}

	// parse the mod team twitter property
	if(modTeamValue.HasMember(JSON_MOD_TEAM_TWITTER_PROPERTY_NAME)) {
		const rapidjson::Value & modTeamTwitterValue = modTeamValue[JSON_MOD_TEAM_TWITTER_PROPERTY_NAME];

		if(!modTeamTwitterValue.IsString()) {
			spdlog::error("Mod team '{}' property has invalid type: '{}', expected 'string'.", JSON_MOD_TEAM_TWITTER_PROPERTY_NAME, Utilities::typeToString(modTeamTwitterValue.GetType()));
			return nullptr;
		}

		newModTeam->setTwitter(modTeamTwitterValue.GetString());
	}

	// parse the mod team discord property
	if(modTeamValue.HasMember(JSON_MOD_TEAM_DISCORD_PROPERTY_NAME)) {
		const rapidjson::Value & modTeamDiscordValue = modTeamValue[JSON_MOD_TEAM_DISCORD_PROPERTY_NAME];

		if(!modTeamDiscordValue.IsString()) {
			spdlog::error("Mod team '{}' property has invalid type: '{}', expected 'string'.", JSON_MOD_TEAM_DISCORD_PROPERTY_NAME, Utilities::typeToString(modTeamDiscordValue.GetType()));
			return nullptr;
		}

		newModTeam->setDiscord(modTeamDiscordValue.GetString());
	}

	if(!newModTeam->m_location.parseFrom(modTeamValue) || !newModTeam->m_location.isValid()) {
		spdlog::error("Failed to parse valid mod team location.");
		return nullptr;
	}

	// parse the mod team members property
	if(modTeamValue.HasMember(JSON_MOD_TEAM_MEMBERS_PROPERTY_NAME)) {
		const rapidjson::Value & modTeamMembersValue = modTeamValue[JSON_MOD_TEAM_MEMBERS_PROPERTY_NAME];

		if(!modTeamMembersValue.IsArray()) {
			spdlog::error("Mod team '{}' property has invalid type: '{}', expected 'array'.", JSON_MOD_TEAM_MEMBERS_PROPERTY_NAME, Utilities::typeToString(modTeamMembersValue.GetType()));
			return nullptr;
		}

		std::shared_ptr<ModTeamMember> newModTeamMember;

		for(rapidjson::Value::ConstValueIterator i = modTeamMembersValue.Begin(); i != modTeamMembersValue.End(); ++i) {
			newModTeamMember = ModTeamMember::parseFrom(*i);

			if(!ModTeamMember::isValid(newModTeamMember.get())) {
				spdlog::error("Failed to parse mod team member #{}.", newModTeam->m_members.size() + 1);
				return nullptr;
			}

			newModTeamMember->setParentModTeam(newModTeam.get());

			if(newModTeam->hasMember(*newModTeamMember)) {
				spdlog::error("Encountered duplicate mod team member #{}.", newModTeam->m_members.size() + 1);
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
		spdlog::error("Invalid mod team element name: '{}', expected '{}'.", modTeamElement->Name(), XML_MOD_TEAM_ELEMENT_NAME);
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

		for(const std::string_view & attributeName : XML_MOD_TEAM_ATTRIBUTE_NAMES) {
			if(modTeamAttribute->Name() == attributeName) {
				attributeHandled = true;
				break;
			}
		}

		if(!attributeHandled) {
			spdlog::warn("Element '{}' has unexpected attribute '{}'.", XML_MOD_TEAM_ELEMENT_NAME, modTeamAttribute->Name());
		}

		modTeamAttribute = modTeamAttribute->Next();
	}

	// read the mod team attributes
	const char * teamName = modTeamElement->Attribute(XML_MOD_TEAM_NAME_ATTRIBUTE_NAME.c_str());
	const char * teamWebsite = modTeamElement->Attribute(XML_MOD_TEAM_WEBSITE_ATTRIBUTE_NAME.c_str());
	const char * teamEmail = modTeamElement->Attribute(XML_MOD_TEAM_EMAIL_ATTRIBUTE_NAME.c_str());
	const char * teamTwitter = modTeamElement->Attribute(XML_MOD_TEAM_TWITTER_ATTRIBUTE_NAME.c_str());
	const char * teamDiscord = modTeamElement->Attribute(XML_MOD_TEAM_DISCORD_ATTRIBUTE_NAME.c_str());

	// initialize the mod team
	std::unique_ptr<ModTeam> newModTeam = std::make_unique<ModTeam>(teamName == nullptr ? "" : teamName, teamWebsite == nullptr ? "" : teamWebsite, teamEmail == nullptr ? "" : teamEmail);

	if(teamTwitter != nullptr) {
		newModTeam->setTwitter(teamTwitter);
	}

	if(teamDiscord != nullptr) {
		newModTeam->setDiscord(teamDiscord);
	}

	if(!newModTeam->m_location.parseFrom(modTeamElement) || !newModTeam->m_location.isValid()) {
		spdlog::error("Failed to parse valid mod team location.");
		return nullptr;
	}

	// iterate over all of the mod team member elements
	const tinyxml2::XMLElement * modTeamMemberElement = modTeamElement->FirstChildElement();

	if(modTeamMemberElement != nullptr) {
		std::shared_ptr<ModTeamMember> newModTeamMember;

		while(true) {
			if(modTeamMemberElement == nullptr) {
				break;
			}

			newModTeamMember = ModTeamMember::parseFrom(modTeamMemberElement);

			if(!ModTeamMember::isValid(newModTeamMember.get())) {
				spdlog::error("Failed to parse mod team member #{}.", newModTeam->m_members.size() + 1);
				return nullptr;
			}

			newModTeamMember->setParentModTeam(newModTeam.get());

			if(newModTeam->hasMember(*newModTeamMember)) {
				spdlog::error("Encountered duplicate mod team member #{}.", newModTeam->m_members.size() + 1);
				return nullptr;
			}

			newModTeam->m_members.push_back(newModTeamMember);

			modTeamMemberElement = modTeamMemberElement->NextSiblingElement();
		}
	}

	return newModTeam;
}

bool ModTeam::isValid() const {
	if(!m_location.isValid()) {
		return false;
	}

	for(std::vector<std::shared_ptr<ModTeamMember>>::const_iterator i = m_members.begin(); i != m_members.end(); i++) {
		if(!(*i)->isValid()) {
			return false;
		}

		if((*i)->getParentModTeam() != this) {
			return false;
		}

		for(std::vector<std::shared_ptr<ModTeamMember>>::const_iterator j = i + 1; j != m_members.end(); j++) {
			if(Utilities::areStringsEqualIgnoreCase((*i)->getName(), (*j)->getName())) {
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
	if(!Utilities::areStringsEqual(m_name, m.m_name) ||
	   !Utilities::areStringsEqual(m_website, m.m_website) ||
	   !Utilities::areStringsEqual(m_email, m.m_email) ||
	   !Utilities::areStringsEqual(m_twitter, m.m_twitter) ||
	   !Utilities::areStringsEqual(m_discord, m.m_discord) ||
	   m_location != m.m_location ||
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
