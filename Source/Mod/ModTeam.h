#ifndef _MOD_TEAM_H_
#define _MOD_TEAM_H_

#include "Location.h"
#include "ModTeamMember.h"

#include <rapidjson/document.h>

#include <cstdint>
#include <memory>
#include <string>
#include <vector>

class Mod;

namespace tinyxml2 {
	class XMLDocument;
	class XMLElement;
}

class ModTeam final {
	friend class Mod;

public:
	ModTeam(const std::string & name = std::string(), const std::string & website = std::string(), const std::string & email = std::string());
	ModTeam(ModTeam && m) noexcept;
	ModTeam(const ModTeam & m);
	ModTeam & operator = (ModTeam && m) noexcept;
	ModTeam & operator = (const ModTeam & m);
	~ModTeam();

	bool hasName() const;
	const std::string & getName() const;
	const std::string & getWebsite() const;
	const std::string & getEmail() const;
	const std::string & getTwitter() const;
	const std::string & getDiscord() const;
	const Location & getLocation() const;
	Location & getLocation();
	const Mod * getParentMod() const;

	void setName(const std::string & name);
	void setWebsite(const std::string & website);
	void setEmail(const std::string & email);
	void setTwitter(const std::string & twitter);
	void setDiscord(const std::string & discord);
	void setLocation(const Location & location);

	size_t numberOfMembers() const;
	bool hasMember(const ModTeamMember & member) const;
	bool hasMember(const std::string & memberName) const;
	size_t indexOfMember(const ModTeamMember & member) const;
	size_t indexOfMember(const std::string & memberName) const;
	std::shared_ptr<ModTeamMember> getMember(size_t index) const;
	std::shared_ptr<ModTeamMember> getMember(const std::string & memberName) const;
	const std::vector<std::shared_ptr<ModTeamMember>> & getMembers() const;
	bool addMember(const ModTeamMember & member);
	bool removeMember(size_t index);
	bool removeMember(const ModTeamMember & member);
	bool removeMember(const std::string & memberName);
	void clearMembers();

	rapidjson::Value toJSON(rapidjson::MemoryPoolAllocator<rapidjson::CrtAllocator> & allocator) const;
	tinyxml2::XMLElement * toXML(tinyxml2::XMLDocument * document) const;
	static std::unique_ptr<ModTeam> parseFrom(const rapidjson::Value & modTeamValue);
	static std::unique_ptr<ModTeam> parseFrom(const tinyxml2::XMLElement * modTeamElement);

	bool isValid() const;
	static bool isValid(const ModTeam * modTeam);

	bool operator == (const ModTeam & m) const;
	bool operator != (const ModTeam & m) const;

protected:
	void setParentMod(const Mod * mod);
	void updateParent();

private:
	std::string m_name;
	std::string m_website;
	std::string m_email;
	std::string m_twitter;
	std::string m_discord;
	Location m_location;
	std::vector<std::shared_ptr<ModTeamMember>> m_members;
	const Mod * m_parentMod;
};

#endif // _MOD_TEAM_H_
