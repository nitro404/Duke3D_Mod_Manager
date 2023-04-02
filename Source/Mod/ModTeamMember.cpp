#include "ModTeamMember.h"

#include "Mod.h"
#include "ModTeam.h"

#include <Utilities/RapidJSONUtilities.h>
#include <Utilities/StringUtilities.h>

#include <spdlog/spdlog.h>
#include <tinyxml2.h>

#include <array>
#include <string_view>

static const std::string XML_MOD_TEAM_MEMBER_ELEMENT_NAME("member");
static const std::string XML_MOD_TEAM_MEMBER_NAME_ATTRIBUTE_NAME("name");
static const std::string XML_MOD_TEAM_MEMBER_ALIAS_ATTRIBUTE_NAME("alias");
static const std::string XML_MOD_TEAM_MEMBER_EMAIL_ATTRIBUTE_NAME("email");
static const std::string XML_MOD_TEAM_MEMBER_TWITTER_ATTRIBUTE_NAME("twitter");
static const std::string XML_MOD_TEAM_MEMBER_WEBSITE_ATTRIBUTE_NAME("website");
static const std::string XML_MOD_TEAM_MEMBER_YOUTUBE_ATTRIBUTE_NAME("youtube");
static const std::string XML_MOD_TEAM_MEMBER_REDDIT_ATTRIBUTE_NAME("reddit");
static const std::string XML_MOD_TEAM_MEMBER_GITHUB_ATTRIBUTE_NAME("github");
static const std::string XML_MOD_TEAM_MEMBER_DISCORD_ATTRIBUTE_NAME("discord");
static const std::string XML_MOD_TEAM_MEMBER_STEAM_ID_ATTRIBUTE_NAME("steam");
static const std::string XML_MOD_TEAM_MEMBER_AIM_ATTRIBUTE_NAME("aim");
static const std::string XML_MOD_TEAM_MEMBER_ICQ_ATTRIBUTE_NAME("icq");
static const std::string XML_MOD_TEAM_MEMBER_YAHOO_ATTRIBUTE_NAME("yahoo");
static const std::string XML_MOD_TEAM_MEMBER_PHONE_NUMBER_ATTRIBUTE_NAME("phone_number");
static const std::array<std::string_view, 14> XML_MOD_TEAM_MEMBER_ATTRIBUTE_NAMES = {
	XML_MOD_TEAM_MEMBER_NAME_ATTRIBUTE_NAME,
	XML_MOD_TEAM_MEMBER_ALIAS_ATTRIBUTE_NAME,
	XML_MOD_TEAM_MEMBER_EMAIL_ATTRIBUTE_NAME,
	XML_MOD_TEAM_MEMBER_TWITTER_ATTRIBUTE_NAME,
	XML_MOD_TEAM_MEMBER_WEBSITE_ATTRIBUTE_NAME,
	XML_MOD_TEAM_MEMBER_YOUTUBE_ATTRIBUTE_NAME,
	XML_MOD_TEAM_MEMBER_REDDIT_ATTRIBUTE_NAME,
	XML_MOD_TEAM_MEMBER_GITHUB_ATTRIBUTE_NAME,
	XML_MOD_TEAM_MEMBER_DISCORD_ATTRIBUTE_NAME,
	XML_MOD_TEAM_MEMBER_STEAM_ID_ATTRIBUTE_NAME,
	XML_MOD_TEAM_MEMBER_AIM_ATTRIBUTE_NAME,
	XML_MOD_TEAM_MEMBER_ICQ_ATTRIBUTE_NAME,
	XML_MOD_TEAM_MEMBER_YAHOO_ATTRIBUTE_NAME,
	XML_MOD_TEAM_MEMBER_PHONE_NUMBER_ATTRIBUTE_NAME
};

