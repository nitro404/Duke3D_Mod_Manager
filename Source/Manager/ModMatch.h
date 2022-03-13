#ifndef _MOD_MATCH_H_
#define _MOD_MATCH_H_

#include <memory>
#include <string>
#include <variant>
#include <vector>

class Mod;
class ModVersion;
class ModVersionType;

class ModMatch final {
public:
	enum class MatchType {
		Mod,
		ModVersion,
		ModVersionType
	};

	ModMatch(std::shared_ptr<Mod> mod, size_t modIndex);
	ModMatch(std::shared_ptr<Mod> mod, std::shared_ptr<ModVersion> modVersion, size_t modIndex, size_t modVersionIndex);
	ModMatch(std::shared_ptr<Mod> mod, std::shared_ptr<ModVersion> modVersion, std::shared_ptr<ModVersionType> modVersionType, size_t modIndex, size_t modVersionIndex, size_t modVersionTypeIndex);
	ModMatch(ModMatch && m) noexcept;
	ModMatch(const ModMatch & m);
	ModMatch & operator = (ModMatch && m) noexcept;
	ModMatch & operator = (const ModMatch & m);
	~ModMatch();

	bool isModMatch() const;
	bool isModVersionMatch() const;
	bool isModVersionTypeMatch() const;
	MatchType getMatchType() const;
	size_t getModIndex() const;
	size_t getModVersionIndex() const;
	size_t getModVersionTypeIndex() const;
	std::shared_ptr<Mod> getMod() const;
	std::shared_ptr<ModVersion> getModVersion() const;
	std::shared_ptr<ModVersionType> getModVersionType() const;

	std::string toString() const;

	bool isValid() const;
	static bool isValid(const ModMatch * m);

	bool operator == (const ModMatch & m) const;
	bool operator != (const ModMatch & m) const;

private:
	std::shared_ptr<Mod> m_mod;
	std::shared_ptr<ModVersion> m_modVersion;
	std::shared_ptr<ModVersionType> m_modVersionType;
	size_t m_modIndex;
	size_t m_modVersionIndex;
	size_t m_modVersionTypeIndex;
	MatchType m_type;
};

#endif // _MOD_MATCH_H_
