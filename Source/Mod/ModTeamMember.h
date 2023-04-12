#ifndef _MOD_TEAM_MEMBER_H_
#define _MOD_TEAM_MEMBER_H_

#include "Location.h"

#include <rapidjson/document.h>

#include <memory>
#include <string>

class Mod;
class ModTeam;

namespace tinyxml2 {
	class XMLDocument;
	class XMLElement;
}

class ModTeamMember final {
	friend class ModTeam;

public:
	ModTeamMember(const std::string & name, const std::string & alias = {}, const std::string & email = {}, const std::string & website = {});
	ModTeamMember(ModTeamMember && m) noexcept;
	ModTeamMember(const ModTeamMember & m);
	ModTeamMember & operator = (ModTeamMember && m) noexcept;
	ModTeamMember & operator = (const ModTeamMember & m);
	virtual ~ModTeamMember();

	const std::string & getName() const;
	const std::string & getAlias() const;
	const std::string & getEmail() const;
	const std::string & getTwitter() const;
	const std::string & getWebsite() const;
	const std::string & getYouTube() const;
	const std::string & getReddit() const;
	const std::string & getGitHub() const;
	const std::string & getDiscord() const;
	const std::string & getSteamID() const;
	const std::string & getAIM() const;
	const std::string & getICQ() const;
	const std::string & getYahoo() const;
	const std::string & getPhoneNumber() const;
	const Location & getLocation() const;
	Location & getLocation();
	const Mod * getParentMod() const;
	const ModTeam * getParentModTeam() const;

	void setName(const std::string & name);
	void setAlias(const std::string & alias);
	void setEmail(const std::string & email);
	void setTwitter(const std::string & twitter);
	void setWebsite(const std::string & website);
	void setYouTube(const std::string & youTube);
	void setReddit(const std::string & reddit);
	void setGitHub(const std::string & gitHub);
	void setDiscord(const std::string & discord);
	void setSteamID(const std::string & steamID);
	void setAIM(const std::string & aim);
	void setICQ(const std::string & icq);
	void setYahoo(const std::string & yahoo);
	void setPhoneNumber(const std::string & phoneNumber);
	void setLocation(const Location & location);

	rapidjson::Value toJSON(rapidjson::MemoryPoolAllocator<rapidjson::CrtAllocator> & allocator) const;
	tinyxml2::XMLElement * toXML(tinyxml2::XMLDocument * document) const;
	static std::unique_ptr<ModTeamMember> parseFrom(const rapidjson::Value & modTeamMemberValue);
	static std::unique_ptr<ModTeamMember> parseFrom(const tinyxml2::XMLElement * modTeamMemberElement);

	bool isValid() const;
	static bool isValid(const ModTeamMember * m);

	bool operator == (const ModTeamMember & m) const;
	bool operator != (const ModTeamMember & m) const;

protected:
	void setParentModTeam(const ModTeam * modTeam);

private:
	std::string m_name;
	std::string m_alias;
	std::string m_email;
	std::string m_twitter;
	std::string m_website;
	std::string m_youTube;
	std::string m_reddit;
	std::string m_gitHub;
	std::string m_discord;
	std::string m_steamID;
	std::string m_aim;
	std::string m_icq;
	std::string m_yahoo;
	std::string m_phoneNumber;
	Location m_location;
	const ModTeam * m_parentModTeam;
};

#endif // _MOD_TEAM_MEMBER_H_
