#include "Mod.h"

#include "Game/GameVersion.h"
#include "Game/GameVersionCollection.h"
#include "ModDownload.h"
#include "ModGameVersion.h"
#include "ModImage.h"
#include "ModScreenshot.h"
#include "ModTeam.h"
#include "ModTeamMember.h"
#include "ModVersion.h"
#include "ModVersionType.h"
#include "ModVideo.h"
#include "StandAloneMod.h"

#include <Utilities/RapidJSONUtilities.h>
#include <Utilities/StringUtilities.h>

#include <fmt/core.h>
#include <spdlog/spdlog.h>
#include <tinyxml2.h>

#include <algorithm>
#include <array>
#include <sstream>
#include <string_view>

static constexpr const char * JSON_MOD_ID_PROPERTY_NAME = "id";
static constexpr const char * JSON_MOD_NAME_PROPERTY_NAME = "name";
static constexpr const char * JSON_MOD_ALIAS_PROPERTY_NAME = "alias";
static constexpr const char * JSON_MOD_TYPE_PROPERTY_NAME = "type";
static constexpr const char * JSON_MOD_PREFERRED_VERSION_NAME_PROPERTY_NAME = "preferredVersion";
static constexpr const char * JSON_MOD_DEFAULT_VERSION_TYPE_PROPERTY_NAME = "defaultVersionType";
static constexpr const char * JSON_MOD_WEBSITE_PROPERTY_NAME = "website";
static constexpr const char * JSON_MOD_REPOSITORY_URL_PROPERTY_NAME = "repositoryURL";
static constexpr const char * JSON_MOD_TEAM_PROPERTY_NAME = "team";
static constexpr const char * JSON_MOD_VERSIONS_PROPERTY_NAME = "versions";
static constexpr const char * JSON_MOD_DOWNLOADS_PROPERTY_NAME = "downloads";
static constexpr const char * JSON_MOD_SCREENSHOTS_PROPERTY_NAME = "screenshots";
static constexpr const char * JSON_MOD_IMAGES_PROPERTY_NAME = "images";
static constexpr const char * JSON_MOD_VIDEOS_PROPERTY_NAME = "videos";
static constexpr const char * JSON_MOD_NOTES_PROPERTY_NAME = "notes";
static constexpr const char * JSON_MOD_RELATED_MODS_PROPERTY_NAME = "relatedMods";
static constexpr const char * JSON_MOD_SIMILAR_MODS_PROPERTY_NAME = "similarMods";
static const std::array<std::string_view, 17> JSON_MOD_PROPERTY_NAMES = {
	JSON_MOD_ID_PROPERTY_NAME,
	JSON_MOD_NAME_PROPERTY_NAME,
	JSON_MOD_ALIAS_PROPERTY_NAME,
	JSON_MOD_TYPE_PROPERTY_NAME,
	JSON_MOD_PREFERRED_VERSION_NAME_PROPERTY_NAME,
	JSON_MOD_DEFAULT_VERSION_TYPE_PROPERTY_NAME,
	JSON_MOD_WEBSITE_PROPERTY_NAME,
	JSON_MOD_REPOSITORY_URL_PROPERTY_NAME,
	JSON_MOD_TEAM_PROPERTY_NAME,
	JSON_MOD_VERSIONS_PROPERTY_NAME,
	JSON_MOD_DOWNLOADS_PROPERTY_NAME,
	JSON_MOD_SCREENSHOTS_PROPERTY_NAME,
	JSON_MOD_IMAGES_PROPERTY_NAME,
	JSON_MOD_VIDEOS_PROPERTY_NAME,
	JSON_MOD_NOTES_PROPERTY_NAME,
	JSON_MOD_RELATED_MODS_PROPERTY_NAME,
	JSON_MOD_SIMILAR_MODS_PROPERTY_NAME
};

static const std::string XML_MOD_ELEMENT_NAME("mod");
static const std::string XML_MOD_TEAM_ELEMENT_NAME("team");
static const std::string XML_VERSIONS_ELEMENT_NAME("versions");
static const std::string XML_DOWNLOADS_ELEMENT_NAME("downloads");
static const std::string XML_SCREENSHOTS_ELEMENT_NAME("screenshots");
static const std::string XML_IMAGES_ELEMENT_NAME("images");
static const std::string XML_VIDEOS_ELEMENT_NAME("videos");
static const std::string XML_RELATED_ELEMENT_NAME("related");
static const std::string XML_RELATED_MOD_ELEMENT_NAME("mod");
static const std::string XML_SIMILAR_ELEMENT_NAME("similar");
static const std::string XML_SIMILAR_MOD_ELEMENT_NAME("mod");
static const std::string XML_NOTES_ELEMENT_NAME("notes");
static const std::string XML_NOTE_ELEMENT_NAME("note");
static const std::array<std::string_view, 9> XML_MOD_CHILD_ELEMENT_NAMES = {
	XML_MOD_TEAM_ELEMENT_NAME,
	XML_VERSIONS_ELEMENT_NAME,
	XML_DOWNLOADS_ELEMENT_NAME,
	XML_SCREENSHOTS_ELEMENT_NAME,
	XML_IMAGES_ELEMENT_NAME,
	XML_VIDEOS_ELEMENT_NAME,
	XML_RELATED_ELEMENT_NAME,
	XML_SIMILAR_ELEMENT_NAME,
	XML_NOTES_ELEMENT_NAME
};

static const std::string XML_MOD_ID_ATTRIBUTE_NAME("id");
static const std::string XML_MOD_NAME_ATTRIBUTE_NAME("name");
static const std::string XML_MOD_ALIAS_ATTRIBUTE_NAME("alias");
static const std::string XML_MOD_TYPE_ATTRIBUTE_NAME("type");
static const std::string XML_MOD_VERSION_ATTRIBUTE_NAME("version");
static const std::string XML_MOD_VERSION_TYPE_ATTRIBUTE_NAME("version_type");
static const std::string XML_MOD_WEBSITE_ATTRIBUTE_NAME("website");
static const std::string XML_MOD_REPOSITORY_ATTRIBUTE_NAME("repository");
static const std::array<std::string_view, 8> XML_MOD_ATTRIBUTE_NAMES = {
	XML_MOD_ID_ATTRIBUTE_NAME,
	XML_MOD_NAME_ATTRIBUTE_NAME,
	XML_MOD_ALIAS_ATTRIBUTE_NAME,
	XML_MOD_TYPE_ATTRIBUTE_NAME,
	XML_MOD_VERSION_ATTRIBUTE_NAME,
	XML_MOD_VERSION_TYPE_ATTRIBUTE_NAME,
	XML_MOD_WEBSITE_ATTRIBUTE_NAME,
	XML_MOD_REPOSITORY_ATTRIBUTE_NAME
};
static const std::string XML_RELATED_MOD_ID_ATTRIBUTE_NAME("id");
static const std::string XML_SIMILAR_MOD_ID_ATTRIBUTE_NAME("id");

Mod::Mod(const std::string & id, const std::string & name, const std::string & type)
	: m_id(Utilities::trimString(id))
	, m_name(Utilities::trimString(name))
	, m_type(Utilities::trimString(type)) { }

Mod::Mod(Mod && m) noexcept
	: m_id(std::move(m.m_id))
	, m_name(std::move(m.m_name))
	, m_alias(std::move(m.m_alias))
	, m_type(std::move(m.m_type))
	, m_preferredVersion(std::move(m.m_preferredVersion))
	, m_defaultVersionType(std::move(m.m_defaultVersionType))
	, m_website(std::move(m.m_website))
	, m_repositoryURL(std::move(m.m_repositoryURL))
	, m_team(m.m_team == nullptr ? nullptr : std::move(m.m_team))
	, m_versions(std::move(m.m_versions))
	, m_downloads(std::move(m.m_downloads))
	, m_screenshots(std::move(m.m_screenshots))
	, m_images(std::move(m.m_images))
	, m_videos(std::move(m.m_videos))
	, m_notes(std::move(m.m_notes))
	, m_relatedMods(std::move(m.m_relatedMods))
	, m_similarMods(std::move(m.m_similarMods)) {
	updateParent();
}

Mod::Mod(const Mod & m)
	: m_id(m.m_id)
	, m_name(m.m_name)
	, m_alias(m.m_alias)
	, m_type(m.m_type)
	, m_preferredVersion(m.m_preferredVersion)
	, m_defaultVersionType(m.m_defaultVersionType)
	, m_website(m.m_website)
	, m_repositoryURL(m.m_repositoryURL)
	, m_notes(m.m_notes)
	, m_relatedMods(m.m_relatedMods)
	, m_similarMods(m.m_similarMods) {
	if(m.m_team != nullptr) {
		m_team = std::make_shared<ModTeam>(*m.m_team);
	}

	for(std::vector<std::shared_ptr<ModVersion>>::const_iterator i = m.m_versions.begin(); i != m.m_versions.end(); ++i) {
		m_versions.push_back(std::make_shared<ModVersion>(**i));
	}

	for(std::vector<std::shared_ptr<ModDownload>>::const_iterator i = m.m_downloads.begin(); i != m.m_downloads.end(); ++i) {
		m_downloads.push_back(std::make_shared<ModDownload>(**i));
	}

	for(std::vector<std::shared_ptr<ModScreenshot>>::const_iterator i = m.m_screenshots.begin(); i != m.m_screenshots.end(); ++i) {
		m_screenshots.push_back(std::make_shared<ModScreenshot>(**i));
	}

	for(std::vector<std::shared_ptr<ModImage>>::const_iterator i = m.m_images.begin(); i != m.m_images.end(); ++i) {
		m_images.push_back(std::make_shared<ModImage>(**i));
	}

	for(std::vector<std::shared_ptr<ModVideo>>::const_iterator i = m.m_videos.begin(); i != m.m_videos.end(); ++i) {
		m_videos.push_back(std::make_shared<ModVideo>(**i));
	}

	updateParent();
}

Mod & Mod::operator = (Mod && m) noexcept {
	if(this != &m) {
		m_id = std::move(m.m_id);
		m_name = std::move(m.m_name);
		m_alias = std::move(m.m_alias);
		m_type = std::move(m.m_type);
		m_preferredVersion = std::move(m.m_preferredVersion);
		m_defaultVersionType = std::move(m.m_defaultVersionType);
		m_website = std::move(m.m_website);
		m_repositoryURL = std::move(m.m_repositoryURL);
		m_team = m.m_team == nullptr ? nullptr : std::move(m.m_team);
		m_versions = std::move(m.m_versions);
		m_downloads = std::move(m.m_downloads);
		m_screenshots = std::move(m.m_screenshots);
		m_images = std::move(m.m_images);
		m_videos = std::move(m.m_videos);
		m_notes = std::move(m.m_notes);
		m_relatedMods = std::move(m.m_relatedMods);
		m_similarMods = std::move(m.m_similarMods);

		updateParent();
	}

	return *this;
}

Mod & Mod::operator = (const Mod & m) {
	m_versions.clear();
	m_downloads.clear();
	m_screenshots.clear();
	m_images.clear();
	m_videos.clear();

	m_id = m.m_id;
	m_name = m.m_name;
	m_alias = m.m_alias;
	m_type = m.m_type;
	m_preferredVersion = m.m_preferredVersion;
	m_defaultVersionType = m.m_defaultVersionType;
	m_website = m.m_website;
	m_repositoryURL = m.m_repositoryURL;
	m_team = m.m_team == nullptr ? nullptr : std::make_shared<ModTeam>(*m.m_team);
	m_notes = m.m_notes;
	m_relatedMods = m.m_relatedMods;
	m_similarMods = m.m_similarMods;

	for(std::vector<std::shared_ptr<ModVersion>>::const_iterator i = m.m_versions.begin(); i != m.m_versions.end(); ++i) {
		m_versions.push_back(std::make_shared<ModVersion>(**i));
	}

	for(std::vector<std::shared_ptr<ModDownload>>::const_iterator i = m.m_downloads.begin(); i != m.m_downloads.end(); ++i) {
		m_downloads.push_back(std::make_shared<ModDownload>(**i));
	}

	for(std::vector<std::shared_ptr<ModScreenshot>>::const_iterator i = m.m_screenshots.begin(); i != m.m_screenshots.end(); ++i) {
		m_screenshots.push_back(std::make_shared<ModScreenshot>(**i));
	}

	for(std::vector<std::shared_ptr<ModImage>>::const_iterator i = m.m_images.begin(); i != m.m_images.end(); ++i) {
		m_images.push_back(std::make_shared<ModImage>(**i));
	}

	for(std::vector<std::shared_ptr<ModVideo>>::const_iterator i = m.m_videos.begin(); i != m.m_videos.end(); ++i) {
		m_videos.push_back(std::make_shared<ModVideo>(**i));
	}

	updateParent();

	return *this;
}

Mod::~Mod() = default;

const std::string & Mod::getID() const {
	return m_id;
}

const std::string & Mod::getName() const {
	return m_name;
}

std::string Mod::getFullName(size_t versionIndex, size_t versionTypeIndex) const {
	std::stringstream fullModName;
	fullModName << m_name;

	const std::shared_ptr<ModVersion> version = getVersion(versionIndex);

	if(version == nullptr) {
		return fullModName.str();
	}

	if(!version->getVersion().empty()) {
		fullModName << " " + version->getVersion();
	}

	const std::shared_ptr<ModVersionType> type = version->getType(versionTypeIndex);

	if(type == nullptr) {
		return fullModName.str();
	}

	if(!type->getType().empty()) {
		fullModName << " " + type->getType();
	}

	return fullModName.str();
}

bool Mod::hasAlias() const {
	return !m_alias.empty();
}

const std::string & Mod::getAlias() const {
	return m_alias;
}

const std::string & Mod::getType() const {
	return m_type;
}

const std::string & Mod::getPreferredVersionName() const {
	return m_preferredVersion;
}

size_t Mod::indexOfPreferredVersion() const {
	return indexOfVersion(m_preferredVersion);
}

size_t Mod::indexOfDefaultVersionType() const {
	return getPreferredVersion()->indexOfType(m_defaultVersionType);
}

std::shared_ptr<ModVersion> Mod::getPreferredVersion() const {
	return getVersion(m_preferredVersion);
}

const std::string & Mod::getDefaultVersionType() const {
	return m_defaultVersionType;
}

std::optional<Date> Mod::getInitialReleaseDate() const {
	std::shared_ptr<ModVersion> initialVersion(getInitialVersion());

	if(initialVersion == nullptr) {
		return {};
	}

	return initialVersion->getReleaseDate();
}

std::optional<Date> Mod::getLatestReleaseDate() const {
	std::shared_ptr<ModVersion> latestVersion(getLatestVersion());

	if(latestVersion == nullptr) {
		return {};
	}

	return latestVersion->getReleaseDate();
}

std::string Mod::getInitialReleaseDateAsString() const {
	std::optional<Date> initialReleaseDate(getInitialReleaseDate());

	return initialReleaseDate.has_value() ? initialReleaseDate.value().toString() : std::string();
}

std::string Mod::getLatestReleaseDateAsString() const {
	std::optional<Date> latestReleaseDate(getLatestReleaseDate());

	return latestReleaseDate.has_value() ? latestReleaseDate.value().toString() : std::string();
}

