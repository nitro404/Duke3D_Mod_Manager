#include "ModMatch.h"

#include "Mod/Mod.h"
#include "Mod/ModVersion.h"
#include "Mod/ModVersionType.h"

ModMatch::ModMatch(std::shared_ptr<Mod> mod, size_t modIndex)
	: m_mod(mod)
	, m_type(MatchType::Mod)
	, m_modIndex(modIndex)
	, m_modVersionIndex(std::numeric_limits<size_t>::max())
	, m_modVersionTypeIndex(std::numeric_limits<size_t>::max()) { }

ModMatch::ModMatch(std::shared_ptr<Mod> mod, std::shared_ptr<ModVersion> modVersion, size_t modIndex, size_t modVersionIndex)
	: m_mod(mod)
	, m_modVersion(modVersion)
	, m_type(MatchType::ModVersion)
	, m_modIndex(modIndex)
	, m_modVersionIndex(modVersionIndex)
	, m_modVersionTypeIndex(std::numeric_limits<size_t>::max()) { }

ModMatch::ModMatch(std::shared_ptr<Mod> mod, std::shared_ptr<ModVersion> modVersion, std::shared_ptr<ModVersionType> modVersionType, size_t modIndex, size_t modVersionIndex, size_t modVersionTypeIndex)
	: m_mod(mod)
	, m_modVersion(modVersion)
	, m_modVersionType(modVersionType)
	, m_type(MatchType::ModVersionType)
	, m_modIndex(modIndex)
	, m_modVersionIndex(modVersionIndex)
	, m_modVersionTypeIndex(modVersionTypeIndex) { }

ModMatch::ModMatch(ModMatch && m) noexcept
	: m_mod(std::move(m.m_mod))
	, m_modVersion(std::move(m.m_modVersion))
	, m_modVersionType(std::move(m.m_modVersionType))
	, m_type(m.m_type)
	, m_modIndex(m.m_modIndex)
	, m_modVersionIndex(m.m_modVersionIndex)
	, m_modVersionTypeIndex(m.m_modVersionTypeIndex) { }

ModMatch::ModMatch(const ModMatch & m)
	: m_mod(m.m_mod)
	, m_modVersion(m.m_modVersion)
	, m_modVersionType(m.m_modVersionType)
	, m_type(m.m_type)
	, m_modIndex(m.m_modIndex)
	, m_modVersionIndex(m.m_modVersionIndex)
	, m_modVersionTypeIndex(m.m_modVersionTypeIndex) { }

ModMatch & ModMatch::operator = (ModMatch && m) noexcept {
	if(this != &m) {
		m_mod = std::move(m.m_mod);
		m_modVersion = std::move(m.m_modVersion);
		m_modVersionType = std::move(m.m_modVersionType);
		m_type = m.m_type;
		m_modIndex = m.m_modIndex;
		m_modVersionIndex = m.m_modVersionIndex;
		m_modVersionTypeIndex = m.m_modVersionTypeIndex;
	}

	return *this;
}

ModMatch & ModMatch::operator = (const ModMatch & m) {
	m_mod = m.m_mod;
	m_modVersion = m.m_modVersion;
	m_modVersionType = m.m_modVersionType;
	m_type = m.m_type;
	m_modIndex = m.m_modIndex;
	m_modVersionIndex = m.m_modVersionIndex;
	m_modVersionTypeIndex = m.m_modVersionTypeIndex;

	return *this;
}

ModMatch::~ModMatch() = default;

bool ModMatch::isModMatch() const {
	return m_type == MatchType::Mod;
}

bool ModMatch::isModVersionMatch() const {
	return m_type == MatchType::ModVersion;
}

bool ModMatch::isModVersionTypeMatch() const {
	return m_type == MatchType::ModVersionType;
}

ModMatch::MatchType ModMatch::getMatchType() const {
	return m_type;
}

size_t ModMatch::getModIndex() const {
	return m_modIndex;
}

size_t ModMatch::getModVersionIndex() const {
	return m_modVersionIndex;
}

size_t ModMatch::getModVersionTypeIndex() const {
	return m_modVersionTypeIndex;
}

std::shared_ptr<Mod> ModMatch::getMod() const {
	return m_mod;
}

std::shared_ptr<ModVersion> ModMatch::getModVersion() const {
	return m_modVersion;
}

std::shared_ptr<ModVersionType> ModMatch::getModVersionType() const {
	return m_modVersionType;
}

std::string ModMatch::getModName() const {
	return m_mod != nullptr ? m_mod->getName() : "";
}

std::optional<std::string> ModMatch::getModVersionName() const {
	return m_modVersion != nullptr ? m_modVersion->getVersion() : std::optional<std::string>();
}

std::optional<std::string> ModMatch::getModVersionTypeName() const {
	return m_modVersionType != nullptr ? m_modVersionType->getType() : std::optional<std::string>();
}

std::string ModMatch::toString() const {
	if(isModMatch()) {
		if(m_mod != nullptr) {
			return m_mod->getFullName();
		}
	}
	else if(isModVersionMatch()) {
		if(m_modVersion != nullptr) {
			return m_modVersion->getFullName();
		}
	}
	else if(isModVersionTypeMatch()) {
		if(m_modVersionType != nullptr) {
			return m_modVersionType->getFullName();
		}
	}

	return "";
}

bool ModMatch::isValid() const {
	if(m_mod == nullptr || m_modIndex == std::numeric_limits<size_t>::max()) {
		return false;
	}

	if((isModVersionMatch() || isModVersionTypeMatch()) && (m_modVersion == nullptr || m_modVersionIndex == std::numeric_limits<size_t>::max())) {
		return false;
	}

	if(isModVersionTypeMatch() && (m_modVersionType == nullptr || m_modVersionTypeIndex == std::numeric_limits<size_t>::max())) {
		return false;
	}

	return true;
}

bool ModMatch::isValid(const ModMatch * m) {
	return m != nullptr && m->isValid();
}

bool ModMatch::operator == (const ModMatch & m) const {
	return m_mod == m.m_mod &&
		   m_modVersion == m.m_modVersion &&
		   m_modVersionType == m.m_modVersionType;
}

bool ModMatch::operator != (const ModMatch & m) const {
	return !operator == (m);
}