static constexpr const char * JSON_MOD_TEAM_MEMBER_NAME_PROPERTY_NAME = "name";
static constexpr const char * JSON_MOD_TEAM_MEMBER_ALIAS_PROPERTY_NAME = "alias";
static constexpr const char * JSON_MOD_TEAM_MEMBER_EMAIL_PROPERTY_NAME = "email";
static constexpr const char * JSON_MOD_TEAM_MEMBER_TWITTER_PROPERTY_NAME = "twitter";
static constexpr const char * JSON_MOD_TEAM_MEMBER_WEBSITE_PROPERTY_NAME = "website";
static constexpr const char * JSON_MOD_TEAM_MEMBER_YOUTUBE_PROPERTY_NAME = "youTube";
static constexpr const char * JSON_MOD_TEAM_MEMBER_REDDIT_PROPERTY_NAME = "reddit";
static constexpr const char * JSON_MOD_TEAM_MEMBER_GITHUB_PROPERTY_NAME = "gitHub";
static constexpr const char * JSON_MOD_TEAM_MEMBER_DISCORD_PROPERTY_NAME = "discord";
static constexpr const char * JSON_MOD_TEAM_MEMBER_STEAM_ID_PROPERTY_NAME = "steamID";
static constexpr const char * JSON_MOD_TEAM_MEMBER_AIM_PROPERTY_NAME = "aim";
static constexpr const char * JSON_MOD_TEAM_MEMBER_ICQ_PROPERTY_NAME = "icq";
static constexpr const char * JSON_MOD_TEAM_MEMBER_YAHOO_PROPERTY_NAME = "yahoo";
static constexpr const char * JSON_MOD_TEAM_MEMBER_PHONE_NUMBER_PROPERTY_NAME = "phoneNumber";
static const std::array<std::string_view, 14> JSON_MOD_TEAM_MEMBER_PROPERTY_NAMES = {
	JSON_MOD_TEAM_MEMBER_NAME_PROPERTY_NAME,
	JSON_MOD_TEAM_MEMBER_ALIAS_PROPERTY_NAME,
	JSON_MOD_TEAM_MEMBER_EMAIL_PROPERTY_NAME,
	JSON_MOD_TEAM_MEMBER_TWITTER_PROPERTY_NAME,
	JSON_MOD_TEAM_MEMBER_WEBSITE_PROPERTY_NAME,
	JSON_MOD_TEAM_MEMBER_YOUTUBE_PROPERTY_NAME,
	JSON_MOD_TEAM_MEMBER_REDDIT_PROPERTY_NAME,
	JSON_MOD_TEAM_MEMBER_GITHUB_PROPERTY_NAME,
	JSON_MOD_TEAM_MEMBER_DISCORD_PROPERTY_NAME,
	JSON_MOD_TEAM_MEMBER_STEAM_ID_PROPERTY_NAME,
	JSON_MOD_TEAM_MEMBER_AIM_PROPERTY_NAME,
	JSON_MOD_TEAM_MEMBER_ICQ_PROPERTY_NAME,
	JSON_MOD_TEAM_MEMBER_YAHOO_PROPERTY_NAME,
	JSON_MOD_TEAM_MEMBER_PHONE_NUMBER_PROPERTY_NAME
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
	, m_twitter(std::move(m.m_twitter))
	, m_website(std::move(m.m_website))
	, m_youTube(std::move(m.m_youTube))
	, m_reddit(std::move(m.m_reddit))
	, m_gitHub(std::move(m.m_gitHub))
	, m_discord(std::move(m.m_discord))
	, m_steamID(std::move(m.m_steamID))
	, m_aim(std::move(m.m_aim))
	, m_icq(std::move(m.m_icq))
	, m_yahoo(std::move(m.m_yahoo))
	, m_phoneNumber(std::move(m.m_phoneNumber))
	, m_parentModTeam(nullptr) { }

ModTeamMember::ModTeamMember(const ModTeamMember & m)
	: m_name(m.m_name)
	, m_alias(m.m_alias)
	, m_email(m.m_email)
	, m_twitter(m.m_twitter)
	, m_website(m.m_website)
	, m_youTube(m.m_youTube)
	, m_reddit(m.m_reddit)
	, m_gitHub(m.m_gitHub)
	, m_discord(m.m_discord)
	, m_steamID(m.m_steamID)
	, m_aim(m.m_aim)
	, m_icq(m.m_icq)
	, m_yahoo(m.m_yahoo)
	, m_phoneNumber(m.m_phoneNumber)
	, m_parentModTeam(nullptr) { }

ModTeamMember & ModTeamMember::operator = (ModTeamMember && m) noexcept {
	if(this != &m) {
		m_name = std::move(m.m_name);
		m_alias = std::move(m.m_alias);
		m_email = std::move(m.m_email);
		m_twitter = std::move(m.m_twitter);
		m_website = std::move(m.m_website);
		m_youTube = std::move(m.m_youTube);
		m_reddit = std::move(m.m_reddit);
		m_gitHub = std::move(m.m_gitHub);
		m_discord = std::move(m.m_discord);
		m_steamID = std::move(m.m_steamID);
		m_aim = std::move(m.m_aim);
		m_icq = std::move(m.m_icq);
		m_yahoo = std::move(m.m_yahoo);
		m_phoneNumber = std::move(m.m_phoneNumber);
	}

	return *this;
}

ModTeamMember & ModTeamMember::operator = (const ModTeamMember & m) {
	m_name = m.m_name;
	m_alias = m.m_alias;
	m_email = m.m_email;
	m_twitter = m.m_twitter;
	m_website = m.m_website;
	m_youTube = m.m_youTube;
	m_reddit = m.m_reddit;
	m_gitHub = m.m_gitHub;
	m_discord = m.m_discord;
	m_steamID = m.m_steamID;
	m_aim = m.m_aim;
	m_icq = m.m_icq;
	m_yahoo = m.m_yahoo;
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

const std::string & ModTeamMember::getTwitter() const {
	return m_twitter;
}

const std::string & ModTeamMember::getWebsite() const {
	return m_website;
}

const std::string & ModTeamMember::getYouTube() const {
	return m_youTube;
}

const std::string & ModTeamMember::getReddit() const {
	return m_reddit;
}

const std::string & ModTeamMember::getGitHub() const {
	return m_gitHub;
}

const std::string & ModTeamMember::getDiscord() const {
	return m_discord;
}

const std::string & ModTeamMember::getSteamID() const {
	return m_steamID;
}

const std::string & ModTeamMember::getAIM() const {
	return m_aim;
}

const std::string & ModTeamMember::getICQ() const {
	return m_icq;
}

const std::string & ModTeamMember::getYahoo() const {
	return m_yahoo;
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

void ModTeamMember::setAlias(const std::string & alias) {
	m_alias = Utilities::trimString(alias);
}

void ModTeamMember::setEmail(const std::string & email) {
	m_email = Utilities::trimString(email);
}

void ModTeamMember::setTwitter(const std::string & twitter) {
	m_twitter = Utilities::trimString(twitter);
}

void ModTeamMember::setWebsite(const std::string & website) {
	m_website = Utilities::trimString(website);
}

void ModTeamMember::setYouTube(const std::string & youTube) {
	m_youTube = Utilities::trimString(youTube);
}

void ModTeamMember::setReddit(const std::string & reddit) {
	m_reddit = Utilities::trimString(reddit);
}

void ModTeamMember::setGitHub(const std::string & gitHub) {
	m_gitHub = Utilities::trimString(gitHub);
}

void ModTeamMember::setDiscord(const std::string & discord) {
	m_discord = Utilities::trimString(discord);
}

void ModTeamMember::setSteamID(const std::string & steamID) {
	m_steamID = Utilities::trimString(steamID);
}

void ModTeamMember::setAIM(const std::string & aim) {
	m_aim = Utilities::trimString(aim);
}

void ModTeamMember::setICQ(const std::string & icq) {
	m_icq = Utilities::trimString(icq);
}

void ModTeamMember::setYahoo(const std::string & yahoo) {
	m_yahoo = Utilities::trimString(yahoo);
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

	if(!m_twitter.empty()) {
		rapidjson::Value twitterValue(m_twitter.c_str(), allocator);
		modTeamMemberValue.AddMember(rapidjson::StringRef(JSON_MOD_TEAM_MEMBER_TWITTER_PROPERTY_NAME), twitterValue, allocator);
	}

	if(!m_website.empty()) {
		rapidjson::Value websiteValue(m_website.c_str(), allocator);
		modTeamMemberValue.AddMember(rapidjson::StringRef(JSON_MOD_TEAM_MEMBER_WEBSITE_PROPERTY_NAME), websiteValue, allocator);
	}

	if(!m_youTube.empty()) {
		rapidjson::Value youTubeValue(m_youTube.c_str(), allocator);
		modTeamMemberValue.AddMember(rapidjson::StringRef(JSON_MOD_TEAM_MEMBER_YOUTUBE_PROPERTY_NAME), youTubeValue, allocator);
	}

	if(!m_reddit.empty()) {
		rapidjson::Value redditValue(m_reddit.c_str(), allocator);
		modTeamMemberValue.AddMember(rapidjson::StringRef(JSON_MOD_TEAM_MEMBER_REDDIT_PROPERTY_NAME), redditValue, allocator);
	}

	if(!m_gitHub.empty()) {
		rapidjson::Value gitHubValue(m_gitHub.c_str(), allocator);
		modTeamMemberValue.AddMember(rapidjson::StringRef(JSON_MOD_TEAM_MEMBER_GITHUB_PROPERTY_NAME), gitHubValue, allocator);
	}

	if(!m_discord.empty()) {
		rapidjson::Value discordValue(m_discord.c_str(), allocator);
		modTeamMemberValue.AddMember(rapidjson::StringRef(JSON_MOD_TEAM_MEMBER_DISCORD_PROPERTY_NAME), discordValue, allocator);
	}

	if(!m_steamID.empty()) {
		rapidjson::Value steamIDValue(m_steamID.c_str(), allocator);
		modTeamMemberValue.AddMember(rapidjson::StringRef(JSON_MOD_TEAM_MEMBER_STEAM_ID_PROPERTY_NAME), steamIDValue, allocator);
	}

	if(!m_aim.empty()) {
		rapidjson::Value aimValue(m_aim.c_str(), allocator);
		modTeamMemberValue.AddMember(rapidjson::StringRef(JSON_MOD_TEAM_MEMBER_AIM_PROPERTY_NAME), aimValue, allocator);
	}

	if(!m_icq.empty()) {
		rapidjson::Value icqValue(m_icq.c_str(), allocator);
		modTeamMemberValue.AddMember(rapidjson::StringRef(JSON_MOD_TEAM_MEMBER_ICQ_PROPERTY_NAME), icqValue, allocator);
	}

	if(!m_yahoo.empty()) {
		rapidjson::Value yahooValue(m_yahoo.c_str(), allocator);
		modTeamMemberValue.AddMember(rapidjson::StringRef(JSON_MOD_TEAM_MEMBER_YAHOO_PROPERTY_NAME), yahooValue, allocator);
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

	if(!m_twitter.empty()) {
		modTeamMemberElement->SetAttribute(XML_MOD_TEAM_MEMBER_TWITTER_ATTRIBUTE_NAME.c_str(), m_twitter.c_str());
	}

	if(!m_website.empty()) {
		modTeamMemberElement->SetAttribute(XML_MOD_TEAM_MEMBER_WEBSITE_ATTRIBUTE_NAME.c_str(), m_website.c_str());
	}

	if(!m_youTube.empty()) {
		modTeamMemberElement->SetAttribute(XML_MOD_TEAM_MEMBER_YOUTUBE_ATTRIBUTE_NAME.c_str(), m_youTube.c_str());
	}

	if(!m_reddit.empty()) {
		modTeamMemberElement->SetAttribute(XML_MOD_TEAM_MEMBER_REDDIT_ATTRIBUTE_NAME.c_str(), m_reddit.c_str());
	}

	if(!m_gitHub.empty()) {
		modTeamMemberElement->SetAttribute(XML_MOD_TEAM_MEMBER_GITHUB_ATTRIBUTE_NAME.c_str(), m_gitHub.c_str());
	}

	if(!m_discord.empty()) {
		modTeamMemberElement->SetAttribute(XML_MOD_TEAM_MEMBER_DISCORD_ATTRIBUTE_NAME.c_str(), m_discord.c_str());
	}

	if(!m_steamID.empty()) {
		modTeamMemberElement->SetAttribute(XML_MOD_TEAM_MEMBER_STEAM_ID_ATTRIBUTE_NAME.c_str(), m_steamID.c_str());
	}

	if(!m_aim.empty()) {
		modTeamMemberElement->SetAttribute(XML_MOD_TEAM_MEMBER_AIM_ATTRIBUTE_NAME.c_str(), m_aim.c_str());
	}

	if(!m_icq.empty()) {
		modTeamMemberElement->SetAttribute(XML_MOD_TEAM_MEMBER_ICQ_ATTRIBUTE_NAME.c_str(), m_icq.c_str());
	}

	if(!m_yahoo.empty()) {
		modTeamMemberElement->SetAttribute(XML_MOD_TEAM_MEMBER_YAHOO_ATTRIBUTE_NAME.c_str(), m_yahoo.c_str());
	}

	if(!m_phoneNumber.empty()) {
		modTeamMemberElement->SetAttribute(XML_MOD_TEAM_MEMBER_PHONE_NUMBER_ATTRIBUTE_NAME.c_str(), m_phoneNumber.c_str());
	}

	return modTeamMemberElement;
}

std::unique_ptr<ModTeamMember> ModTeamMember::parseFrom(const rapidjson::Value & modTeamMemberValue) {
	if(!modTeamMemberValue.IsObject()) {
		spdlog::error("Invalid mod team member type: '{}', expected 'object'.", Utilities::typeToString(modTeamMemberValue.GetType()));
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
			spdlog::warn("Mod team member has unexpected property '{}'.", i->name.GetString());
		}
	}

	// parse mod team member name
	if(!modTeamMemberValue.HasMember(JSON_MOD_TEAM_MEMBER_NAME_PROPERTY_NAME)) {
		spdlog::error("Mod team member is missing '{}' property'.", JSON_MOD_TEAM_MEMBER_NAME_PROPERTY_NAME);
		return nullptr;
	}

	const rapidjson::Value & modTeamMemberNameValue = modTeamMemberValue[JSON_MOD_TEAM_MEMBER_NAME_PROPERTY_NAME];

	if(!modTeamMemberNameValue.IsString()) {
		spdlog::error("Mod team member has an invalid '{}' property type: '{}', expected 'string'.", JSON_MOD_TEAM_MEMBER_NAME_PROPERTY_NAME, Utilities::typeToString(modTeamMemberNameValue.GetType()));
		return nullptr;
	}

	std::string modTeamMemberName(Utilities::trimString(modTeamMemberNameValue.GetString()));

	if(modTeamMemberName.empty()) {
		spdlog::error("Mod team member '{}' property cannot be empty.", JSON_MOD_TEAM_MEMBER_NAME_PROPERTY_NAME);
		return nullptr;
	}

	// initialize the mod team member
	std::unique_ptr<ModTeamMember> newModTeamMember = std::make_unique<ModTeamMember>(modTeamMemberName);

	// parse the mod team member alias property
	if(modTeamMemberValue.HasMember(JSON_MOD_TEAM_MEMBER_ALIAS_PROPERTY_NAME)) {
		const rapidjson::Value & modTeamMemberAliasValue = modTeamMemberValue[JSON_MOD_TEAM_MEMBER_ALIAS_PROPERTY_NAME];

		if(!modTeamMemberAliasValue.IsString()) {
			spdlog::error("Mod team member '{}' property has invalid type: '{}', expected 'string'.", JSON_MOD_TEAM_MEMBER_ALIAS_PROPERTY_NAME, Utilities::typeToString(modTeamMemberAliasValue.GetType()));
			return nullptr;
		}

		newModTeamMember->setAlias(modTeamMemberAliasValue.GetString());
	}

	// parse the mod team member email property
	if(modTeamMemberValue.HasMember(JSON_MOD_TEAM_MEMBER_EMAIL_PROPERTY_NAME)) {
		const rapidjson::Value & modTeamMemberEmailValue = modTeamMemberValue[JSON_MOD_TEAM_MEMBER_EMAIL_PROPERTY_NAME];

		if(!modTeamMemberEmailValue.IsString()) {
			spdlog::error("Mod team member '{}' property has invalid type: '{}', expected 'string'.", JSON_MOD_TEAM_MEMBER_EMAIL_PROPERTY_NAME, Utilities::typeToString(modTeamMemberEmailValue.GetType()));
			return nullptr;
		}

		newModTeamMember->setEmail(modTeamMemberEmailValue.GetString());
	}

	// parse the mod team member Twitter property
	if(modTeamMemberValue.HasMember(JSON_MOD_TEAM_MEMBER_TWITTER_PROPERTY_NAME)) {
		const rapidjson::Value & modTeamMemberTwitterValue = modTeamMemberValue[JSON_MOD_TEAM_MEMBER_TWITTER_PROPERTY_NAME];

		if(!modTeamMemberTwitterValue.IsString()) {
			spdlog::error("Mod team member '{}' property has invalid type: '{}', expected 'string'.", JSON_MOD_TEAM_MEMBER_TWITTER_PROPERTY_NAME, Utilities::typeToString(modTeamMemberTwitterValue.GetType()));
			return nullptr;
		}

		newModTeamMember->setTwitter(modTeamMemberTwitterValue.GetString());
	}

	// parse the mod team member website property
	if(modTeamMemberValue.HasMember(JSON_MOD_TEAM_MEMBER_WEBSITE_PROPERTY_NAME)) {
		const rapidjson::Value & modTeamMemberWebsiteValue = modTeamMemberValue[JSON_MOD_TEAM_MEMBER_WEBSITE_PROPERTY_NAME];

		if(!modTeamMemberWebsiteValue.IsString()) {
			spdlog::error("Mod team member '{}' property has invalid type: '{}', expected 'string'.", JSON_MOD_TEAM_MEMBER_WEBSITE_PROPERTY_NAME, Utilities::typeToString(modTeamMemberWebsiteValue.GetType()));
			return nullptr;
		}

		newModTeamMember->setWebsite(modTeamMemberWebsiteValue.GetString());
	}

	// parse the mod team member YouTube property
	if(modTeamMemberValue.HasMember(JSON_MOD_TEAM_MEMBER_YOUTUBE_PROPERTY_NAME)) {
		const rapidjson::Value & modTeamMemberYouTubeValue = modTeamMemberValue[JSON_MOD_TEAM_MEMBER_YOUTUBE_PROPERTY_NAME];

		if(!modTeamMemberYouTubeValue.IsString()) {
			spdlog::error("Mod team member '{}' property has invalid type: '{}', expected 'string'.", JSON_MOD_TEAM_MEMBER_YOUTUBE_PROPERTY_NAME, Utilities::typeToString(modTeamMemberYouTubeValue.GetType()));
			return nullptr;
		}

		newModTeamMember->setYouTube(modTeamMemberYouTubeValue.GetString());
	}

	// parse the mod team member Reddit property
	if(modTeamMemberValue.HasMember(JSON_MOD_TEAM_MEMBER_REDDIT_PROPERTY_NAME)) {
		const rapidjson::Value & modTeamMemberRedditValue = modTeamMemberValue[JSON_MOD_TEAM_MEMBER_REDDIT_PROPERTY_NAME];

		if(!modTeamMemberRedditValue.IsString()) {
			spdlog::error("Mod team member '{}' property has invalid type: '{}', expected 'string'.", JSON_MOD_TEAM_MEMBER_REDDIT_PROPERTY_NAME, Utilities::typeToString(modTeamMemberRedditValue.GetType()));
			return nullptr;
		}

		newModTeamMember->setReddit(modTeamMemberRedditValue.GetString());
	}

	// parse the mod team member GitHub property
	if(modTeamMemberValue.HasMember(JSON_MOD_TEAM_MEMBER_GITHUB_PROPERTY_NAME)) {
		const rapidjson::Value & modTeamMemberGitHubValue = modTeamMemberValue[JSON_MOD_TEAM_MEMBER_GITHUB_PROPERTY_NAME];

		if(!modTeamMemberGitHubValue.IsString()) {
			spdlog::error("Mod team member '{}' property has invalid type: '{}', expected 'string'.", JSON_MOD_TEAM_MEMBER_GITHUB_PROPERTY_NAME, Utilities::typeToString(modTeamMemberGitHubValue.GetType()));
			return nullptr;
		}

		newModTeamMember->setGitHub(modTeamMemberGitHubValue.GetString());
	}

	// parse the mod team member Discord property
	if(modTeamMemberValue.HasMember(JSON_MOD_TEAM_MEMBER_DISCORD_PROPERTY_NAME)) {
		const rapidjson::Value & modTeamMemberDiscordValue = modTeamMemberValue[JSON_MOD_TEAM_MEMBER_DISCORD_PROPERTY_NAME];

		if(!modTeamMemberDiscordValue.IsString()) {
			spdlog::error("Mod team member '{}' property has invalid type: '{}', expected 'string'.", JSON_MOD_TEAM_MEMBER_DISCORD_PROPERTY_NAME, Utilities::typeToString(modTeamMemberDiscordValue.GetType()));
			return nullptr;
		}

		newModTeamMember->setDiscord(modTeamMemberDiscordValue.GetString());
	}

	// parse the mod team member Steam ID property
	if(modTeamMemberValue.HasMember(JSON_MOD_TEAM_MEMBER_STEAM_ID_PROPERTY_NAME)) {
		const rapidjson::Value & modTeamMemberSteamIDValue = modTeamMemberValue[JSON_MOD_TEAM_MEMBER_STEAM_ID_PROPERTY_NAME];

		if(!modTeamMemberSteamIDValue.IsString()) {
			spdlog::error("Mod team member '{}' property has invalid type: '{}', expected 'string'.", JSON_MOD_TEAM_MEMBER_STEAM_ID_PROPERTY_NAME, Utilities::typeToString(modTeamMemberSteamIDValue.GetType()));
			return nullptr;
		}

		newModTeamMember->setSteamID(modTeamMemberSteamIDValue.GetString());
	}

	// parse the mod team member AOL Instant Messenger property
	if(modTeamMemberValue.HasMember(JSON_MOD_TEAM_MEMBER_AIM_PROPERTY_NAME)) {
		const rapidjson::Value & modTeamMemberAIMValue = modTeamMemberValue[JSON_MOD_TEAM_MEMBER_AIM_PROPERTY_NAME];

		if(!modTeamMemberAIMValue.IsString()) {
			spdlog::error("Mod team member '{}' property has invalid type: '{}', expected 'string'.", JSON_MOD_TEAM_MEMBER_AIM_PROPERTY_NAME, Utilities::typeToString(modTeamMemberAIMValue.GetType()));
			return nullptr;
		}

		newModTeamMember->setAIM(modTeamMemberAIMValue.GetString());
	}

	// parse the mod team member ICQ property
	if(modTeamMemberValue.HasMember(JSON_MOD_TEAM_MEMBER_ICQ_PROPERTY_NAME)) {
		const rapidjson::Value & modTeamMemberICQValue = modTeamMemberValue[JSON_MOD_TEAM_MEMBER_ICQ_PROPERTY_NAME];

		if(!modTeamMemberICQValue.IsString()) {
			spdlog::error("Mod team member '{}' property has invalid type: '{}', expected 'string'.", JSON_MOD_TEAM_MEMBER_ICQ_PROPERTY_NAME, Utilities::typeToString(modTeamMemberICQValue.GetType()));
			return nullptr;
		}

		newModTeamMember->setICQ(modTeamMemberICQValue.GetString());
	}

	// parse the mod team member Yahoo property
	if(modTeamMemberValue.HasMember(JSON_MOD_TEAM_MEMBER_YAHOO_PROPERTY_NAME)) {
		const rapidjson::Value & modTeamMemberYahooValue = modTeamMemberValue[JSON_MOD_TEAM_MEMBER_YAHOO_PROPERTY_NAME];

		if(!modTeamMemberYahooValue.IsString()) {
			spdlog::error("Mod team member '{}' property has invalid type: '{}', expected 'string'.", JSON_MOD_TEAM_MEMBER_YAHOO_PROPERTY_NAME, Utilities::typeToString(modTeamMemberYahooValue.GetType()));
			return nullptr;
		}

		newModTeamMember->setYahoo(modTeamMemberYahooValue.GetString());
	}

	// parse the mod team member phone number property
	if(modTeamMemberValue.HasMember(JSON_MOD_TEAM_MEMBER_PHONE_NUMBER_PROPERTY_NAME)) {
		const rapidjson::Value & modTeamMemberPhoneNumberValue = modTeamMemberValue[JSON_MOD_TEAM_MEMBER_PHONE_NUMBER_PROPERTY_NAME];

		if(!modTeamMemberPhoneNumberValue.IsString()) {
			spdlog::error("Mod team member '{}' property has invalid type: '{}', expected 'string'.", JSON_MOD_TEAM_MEMBER_PHONE_NUMBER_PROPERTY_NAME, Utilities::typeToString(modTeamMemberPhoneNumberValue.GetType()));
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
		spdlog::error("Invalid mod team element name: '{}', expected '{}'.", modTeamMemberElement->Name(), XML_MOD_TEAM_MEMBER_ELEMENT_NAME);
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

		for(const std::string_view & attributeName : XML_MOD_TEAM_MEMBER_ATTRIBUTE_NAMES) {
			if(modTeamMemberAttribute->Name() == attributeName) {
				attributeHandled = true;
				break;
			}
		}

		if(!attributeHandled) {
			spdlog::warn("Element '{}' has unexpected attribute '{}'.", XML_MOD_TEAM_MEMBER_ELEMENT_NAME, modTeamMemberAttribute->Name());
		}

		modTeamMemberAttribute = modTeamMemberAttribute->Next();
	}

	// check for unexpected mod team member element child elements
	if(modTeamMemberElement->FirstChildElement() != nullptr) {
		spdlog::warn("Element '{}' has an unexpected child element.", XML_MOD_TEAM_MEMBER_ELEMENT_NAME);
	}

	// read the mod team attributes
	const char * teamMemberName = modTeamMemberElement->Attribute(XML_MOD_TEAM_MEMBER_NAME_ATTRIBUTE_NAME.c_str());
	const char * teamMemberAlias = modTeamMemberElement->Attribute(XML_MOD_TEAM_MEMBER_ALIAS_ATTRIBUTE_NAME.c_str());
	const char * teamMemberEmail = modTeamMemberElement->Attribute(XML_MOD_TEAM_MEMBER_EMAIL_ATTRIBUTE_NAME.c_str());
	const char * teamMemberTwitter = modTeamMemberElement->Attribute(XML_MOD_TEAM_MEMBER_TWITTER_ATTRIBUTE_NAME.c_str());
	const char * teamMemberWebsite = modTeamMemberElement->Attribute(XML_MOD_TEAM_MEMBER_WEBSITE_ATTRIBUTE_NAME.c_str());
	const char * teamMemberYouTube = modTeamMemberElement->Attribute(XML_MOD_TEAM_MEMBER_YOUTUBE_ATTRIBUTE_NAME.c_str());
	const char * teamMemberReddit = modTeamMemberElement->Attribute(XML_MOD_TEAM_MEMBER_REDDIT_ATTRIBUTE_NAME.c_str());
	const char * teamMemberGitHub = modTeamMemberElement->Attribute(XML_MOD_TEAM_MEMBER_GITHUB_ATTRIBUTE_NAME.c_str());
	const char * teamMemberDiscord = modTeamMemberElement->Attribute(XML_MOD_TEAM_MEMBER_DISCORD_ATTRIBUTE_NAME.c_str());
	const char * teamMemberSteamID = modTeamMemberElement->Attribute(XML_MOD_TEAM_MEMBER_STEAM_ID_ATTRIBUTE_NAME.c_str());
	const char * teamMemberAIM = modTeamMemberElement->Attribute(XML_MOD_TEAM_MEMBER_AIM_ATTRIBUTE_NAME.c_str());
	const char * teamMemberICQ = modTeamMemberElement->Attribute(XML_MOD_TEAM_MEMBER_ICQ_ATTRIBUTE_NAME.c_str());
	const char * teamMemberYahoo = modTeamMemberElement->Attribute(XML_MOD_TEAM_MEMBER_YAHOO_ATTRIBUTE_NAME.c_str());
	const char * teamMemberPhoneNumber = modTeamMemberElement->Attribute(XML_MOD_TEAM_MEMBER_PHONE_NUMBER_ATTRIBUTE_NAME.c_str());

	if(teamMemberName == nullptr || Utilities::stringLength(teamMemberName) == 0) {
		spdlog::error("Attribute '{}' is missing from '{}' element.", XML_MOD_TEAM_MEMBER_NAME_ATTRIBUTE_NAME, XML_MOD_TEAM_MEMBER_ELEMENT_NAME);
		return nullptr;
	}

	// initialize the mod team member
	std::unique_ptr<ModTeamMember> newModTeamMember = std::make_unique<ModTeamMember>(teamMemberName, teamMemberAlias == nullptr ? "" : teamMemberAlias, teamMemberEmail == nullptr ? "" : teamMemberEmail, teamMemberWebsite == nullptr ? "" : teamMemberWebsite);

	if(teamMemberTwitter != nullptr) {
		newModTeamMember->setTwitter(teamMemberTwitter);
	}

	if(teamMemberYouTube != nullptr) {
		newModTeamMember->setYouTube(teamMemberYouTube);
	}

	if(teamMemberReddit != nullptr) {
		newModTeamMember->setReddit(teamMemberReddit);
	}

	if(teamMemberGitHub != nullptr) {
		newModTeamMember->setGitHub(teamMemberGitHub);
	}

	if(teamMemberDiscord != nullptr) {
		newModTeamMember->setDiscord(teamMemberDiscord);
	}

	if(teamMemberSteamID != nullptr) {
		newModTeamMember->setSteamID(teamMemberSteamID);
	}

	if(teamMemberAIM != nullptr) {
		newModTeamMember->setAIM(teamMemberAIM);
	}

	if(teamMemberICQ != nullptr) {
		newModTeamMember->setICQ(teamMemberICQ);
	}

	if(teamMemberYahoo != nullptr) {
		newModTeamMember->setYahoo(teamMemberYahoo);
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
	return Utilities::areStringsEqualIgnoreCase(m_name, m.m_name) &&
		   Utilities::areStringsEqualIgnoreCase(m_alias, m.m_alias) &&
		   Utilities::areStringsEqualIgnoreCase(m_email, m.m_email) &&
		   Utilities::areStringsEqualIgnoreCase(m_twitter, m.m_twitter) &&
		   Utilities::areStringsEqualIgnoreCase(m_website, m.m_website) &&
		   Utilities::areStringsEqualIgnoreCase(m_youTube, m.m_youTube) &&
		   Utilities::areStringsEqualIgnoreCase(m_reddit, m.m_reddit) &&
		   Utilities::areStringsEqualIgnoreCase(m_gitHub, m.m_gitHub) &&
		   Utilities::areStringsEqualIgnoreCase(m_discord, m.m_discord) &&
		   Utilities::areStringsEqualIgnoreCase(m_steamID, m.m_steamID) &&
		   Utilities::areStringsEqualIgnoreCase(m_aim, m.m_aim) &&
		   Utilities::areStringsEqual(m_icq, m.m_icq) &&
		   Utilities::areStringsEqualIgnoreCase(m_yahoo, m.m_yahoo) &&
		   Utilities::areStringsEqual(m_phoneNumber, m.m_phoneNumber);
}

bool ModTeamMember::operator != (const ModTeamMember & m) const {
	return !operator == (m);
}