const std::string & Mod::getWebsite() const {
	return m_website;
}

const std::string & Mod::getRepositoryURL() const {
	return m_repositoryURL;
}

bool Mod::isStandAlone() const {
	if(!isValid(true)) {
		return false;
	}

	for(size_t i = 0; i < numberOfVersions(); i++) {
		std::shared_ptr<ModVersion> modVersion(getVersion(i));

		for(size_t j = 0; j < modVersion->numberOfTypes(); j++) {
			std::shared_ptr<ModVersionType> modVersionType(modVersion->getType(j));

			for(size_t k = 0; k < modVersionType->numberOfGameVersions(); k++) {
				if(modVersionType->getGameVersion(k)->isStandAlone()) {
					return true;
				}
			}
		}
	}

	return false;
}

bool Mod::hasTeam() const {
	return m_team != nullptr;
}

std::shared_ptr<ModTeam> Mod::getTeam() const {
	return m_team;
}

void Mod::setID(const std::string & id) {
	m_id = Utilities::trimString(id);
}

void Mod::setName(const std::string & name) {
	m_name = Utilities::trimString(name);
}

void Mod::setType(const std::string & type) {
	m_type = Utilities::trimString(type);
}

void Mod::setAlias(const std::string & alias) {
	m_alias = Utilities::trimString(alias);
}

void Mod::setPreferredVersionName(const std::string & preferredVersionName) {
	m_preferredVersion = Utilities::trimString(preferredVersionName);
}

void Mod::setDefaultVersionType(const std::string & versionType) {
	m_defaultVersionType = Utilities::trimString(versionType);
}

void Mod::setWebsite(const std::string & website) {
	m_website = Utilities::trimString(website);
}

void Mod::setRepositoryURL(const std::string & repositoryURL) {
	m_repositoryURL = Utilities::trimString(repositoryURL);
}

void Mod::setTeam(const ModTeam & team) {
	m_team = std::make_shared<ModTeam>(team);
	m_team->setParentMod(this);
}

void Mod::removeTeam() {
	m_team->setParentMod(nullptr);
	m_team = nullptr;
}

bool Mod::addTeamMember(const ModTeamMember & teamMember) {
	if(m_team == nullptr) {
		m_team = std::make_shared<ModTeam>();
	}

	return m_team->addMember(teamMember);
}

bool Mod::copyHiddenPropertiesFrom(const Mod & mod) {
	if(!isValid(true) || !mod.isValid(true) || !Utilities::areStringsEqualIgnoreCase(m_id, mod.m_id) || m_versions.size() != mod.m_versions.size()) {
		return false;
	}

	for(size_t i = 0; i < m_versions.size(); i++) {
		if(!m_versions[i]->copyHiddenPropertiesFrom(*mod.m_versions[i])) {
			return false;
		}
	}

	return true;
}

size_t Mod::numberOfVersions() const {
	return m_versions.size();
}

bool Mod::hasVersion(const ModVersion & version) const {
	for(std::vector<std::shared_ptr<ModVersion>>::const_iterator i = m_versions.begin(); i != m_versions.end(); ++i) {
		if(Utilities::areStringsEqualIgnoreCase((*i)->getVersion(), version.getVersion())) {
			return true;
		}
	}

	return false;
}

bool Mod::hasVersion(const std::string & version) const {
	for(std::vector<std::shared_ptr<ModVersion>>::const_iterator i = m_versions.begin(); i != m_versions.end(); ++i) {
		if(Utilities::areStringsEqualIgnoreCase((*i)->getVersion(), version)) {
			return true;
		}
	}

	return false;
}

size_t Mod::indexOfVersion(const ModVersion & version) const {
	for(size_t i = 0; i < m_versions.size(); i++) {
		if(Utilities::areStringsEqualIgnoreCase(m_versions[i]->getVersion(), version.getVersion())) {
			return i;
		}
	}

	return std::numeric_limits<size_t>::max();
}

size_t Mod::indexOfVersion(const std::string & version) const {
	for(size_t i = 0; i < m_versions.size(); i++) {
		if(Utilities::areStringsEqualIgnoreCase(m_versions[i]->getVersion(), version)) {
			return i;
		}
	}

	return std::numeric_limits<size_t>::max();
}

size_t Mod::indexOfStandAloneModVersion(const StandAloneMod & standAloneMod) const {
	if(!standAloneMod.isValid() || !Utilities::areStringsEqualIgnoreCase(m_id, standAloneMod.getID())) {
		return std::numeric_limits<size_t>::max();
	}

	return indexOfVersion(standAloneMod.getVersion());
}

std::shared_ptr<ModVersion> Mod::getVersion(size_t index) const {
	if(index >= m_versions.size()) {
		return nullptr;
	}

	return m_versions[index];
}

std::shared_ptr<ModVersion> Mod::getVersion(const std::string & version) const {
	for(std::vector<std::shared_ptr<ModVersion>>::const_iterator i = m_versions.begin(); i != m_versions.end(); ++i) {
		if(Utilities::areStringsEqualIgnoreCase((*i)->getVersion(), version)) {
			return *i;
		}
	}

	return nullptr;
}

size_t Mod::indexOfInitialVersion() const {
	size_t initialVersionIndex = std::numeric_limits<size_t>::max();

	for(size_t i = 0; i < m_versions.size(); i++) {
		const std::optional<Date> & currentReleaseDate(m_versions[i]->getReleaseDate());

		if(initialVersionIndex == std::numeric_limits<size_t>::max() || (currentReleaseDate.has_value() && (!m_versions[initialVersionIndex]->getReleaseDate().has_value() || currentReleaseDate.value() < m_versions[initialVersionIndex]->getReleaseDate().value()))) {
			initialVersionIndex = i;
		}
	}

	return initialVersionIndex;
}

std::shared_ptr<ModVersion> Mod::getInitialVersion() const {
	std::shared_ptr<ModVersion> initialVersion;

	for(std::vector<std::shared_ptr<ModVersion>>::const_iterator i = m_versions.begin(); i != m_versions.end(); ++i) {
		const std::optional<Date> & currentReleaseDate((*i)->getReleaseDate());

		if(initialVersion == nullptr || (currentReleaseDate.has_value() && (!initialVersion->getReleaseDate().has_value() || currentReleaseDate.value() < initialVersion->getReleaseDate().value()))) {
			initialVersion = *i;
		}
	}

	return initialVersion;
}

size_t Mod::indexOfLatestVersion() const {
	size_t latestVersionIndex = std::numeric_limits<size_t>::max();

	for(size_t i = 0; i < m_versions.size(); i++) {
		const std::optional<Date> & currentReleaseDate(m_versions[i]->getReleaseDate());

		if(latestVersionIndex == std::numeric_limits<size_t>::max() || (currentReleaseDate.has_value() && (!m_versions[latestVersionIndex]->getReleaseDate().has_value() || currentReleaseDate.value() >= m_versions[latestVersionIndex]->getReleaseDate().value()))) {
			latestVersionIndex = i;
		}
	}

	return latestVersionIndex;
}

std::shared_ptr<ModVersion> Mod::getLatestVersion() const {
	std::shared_ptr<ModVersion> latestVersion;

	for(std::vector<std::shared_ptr<ModVersion>>::const_iterator i = m_versions.begin(); i != m_versions.end(); ++i) {
		const std::optional<Date> & currentReleaseDate((*i)->getReleaseDate());

		if(latestVersion == nullptr || (currentReleaseDate.has_value() && (!latestVersion->getReleaseDate().has_value() || currentReleaseDate.value() >= latestVersion->getReleaseDate().value()))) {
			latestVersion = *i;
		}
	}

	return latestVersion;
}

const std::vector<std::shared_ptr<ModVersion>> & Mod::getVersions() const {
	return m_versions;
}

std::shared_ptr<ModVersion> Mod::getStandAloneModVersion(const StandAloneMod & standAloneMod) const {
	size_t standAloneModVersionIndex = indexOfStandAloneModVersion(standAloneMod);

	if(standAloneModVersionIndex == std::numeric_limits<size_t>::max()) {
		return nullptr;
	}

	return m_versions[standAloneModVersionIndex];
}

std::vector<std::string> Mod::getVersionDisplayNames(const std::string & emptySubstitution) const {
	std::vector<std::string> versionDisplayNames;

	for(const std::shared_ptr<ModVersion> & version : m_versions) {
		if(version->getVersion().empty()) {
			versionDisplayNames.push_back(emptySubstitution);
		}
		else {
			versionDisplayNames.push_back(version->getVersion());
		}
	}

	return versionDisplayNames;
}

bool Mod::addVersion(const ModVersion & version) {
	if(!version.isValid() || hasVersion(version)) {
		return false;
	}

	std::shared_ptr<ModVersion> newModVersion = std::make_shared<ModVersion>(version);
	newModVersion->setParentMod(this);

	m_versions.push_back(newModVersion);

	return true;
}

bool Mod::removeVersion(size_t index) {
	if(index >= m_versions.size()) {
		return false;
	}

	m_versions[index]->setParentMod(nullptr);
	m_versions.erase(m_versions.begin() + index);

	return true;
}

bool Mod::removeVersion(const ModVersion & version) {
	for(std::vector<std::shared_ptr<ModVersion>>::const_iterator i = m_versions.begin(); i != m_versions.end(); ++i) {
		if(Utilities::areStringsEqualIgnoreCase((*i)->getVersion(), version.getVersion())) {
			(*i)->setParentMod(nullptr);
			m_versions.erase(i);

			return true;
		}
	}

	return false;
}

bool Mod::removeVersion(const std::string & version) {
	for(std::vector<std::shared_ptr<ModVersion>>::const_iterator i = m_versions.begin(); i != m_versions.end(); ++i) {
		if(Utilities::areStringsEqualIgnoreCase((*i)->getVersion(), version)) {
			(*i)->setParentMod(nullptr);
			m_versions.erase(i);

			return true;
		}
	}

	return false;
}

void Mod::clearVersions() {
	m_versions.clear();
}

size_t Mod::numberOfDownloads() const {
	return m_downloads.size();
}

bool Mod::hasDownload(const ModDownload & download) const {
	for(std::vector<std::shared_ptr<ModDownload>>::const_iterator i = m_downloads.begin(); i != m_downloads.end(); ++i) {
		if(Utilities::areStringsEqualIgnoreCase((*i)->getFileName(), download.getFileName())) {
			return true;
		}
	}

	return false;
}

bool Mod::hasDownload(const std::string & fileName) const {
	if(fileName.empty()) {
		return false;
	}

	for(std::vector<std::shared_ptr<ModDownload>>::const_iterator i = m_downloads.begin(); i != m_downloads.end(); ++i) {
		if(Utilities::areStringsEqualIgnoreCase((*i)->getFileName(), fileName)) {
			return true;
		}
	}

	return false;
}

bool Mod::hasDownloadOfType(const std::string & type) const {
	if(type.empty()) {
		return false;
	}

	for(std::vector<std::shared_ptr<ModDownload>>::const_iterator i = m_downloads.begin(); i != m_downloads.end(); ++i) {
		if(Utilities::areStringsEqualIgnoreCase((*i)->getType(), type)) {
			return true;
		}
	}

	return false;
}

size_t Mod::indexOfDownload(const ModDownload & download) const {
	for(size_t i = 0; i < m_downloads.size(); i++) {
		if(Utilities::areStringsEqualIgnoreCase(m_downloads[i]->getFileName(), download.getFileName())) {
			return i;
		}
	}

	return std::numeric_limits<size_t>::max();
}

size_t Mod::indexOfDownload(const std::string & fileName) const {
	if(fileName.empty()) {
		return std::numeric_limits<size_t>::max();
	}

	for(size_t i = 0; i < m_downloads.size(); i++) {
		if(Utilities::areStringsEqualIgnoreCase(m_downloads[i]->getFileName(), fileName)) {
			return i;
		}
	}

	return std::numeric_limits<size_t>::max();
}

size_t Mod::indexOfDownloadByType(const std::string & type) const {
	if(type.empty()) {
		return std::numeric_limits<size_t>::max();
	}

	for(size_t i = 0; i < m_downloads.size(); i++) {
		if(Utilities::areStringsEqualIgnoreCase(m_downloads[i]->getType(), type)) {
			return i;
		}
	}

	return std::numeric_limits<size_t>::max();
}

std::shared_ptr<ModDownload> Mod::getDownload(size_t index) const {
	if(index >= m_downloads.size()) {
		return nullptr;
	}

	return m_downloads[index];
}

std::shared_ptr<ModDownload> Mod::getDownload(const std::string & fileName) const {
	if(fileName.empty()) {
		return nullptr;
	}

	for(std::vector<std::shared_ptr<ModDownload>>::const_iterator i = m_downloads.begin(); i != m_downloads.end(); ++i) {
		if(Utilities::areStringsEqualIgnoreCase((*i)->getFileName(), fileName)) {
			return *i;
		}
	}

	return nullptr;
}

std::shared_ptr<ModDownload> Mod::getDownloadByType(const std::string & type) const {
	if(type.empty()) {
		return nullptr;
	}

	for(std::vector<std::shared_ptr<ModDownload>>::const_iterator i = m_downloads.begin(); i != m_downloads.end(); ++i) {
		if(Utilities::areStringsEqualIgnoreCase((*i)->getType(), type)) {
			return *i;
		}
	}

	return nullptr;
}

std::optional<std::string> Mod::getDownloadFileNameByType(const std::string & type) const {
	if(type.empty()) {
		return {};
	}

	for(std::vector<std::shared_ptr<ModDownload>>::const_iterator i = m_downloads.begin(); i != m_downloads.end(); ++i) {
		if(Utilities::areStringsEqualIgnoreCase((*i)->getType(), type)) {
			return (*i)->getFileName();
		}
	}

	return {};
}

std::shared_ptr<ModDownload> Mod::getDownloadForGameVersion(const ModGameVersion * modGameVersion) const {
	if(!ModGameVersion::isValid(modGameVersion, true) || modGameVersion->getParentMod() != this) {
		return nullptr;
	}

	const ModVersion * modVersion = modGameVersion->getParentModVersion();

	if(modVersion == nullptr) {
		return nullptr;
	}

	std::shared_ptr<ModDownload> partiallyMatchingModDownload;

	for(const std::shared_ptr<ModDownload> & modDownload : m_downloads) {
		if(modDownload->isModManagerFiles() &&
		   Utilities::areStringsEqualIgnoreCase(modDownload->getVersion(), modVersion->getVersion()) &&
		   Utilities::areStringsEqualIgnoreCase(modDownload->getGameVersionID(), modGameVersion->getGameVersionID())) {
			if(modVersion->numberOfTypes() == 1) {
				return modDownload;
			}
			else if(Utilities::areStringsEqualIgnoreCase(modDownload->getVersionType(), modGameVersion->getParentModVersionType()->getType())) {
				return modDownload;
			}
			else {
				partiallyMatchingModDownload = modDownload;
			}
		}
	}

	return partiallyMatchingModDownload;
}

std::shared_ptr<ModVersion> Mod::getModVersionForDownload(const ModDownload * modDownload) const {
	if(!ModDownload::isValid(modDownload, true) || modDownload->getParentMod() != this) {
		return nullptr;
	}

	return getVersion(modDownload->getVersion());
}

std::shared_ptr<ModVersionType> Mod::getModVersionTypeForDownload(const ModDownload * modDownload) const {
	if(!ModDownload::isValid(modDownload, true) || modDownload->getParentMod() != this) {
		return nullptr;
	}

	std::shared_ptr<ModVersion> modVersion(getVersion(modDownload->getVersion()));

	if(modVersion == nullptr) {
		return nullptr;
	}

	return modVersion->getType(modDownload->getVersionType());
}

const std::vector<std::shared_ptr<ModDownload>> & Mod::getDownloads() const {
	return m_downloads;
}

bool Mod::addDownload(const ModDownload & download) {
	if(!download.isValid() || hasDownload(download)) {
		return false;
	}

	std::shared_ptr<ModDownload> newDownload = std::make_shared<ModDownload>(download);
	newDownload->setParentMod(this);

	m_downloads.push_back(newDownload);

	return true;
}

bool Mod::removeDownload(size_t index) {
	if(index >= m_downloads.size()) {
		return false;
	}

	m_downloads[index]->setParentMod(nullptr);
	m_downloads.erase(m_downloads.begin() + index);

	return true;
}

bool Mod::removeDownload(const ModDownload & download) {
	for(std::vector<std::shared_ptr<ModDownload>>::const_iterator i = m_downloads.begin(); i != m_downloads.end(); ++i) {
		if(Utilities::areStringsEqualIgnoreCase((*i)->getFileName(), download.getFileName())) {
			(*i)->setParentMod(nullptr);
			m_downloads.erase(i);

			return true;
		}
	}

	return false;
}

bool Mod::removeDownload(const std::string & fileName) {
	if(fileName.empty()) {
		return false;
	}

	for(std::vector<std::shared_ptr<ModDownload>>::const_iterator i = m_downloads.begin(); i != m_downloads.end(); ++i) {
		if(Utilities::areStringsEqualIgnoreCase((*i)->getFileName(), fileName)) {
			(*i)->setParentMod(nullptr);
			m_downloads.erase(i);

			return true;
		}
	}

	return false;
}

bool Mod::removeDownloadByType(const std::string & type) {
	if(type.empty()) {
		return false;
	}

	for(std::vector<std::shared_ptr<ModDownload>>::const_iterator i = m_downloads.begin(); i != m_downloads.end(); ++i) {
		if(Utilities::areStringsEqualIgnoreCase((*i)->getType(), type)) {
			(*i)->setParentMod(nullptr);
			m_downloads.erase(i);

			return true;
		}
	}

	return false;
}

void Mod::clearDownloads() {
	m_downloads.clear();
}

size_t Mod::numberOfScreenshots() const {
	return m_screenshots.size();
}

bool Mod::hasScreenshot(const ModScreenshot & screenshot) const {
	for(std::vector<std::shared_ptr<ModScreenshot>>::const_iterator i = m_screenshots.begin(); i != m_screenshots.end(); ++i) {
		if(Utilities::areStringsEqualIgnoreCase((*i)->getFileName(), screenshot.getFileName())) {
			return true;
		}
	}

	return false;
}

bool Mod::hasScreenshot(const std::string & fileName) const {
	if(fileName.empty()) {
		return false;
	}

	for(std::vector<std::shared_ptr<ModScreenshot>>::const_iterator i = m_screenshots.begin(); i != m_screenshots.end(); ++i) {
		if(Utilities::areStringsEqualIgnoreCase((*i)->getFileName(), fileName)) {
			return true;
		}
	}

	return false;
}

size_t Mod::indexOfScreenshot(const ModScreenshot & screenshot) const {
	for(size_t i = 0; i < m_screenshots.size(); i++) {
		if(Utilities::areStringsEqualIgnoreCase(m_screenshots[i]->getFileName(), screenshot.getFileName())) {
			return i;
		}
	}

	return std::numeric_limits<size_t>::max();
}

size_t Mod::indexOfScreenshot(const std::string & fileName) const {
	if(fileName.empty()) {
		return std::numeric_limits<size_t>::max();
	}

	for(size_t i = 0; i < m_screenshots.size(); i++) {
		if(Utilities::areStringsEqualIgnoreCase(m_screenshots[i]->getFileName(), fileName)) {
			return i;
		}
	}

	return std::numeric_limits<size_t>::max();
}

std::shared_ptr<ModScreenshot> Mod::getScreenshot(size_t index) const {
	if(index >= m_screenshots.size()) {
		return nullptr;
	}

	return m_screenshots[index];
}

std::shared_ptr<ModScreenshot> Mod::getScreenshot(const std::string & fileName) const {
	if(fileName.empty()) {
		return nullptr;
	}

	for(std::vector<std::shared_ptr<ModScreenshot>>::const_iterator i = m_screenshots.begin(); i != m_screenshots.end(); ++i) {
		if(Utilities::areStringsEqualIgnoreCase((*i)->getFileName(), fileName)) {
			return *i;
		}
	}

	return nullptr;
}

const std::vector<std::shared_ptr<ModScreenshot>> & Mod::getScreenshots() const {
	return m_screenshots;
}

bool Mod::addScreenshot(const ModScreenshot & screenshot) {
	if(!screenshot.isValid() || hasScreenshot(screenshot)) {
		return false;
	}

	std::shared_ptr<ModScreenshot> newScreenshot = std::make_shared<ModScreenshot>(screenshot);
	newScreenshot->setParentMod(this);

	m_screenshots.push_back(newScreenshot);

	return true;
}

bool Mod::removeScreenshot(size_t index) {
	if(index >= m_screenshots.size()) {
		return false;
	}

	m_screenshots[index]->setParentMod(nullptr);
	m_screenshots.erase(m_screenshots.begin() + index);

	return true;
}

bool Mod::removeScreenshot(const ModScreenshot & screenshot) {
	for(std::vector<std::shared_ptr<ModScreenshot>>::const_iterator i = m_screenshots.begin(); i != m_screenshots.end(); ++i) {
		if(Utilities::areStringsEqualIgnoreCase((*i)->getFileName(), screenshot.getFileName())) {
			(*i)->setParentMod(nullptr);
			m_screenshots.erase(i);

			return true;
		}
	}

	return false;
}

bool Mod::removeScreenshot(const std::string & fileName) {
	if(fileName.empty()) {
		return false;
	}

	for(std::vector<std::shared_ptr<ModScreenshot>>::const_iterator i = m_screenshots.begin(); i != m_screenshots.end(); ++i) {
		if(Utilities::areStringsEqualIgnoreCase((*i)->getFileName(), fileName)) {
			(*i)->setParentMod(nullptr);
			m_screenshots.erase(i);

			return true;
		}
	}

	return false;
}

void Mod::clearScreenshots() {
	m_screenshots.clear();
}

size_t Mod::numberOfImages() const {
	return m_images.size();
}

bool Mod::hasImage(const ModImage & image) const {
	for(std::vector<std::shared_ptr<ModImage>>::const_iterator i = m_images.begin(); i != m_images.end(); ++i) {
		if(Utilities::areStringsEqualIgnoreCase((*i)->getFileName(), image.getFileName())) {
			return true;
		}
	}

	return false;
}

bool Mod::hasImage(const std::string & fileName) const {
	if(fileName.empty()) {
		return false;
	}

	for(std::vector<std::shared_ptr<ModImage>>::const_iterator i = m_images.begin(); i != m_images.end(); ++i) {
		if(Utilities::areStringsEqualIgnoreCase((*i)->getFileName(), fileName)) {
			return true;
		}
	}

	return false;
}

size_t Mod::indexOfImage(const ModImage & image) const {
	for(size_t i = 0; i < m_images.size(); i++) {
		if(Utilities::areStringsEqualIgnoreCase(m_images[i]->getFileName(), image.getFileName())) {
			return i;
		}
	}

	return std::numeric_limits<size_t>::max();
}

size_t Mod::indexOfImage(const std::string & fileName) const {
	if(fileName.empty()) {
		return std::numeric_limits<size_t>::max();
	}

	for(size_t i = 0; i < m_images.size(); i++) {
		if(Utilities::areStringsEqualIgnoreCase(m_images[i]->getFileName(), fileName)) {
			return i;
		}
	}

	return std::numeric_limits<size_t>::max();
}

std::shared_ptr<ModImage> Mod::getImage(size_t index) const {
	if(index >= m_images.size()) {
		return nullptr;
	}

	return m_images[index];
}

std::shared_ptr<ModImage> Mod::getImage(const std::string & fileName) const {
	if(fileName.empty()) {
		return nullptr;
	}

	for(std::vector<std::shared_ptr<ModImage>>::const_iterator i = m_images.begin(); i != m_images.end(); ++i) {
		if(Utilities::areStringsEqualIgnoreCase((*i)->getFileName(), fileName)) {
			return *i;
		}
	}

	return nullptr;
}

const std::vector<std::shared_ptr<ModImage>> & Mod::getImages() const {
	return m_images;
}

bool Mod::addImage(const ModImage & image) {
	if(!image.isValid() || hasImage(image)) {
		return false;
	}

	std::shared_ptr newImage = std::make_shared<ModImage>(image);
	newImage->setParentMod(this);

	m_images.push_back(newImage);

	return true;
}

bool Mod::removeImage(size_t index) {
	if(index >= m_images.size()) {
		return false;
	}

	m_images[index]->setParentMod(nullptr);
	m_images.erase(m_images.begin() + index);

	return true;
}

bool Mod::removeImage(const ModImage & image) {
	for(std::vector<std::shared_ptr<ModImage>>::const_iterator i = m_images.begin(); i != m_images.end(); ++i) {
		if(Utilities::areStringsEqualIgnoreCase((*i)->getFileName(), image.getFileName())) {
			(*i)->setParentMod(nullptr);
			m_images.erase(i);

			return true;
		}
	}

	return false;
}

bool Mod::removeImage(const std::string & fileName) {
	if(fileName.empty()) {
		return false;
	}

	for(std::vector<std::shared_ptr<ModImage>>::const_iterator i = m_images.begin(); i != m_images.end(); ++i) {
		if(Utilities::areStringsEqualIgnoreCase((*i)->getFileName(), fileName)) {
			(*i)->setParentMod(nullptr);
			m_images.erase(i);

			return true;
		}
	}

	return false;
}

void Mod::clearImages() {
	m_images.clear();
}

size_t Mod::numberOfVideos() const {
	return m_videos.size();
}

bool Mod::hasVideo(const ModVideo & video) const {
	for(std::vector<std::shared_ptr<ModVideo>>::const_iterator i = m_videos.begin(); i != m_videos.end(); ++i) {
		if(Utilities::areStringsEqualIgnoreCase((*i)->getURL(), video.getURL())) {
			return true;
		}
	}

	return false;
}

bool Mod::hasVideo(const std::string & url) const {
	if(url.empty()) {
		return false;
	}

	for(std::vector<std::shared_ptr<ModVideo>>::const_iterator i = m_videos.begin(); i != m_videos.end(); ++i) {
		if(Utilities::areStringsEqualIgnoreCase((*i)->getURL(), url)) {
			return true;
		}
	}

	return false;
}

size_t Mod::indexOfVideo(const ModVideo & video) const {
	for(size_t i = 0; i < m_videos.size(); i++) {
		if(Utilities::areStringsEqualIgnoreCase(m_videos[i]->getURL(), video.getURL())) {
			return i;
		}
	}

	return std::numeric_limits<size_t>::max();
}

size_t Mod::indexOfVideo(const std::string & url) const {
	if(url.empty()) {
		return std::numeric_limits<size_t>::max();
	}

	for(size_t i = 0; i < m_videos.size(); i++) {
		if(Utilities::areStringsEqualIgnoreCase(m_videos[i]->getURL(), url)) {
			return i;
		}
	}

	return std::numeric_limits<size_t>::max();
}

std::shared_ptr<ModVideo> Mod::getVideo(size_t index) const {
	if(index >= m_videos.size()) {
		return nullptr;
	}

	return m_videos[index];
}

std::shared_ptr<ModVideo> Mod::getVideo(const std::string & url) const {
	if(url.empty()) {
		return nullptr;
	}

	for(std::vector<std::shared_ptr<ModVideo>>::const_iterator i = m_videos.begin(); i != m_videos.end(); ++i) {
		if(Utilities::areStringsEqualIgnoreCase((*i)->getURL(), url)) {
			return *i;
		}
	}

	return nullptr;
}

const std::vector<std::shared_ptr<ModVideo>> & Mod::getVideos() const {
	return m_videos;
}

bool Mod::addVideo(const ModVideo & video) {
	if(!video.isValid() || hasVideo(video)) {
		return false;
	}

	std::shared_ptr<ModVideo> newVideo = std::make_shared<ModVideo>(video);
	newVideo->setParentMod(this);

	m_videos.push_back(newVideo);

	return true;
}

bool Mod::removeVideo(size_t index) {
	if(index >= m_videos.size()) {
		return false;
	}

	m_videos[index]->setParentMod(nullptr);
	m_videos.erase(m_videos.begin() + index);

	return true;
}

bool Mod::removeVideo(const ModVideo & video) {
	for(std::vector<std::shared_ptr<ModVideo>>::const_iterator i = m_videos.begin(); i != m_videos.end(); ++i) {
		if(Utilities::areStringsEqualIgnoreCase((*i)->getURL(), video.getURL())) {
			(*i)->setParentMod(nullptr);
			m_videos.erase(i);

			return true;
		}
	}

	return false;
}

bool Mod::removeVideo(const std::string & url) {
	if(url.empty()) {
		return false;
	}

	for(std::vector<std::shared_ptr<ModVideo>>::const_iterator i = m_videos.begin(); i != m_videos.end(); ++i) {
		if(Utilities::areStringsEqualIgnoreCase((*i)->getURL(), url)) {
			(*i)->setParentMod(nullptr);
			m_videos.erase(i);

			return true;
		}
	}

	return false;
}

void Mod::clearVideos() {
	m_videos.clear();
}

size_t Mod::numberOfNotes() const {
	return m_notes.size();
}

bool Mod::hasNote(const std::string & note) const {
	for(std::vector<std::string>::const_iterator i = m_notes.begin(); i != m_notes.end(); ++i) {
		if(*i == note) {
			return true;
		}
	}

	return false;
}

size_t Mod::indexOfNote(const std::string & note) const {
	for(size_t i = 0; i < m_notes.size(); i++) {
		if(m_notes[i] == note) {
			return i;
		}
	}

	return std::numeric_limits<size_t>::max();
}

const std::string & Mod::getNote(size_t index) const {
	if(index >= m_notes.size()) {
		return Utilities::emptyString;
	}

	return m_notes[index];
}

const std::vector<std::string> & Mod::getNotes() const {
	return m_notes;
}

bool Mod::addNote(const std::string & note) {
	if(note.empty() || hasNote(note)) {
		return false;
	}

	m_notes.emplace_back(note);

	return true;
}

bool Mod::removeNote(size_t index) {
	if(index >= m_notes.size()) {
		return false;
	}

	m_notes.erase(m_notes.begin() + index);

	return true;
}

bool Mod::removeNote(const std::string & note) {
	for(std::vector<std::string>::const_iterator i = m_notes.begin(); i != m_notes.end(); ++i) {
		if(*i == note) {
			return true;
		}
	}

	return false;
}

void Mod::clearNotes() {
	m_notes.clear();
}

size_t Mod::numberOfRelatedMods() const {
	return m_relatedMods.size();
}

bool Mod::hasRelatedMod(const std::string & relatedMod) const {
	for(std::vector<std::string>::const_iterator i = m_relatedMods.begin(); i != m_relatedMods.end(); ++i) {
		if(*i == relatedMod) {
			return true;
		}
	}

	return false;
}

size_t Mod::indexOfRelatedMod(const std::string & relatedMod) const {
	for(size_t i = 0; i < m_relatedMods.size(); i++) {
		if(m_relatedMods[i] == relatedMod) {
			return i;
		}
	}

	return std::numeric_limits<size_t>::max();
}

const std::string & Mod::getRelatedMod(size_t index) const {
	if(index >= m_relatedMods.size()) {
		return Utilities::emptyString;
	}

	return m_relatedMods[index];
}

const std::vector<std::string> & Mod::getRelatedMods() const {
	return m_relatedMods;
}

bool Mod::addRelatedMod(const std::string & relatedMod) {
	if(relatedMod.empty() || hasRelatedMod(relatedMod)) {
		return false;
	}

	m_relatedMods.emplace_back(relatedMod);

	return true;
}

bool Mod::removeRelatedMod(size_t index) {
	if(index >= m_relatedMods.size()) {
		return false;
	}

	m_relatedMods.erase(m_relatedMods.begin() + index);

	return true;
}

bool Mod::removeRelatedMod(const std::string & relatedMod) {
	for(std::vector<std::string>::const_iterator i = m_relatedMods.begin(); i != m_relatedMods.end(); ++i) {
		if(*i == relatedMod) {
			return true;
		}
	}

	return false;
}

void Mod::clearRelatedMods() {
	m_relatedMods.clear();
}

size_t Mod::numberOfSimilarMods() const {
	return m_similarMods.size();
}

bool Mod::hasSimilarMod(const std::string & similarMod) const {
	for(std::vector<std::string>::const_iterator i = m_similarMods.begin(); i != m_similarMods.end(); ++i) {
		if(*i == similarMod) {
			return true;
		}
	}

	return false;
}

size_t Mod::indexOfSimilarMod(const std::string & similarMod) const {
	for(size_t i = 0; i < m_similarMods.size(); i++) {
		if(m_similarMods[i] == similarMod) {
			return i;
		}
	}

	return std::numeric_limits<size_t>::max();
}

const std::string & Mod::getSimilarMod(size_t index) const {
	if(index >= m_similarMods.size()) {
		return Utilities::emptyString;
	}

	return m_similarMods[index];
}

const std::vector<std::string> & Mod::getSimilarMods() const {
	return m_similarMods;
}

bool Mod::addSimilarMod(const std::string & similarMod) {
	if(similarMod.empty() || hasSimilarMod(similarMod)) {
		return false;
	}

	m_similarMods.emplace_back(similarMod);

	return true;
}

bool Mod::removeSimilarMod(size_t index) {
	if(index >= m_similarMods.size()) {
		return false;
	}

	m_similarMods.erase(m_similarMods.begin() + index);

	return true;
}

bool Mod::removeSimilarMod(const std::string & similarMod) {
	for(std::vector<std::string>::const_iterator i = m_similarMods.begin(); i != m_similarMods.end(); ++i) {
		if(*i == similarMod) {
			return true;
		}
	}

	return false;
}

void Mod::clearSimilarMods() {
	m_similarMods.clear();
}

rapidjson::Value Mod::toJSON(rapidjson::MemoryPoolAllocator<rapidjson::CrtAllocator> & allocator) const {
	rapidjson::Value modValue(rapidjson::kObjectType);

	rapidjson::Value idValue(m_id.c_str(), allocator);
	modValue.AddMember(rapidjson::StringRef(JSON_MOD_ID_PROPERTY_NAME), idValue, allocator);

	rapidjson::Value nameValue(m_name.c_str(), allocator);
	modValue.AddMember(rapidjson::StringRef(JSON_MOD_NAME_PROPERTY_NAME), nameValue, allocator);

	if(!m_alias.empty()) {
		rapidjson::Value aliasValue(m_alias.c_str(), allocator);
		modValue.AddMember(rapidjson::StringRef(JSON_MOD_ALIAS_PROPERTY_NAME), aliasValue, allocator);
	}

	rapidjson::Value typeValue(m_type.c_str(), allocator);
	modValue.AddMember(rapidjson::StringRef(JSON_MOD_TYPE_PROPERTY_NAME), typeValue, allocator);

	if(!m_preferredVersion.empty()) {
		rapidjson::Value preferredVersionNameValue(m_preferredVersion.c_str(), allocator);
		modValue.AddMember(rapidjson::StringRef(JSON_MOD_PREFERRED_VERSION_NAME_PROPERTY_NAME), preferredVersionNameValue, allocator);
	}

	if(!m_defaultVersionType.empty()) {
		rapidjson::Value defaultVersionTypeValue(m_defaultVersionType.c_str(), allocator);
		modValue.AddMember(rapidjson::StringRef(JSON_MOD_DEFAULT_VERSION_TYPE_PROPERTY_NAME), defaultVersionTypeValue, allocator);
	}

	if(!m_website.empty()) {
		rapidjson::Value websiteValue(m_website.c_str(), allocator);
		modValue.AddMember(rapidjson::StringRef(JSON_MOD_WEBSITE_PROPERTY_NAME), websiteValue, allocator);
	}

	if(!m_repositoryURL.empty()) {
		rapidjson::Value repositoryURLValue(m_repositoryURL.c_str(), allocator);
		modValue.AddMember(rapidjson::StringRef(JSON_MOD_REPOSITORY_URL_PROPERTY_NAME), repositoryURLValue, allocator);
	}

	if(m_team != nullptr) {
		rapidjson::Value teamValue(m_team->toJSON(allocator));
		modValue.AddMember(rapidjson::StringRef(JSON_MOD_TEAM_PROPERTY_NAME), teamValue, allocator);
	}

	rapidjson::Value versionsValue(rapidjson::kArrayType);
	versionsValue.Reserve(m_versions.size(), allocator);

	for(const std::shared_ptr<ModVersion> & modVersion : m_versions) {
		versionsValue.PushBack(modVersion->toJSON(allocator), allocator);
	}

	modValue.AddMember(rapidjson::StringRef(JSON_MOD_VERSIONS_PROPERTY_NAME), versionsValue, allocator);

	rapidjson::Value downloadsValue(rapidjson::kArrayType);
	downloadsValue.Reserve(m_downloads.size(), allocator);

	for(const std::shared_ptr<ModDownload> & modDownload : m_downloads) {
		downloadsValue.PushBack(modDownload->toJSON(allocator), allocator);
	}

	modValue.AddMember(rapidjson::StringRef(JSON_MOD_DOWNLOADS_PROPERTY_NAME), downloadsValue, allocator);

	if(!m_screenshots.empty()) {
		rapidjson::Value screenshotsValue(rapidjson::kArrayType);
		screenshotsValue.Reserve(m_screenshots.size(), allocator);

		for(const std::shared_ptr<ModScreenshot> & screenshot : m_screenshots) {
			screenshotsValue.PushBack(screenshot->toJSON(allocator), allocator);
		}

		modValue.AddMember(rapidjson::StringRef(JSON_MOD_SCREENSHOTS_PROPERTY_NAME), screenshotsValue, allocator);
	}

	if(!m_images.empty()) {
		rapidjson::Value imagesValue(rapidjson::kArrayType);
		imagesValue.Reserve(m_images.size(), allocator);

		for(const std::shared_ptr<ModImage> & image : m_images) {
			imagesValue.PushBack(image->toJSON(allocator), allocator);
		}

		modValue.AddMember(rapidjson::StringRef(JSON_MOD_IMAGES_PROPERTY_NAME), imagesValue, allocator);
	}

	if(!m_videos.empty()) {
		rapidjson::Value videosValue(rapidjson::kArrayType);
		videosValue.Reserve(m_videos.size(), allocator);

		for(const std::shared_ptr<ModVideo> & video : m_videos) {
			videosValue.PushBack(video->toJSON(allocator), allocator);
		}

		modValue.AddMember(rapidjson::StringRef(JSON_MOD_VIDEOS_PROPERTY_NAME), videosValue, allocator);
	}

	if(!m_notes.empty()) {
		rapidjson::Value notesValue(rapidjson::kArrayType);
		notesValue.Reserve(m_notes.size(), allocator);

		for(const std::string & note : m_notes) {
			rapidjson::Value noteValue(note.c_str(), allocator);
			notesValue.PushBack(noteValue, allocator);
		}

		modValue.AddMember(rapidjson::StringRef(JSON_MOD_NOTES_PROPERTY_NAME), notesValue, allocator);
	}

	if(!m_relatedMods.empty()) {
		rapidjson::Value relatedModsValue(rapidjson::kArrayType);
		relatedModsValue.Reserve(m_relatedMods.size(), allocator);

		for(const std::string & relatedMod : m_relatedMods) {
			rapidjson::Value relatedModValue(relatedMod.c_str(), allocator);
			relatedModsValue.PushBack(relatedModValue, allocator);
		}

		modValue.AddMember(rapidjson::StringRef(JSON_MOD_RELATED_MODS_PROPERTY_NAME), relatedModsValue, allocator);
	}

	if(!m_similarMods.empty()) {
		rapidjson::Value similarModsValue(rapidjson::kArrayType);
		similarModsValue.Reserve(m_similarMods.size(), allocator);

		for(const std::string & similarMod : m_similarMods) {
			rapidjson::Value similarModValue(similarMod.c_str(), allocator);
			similarModsValue.PushBack(similarModValue, allocator);
		}

		modValue.AddMember(rapidjson::StringRef(JSON_MOD_SIMILAR_MODS_PROPERTY_NAME), similarModsValue, allocator);
	}

	return modValue;
}

tinyxml2::XMLElement * Mod::toXML(tinyxml2::XMLDocument * document) const {
	if(document == nullptr) {
		return nullptr;
	}

	tinyxml2::XMLElement * modElement = document->NewElement(XML_MOD_ELEMENT_NAME.c_str());

	modElement->SetAttribute(XML_MOD_NAME_ATTRIBUTE_NAME.c_str(), m_name.c_str());
	modElement->SetAttribute(XML_MOD_ID_ATTRIBUTE_NAME.c_str(), m_id.c_str());

	if(!m_alias.empty()) {
		modElement->SetAttribute(XML_MOD_ALIAS_ATTRIBUTE_NAME.c_str(), m_alias.c_str());
	}

	modElement->SetAttribute(XML_MOD_TYPE_ATTRIBUTE_NAME.c_str(), m_type.c_str());

	if(!m_preferredVersion.empty()) {
		modElement->SetAttribute(XML_MOD_VERSION_ATTRIBUTE_NAME.c_str(), m_preferredVersion.c_str());
	}

	if(!m_defaultVersionType.empty()) {
		modElement->SetAttribute(XML_MOD_VERSION_TYPE_ATTRIBUTE_NAME.c_str(), m_defaultVersionType.c_str());
	}

	if(!m_website.empty()) {
		modElement->SetAttribute(XML_MOD_WEBSITE_ATTRIBUTE_NAME.c_str(), m_website.c_str());
	}

	if(!m_repositoryURL.empty()) {
		modElement->SetAttribute(XML_MOD_REPOSITORY_ATTRIBUTE_NAME.c_str(), m_repositoryURL.c_str());
	}

	if(m_team != nullptr) {
		modElement->InsertEndChild(m_team->toXML(document));
	}

	tinyxml2::XMLElement * filesElement = document->NewElement(XML_VERSIONS_ELEMENT_NAME.c_str());

	for(std::vector<std::shared_ptr<ModVersion>>::const_iterator i = m_versions.begin(); i != m_versions.end(); ++i) {
		filesElement->InsertEndChild((*i)->toXML(document));
	}

	modElement->InsertEndChild(filesElement);

	tinyxml2::XMLElement * downloadsElement = document->NewElement(XML_DOWNLOADS_ELEMENT_NAME.c_str());

	for(std::vector<std::shared_ptr<ModDownload>>::const_iterator i = m_downloads.begin(); i != m_downloads.end(); ++i) {
		downloadsElement->InsertEndChild((*i)->toXML(document));
	}

	modElement->InsertEndChild(downloadsElement);

	if(!m_images.empty()) {
		tinyxml2::XMLElement * imagesElement = document->NewElement(XML_IMAGES_ELEMENT_NAME.c_str());

		for(std::vector<std::shared_ptr<ModImage>>::const_iterator i = m_images.begin(); i != m_images.end(); ++i) {
			imagesElement->InsertEndChild((*i)->toXML(document));
		}

		modElement->InsertEndChild(imagesElement);
	}

	if(!m_screenshots.empty()) {
		tinyxml2::XMLElement * screenshotsElement = document->NewElement(XML_SCREENSHOTS_ELEMENT_NAME.c_str());

		for(std::vector<std::shared_ptr<ModScreenshot>>::const_iterator i = m_screenshots.begin(); i != m_screenshots.end(); ++i) {
			screenshotsElement->InsertEndChild((*i)->toXML(document));
		}

		modElement->InsertEndChild(screenshotsElement);
	}

	if(!m_videos.empty()) {
		tinyxml2::XMLElement * videosElement = document->NewElement(XML_VIDEOS_ELEMENT_NAME.c_str());

		for(std::vector<std::shared_ptr<ModVideo>>::const_iterator i = m_videos.begin(); i != m_videos.end(); ++i) {
			videosElement->InsertEndChild((*i)->toXML(document));
		}

		modElement->InsertEndChild(videosElement);
	}

	if(!m_notes.empty()) {
		tinyxml2::XMLElement * noteElement = nullptr;
		tinyxml2::XMLElement * notesElement = document->NewElement(XML_NOTES_ELEMENT_NAME.c_str());

		for(std::vector<std::string>::const_iterator i = m_notes.begin(); i != m_notes.end(); ++i) {
			noteElement = document->NewElement(XML_NOTE_ELEMENT_NAME.c_str());
			noteElement->SetText((*i).c_str());
			notesElement->InsertEndChild(noteElement);
		}

		modElement->InsertEndChild(notesElement);
	}

	if(!m_relatedMods.empty()) {
		tinyxml2::XMLElement * relatedModElement = nullptr;
		tinyxml2::XMLElement * relatedModsElement = document->NewElement(XML_RELATED_ELEMENT_NAME.c_str());

		for(std::vector<std::string>::const_iterator i = m_relatedMods.begin(); i != m_relatedMods.end(); ++i) {
			relatedModElement = document->NewElement(XML_RELATED_MOD_ELEMENT_NAME.c_str());
			relatedModElement->SetAttribute(XML_RELATED_MOD_ID_ATTRIBUTE_NAME.c_str(), (*i).c_str());
			relatedModsElement->InsertEndChild(relatedModElement);
		}

		modElement->InsertEndChild(relatedModsElement);
	}

	if(!m_similarMods.empty()) {
		tinyxml2::XMLElement * similarModElement = nullptr;
		tinyxml2::XMLElement * similarModsElement = document->NewElement(XML_SIMILAR_ELEMENT_NAME.c_str());

		for(std::vector<std::string>::const_iterator i = m_similarMods.begin(); i != m_similarMods.end(); ++i) {
			similarModElement = document->NewElement(XML_SIMILAR_MOD_ELEMENT_NAME.c_str());
			similarModElement->SetAttribute(XML_SIMILAR_MOD_ID_ATTRIBUTE_NAME.c_str(), (*i).c_str());
			similarModsElement->InsertEndChild(similarModElement);
		}

		modElement->InsertEndChild(similarModsElement);
	}

	return modElement;
}

std::unique_ptr<Mod> Mod::parseFrom(const rapidjson::Value & modValue, bool skipFileInfoValidation) {
	if(!modValue.IsObject()) {
		spdlog::error("Invalid mod type: '{}', expected 'object'.", Utilities::typeToString(modValue.GetType()));
		return nullptr;
	}

	// check for unhandled mod element properties
	bool propertyHandled = false;

	for(rapidjson::Value::ConstMemberIterator i = modValue.MemberBegin(); i != modValue.MemberEnd(); ++i) {
		propertyHandled = false;

		for(const std::string_view propertyName : JSON_MOD_PROPERTY_NAMES) {
			if(i->name.GetString() == propertyName) {
				propertyHandled = true;
				break;
			}
		}

		if(!propertyHandled) {
			spdlog::warn("Mod has unexpected property '{}'.", i->name.GetString());
		}
	}

	// parse mod id
	if(!modValue.HasMember(JSON_MOD_ID_PROPERTY_NAME)) {
		spdlog::error("Mod is missing '{}' property.", JSON_MOD_ID_PROPERTY_NAME);
		return nullptr;
	}

	const rapidjson::Value & modIDValue = modValue[JSON_MOD_ID_PROPERTY_NAME];

	if(!modIDValue.IsString()) {
		spdlog::error("Mod has an invalid '{}' property type: '{}', expected 'string'.", JSON_MOD_ID_PROPERTY_NAME, Utilities::typeToString(modIDValue.GetType()));
		return nullptr;
	}

	std::string modID(Utilities::trimString(modIDValue.GetString()));

	if(modID.empty()) {
		spdlog::error("Mod '{}' property cannot be empty.", JSON_MOD_ID_PROPERTY_NAME);
		return nullptr;
	}

	// parse mod name
	if(!modValue.HasMember(JSON_MOD_NAME_PROPERTY_NAME)) {
		spdlog::error("Mod '{}' is missing '{}' property.", modID, JSON_MOD_NAME_PROPERTY_NAME);
		return nullptr;
	}

	const rapidjson::Value & modNameValue = modValue[JSON_MOD_NAME_PROPERTY_NAME];

	if(!modNameValue.IsString()) {
		spdlog::error("Mod '{}' has an invalid '{}' property type: '{}', expected 'string'.", modID, JSON_MOD_NAME_PROPERTY_NAME, Utilities::typeToString(modNameValue.GetType()));
		return nullptr;
	}

	std::string modName(Utilities::trimString(modNameValue.GetString()));

	if(modName.empty()) {
		spdlog::error("Mod '{}' '{}' property cannot be empty.", modID, JSON_MOD_NAME_PROPERTY_NAME);
		return nullptr;
	}

	// parse mod type
	if(!modValue.HasMember(JSON_MOD_TYPE_PROPERTY_NAME)) {
		spdlog::error("Mod '{}' is missing '{}' property.", modID, JSON_MOD_TYPE_PROPERTY_NAME);
		return nullptr;
	}

	const rapidjson::Value & modTypeValue = modValue[JSON_MOD_TYPE_PROPERTY_NAME];

	if(!modTypeValue.IsString()) {
		spdlog::error("Mod '{}' has an invalid '{}' property type: '{}', expected 'string'.", modID, JSON_MOD_TYPE_PROPERTY_NAME, Utilities::typeToString(modTypeValue.GetType()));
		return nullptr;
	}

	std::string modType(Utilities::trimString(modTypeValue.GetString()));

	if(modType.empty()) {
		spdlog::error("Mod '{}' '{}' property cannot be empty.", modID, JSON_MOD_TYPE_PROPERTY_NAME);
		return nullptr;
	}

	// initialize the mod
	std::unique_ptr<Mod> newMod = std::make_unique<Mod>(modID, modName, modType);

	// parse the mod alias property
	if(modValue.HasMember(JSON_MOD_ALIAS_PROPERTY_NAME)) {
		const rapidjson::Value & modAliasValue = modValue[JSON_MOD_ALIAS_PROPERTY_NAME];

		if(!modAliasValue.IsString()) {
			spdlog::error("Mod '{}' '{}' property has invalid type: '{}', expected 'string'.", modID, JSON_MOD_ALIAS_PROPERTY_NAME, Utilities::typeToString(modAliasValue.GetType()));
			return nullptr;
		}

		newMod->setAlias(modAliasValue.GetString());
	}

	// parse the mod preferred version name property
	if(modValue.HasMember(JSON_MOD_PREFERRED_VERSION_NAME_PROPERTY_NAME)) {
		const rapidjson::Value & modPreferredVersionNameValue = modValue[JSON_MOD_PREFERRED_VERSION_NAME_PROPERTY_NAME];

		if(!modPreferredVersionNameValue.IsString()) {
			spdlog::error("Mod '{}' '{}' property has invalid type: '{}', expected 'string'.", modID, JSON_MOD_PREFERRED_VERSION_NAME_PROPERTY_NAME, Utilities::typeToString(modPreferredVersionNameValue.GetType()));
			return nullptr;
		}

		newMod->setPreferredVersionName(modPreferredVersionNameValue.GetString());
	}

	// parse the mod default version type property
	if(modValue.HasMember(JSON_MOD_DEFAULT_VERSION_TYPE_PROPERTY_NAME)) {
		const rapidjson::Value & modDefaultVersionTypeValue = modValue[JSON_MOD_DEFAULT_VERSION_TYPE_PROPERTY_NAME];

		if(!modDefaultVersionTypeValue.IsString()) {
			spdlog::error("Mod '{}' '{}' property has invalid type: '{}', expected 'string'.", modID, JSON_MOD_DEFAULT_VERSION_TYPE_PROPERTY_NAME, Utilities::typeToString(modDefaultVersionTypeValue.GetType()));
			return nullptr;
		}

		newMod->setDefaultVersionType(modDefaultVersionTypeValue.GetString());
	}

	// parse the mod website property
	if(modValue.HasMember(JSON_MOD_WEBSITE_PROPERTY_NAME)) {
		const rapidjson::Value & modWebsiteValue = modValue[JSON_MOD_WEBSITE_PROPERTY_NAME];

		if(!modWebsiteValue.IsString()) {
			spdlog::error("Mod '{}' '{}' property has invalid type: '{}', expected 'string'.", modID, JSON_MOD_WEBSITE_PROPERTY_NAME, Utilities::typeToString(modWebsiteValue.GetType()));
			return nullptr;
		}

		newMod->setWebsite(modWebsiteValue.GetString());
	}

	// parse the mod repository URL property
	if(modValue.HasMember(JSON_MOD_REPOSITORY_URL_PROPERTY_NAME)) {
		const rapidjson::Value & modRepositoryURLValue = modValue[JSON_MOD_REPOSITORY_URL_PROPERTY_NAME];

		if(!modRepositoryURLValue.IsString()) {
			spdlog::error("Mod '{}' '{}' property has invalid type: '{}', expected 'string'.", modID, JSON_MOD_REPOSITORY_URL_PROPERTY_NAME, Utilities::typeToString(modRepositoryURLValue.GetType()));
			return nullptr;
		}

		newMod->setRepositoryURL(modRepositoryURLValue.GetString());
	}

	// parse the mod team property
	if(modValue.HasMember(JSON_MOD_TEAM_PROPERTY_NAME)) {
		const rapidjson::Value & modTeamValue = modValue[JSON_MOD_TEAM_PROPERTY_NAME];

		if(!modTeamValue.IsObject()) {
			spdlog::error("Mod '{}' '{}' property has invalid type: '{}', expected 'object'.", modID, JSON_MOD_TEAM_PROPERTY_NAME, Utilities::typeToString(modTeamValue.GetType()));
			return nullptr;
		}

		std::unique_ptr<ModTeam> newModTeam = std::move(ModTeam::parseFrom(modTeamValue));

		if(!ModTeam::isValid(newModTeam.get())) {
			spdlog::error("Failed to parse mod team for mod with ID '{}'.", modID);
			return nullptr;
		}

		newMod->m_team = std::shared_ptr<ModTeam>(newModTeam.release());
		newMod->m_team->setParentMod(newMod.get());
	}

	// parse the mod versions property
	if(!modValue.HasMember(JSON_MOD_VERSIONS_PROPERTY_NAME)) {
		spdlog::error("Mod '{}' is missing or has an invalid '{}' property type: '{}', expected 'string'.", modID, JSON_MOD_VERSIONS_PROPERTY_NAME, Utilities::typeToString(modValue[JSON_MOD_VERSIONS_PROPERTY_NAME].GetType()));
		return nullptr;
	}

	const rapidjson::Value & modVersionsValue = modValue[JSON_MOD_VERSIONS_PROPERTY_NAME];

	if(!modVersionsValue.IsArray()) {
		spdlog::error("Mod '{}' '{}' property has invalid type: '{}', expected 'array'.", modID, JSON_MOD_VERSIONS_PROPERTY_NAME, Utilities::typeToString(modVersionsValue.GetType()));
		return nullptr;
	}

	if(modVersionsValue.Empty()) {
		spdlog::error("Mod '{}' '{}' property cannot be empty.", modID, JSON_MOD_VERSIONS_PROPERTY_NAME);
		return nullptr;
	}

	std::shared_ptr<ModVersion> newModVersion;

	for(rapidjson::Value::ConstValueIterator i = modVersionsValue.Begin(); i != modVersionsValue.End(); ++i) {
		newModVersion = std::shared_ptr<ModVersion>(ModVersion::parseFrom(*i, modValue, skipFileInfoValidation).release());

		if(!ModVersion::isValid(newModVersion.get(), skipFileInfoValidation)) {
			spdlog::error("Failed to parse mod version #{} for mod with ID '{}'.", newMod->m_versions.size() + 1, modID);
			return nullptr;
		}

		newModVersion->setParentMod(newMod.get());

		if(newMod->hasVersion(*newModVersion)) {
			spdlog::error("Encountered duplicate mod version #{} for mod with ID '{}'.", newMod->m_versions.size() + 1, modID);
			return nullptr;
		}

		newMod->m_versions.push_back(newModVersion);
	}

	// parse the mod downloads property
	if(!modValue.HasMember(JSON_MOD_DOWNLOADS_PROPERTY_NAME)) {
		spdlog::error("Mod '{}' is missing or has an invalid '{}' property type: '{}', expected 'string'.", modID, JSON_MOD_DOWNLOADS_PROPERTY_NAME, Utilities::typeToString(modValue[JSON_MOD_DOWNLOADS_PROPERTY_NAME].GetType()));
		return nullptr;
	}

	const rapidjson::Value & modDownloadsValue = modValue[JSON_MOD_DOWNLOADS_PROPERTY_NAME];

	if(!modDownloadsValue.IsArray()) {
		spdlog::error("Mod '{}' '{}' property has invalid type: '{}', expected 'array'.", modID, JSON_MOD_DOWNLOADS_PROPERTY_NAME, Utilities::typeToString(modDownloadsValue.GetType()));
		return nullptr;
	}

	if(modDownloadsValue.Empty()) {
		spdlog::error("Mod '{}' '{}' property cannot be empty.", modID, JSON_MOD_DOWNLOADS_PROPERTY_NAME);
		return nullptr;
	}

	std::shared_ptr<ModDownload> newModDownload;

	for(rapidjson::Value::ConstValueIterator i = modDownloadsValue.Begin(); i != modDownloadsValue.End(); ++i) {
		newModDownload = std::shared_ptr<ModDownload>(ModDownload::parseFrom(*i, skipFileInfoValidation).release());

		if(!ModDownload::isValid(newModDownload.get(), skipFileInfoValidation)) {
			spdlog::error("Failed to parse mod download #{} for mod with ID '{}'.", newMod->m_downloads.size() + 1, modID);
			return nullptr;
		}

		newModDownload->setParentMod(newMod.get());

		if(newMod->hasDownload(*newModDownload)) {
			spdlog::error("Encountered duplicate mod download #{} for mod with ID '{}'.", newMod->m_downloads.size() + 1, modID);
			return nullptr;
		}

		newMod->m_downloads.push_back(newModDownload);
	}

	// parse the mod screenshots property
	if(modValue.HasMember(JSON_MOD_SCREENSHOTS_PROPERTY_NAME)) {
		const rapidjson::Value & modScreenshotsValue = modValue[JSON_MOD_SCREENSHOTS_PROPERTY_NAME];

		if(!modScreenshotsValue.IsArray()) {
			spdlog::error("Mod '{}' '{}' property has invalid type: '{}', expected 'array'.", modID, JSON_MOD_SCREENSHOTS_PROPERTY_NAME, Utilities::typeToString(modScreenshotsValue.GetType()));
			return nullptr;
		}

		std::shared_ptr<ModScreenshot> newModScreenshot;

		for(rapidjson::Value::ConstValueIterator i = modScreenshotsValue.Begin(); i != modScreenshotsValue.End(); ++i) {
			newModScreenshot = std::shared_ptr<ModScreenshot>(ModScreenshot::parseFrom(*i, skipFileInfoValidation).release());

			if(!ModScreenshot::isValid(newModScreenshot.get(), skipFileInfoValidation)) {
				spdlog::error("Failed to parse mod screenshot #{} for mod with ID '{}'.", newMod->m_screenshots.size() + 1, modID);
				return nullptr;
			}

			newModScreenshot->setParentMod(newMod.get());

			if(newMod->hasScreenshot(*newModScreenshot)) {
				spdlog::error("Encountered duplicate mod screenshot #{} for mod with ID '{}'.", newMod->m_screenshots.size() + 1, modID);
				return nullptr;
			}

			newMod->m_screenshots.push_back(newModScreenshot);
		}
	}

	// parse the mod images property
	if(modValue.HasMember(JSON_MOD_IMAGES_PROPERTY_NAME)) {
		const rapidjson::Value & modImagesValue = modValue[JSON_MOD_IMAGES_PROPERTY_NAME];

		if(!modImagesValue.IsArray()) {
			spdlog::error("Mod '{}' '{}' property has invalid type: '{}', expected 'array'.", modID, JSON_MOD_IMAGES_PROPERTY_NAME, Utilities::typeToString(modImagesValue.GetType()));
			return nullptr;
		}

		std::shared_ptr<ModImage> newModImage;

		for(rapidjson::Value::ConstValueIterator i = modImagesValue.Begin(); i != modImagesValue.End(); ++i) {
			newModImage = std::shared_ptr<ModImage>(ModImage::parseFrom(*i, skipFileInfoValidation).release());

			if(!ModImage::isValid(newModImage.get(), skipFileInfoValidation)) {
				spdlog::error("Failed to parse mod image #{} for mod with ID '{}'.", newMod->m_images.size() + 1, modID);
				return nullptr;
			}

			newModImage->setParentMod(newMod.get());

			if(newMod->hasImage(*newModImage)) {
				spdlog::error("Encountered duplicate mod image #{} for mod with ID '{}'.", newMod->m_images.size() + 1, modID);
				return nullptr;
			}

			newMod->m_images.push_back(newModImage);
		}
	}

	// parse the mod videos property
	if(modValue.HasMember(JSON_MOD_VIDEOS_PROPERTY_NAME)) {
		const rapidjson::Value & modVideosValue = modValue[JSON_MOD_VIDEOS_PROPERTY_NAME];

		if(!modVideosValue.IsArray()) {
			spdlog::error("Mod '{}' '{}' property has invalid type: '{}', expected 'array'.", modID, JSON_MOD_VIDEOS_PROPERTY_NAME, Utilities::typeToString(modVideosValue.GetType()));
			return nullptr;
		}

		std::shared_ptr<ModVideo> newModVideo;

		for(rapidjson::Value::ConstValueIterator i = modVideosValue.Begin(); i != modVideosValue.End(); ++i) {
			newModVideo = std::shared_ptr<ModVideo>(std::move(ModVideo::parseFrom(*i)).release());

			if(!ModVideo::isValid(newModVideo.get())) {
				spdlog::error("Failed to parse mod video #{} for mod with ID '{}'.", newMod->m_videos.size() + 1, modID);
				return nullptr;
			}

			newModVideo->setParentMod(newMod.get());

			if(newMod->hasVideo(*newModVideo)) {
				spdlog::error("Encountered duplicate mod video #{} for mod with ID '{}'.", newMod->m_videos.size() + 1, modID);
				return nullptr;
			}

			newMod->m_videos.push_back(newModVideo);
		}
	}

	// parse the mod notes property
	if(modValue.HasMember(JSON_MOD_NOTES_PROPERTY_NAME)) {
		const rapidjson::Value & modNotesValue = modValue[JSON_MOD_NOTES_PROPERTY_NAME];

		if(!modNotesValue.IsArray()) {
			spdlog::error("Mod '{}' '{}' property has invalid type: '{}', expected 'array'.", modID, JSON_MOD_NOTES_PROPERTY_NAME, Utilities::typeToString(modNotesValue.GetType()));
			return nullptr;
		}

		for(rapidjson::Value::ConstValueIterator i = modNotesValue.Begin(); i != modNotesValue.End(); ++i) {
			std::string note(Utilities::trimString((*i).GetString()));

			if(note.empty()) {
				spdlog::error("Encountered empty note #{} for mod with ID '{}'.", newMod->m_notes.size() + 1, modID);
				return nullptr;
			}

			if(newMod->hasNote(note)) {
				spdlog::error("Encountered duplicate mod note #{} for mod with ID '{}'.", newMod->m_notes.size() + 1, modID);
				return nullptr;
			}

			newMod->m_notes.emplace_back(note);
		}
	}

	// parse the mod related mods property
	if(modValue.HasMember(JSON_MOD_RELATED_MODS_PROPERTY_NAME)) {
		const rapidjson::Value & modRelatedModsValue = modValue[JSON_MOD_RELATED_MODS_PROPERTY_NAME];

		if(!modRelatedModsValue.IsArray()) {
			spdlog::error("Mod '{}' '{}' property has invalid type: '{}', expected 'array'.", modID, JSON_MOD_RELATED_MODS_PROPERTY_NAME, Utilities::typeToString(modRelatedModsValue.GetType()));
			return nullptr;
		}

		for(rapidjson::Value::ConstValueIterator i = modRelatedModsValue.Begin(); i != modRelatedModsValue.End(); ++i) {
			std::string relatedMod(Utilities::trimString((*i).GetString()));

			if(relatedMod.empty()) {
				spdlog::error("Encountered empty related mod #{} for mod with ID '{}'.", newMod->m_relatedMods.size() + 1, modID);
				return nullptr;
			}

			if(newMod->hasRelatedMod(relatedMod)) {
				spdlog::error("Encountered duplicate mod related mod #{} for mod with ID '{}'.", newMod->m_relatedMods.size() + 1, modID);
				return nullptr;
			}

			newMod->m_relatedMods.emplace_back(relatedMod);
		}
	}

	// parse the mod similar mods property
	if(modValue.HasMember(JSON_MOD_SIMILAR_MODS_PROPERTY_NAME)) {
		const rapidjson::Value & modSimilarModsValue = modValue[JSON_MOD_SIMILAR_MODS_PROPERTY_NAME];

		if(!modSimilarModsValue.IsArray()) {
			spdlog::error("Mod '{}' '{}' property has invalid type: '{}', expected 'array'.", modID, JSON_MOD_SIMILAR_MODS_PROPERTY_NAME, Utilities::typeToString(modSimilarModsValue.GetType()));
			return nullptr;
		}

		for(rapidjson::Value::ConstValueIterator i = modSimilarModsValue.Begin(); i != modSimilarModsValue.End(); ++i) {
			std::string similarMod(Utilities::trimString((*i).GetString()));

			if(similarMod.empty()) {
				spdlog::error("Encountered empty similar mod #{} for mod with ID '{}'.", newMod->m_similarMods.size() + 1, modID);
				return nullptr;
			}

			if(newMod->hasSimilarMod(similarMod)) {
				spdlog::error("Encountered duplicate mod similar mod #{} for mod with ID '{}'.", newMod->m_similarMods.size() + 1, modID);
				return nullptr;
			}

			newMod->m_similarMods.emplace_back(similarMod);
		}
	}

	return newMod;
}

std::unique_ptr<Mod> Mod::parseFrom(const tinyxml2::XMLElement * modElement, bool skipFileInfoValidation) {
	if(modElement == nullptr) {
		return nullptr;
	}

	// verify element name
	if(modElement->Name() != XML_MOD_ELEMENT_NAME) {
		spdlog::error("Invalid mod element name: '{}', expected '{}'.", modElement->Name(), XML_MOD_ELEMENT_NAME);
		return nullptr;
	}

	// check for unhandled mod element attributes
	bool attributeHandled = false;
	const tinyxml2::XMLAttribute * modAttribute = modElement->FirstAttribute();

	while(true) {
		if(modAttribute == nullptr) {
			break;
		}

		attributeHandled = false;

		for(const std::string_view & attributeName : XML_MOD_ATTRIBUTE_NAMES) {
			if(modAttribute->Name() == attributeName) {
				attributeHandled = true;
				break;
			}
		}

		if(!attributeHandled) {
			spdlog::warn("Element '{}' has unexpected attribute '{}'.", XML_MOD_ELEMENT_NAME, modAttribute->Name());
		}

		modAttribute = modAttribute->Next();
	}

	// check for unhandled mod element child elements
	bool elementHandled = false;
	const tinyxml2::XMLElement * modChildElement = modElement->FirstChildElement();

	while(true) {
		if(modChildElement == nullptr) {
			break;
		}

		elementHandled = false;

		for(const std::string_view & elementName : XML_MOD_CHILD_ELEMENT_NAMES) {
			if(modChildElement->Name() == elementName) {
				elementHandled = true;
				break;
			}
		}

		if(!elementHandled) {
			spdlog::warn("Element '{}' has unexpected child element '{}'.", XML_MOD_ELEMENT_NAME, modChildElement->Name());
		}

		modChildElement = modChildElement->NextSiblingElement();
	}

	// read the mod id attribute value
	const char * modID = modElement->Attribute(XML_MOD_ID_ATTRIBUTE_NAME.c_str());

	if(Utilities::stringLength(modID) == 0) {
		spdlog::error("Attribute '{}' is missing from '{}' element.", XML_MOD_ID_ATTRIBUTE_NAME, XML_MOD_ELEMENT_NAME);
		return nullptr;
	}

	// read the mod name attribute value
	const char * modName = modElement->Attribute(XML_MOD_NAME_ATTRIBUTE_NAME.c_str());

	if(Utilities::stringLength(modName) == 0) {
		spdlog::error("Attribute '{}' is missing from '{}' element.", XML_MOD_NAME_ATTRIBUTE_NAME, XML_MOD_ELEMENT_NAME);
		return nullptr;
	}

	// read the mod type attribute value
	const char * modType = modElement->Attribute(XML_MOD_TYPE_ATTRIBUTE_NAME.c_str());

	if(Utilities::stringLength(modType) == 0) {
		spdlog::error("Attribute '{}' is missing from '{}' element with ID '{}'.", XML_MOD_TYPE_ATTRIBUTE_NAME, XML_MOD_ELEMENT_NAME, modID);
		return nullptr;
	}

	// read the mod alias attribute value
	const char * modAlias = modElement->Attribute(XML_MOD_ALIAS_ATTRIBUTE_NAME.c_str());

	// read the mod version attribute value
	const char * modVersion = modElement->Attribute(XML_MOD_VERSION_ATTRIBUTE_NAME.c_str());

	// read the mod version type attribute value
	const char * modVersionType = modElement->Attribute(XML_MOD_VERSION_TYPE_ATTRIBUTE_NAME.c_str());

	// read the mod website attribute value
	const char * modWebsite = modElement->Attribute(XML_MOD_WEBSITE_ATTRIBUTE_NAME.c_str());

	// read the mod repository URL attribute value
	const char * modRepositoryURL = modElement->Attribute(XML_MOD_REPOSITORY_ATTRIBUTE_NAME.c_str());

	// initialize the mod
	std::unique_ptr<Mod> mod = std::make_unique<Mod>(modID, modName, modType);

	if(modAlias != nullptr) {
		mod->setAlias(modAlias);
	}

	if(modVersion != nullptr) {
		mod->setPreferredVersionName(modVersion);
	}

	if(modVersionType != nullptr) {
		mod->setDefaultVersionType(modVersionType);
	}

	if(modWebsite != nullptr) {
		mod->setWebsite(modWebsite);
	}

	if(modRepositoryURL != nullptr) {
		mod->setRepositoryURL(modRepositoryURL);
	}

	// check for the mod team element
	const tinyxml2::XMLElement * modTeamElement = modElement->FirstChildElement(XML_MOD_TEAM_ELEMENT_NAME.c_str());

	if(modTeamElement != nullptr) {
		if(modTeamElement->NextSiblingElement(XML_MOD_TEAM_ELEMENT_NAME.c_str())) {
			spdlog::error("Encountered more than one '{}' child element of '{}' element with ID '{}'.", XML_MOD_TEAM_ELEMENT_NAME, XML_MOD_ELEMENT_NAME, modID);
			return nullptr;
		}

		std::unique_ptr<ModTeam> newModTeam(std::move(ModTeam::parseFrom(modTeamElement)));

		if(!ModTeam::isValid(newModTeam.get())) {
			spdlog::error("Failed to parse mod team for '{}' element with ID '{}'.", XML_MOD_ELEMENT_NAME, modID);
			return nullptr;
		}

		mod->m_team = std::shared_ptr<ModTeam>(newModTeam.release());
		mod->m_team->setParentMod(mod.get());
	}

	// get mod files element
	const tinyxml2::XMLElement * modFilesElement = modElement->FirstChildElement(XML_VERSIONS_ELEMENT_NAME.c_str());

	if(modFilesElement == nullptr) {
		spdlog::error("Element '{}' is missing from '{}' element with ID '{}'.", XML_VERSIONS_ELEMENT_NAME, XML_MOD_ELEMENT_NAME, modID);
		return nullptr;
	}

	if(modFilesElement->NextSiblingElement(XML_VERSIONS_ELEMENT_NAME.c_str())) {
		spdlog::error("Encountered more than one '{}' child element of '{}' element with ID '{}'.", XML_VERSIONS_ELEMENT_NAME, XML_MOD_ELEMENT_NAME, modID);
		return nullptr;
	}

	// iterate over all of the mod version elements
	const tinyxml2::XMLElement * modVersionElement = modFilesElement->FirstChildElement();

	if(modVersionElement == nullptr) {
		spdlog::error("Element '{}' has no children in element '{}' with ID '{}'.", XML_VERSIONS_ELEMENT_NAME, XML_MOD_ELEMENT_NAME, modID);
		return nullptr;
	}

	std::shared_ptr<ModVersion> newModVersion;

	while(true) {
		if(modVersionElement == nullptr) {
			break;
		}

		newModVersion = std::shared_ptr<ModVersion>(std::move(ModVersion::parseFrom(modVersionElement, skipFileInfoValidation)).release());

		if(!ModVersion::isValid(newModVersion.get(), skipFileInfoValidation)) {
			spdlog::error("Failed to parse mod version #{} for '{}' element with ID '{}'.", mod->m_versions.size() + 1, XML_MOD_ELEMENT_NAME, modID);
			return nullptr;
		}

		newModVersion->setParentMod(mod.get());

		if(mod->hasVersion(*newModVersion)) {
			spdlog::error("Encountered duplicate mod version #{} for '{}' element with ID '{}'.", mod->m_versions.size() + 1, XML_MOD_ELEMENT_NAME, modID);
			return nullptr;
		}

		mod->m_versions.push_back(newModVersion);

		modVersionElement = modVersionElement->NextSiblingElement();
	}

	// iterate over all of the mod download elements
	const tinyxml2::XMLElement * modDownloadsElement = modElement->FirstChildElement(XML_DOWNLOADS_ELEMENT_NAME.c_str());

	if(modDownloadsElement == nullptr) {
		spdlog::error("Element '{}' with id '{}' is missing child element '{}'.", XML_MOD_ELEMENT_NAME, modID, XML_DOWNLOADS_ELEMENT_NAME);
		return nullptr;
	}

	if(modDownloadsElement->NextSiblingElement(XML_DOWNLOADS_ELEMENT_NAME.c_str())) {
		spdlog::error("Encountered more than one '{}' child element of '{}' element with ID '{}'.", XML_DOWNLOADS_ELEMENT_NAME, XML_MOD_ELEMENT_NAME, modID);
		return nullptr;
	}

	const tinyxml2::XMLElement * modDownloadElement = modDownloadsElement->FirstChildElement();

	std::shared_ptr<ModDownload> newModDownload;

	while(true) {
		if(modDownloadElement == nullptr) {
			break;
		}

		newModDownload = std::shared_ptr<ModDownload>(ModDownload::parseFrom(modDownloadElement, skipFileInfoValidation).release());

		if(!ModDownload::isValid(newModDownload.get(), skipFileInfoValidation)) {
			spdlog::error("Failed to parse mod download #{} for '{}' element with ID '{}'.", mod->m_downloads.size() + 1, XML_MOD_ELEMENT_NAME, modID);
			return nullptr;
		}

		newModDownload->setParentMod(mod.get());

		if(mod->hasDownload(*newModDownload)) {
			spdlog::error("Encountered duplicate mod download #{} for '{}' element with ID '{}'.", mod->m_downloads.size() + 1, XML_MOD_ELEMENT_NAME, modID);
			return nullptr;
		}

		mod->m_downloads.push_back(newModDownload);

		modDownloadElement = modDownloadElement->NextSiblingElement();
	}

	// iterate over all of the mod screenshot elements
	const tinyxml2::XMLElement * modScreenshotsElement = modElement->FirstChildElement(XML_SCREENSHOTS_ELEMENT_NAME.c_str());

	if(modScreenshotsElement != nullptr) {
		if(modScreenshotsElement->NextSiblingElement(XML_SCREENSHOTS_ELEMENT_NAME.c_str())) {
			spdlog::error("Encountered more than one '{}' child element of '{}' element with ID '{}'.", XML_SCREENSHOTS_ELEMENT_NAME, XML_MOD_ELEMENT_NAME, modID);
			return nullptr;
		}

		const tinyxml2::XMLElement * modScreenshotElement = modScreenshotsElement->FirstChildElement();

		std::shared_ptr<ModScreenshot> newModScreenshot;

		while(true) {
			if(modScreenshotElement == nullptr) {
				break;
			}

			newModScreenshot = std::shared_ptr<ModScreenshot>(ModScreenshot::parseFrom(modScreenshotElement, skipFileInfoValidation).release());

			if(!ModScreenshot::isValid(newModScreenshot.get(), skipFileInfoValidation)) {
				spdlog::error("Failed to parse mod screenshot #{} for '{}' element with ID '{}'.", mod->m_screenshots.size() + 1, XML_MOD_ELEMENT_NAME, modID);
				return nullptr;
			}

			newModScreenshot->setParentMod(mod.get());

			if(mod->hasScreenshot(*newModScreenshot)) {
				spdlog::error("Encountered duplicate mod download #{} for '{}' element with ID '{}'.", mod->m_screenshots.size() + 1, XML_MOD_ELEMENT_NAME, modID);
				return nullptr;
			}

			mod->m_screenshots.push_back(newModScreenshot);

			modScreenshotElement = modScreenshotElement->NextSiblingElement();
		}
	}

	// iterate over all of the mod image elements
	const tinyxml2::XMLElement * modImagesElement = modElement->FirstChildElement(XML_IMAGES_ELEMENT_NAME.c_str());

	if(modImagesElement != nullptr) {
		if(modImagesElement->NextSiblingElement(XML_IMAGES_ELEMENT_NAME.c_str())) {
			spdlog::error("Encountered more than one '{}' child element of '{}' element with ID '{}'.", XML_IMAGES_ELEMENT_NAME, XML_MOD_ELEMENT_NAME, modID);
			return nullptr;
		}

		const tinyxml2::XMLElement * modImageElement = modImagesElement->FirstChildElement();

		std::shared_ptr<ModImage> newModImage;

		while(true) {
			if(modImageElement == nullptr) {
				break;
			}

			newModImage = std::shared_ptr<ModImage>(ModImage::parseFrom(modImageElement, skipFileInfoValidation).release());

			if(!ModImage::isValid(newModImage.get(), skipFileInfoValidation)) {
				spdlog::error("Failed to parse mod image #{} for '{}' element with ID '{}'.", mod->m_images.size() + 1, XML_MOD_ELEMENT_NAME, modID);
				return nullptr;
			}

			newModImage->setParentMod(mod.get());

			if(mod->hasImage(*newModImage)) {
				spdlog::error("Encountered duplicate mod download #{} for '{}' element with ID '{}'.", mod->m_images.size() + 1, XML_MOD_ELEMENT_NAME, modID);
				return nullptr;
			}

			mod->m_images.push_back(newModImage);

			modImageElement = modImageElement->NextSiblingElement();
		}
	}

	// iterate over all of the mod video elements
	const tinyxml2::XMLElement * modVideosElement = modElement->FirstChildElement(XML_VIDEOS_ELEMENT_NAME.c_str());

	if(modVideosElement != nullptr) {
		if(modVideosElement->NextSiblingElement(XML_VIDEOS_ELEMENT_NAME.c_str())) {
			spdlog::error("Encountered more than one '{}' child element of '{}' element with ID '{}'.", XML_VIDEOS_ELEMENT_NAME, XML_MOD_ELEMENT_NAME, modID);
			return nullptr;
		}

		const tinyxml2::XMLElement * modVideoElement = modVideosElement->FirstChildElement();

		std::shared_ptr<ModVideo> newModVideo;

		while(true) {
			if(modVideoElement == nullptr) {
				break;
			}

			newModVideo = std::shared_ptr<ModVideo>(std::move(ModVideo::parseFrom(modVideoElement)).release());

			if(!ModVideo::isValid(newModVideo.get())) {
				spdlog::error("Failed to parse mod video #{} for '{}' element with ID '{}'.", mod->m_videos.size() + 1, XML_MOD_ELEMENT_NAME, modID);
				return nullptr;
			}

			newModVideo->setParentMod(mod.get());

			if(mod->hasVideo(*newModVideo)) {
				spdlog::error("Encountered duplicate mod download #{} for '{}' element with ID '{}'.", mod->m_videos.size() + 1, XML_MOD_ELEMENT_NAME, modID);
				return nullptr;
			}

			mod->m_videos.push_back(newModVideo);

			modVideoElement = modVideoElement->NextSiblingElement();
		}
	}

	// iterate over all of the note elements
	const tinyxml2::XMLElement * modNotesElement = modElement->FirstChildElement(XML_NOTES_ELEMENT_NAME.c_str());

	if(modNotesElement != nullptr) {
		if(modNotesElement->NextSiblingElement(XML_NOTES_ELEMENT_NAME.c_str())) {
			spdlog::error("Encountered more than one '{}' child element of '{}' element with ID '{}'.", XML_NOTES_ELEMENT_NAME, XML_MOD_ELEMENT_NAME, modID);
			return nullptr;
		}

		const tinyxml2::XMLElement * modNoteElement = modNotesElement->FirstChildElement();

		while(true) {
			if(modNoteElement == nullptr) {
				break;
			}

			if(modNoteElement->Name() != XML_NOTE_ELEMENT_NAME) {
				spdlog::error("Encountered '{}' element child with name '{}', expected '{}' in '{}' element with ID '{}'.", XML_NOTES_ELEMENT_NAME, modNoteElement->Name(), XML_NOTE_ELEMENT_NAME, XML_MOD_ELEMENT_NAME, modID);
				return nullptr;
			}

			const char * note = modNoteElement->GetText();

			if(note == nullptr || Utilities::stringLength(note) == 0) {
				spdlog::error("Encountered '{}' element child with no text content in '{}' element with ID '{}'.", XML_NOTES_ELEMENT_NAME, XML_MOD_ELEMENT_NAME, modID);
				return nullptr;
			}

			mod->m_notes.push_back(note);

			modNoteElement = modNoteElement->NextSiblingElement();
		}
	}

	// iterate over all of the related mod elements
	const tinyxml2::XMLElement * modRelatedElement = modElement->FirstChildElement(XML_RELATED_ELEMENT_NAME.c_str());

	if(modRelatedElement != nullptr) {
		if(modRelatedElement->NextSiblingElement(XML_RELATED_ELEMENT_NAME.c_str())) {
			spdlog::error("Encountered more than one '{}' child element of '{}' element with ID '{}'.", XML_RELATED_ELEMENT_NAME, XML_MOD_ELEMENT_NAME, modID);
			return nullptr;
		}

		const tinyxml2::XMLElement * modRelatedModElement = modRelatedElement->FirstChildElement();

		while(true) {
			if(modRelatedModElement == nullptr) {
				break;
			}

			if(modRelatedModElement->Name() != XML_RELATED_MOD_ELEMENT_NAME) {
				spdlog::error("Encountered '{}' element child with name '{}', expected '{}' in '{}' element with ID '{}'.", XML_RELATED_ELEMENT_NAME, modRelatedModElement->Name(), XML_RELATED_MOD_ELEMENT_NAME, XML_MOD_ELEMENT_NAME, modID);
				return nullptr;
			}

			const char * relatedModID = modRelatedModElement->Attribute(XML_RELATED_MOD_ID_ATTRIBUTE_NAME.c_str());

			if(relatedModID == nullptr || Utilities::stringLength(relatedModID) == 0) {
				spdlog::error("Encountered '{}' element child with missing '{}' attribute in '{}' element with ID '{}'.", XML_RELATED_ELEMENT_NAME, XML_RELATED_MOD_ID_ATTRIBUTE_NAME, XML_MOD_ELEMENT_NAME, modID);
				return nullptr;
			}

			mod->m_relatedMods.push_back(relatedModID);

			modRelatedModElement = modRelatedModElement->NextSiblingElement();
		}
	}

	// iterate over all of the similar mod elements
	const tinyxml2::XMLElement * modSimilarElement = modElement->FirstChildElement(XML_SIMILAR_ELEMENT_NAME.c_str());

	if(modSimilarElement != nullptr) {
		if(modSimilarElement->NextSiblingElement(XML_SIMILAR_ELEMENT_NAME.c_str())) {
			spdlog::error("Encountered more than one '{}' child element of '{}' element with ID '{}'.", XML_SIMILAR_ELEMENT_NAME, XML_MOD_ELEMENT_NAME, modID);
			return nullptr;
		}

		const tinyxml2::XMLElement * modSimilarModElement = modSimilarElement->FirstChildElement();

		while(true) {
			if(modSimilarModElement == nullptr) {
				break;
			}

			if(modSimilarModElement->Name() != XML_SIMILAR_MOD_ELEMENT_NAME) {
				spdlog::error("Encountered '{}' element child with name '{}', expected '{}' in '{}' element with ID '{}'.", XML_SIMILAR_ELEMENT_NAME, modSimilarModElement->Name(), XML_SIMILAR_MOD_ELEMENT_NAME, XML_MOD_ELEMENT_NAME, modID);
				return nullptr;
			}

			const char * similarModID = modSimilarModElement->Attribute(XML_SIMILAR_MOD_ID_ATTRIBUTE_NAME.c_str());

			if(similarModID == nullptr || Utilities::stringLength(similarModID) == 0) {
				spdlog::error("Encountered '{}' element child with missing '{}' attribute in '{}' element with ID '{}'.", XML_SIMILAR_ELEMENT_NAME, XML_SIMILAR_MOD_ID_ATTRIBUTE_NAME, XML_MOD_ELEMENT_NAME, modID);
				return nullptr;
			}

			mod->m_similarMods.push_back(similarModID);

			modSimilarModElement = modSimilarModElement->NextSiblingElement();
		}
	}

	return mod;
}

bool Mod::isGameVersionSupported(const GameVersion & gameVersion) const {
	if(!gameVersion.isValid()) {
		return false;
	}

	for(size_t i = 0; i < m_versions.size(); i++) {
		if(m_versions[i]->isGameVersionSupported(gameVersion)) {
			return true;
		}
	}

	return false;
}

bool Mod::isGameVersionCompatible(const GameVersion & gameVersion) const {
	if(!gameVersion.isValid()) {
		return false;
	}

	for(size_t i = 0; i < m_versions.size(); i++) {
		if(m_versions[i]->isGameVersionCompatible(gameVersion)) {
			return true;
		}
	}

	return false;
}

std::vector<std::shared_ptr<GameVersion>> Mod::getSupportedGameVersions(const GameVersionCollection & gameVersions) const {
	return getSupportedGameVersions(gameVersions.getGameVersions());
}

std::vector<std::shared_ptr<GameVersion>> Mod::getSupportedGameVersions(const std::vector<std::shared_ptr<GameVersion>> & gameVersions) const {
	std::vector<std::shared_ptr<GameVersion>> supportedGameVersions;

	for(const std::shared_ptr<GameVersion> & gameVersion : gameVersions) {
		if(isGameVersionSupported(*gameVersion)) {
			supportedGameVersions.push_back(gameVersion);
		}
	}

	return supportedGameVersions;
}

std::vector<std::string> Mod::getSupportedGameVersionLongNames(const GameVersionCollection & gameVersions) const {
	return getSupportedGameVersionLongNames(gameVersions.getGameVersions());
}

std::vector<std::string> Mod::getSupportedGameVersionLongNames(const std::vector<std::shared_ptr<GameVersion>> & gameVersions) const {
	std::vector<std::string> supportedGameVersionLongNames;

	for(const std::shared_ptr<GameVersion> & gameVersion : gameVersions) {
		if(isGameVersionSupported(*gameVersion)) {
			supportedGameVersionLongNames.push_back(gameVersion->getLongName());
		}
	}

	return supportedGameVersionLongNames;
}

std::vector<std::string> Mod::getSupportedGameVersionShortNames(const GameVersionCollection & gameVersions) const {
	return getSupportedGameVersionShortNames(gameVersions.getGameVersions());
}

std::vector<std::string> Mod::getSupportedGameVersionShortNames(const std::vector<std::shared_ptr<GameVersion>> & gameVersions) const {
	std::vector<std::string> supportedGameVersionShortNames;

	for(const std::shared_ptr<GameVersion> & gameVersion : gameVersions) {
		if(isGameVersionSupported(*gameVersion)) {
			supportedGameVersionShortNames.push_back(gameVersion->getShortName());
		}
	}

	return supportedGameVersionShortNames;
}

std::vector<std::shared_ptr<GameVersion>> Mod::getCompatibleGameVersions(const GameVersionCollection & gameVersions) const {
	return getCompatibleGameVersions(gameVersions.getGameVersions());
}

std::vector<std::shared_ptr<GameVersion>> Mod::getCompatibleGameVersions(const std::vector<std::shared_ptr<GameVersion>> & gameVersions) const {
	std::vector<std::shared_ptr<GameVersion>> compatibleGameVersions;

	for(const std::shared_ptr<GameVersion> & gameVersion : gameVersions) {
		if(isGameVersionCompatible(*gameVersion)) {
			compatibleGameVersions.push_back(gameVersion);
		}
	}

	return compatibleGameVersions;
}

std::vector<std::string> Mod::getCompatibleGameVersionLongNames(const GameVersionCollection & gameVersions) const {
	return getCompatibleGameVersionLongNames(gameVersions.getGameVersions());
}

std::vector<std::string> Mod::getCompatibleGameVersionLongNames(const std::vector<std::shared_ptr<GameVersion>> & gameVersions) const {
	std::vector<std::string> compatibleGameVersionLongNames;

	for(const std::shared_ptr<GameVersion> & gameVersion : gameVersions) {
		if(isGameVersionCompatible(*gameVersion)) {
			compatibleGameVersionLongNames.push_back(gameVersion->getLongName());
		}
	}

	return compatibleGameVersionLongNames;
}

std::vector<std::string> Mod::getCompatibleGameVersionShortNames(const GameVersionCollection & gameVersions) const {
	return getCompatibleGameVersionShortNames(gameVersions.getGameVersions());
}

std::vector<std::string> Mod::getCompatibleGameVersionShortNames(const std::vector<std::shared_ptr<GameVersion>> & gameVersions) const {
	std::vector<std::string> compatibleGameVersionShortNames;

	for(const std::shared_ptr<GameVersion> & gameVersion : gameVersions) {
		if(isGameVersionCompatible(*gameVersion)) {
			compatibleGameVersionShortNames.push_back(gameVersion->getShortName());
		}
	}

	return compatibleGameVersionShortNames;
}

bool Mod::checkVersions(bool verbose) const {
	if(!m_preferredVersion.empty()) {
		bool hasVersion = false;

		for(std::vector<std::shared_ptr<ModVersion>>::const_iterator i = m_versions.begin(); i != m_versions.end(); ++i) {
			if(Utilities::areStringsEqualIgnoreCase((*i)->getVersion(), m_preferredVersion)) {
				hasVersion = true;
			}
		}

		if(!hasVersion) {
			if(verbose) {
				spdlog::warn("Mod '{}' has no version: '{}'.", m_id, m_preferredVersion);
			}

			return false;
		}
	}

	bool hasOriginalDownload = false;
	bool hasModManagerDownload = false;
	bool hasRepairedDownload = false;

	for(std::vector<std::shared_ptr<ModVersion>>::const_iterator i = m_versions.begin(); i != m_versions.end(); ++i) {
		hasOriginalDownload = false;
		hasModManagerDownload = false;
		hasRepairedDownload = false;

		for(std::vector<std::shared_ptr<ModDownload>>::const_iterator j = m_downloads.begin(); j != m_downloads.end(); ++j) {
			if(Utilities::areStringsEqualIgnoreCase((*i)->getVersion(), (*j)->getVersion())) {
				if(Utilities::areStringsEqualIgnoreCase((*j)->getType(), ModDownload::ORIGINAL_FILES_TYPE)) {
					hasOriginalDownload = true;
				}
				else if(Utilities::areStringsEqualIgnoreCase((*j)->getType(), ModDownload::MOD_MANAGER_FILES_TYPE)) {
					hasModManagerDownload = true;

					if((*i)->getRepaired().has_value() && (*j)->getRepaired().has_value() && (*i)->isRepaired() == (*j)->isRepaired()) {
						hasRepairedDownload = true;
					}
				}
			}
		}

		if(!hasOriginalDownload || !hasModManagerDownload) {
			if(verbose) {
				spdlog::warn("Mod '{}' is missing download {}of type: '{}'.", m_id, (*i)->getVersion().empty() ? "" : fmt::format("for version: '{}' ", (*i)->getVersion()), !hasOriginalDownload ? ModDownload::ORIGINAL_FILES_TYPE : ModDownload::MOD_MANAGER_FILES_TYPE);
			}

			return false;
		}

		if((*i)->getRepaired().has_value() && !hasRepairedDownload) {
			if(verbose) {
				spdlog::warn("Mod '{}' is missing download {}of type: '{}', with repaired property set to: '{}' .", m_id, (*i)->getVersion().empty() ? "" : fmt::format("for version: '{}' ", (*i)->getVersion()), ModDownload::MOD_MANAGER_FILES_TYPE, (*i)->isRepaired());
			}

			return false;
		}
	}

	return true;
}

bool Mod::checkVersionTypes(bool verbose) const {
	if(m_defaultVersionType.empty()) {
		return true;
	}

	std::shared_ptr<ModVersion> modVersion;

	for(size_t i = 0; i < m_versions.size(); i++) {
		modVersion = m_versions[i];

		for(size_t j = 0; j < modVersion->numberOfTypes(); j++) {
			if(Utilities::areStringsEqualIgnoreCase(modVersion->getType(j)->getType(), m_defaultVersionType)) {
				return true;
			}
		}
	}

	if(verbose) {
		spdlog::warn("Mod '{}' has no version type: '{}'.", m_id, m_defaultVersionType);
	}

	return false;
}

bool Mod::checkGameVersionsHaveCorrespondingDownloads(bool verbose) const {
	std::shared_ptr<ModVersion> modVersion;
	std::shared_ptr<ModVersionType> modVersionType;
	std::shared_ptr<ModGameVersion> modGameVersion;
	std::shared_ptr<ModDownload> modDownload;
	bool hasDownload = false;

	for(size_t i = 0; i < m_versions.size(); i++) {
		modVersion = m_versions[i];

		for(size_t j = 0; j < modVersion->numberOfTypes(); j++) {
			modVersionType = modVersion->getType(j);

			for(size_t k = 0; k < modVersionType->numberOfGameVersions(); k++) {
				modGameVersion = modVersionType->getGameVersion(k);
				hasDownload = false;

				for(size_t l = 0; l < m_downloads.size(); l++) {
					modDownload = m_downloads[l];

					if(Utilities::areStringsEqualIgnoreCase(modDownload->getType(), ModDownload::MOD_MANAGER_FILES_TYPE) &&
					   Utilities::areStringsEqualIgnoreCase(modDownload->getGameVersionID(), modGameVersion->getGameVersionID()) &&
					   Utilities::areStringsEqualIgnoreCase(modDownload->getVersion(), modVersion->getVersion())) {
						hasDownload = true;
						break;
					}
				}

				if(!hasDownload) {
					if(verbose) {
						spdlog::warn("Mod '{}' is missing '{}' download for game version with ID '{}'.", getFullName(i, j), ModDownload::MOD_MANAGER_FILES_TYPE, modGameVersion->getGameVersionID());
					}

					return false;
				}
			}
		}
	}

	return true;
}

bool Mod::checkSplitDownloadsNotMissingParts(bool verbose) const {
	std::vector<bool> verifiedDownloads(m_downloads.size(), false);

	std::shared_ptr<ModDownload> a;
	std::shared_ptr<ModDownload> b;

	for(size_t i = 0; i < m_downloads.size(); i++) {
		a = m_downloads[i];

		if(verifiedDownloads[i]) {
			continue;
		}

		if(a->getPartCount() > 1) {
			std::vector<bool> parts(a->getPartCount(), false);

			if(a->getPartNumber() <= a->getPartCount()) {
				parts[a->getPartNumber() - 1] = true;
			}

			for(size_t j = i + 1; j < m_downloads.size(); j++) {
				b = m_downloads[j];

				if(a->getPartCount() == b->getPartCount() &&
				   Utilities::areStringsEqualIgnoreCase(a->getVersion(), b->getVersion())) {
					if(parts[b->getPartNumber() - 1]) {
						if(verbose) {
							spdlog::warn("Mod '{}' is has duplicate download {}part #{} of {}.", m_id, a->getVersion().empty() ? "" : fmt::format("version {} ", a->getVersion()), b->getPartNumber(), a->getPartCount());
						}

						return false;
					}

					verifiedDownloads[j] = true;
					parts[b->getPartNumber() - 1] = true;
				}
			}

			for(size_t j = 0; j < a->getPartCount(); j++) {
				if(!parts[j]) {
					if(verbose) {
						spdlog::warn("Mod '{}' is missing download {}part #{} of {}.", m_id, a->getVersion().empty() ? "" : fmt::format("version {} ", a->getVersion()), j + 1, a->getPartCount());
					}

					return false;
				}
			}
		}
		else {
			verifiedDownloads[i] = true;
		}
	}

	return true;
}

bool Mod::isValid(bool skipFileInfoValidation) const {
	if(m_id.empty() ||
	   m_name.empty() ||
	   m_type.empty() ||
	   m_versions.empty() ||
	   m_downloads.empty() ||
	   Utilities::areStringsEqualIgnoreCase(m_id, m_alias) ||
	   (m_team != nullptr && !m_team->isValid()) ||
	   !checkVersions() ||
	   !checkVersionTypes() ||
	   !checkGameVersionsHaveCorrespondingDownloads() ||
	   !checkSplitDownloadsNotMissingParts()) {
		return false;
	}

	if(!m_alias.empty() && Utilities::areStringsEqualIgnoreCase(m_id, m_alias)) {
		return false;
	}

	for(std::vector<std::shared_ptr<ModVersion>>::const_iterator i = m_versions.begin(); i != m_versions.end(); ++i) {
		if(!(*i)->isValid(skipFileInfoValidation)) {
			return false;
		}

		if((*i)->getParentMod() != this) {
			return false;
		}

		for(std::vector<std::shared_ptr<ModVersion>>::const_iterator j = i + 1; j != m_versions.end(); ++j) {
			if(Utilities::areStringsEqualIgnoreCase((*i)->getVersion(), (*j)->getVersion())) {
				return false;
			}
		}
	}

	for(std::vector<std::shared_ptr<ModDownload>>::const_iterator i = m_downloads.begin(); i != m_downloads.end(); ++i) {
		if(!(*i)->isValid(skipFileInfoValidation)) {
			return false;
		}

		if((*i)->getParentMod() != this) {
			return false;
		}

		for(std::vector<std::shared_ptr<ModDownload>>::const_iterator j = i + 1; j != m_downloads.end(); ++j) {
			if(Utilities::areStringsEqualIgnoreCase((*i)->getFileName(), (*j)->getFileName())) {
				return false;
			}
		}
	}

	for(std::vector<std::shared_ptr<ModScreenshot>>::const_iterator i = m_screenshots.begin(); i != m_screenshots.end(); ++i) {
		if(!(*i)->isValid()) {
			return false;
		}

		if((*i)->getParentMod() != this) {
			return false;
		}

		for(std::vector<std::shared_ptr<ModScreenshot>>::const_iterator j = i + 1; j != m_screenshots.end(); j++) {
			if(Utilities::areStringsEqualIgnoreCase((*i)->getFileName(), (*j)->getFileName())) {
				return false;
			}
		}
	}

	for(std::vector<std::shared_ptr<ModImage>>::const_iterator i = m_images.begin(); i != m_images.end(); ++i) {
		if(!(*i)->isValid()) {
			return false;
		}

		if((*i)->getParentMod() != this) {
			return false;
		}

		for(std::vector<std::shared_ptr<ModImage>>::const_iterator j = i + 1; j != m_images.end(); ++j) {
			if(Utilities::areStringsEqualIgnoreCase((*i)->getFileName(), (*j)->getFileName())) {
				return false;
			}
		}
	}

	for(std::vector<std::shared_ptr<ModVideo>>::const_iterator i = m_videos.begin(); i != m_videos.end(); ++i) {
		if(!(*i)->isValid()) {
			return false;
		}

		if((*i)->getParentMod() != this) {
			return false;
		}

		for(std::vector<std::shared_ptr<ModVideo>>::const_iterator j = i + 1; j != m_videos.end(); ++j) {
			if(Utilities::areStringsEqualIgnoreCase((*i)->getURL(), (*j)->getURL())) {
				return false;
			}
		}
	}

	for(const std::string & relatedMod : m_relatedMods) {
		if(Utilities::areStringsEqualIgnoreCase(m_id, relatedMod)) {
			return false;
		}
	}

	for(const std::string & similarMod : m_similarMods) {
		if(Utilities::areStringsEqualIgnoreCase(m_id, similarMod)) {
			return false;
		}
	}

	return true;
}

bool Mod::isValid(const Mod * m, bool skipFileInfoValidation) {
	return m != nullptr && m->isValid(skipFileInfoValidation);
}

void Mod::updateParent() {
	if(m_team != nullptr) {
		m_team->setParentMod(this);
		m_team->updateParent();
	}

	for(std::vector<std::shared_ptr<ModVersion>>::const_iterator i = m_versions.begin(); i != m_versions.end(); ++i) {
		(*i)->setParentMod(this);
	}

	for(std::vector<std::shared_ptr<ModDownload>>::const_iterator i = m_downloads.begin(); i != m_downloads.end(); ++i) {
		(*i)->setParentMod(this);
	}

	for(std::vector<std::shared_ptr<ModScreenshot>>::const_iterator i = m_screenshots.begin(); i != m_screenshots.end(); ++i) {
		(*i)->setParentMod(this);
	}

	for(std::vector<std::shared_ptr<ModImage>>::const_iterator i = m_images.begin(); i != m_images.end(); ++i) {
		(*i)->setParentMod(this);
	}

	for(std::vector<std::shared_ptr<ModVideo>>::const_iterator i = m_videos.begin(); i != m_videos.end(); ++i) {
		(*i)->setParentMod(this);
	}
}

bool Mod::operator == (const Mod & m) const {
	if(!Utilities::areStringsEqualIgnoreCase(m_id, m.m_id) ||
	   !Utilities::areStringsEqualIgnoreCase(m_name, m.m_name) ||
	   !Utilities::areStringsEqualIgnoreCase(m_alias, m.m_alias)||
	   !Utilities::areStringsEqualIgnoreCase(m_type, m.m_type) ||
	   !Utilities::areStringsEqualIgnoreCase(m_preferredVersion, m.m_preferredVersion) ||
	   !Utilities::areStringsEqualIgnoreCase(m_defaultVersionType, m.m_defaultVersionType) ||
	   !Utilities::areStringsEqualIgnoreCase(m_website, m.m_website)||
	   !Utilities::areStringsEqualIgnoreCase(m_repositoryURL, m.m_repositoryURL) ||
	   (m_team == nullptr && m.m_team != nullptr) ||
	   (m_team != nullptr && m.m_team == nullptr) ||
	   m_versions.size() == m.m_versions.size() ||
	   m_downloads.size() == m.m_downloads.size() ||
	   m_screenshots.size() == m.m_screenshots.size() ||
	   m_images.size() == m.m_images.size() ||
	   m_videos.size() == m.m_videos.size() ||
	   m_notes.size() == m.m_notes.size() ||
	   m_relatedMods.size() != m.m_relatedMods.size() ||
	   m_similarMods.size() != m.m_similarMods.size()) {
		return false;
	}

	if(m_team != nullptr && m.m_team != nullptr && *m_team != *m.m_team) {
		return false;
	}

	for(size_t i = 0; i < m_versions.size(); i++) {
		if(*m_versions[i] != *m.m_versions[i]) {
			return false;
		}
	}

	for(size_t i = 0; i < m_downloads.size(); i++) {
		if(*m_downloads[i] != *m.m_downloads[i]) {
			return false;
		}
	}

	for(size_t i = 0; i < m_screenshots.size(); i++) {
		if(*m_screenshots[i] != *m.m_screenshots[i]) {
			return false;
		}
	}

	for(size_t i = 0; i < m_images.size(); i++) {
		if(*m_images[i] != *m.m_images[i]) {
			return false;
		}
	}

	for(size_t i = 0; i < m_videos.size(); i++) {
		if(*m_videos[i] != *m.m_videos[i]) {
			return false;
		}
	}

	for(size_t i = 0; i < m_notes.size(); i++) {
		if(!Utilities::areStringsEqual(m_notes[i], m.m_notes[i])) {
			return false;
		}
	}

	for (size_t i = 0; i < m_relatedMods.size(); i++) {
		if(!Utilities::areStringsEqual(m_relatedMods[i], m.m_relatedMods[i])) {
			return false;
		}
	}

	for (size_t i = 0; i < m_similarMods.size(); i++) {
		if(!Utilities::areStringsEqual(m_similarMods[i], m.m_similarMods[i])) {
			return false;
		}
	}

	return true;
}

bool Mod::operator != (const Mod & m) const {
	return !operator == (m);
}
