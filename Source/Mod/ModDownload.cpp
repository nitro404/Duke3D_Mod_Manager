#include "ModDownload.h"

#include "Game/GameVersion.h"
#include "Mod.h"
#include "ModVersion.h"
#include "ModVersionType.h"

#include <Utilities/RapidJSONUtilities.h>
#include <Utilities/StringUtilities.h>

#include <spdlog/spdlog.h>
#include <tinyxml2.h>

#include <array>
#include <string_view>

static const std::string XML_MOD_DOWNLOAD_ELEMENT_NAME("download");
static const std::string XML_MOD_DOWNLOAD_FILE_NAME_ATTRIBUTE_NAME("filename");
static const std::string XML_MOD_DOWNLOAD_FILE_SIZE_ATTRIBUTE_NAME("filesize");
static const std::string XML_MOD_DOWNLOAD_PART_NUMBER_ATTRIBUTE_NAME("part");
static const std::string XML_MOD_DOWNLOAD_PART_COUNT_ATTRIBUTE_NAME("numparts");
static const std::string XML_MOD_DOWNLOAD_VERSION_ATTRIBUTE_NAME("version");
static const std::string XML_MOD_DOWNLOAD_VERSION_TYPE_ATTRIBUTE_NAME("version_type");
static const std::string XML_MOD_DOWNLOAD_SPECIAL_ATTRIBUTE_NAME("special");
static const std::string XML_MOD_DOWNLOAD_GAME_VERSION_ATTRIBUTE_NAME("game");
static const std::string XML_MOD_DOWNLOAD_TYPE_ATTRIBUTE_NAME("type");
static const std::string XML_MOD_DOWNLOAD_SHA1_ATTRIBUTE_NAME("sha1");
static const std::string XML_MOD_DOWNLOAD_CONVERTED_ATTRIBUTE_NAME("state");
static const std::string XML_MOD_DOWNLOAD_CORRUPTED_ATTRIBUTE_NAME("corrupted");
static const std::string XML_MOD_DOWNLOAD_REPAIRED_ATTRIBUTE_NAME("repaired");
static const std::array<std::string_view, 13> XML_MOD_DOWNLOAD_ATTRIBUTE_NAMES = {
	XML_MOD_DOWNLOAD_FILE_NAME_ATTRIBUTE_NAME,
	XML_MOD_DOWNLOAD_FILE_SIZE_ATTRIBUTE_NAME,
	XML_MOD_DOWNLOAD_PART_NUMBER_ATTRIBUTE_NAME,
	XML_MOD_DOWNLOAD_PART_COUNT_ATTRIBUTE_NAME,
	XML_MOD_DOWNLOAD_VERSION_ATTRIBUTE_NAME,
	XML_MOD_DOWNLOAD_VERSION_TYPE_ATTRIBUTE_NAME,
	XML_MOD_DOWNLOAD_SPECIAL_ATTRIBUTE_NAME,
	XML_MOD_DOWNLOAD_GAME_VERSION_ATTRIBUTE_NAME,
	XML_MOD_DOWNLOAD_TYPE_ATTRIBUTE_NAME,
	XML_MOD_DOWNLOAD_SHA1_ATTRIBUTE_NAME,
	XML_MOD_DOWNLOAD_CONVERTED_ATTRIBUTE_NAME,
	XML_MOD_DOWNLOAD_CORRUPTED_ATTRIBUTE_NAME,
	XML_MOD_DOWNLOAD_REPAIRED_ATTRIBUTE_NAME
};

static constexpr const char * JSON_MOD_DOWNLOAD_FILE_NAME_PROPERTY_NAME = "fileName";
static constexpr const char * JSON_MOD_DOWNLOAD_FILE_SIZE_PROPERTY_NAME = "fileSize";
static constexpr const char * JSON_MOD_DOWNLOAD_TYPE_PROPERTY_NAME = "type";
static constexpr const char * JSON_MOD_DOWNLOAD_SHA1_PROPERTY_NAME = "sha1";
static constexpr const char * JSON_MOD_DOWNLOAD_PART_NUMBER_PROPERTY_NAME = "partNumber";
static constexpr const char * JSON_MOD_DOWNLOAD_PART_COUNT_PROPERTY_NAME = "partCount";
static constexpr const char * JSON_MOD_DOWNLOAD_VERSION_PROPERTY_NAME = "version";
static constexpr const char * JSON_MOD_DOWNLOAD_VERSION_TYPE_PROPERTY_NAME = "versionType";
static constexpr const char * JSON_MOD_DOWNLOAD_SPECIAL_PROPERTY_NAME = "special";
static constexpr const char * JSON_MOD_DOWNLOAD_GAME_VERSION_PROPERTY_NAME = "gameVersion";
static constexpr const char * JSON_MOD_DOWNLOAD_CONVERTED_PROPERTY_NAME = "converted";
static constexpr const char * JSON_MOD_DOWNLOAD_CORRUPTED_PROPERTY_NAME = "corrupted";
static constexpr const char * JSON_MOD_DOWNLOAD_REPAIRED_PROPERTY_NAME = "repaired";
static const std::array<std::string_view, 13> JSON_MOD_DOWNLOAD_PROPERTY_NAMES = {
	JSON_MOD_DOWNLOAD_FILE_NAME_PROPERTY_NAME,
	JSON_MOD_DOWNLOAD_FILE_SIZE_PROPERTY_NAME,
	JSON_MOD_DOWNLOAD_TYPE_PROPERTY_NAME,
	JSON_MOD_DOWNLOAD_SHA1_PROPERTY_NAME,
	JSON_MOD_DOWNLOAD_PART_NUMBER_PROPERTY_NAME,
	JSON_MOD_DOWNLOAD_PART_COUNT_PROPERTY_NAME,
	JSON_MOD_DOWNLOAD_VERSION_PROPERTY_NAME,
	JSON_MOD_DOWNLOAD_VERSION_TYPE_PROPERTY_NAME,
	JSON_MOD_DOWNLOAD_SPECIAL_PROPERTY_NAME,
	JSON_MOD_DOWNLOAD_GAME_VERSION_PROPERTY_NAME,
	JSON_MOD_DOWNLOAD_CONVERTED_PROPERTY_NAME,
	JSON_MOD_DOWNLOAD_CORRUPTED_PROPERTY_NAME,
	JSON_MOD_DOWNLOAD_REPAIRED_PROPERTY_NAME
};

const std::string ModDownload::ORIGINAL_FILES_TYPE("Original Files");
const std::string ModDownload::MOD_MANAGER_FILES_TYPE("Mod Manager Files");

ModDownload::ModDownload(const std::string & fileName, uint64_t fileSize, const std::string & type, const std::string & sha1)
	: m_fileName(Utilities::trimString(fileName))
	, m_fileSize(fileSize)
	, m_partNumber(1)
	, m_partCount(1)
	, m_type(Utilities::trimString(type))
	, m_sha1(Utilities::trimString(sha1))
	, m_parentMod(nullptr) { }

ModDownload::ModDownload(ModDownload && d) noexcept
	: m_fileName(std::move(d.m_fileName))
	, m_fileSize(d.m_fileSize)
	, m_partNumber(d.m_partNumber)
	, m_partCount(d.m_partCount)
	, m_version(std::move(d.m_version))
	, m_versionType(std::move(d.m_versionType))
	, m_special(std::move(d.m_special))
	, m_gameVersionID(std::move(d.m_gameVersionID))
	, m_type(std::move(d.m_type))
	, m_sha1(std::move(d.m_sha1))
	, m_converted(d.m_converted)
	, m_corrupted(d.m_corrupted)
	, m_repaired(d.m_repaired)
	, m_parentMod(nullptr) { }

ModDownload::ModDownload(const ModDownload & d)
	: m_fileName(d.m_fileName)
	, m_fileSize(d.m_fileSize)
	, m_partNumber(d.m_partNumber)
	, m_partCount(d.m_partCount)
	, m_version(d.m_version)
	, m_versionType(d.m_versionType)
	, m_special(d.m_special)
	, m_gameVersionID(d.m_gameVersionID)
	, m_type(d.m_type)
	, m_sha1(d.m_sha1)
	, m_converted(d.m_converted)
	, m_corrupted(d.m_corrupted)
	, m_repaired(d.m_repaired)
	, m_parentMod(nullptr) { }

ModDownload & ModDownload::operator = (ModDownload && d) noexcept {
	if(this != &d) {
		m_fileName = std::move(d.m_fileName);
		m_fileSize = d.m_fileSize;
		m_partNumber = d.m_partNumber;
		m_partCount = d.m_partCount;
		m_version = std::move(d.m_version);
		m_versionType = std::move(d.m_versionType);
		m_special = std::move(d.m_special);
		m_gameVersionID = std::move(d.m_gameVersionID);
		m_type = std::move(d.m_type);
		m_sha1 = std::move(d.m_sha1);
		m_converted = d.m_converted;
		m_corrupted = d.m_corrupted;
		m_repaired = d.m_repaired;
	}

	return *this;
}

ModDownload & ModDownload::operator = (const ModDownload & d) {
	m_fileName = d.m_fileName;
	m_fileSize = d.m_fileSize;
	m_partNumber = d.m_partNumber;
	m_partCount = d.m_partCount;
	m_version = d.m_version;
	m_versionType = d.m_versionType;
	m_special = d.m_special;
	m_gameVersionID = d.m_gameVersionID;
	m_type = d.m_type;
	m_sha1 = d.m_sha1;
	m_converted = d.m_converted;
	m_corrupted = d.m_corrupted;
	m_repaired = d.m_repaired;

	return *this;
}

ModDownload::~ModDownload() {
	m_parentMod = nullptr;
}

const std::string & ModDownload::getFileName() const {
	return m_fileName;
}

uint64_t ModDownload::getFileSize() const {
	return m_fileSize;
}

bool ModDownload::hasMultipleParts() const {
	return m_partNumber != 0 && m_partCount > 1;
}

uint8_t ModDownload::getPartNumber() const {
	return m_partNumber;
}

uint8_t ModDownload::getPartCount() const {
	return m_partCount;
}

const std::string & ModDownload::getVersion() const {
	return m_version;
}

const std::string & ModDownload::getVersionType() const {
	return m_versionType;
}

bool ModDownload::hasSpecial() const {
	return !m_special.empty();
}

const std::string & ModDownload::getSpecial() const {
	return m_special;
}

bool ModDownload::hasGameVersionID() const {
	return !m_gameVersionID.empty();
}

const std::string & ModDownload::getGameVersionID() const {
	return m_gameVersionID;
}

const std::string & ModDownload::getType() const {
	return m_type;
}

bool ModDownload::isOriginalFiles() const {
	return Utilities::areStringsEqualIgnoreCase(m_type, ORIGINAL_FILES_TYPE);
}

bool ModDownload::isModManagerFiles() const {
	return Utilities::areStringsEqualIgnoreCase(m_type, MOD_MANAGER_FILES_TYPE);
}

const std::string & ModDownload::getSubfolder() const {
	static const std::string ORIGINAL_FILES_SUBDIRECTORY("original");
	static const std::string MOD_MANAGER_FILES_SUBDIRECTORY("mod_manager");

	if(isOriginalFiles()) {
		return ORIGINAL_FILES_SUBDIRECTORY;
	}
	else if(isModManagerFiles()) {
		return MOD_MANAGER_FILES_SUBDIRECTORY;
	}

	return Utilities::emptyString;
}

const std::string & ModDownload::getSHA1() const {
	return m_sha1;
}

bool ModDownload::isStandAlone() const {
	return Utilities::areStringsEqualIgnoreCase(m_gameVersionID, GameVersion::STANDALONE);
}

bool ModDownload::isEDuke32() const {
	return Utilities::areStringsEqualIgnoreCase(m_gameVersionID, GameVersion::EDUKE32.getID());
}

bool ModDownload::isConverted() const {
	return m_converted.has_value() ? m_converted.value() : false;
}

std::optional<bool> ModDownload::getConverted() const {
	return m_converted;
}

bool ModDownload::isCorrupted() const {
	return m_corrupted.has_value() ? m_corrupted.value() : false;
}

std::optional<bool> ModDownload::getCorrupted() const {
	return m_corrupted;
}

bool ModDownload::isRepaired() const {
	return m_repaired.has_value() ? m_repaired.value() : false;
}

std::optional<bool> ModDownload::getRepaired() const {
	return m_repaired;
}

const Mod * ModDownload::getParentMod() const {
	return m_parentMod;
}

std::shared_ptr<ModVersion> ModDownload::getModVersion() const {
	if(m_parentMod == nullptr) {
		return nullptr;
	}

	return m_parentMod->getModVersionForDownload(this);
}

std::shared_ptr<ModVersionType> ModDownload::getModVersionType() const {
	if(m_parentMod == nullptr) {
		return nullptr;
	}

	return m_parentMod->getModVersionTypeForDownload(this);
}

void ModDownload::setFileName(const std::string & fileName) {
	m_fileName = Utilities::trimString(fileName);
}

void ModDownload::setFileSize(uint64_t fileSize) {
	m_fileSize = fileSize;
}

void ModDownload::setPartNumber(uint8_t partNumber) {
	m_partNumber = partNumber;
}

void ModDownload::setPartCount(uint8_t partCount) {
	m_partCount = partCount;
}

void ModDownload::setVersion(const std::string & version) {
	m_version = Utilities::trimString(version);
}

void ModDownload::setVersionType(const std::string & versionType) {
	m_versionType = Utilities::trimString(versionType);
}

void ModDownload::setSpecial(const std::string & special) {
	m_special = Utilities::trimString(special);
}

void ModDownload::setGameVersionID(const std::string & gameVersionID) {
	m_gameVersionID = Utilities::trimString(gameVersionID);
}

void ModDownload::setType(const std::string & type) {
	m_type = Utilities::trimString(type);
}

void ModDownload::setSHA1(const std::string & sha1) {
	m_sha1 = Utilities::trimString(sha1);
}

void ModDownload::setConverted(bool converted) {
	m_converted = converted;
}

void ModDownload::clearConverted() {
	m_converted.reset();
}

void ModDownload::setCorrupted(bool corrupted) {
	m_corrupted = corrupted;
}

void ModDownload::clearCorrupted() {
	m_corrupted.reset();
}

void ModDownload::setRepaired(bool repaired) {
	m_repaired = repaired;
}

void ModDownload::clearRepaired() {
	m_repaired.reset();
}

void ModDownload::setParentMod(const Mod * mod) {
	m_parentMod = mod;
}

rapidjson::Value ModDownload::toJSON(rapidjson::MemoryPoolAllocator<rapidjson::CrtAllocator> & allocator) const {
	rapidjson::Value modDownloadValue(rapidjson::kObjectType);

	rapidjson::Value fileNameValue(m_fileName.c_str(), allocator);
	modDownloadValue.AddMember(rapidjson::StringRef(JSON_MOD_DOWNLOAD_FILE_NAME_PROPERTY_NAME), fileNameValue, allocator);

	modDownloadValue.AddMember(rapidjson::StringRef(JSON_MOD_DOWNLOAD_FILE_SIZE_PROPERTY_NAME), rapidjson::Value(m_fileSize), allocator);

	rapidjson::Value typeValue(m_type.c_str(), allocator);
	modDownloadValue.AddMember(rapidjson::StringRef(JSON_MOD_DOWNLOAD_TYPE_PROPERTY_NAME), typeValue, allocator);

	if(m_partCount != 1) {
		modDownloadValue.AddMember(rapidjson::StringRef(JSON_MOD_DOWNLOAD_PART_NUMBER_PROPERTY_NAME), rapidjson::Value(m_partNumber), allocator);

		modDownloadValue.AddMember(rapidjson::StringRef(JSON_MOD_DOWNLOAD_PART_COUNT_PROPERTY_NAME), rapidjson::Value(m_partCount), allocator);
	}

	if(!m_version.empty()) {
		rapidjson::Value versionValue(m_version.c_str(), allocator);
		modDownloadValue.AddMember(rapidjson::StringRef(JSON_MOD_DOWNLOAD_VERSION_PROPERTY_NAME), versionValue, allocator);
	}

	if(!m_versionType.empty()) {
		rapidjson::Value versionTypeValue(m_versionType.c_str(), allocator);
		modDownloadValue.AddMember(rapidjson::StringRef(JSON_MOD_DOWNLOAD_VERSION_TYPE_PROPERTY_NAME), versionTypeValue, allocator);
	}

	if(!m_special.empty()) {
		rapidjson::Value specialValue(m_special.c_str(), allocator);
		modDownloadValue.AddMember(rapidjson::StringRef(JSON_MOD_DOWNLOAD_SPECIAL_PROPERTY_NAME), specialValue, allocator);
	}

	if(!m_gameVersionID.empty()) {
		rapidjson::Value gameVersionValue(m_gameVersionID.c_str(), allocator);
		modDownloadValue.AddMember(rapidjson::StringRef(JSON_MOD_DOWNLOAD_GAME_VERSION_PROPERTY_NAME), gameVersionValue, allocator);
	}

	if(m_converted.has_value()) {
		modDownloadValue.AddMember(rapidjson::StringRef(JSON_MOD_DOWNLOAD_CONVERTED_PROPERTY_NAME), rapidjson::Value(m_converted.value()), allocator);
	}

	if(m_corrupted.has_value()) {
		modDownloadValue.AddMember(rapidjson::StringRef(JSON_MOD_DOWNLOAD_CORRUPTED_PROPERTY_NAME), rapidjson::Value(m_corrupted.value()), allocator);
	}

	if(m_repaired.has_value()) {
		modDownloadValue.AddMember(rapidjson::StringRef(JSON_MOD_DOWNLOAD_REPAIRED_PROPERTY_NAME), rapidjson::Value(m_repaired.value()), allocator);
	}

	if(!m_sha1.empty()) {
		rapidjson::Value sha1Value(m_sha1.c_str(), allocator);
		modDownloadValue.AddMember(rapidjson::StringRef(JSON_MOD_DOWNLOAD_SHA1_PROPERTY_NAME), sha1Value, allocator);
	}

	return modDownloadValue;
}

tinyxml2::XMLElement * ModDownload::toXML(tinyxml2::XMLDocument * document) const {
	if(document == nullptr) {
		return nullptr;
	}

	tinyxml2::XMLElement * modDownloadElement = document->NewElement(XML_MOD_DOWNLOAD_ELEMENT_NAME.c_str());

	modDownloadElement->SetAttribute(XML_MOD_DOWNLOAD_FILE_NAME_ATTRIBUTE_NAME.c_str(), m_fileName.c_str());

	if(m_fileSize != 0) {
		modDownloadElement->SetAttribute(XML_MOD_DOWNLOAD_FILE_SIZE_ATTRIBUTE_NAME.c_str(), m_fileSize);
	}

	if(!m_version.empty()) {
		modDownloadElement->SetAttribute(XML_MOD_DOWNLOAD_VERSION_ATTRIBUTE_NAME.c_str(), m_version.c_str());
	}

	if(!m_versionType.empty()) {
		modDownloadElement->SetAttribute(XML_MOD_DOWNLOAD_VERSION_TYPE_ATTRIBUTE_NAME.c_str(), m_versionType.c_str());
	}

	if(m_partCount != 1) {
		modDownloadElement->SetAttribute(XML_MOD_DOWNLOAD_PART_NUMBER_ATTRIBUTE_NAME.c_str(), m_partNumber);
		modDownloadElement->SetAttribute(XML_MOD_DOWNLOAD_PART_COUNT_ATTRIBUTE_NAME.c_str(), m_partCount);
	}

	if(!m_special.empty()) {
		modDownloadElement->SetAttribute(XML_MOD_DOWNLOAD_SPECIAL_ATTRIBUTE_NAME.c_str(), m_special.c_str());
	}

	if(m_corrupted.has_value()) {
		modDownloadElement->SetAttribute(XML_MOD_DOWNLOAD_CORRUPTED_ATTRIBUTE_NAME.c_str(), m_corrupted.value());
	}

	if(m_repaired.has_value()) {
		modDownloadElement->SetAttribute(XML_MOD_DOWNLOAD_REPAIRED_ATTRIBUTE_NAME.c_str(), m_repaired.value());
	}

	if(!m_gameVersionID.empty()) {
		modDownloadElement->SetAttribute(XML_MOD_DOWNLOAD_GAME_VERSION_ATTRIBUTE_NAME.c_str(), m_gameVersionID.c_str());
	}

	modDownloadElement->SetAttribute(XML_MOD_DOWNLOAD_TYPE_ATTRIBUTE_NAME.c_str(), m_type.c_str());

	if(m_converted.has_value()) {
		modDownloadElement->SetAttribute(XML_MOD_DOWNLOAD_CONVERTED_ATTRIBUTE_NAME.c_str(), m_converted.value() ? "Converted" : "Native");
	}

	if(!m_sha1.empty()) {
		modDownloadElement->SetAttribute(XML_MOD_DOWNLOAD_SHA1_ATTRIBUTE_NAME.c_str(), m_sha1.c_str());
	}

	return modDownloadElement;
}

std::unique_ptr<ModDownload> ModDownload::parseFrom(const rapidjson::Value & modDownloadValue, bool skipFileInfoValidation) {
	if(!modDownloadValue.IsObject()) {
		spdlog::error("Invalid mod download type: '{}', expected 'object'.", Utilities::typeToString(modDownloadValue.GetType()));
		return nullptr;
	}

	// check for unhandled mod download properties
	bool propertyHandled = false;

	for(rapidjson::Value::ConstMemberIterator i = modDownloadValue.MemberBegin(); i != modDownloadValue.MemberEnd(); ++i) {
		propertyHandled = false;

		for(const std::string_view propertyName : JSON_MOD_DOWNLOAD_PROPERTY_NAMES) {
			if(i->name.GetString() == propertyName) {
				propertyHandled = true;
				break;
			}
		}

		if(!propertyHandled) {
			spdlog::warn("Mod download has unexpected property '{}'.", i->name.GetString());
		}
	}

	// parse mod download file name
	if(!modDownloadValue.HasMember(JSON_MOD_DOWNLOAD_FILE_NAME_PROPERTY_NAME)) {
		spdlog::error("Mod download is missing '{}' property.", JSON_MOD_DOWNLOAD_FILE_NAME_PROPERTY_NAME);
		return nullptr;
	}

	const rapidjson::Value & modDownloadFileNameValue = modDownloadValue[JSON_MOD_DOWNLOAD_FILE_NAME_PROPERTY_NAME];

	if(!modDownloadFileNameValue.IsString()) {
		spdlog::error("Mod download has an invalid '{}' property type: '{}', expected 'string'.", JSON_MOD_DOWNLOAD_FILE_NAME_PROPERTY_NAME, Utilities::typeToString(modDownloadFileNameValue.GetType()));
		return nullptr;
	}

	std::string modDownloadFileName(Utilities::trimString(modDownloadFileNameValue.GetString()));

	if(modDownloadFileName.empty()) {
		spdlog::error("Mod download '{}' property cannot be empty.", JSON_MOD_DOWNLOAD_FILE_NAME_PROPERTY_NAME);
		return nullptr;
	}

	// parse mod download type
	if(!modDownloadValue.HasMember(JSON_MOD_DOWNLOAD_TYPE_PROPERTY_NAME)) {
		spdlog::error("Mod download is missing '{}' property.", JSON_MOD_DOWNLOAD_TYPE_PROPERTY_NAME);
		return nullptr;
	}

	const rapidjson::Value & modDownloadTypeValue = modDownloadValue[JSON_MOD_DOWNLOAD_TYPE_PROPERTY_NAME];

	if(!modDownloadTypeValue.IsString()) {
		spdlog::error("Mod download has an invalid '{}' property type: '{}', expected 'string'.", JSON_MOD_DOWNLOAD_TYPE_PROPERTY_NAME, Utilities::typeToString(modDownloadTypeValue.GetType()));
		return nullptr;
	}

	std::string modDownloadType(Utilities::trimString(modDownloadTypeValue.GetString()));

	if(modDownloadType.empty()) {
		spdlog::error("Mod download '{}' property cannot be empty.", JSON_MOD_DOWNLOAD_TYPE_PROPERTY_NAME);
		return nullptr;
	}

	// parse the mod download sha1 property
	std::string modDownloadSHA1;

	if(modDownloadValue.HasMember(JSON_MOD_DOWNLOAD_SHA1_PROPERTY_NAME)) {
		const rapidjson::Value & modDownloadSHA1Value = modDownloadValue[JSON_MOD_DOWNLOAD_SHA1_PROPERTY_NAME];

		if(!modDownloadSHA1Value.IsString()) {
			spdlog::error("Mod download '{}' property has invalid type: '{}', expected 'string'.", JSON_MOD_DOWNLOAD_SHA1_PROPERTY_NAME, Utilities::typeToString(modDownloadSHA1Value.GetType()));
			return nullptr;
		}

		modDownloadSHA1 = Utilities::trimString(modDownloadSHA1Value.GetString());

		if(modDownloadSHA1.empty()) {
			spdlog::error("Mod download '{}' property cannot be empty.", JSON_MOD_DOWNLOAD_SHA1_PROPERTY_NAME);
			return nullptr;
		}
	}
	else {
		if(!skipFileInfoValidation) {
			spdlog::error("Mod download is missing '{}' property.", JSON_MOD_DOWNLOAD_SHA1_PROPERTY_NAME);
			return nullptr;
		}
	}

	// parse mod download file size
	uint64_t modDownloadFileSize = 0;

	if(modDownloadValue.HasMember(JSON_MOD_DOWNLOAD_FILE_SIZE_PROPERTY_NAME)) {
		const rapidjson::Value & modDownloadFileSizeValue = modDownloadValue[JSON_MOD_DOWNLOAD_FILE_SIZE_PROPERTY_NAME];

		if(!modDownloadFileSizeValue.IsUint64()) {
			spdlog::error("Mod download has an invalid '{}' property type: '{}', expected unsigned long 'number'.", JSON_MOD_DOWNLOAD_FILE_SIZE_PROPERTY_NAME, Utilities::typeToString(modDownloadFileSizeValue.GetType()));
			return nullptr;
		}

		modDownloadFileSize = modDownloadFileSizeValue.GetUint64();

		if(modDownloadFileSize == 0) {
			spdlog::error("Mod download has an invalid '{}' property value, expected positive integer value.", JSON_MOD_DOWNLOAD_FILE_SIZE_PROPERTY_NAME);
			return nullptr;
		}
	}
	else {
		if(!skipFileInfoValidation) {
			spdlog::error("Mod download is missing '{}' property.", JSON_MOD_DOWNLOAD_FILE_SIZE_PROPERTY_NAME);
			return nullptr;
		}
	}

	// initialize the mod download
	std::unique_ptr<ModDownload> newModDownload = std::make_unique<ModDownload>(modDownloadFileName, modDownloadFileSize, modDownloadType, modDownloadSHA1);

	bool error = false;

	// parse the mod download part number property
	if(modDownloadValue.HasMember(JSON_MOD_DOWNLOAD_PART_COUNT_PROPERTY_NAME)) {
		const rapidjson::Value & modDownloadPartCountValue = modDownloadValue[JSON_MOD_DOWNLOAD_PART_COUNT_PROPERTY_NAME];

		if(!modDownloadPartCountValue.IsUint()) {
			spdlog::error("Mod download has an invalid '{}' property type: '{}', expected unsigned integer 'number'.", JSON_MOD_DOWNLOAD_PART_COUNT_PROPERTY_NAME, Utilities::typeToString(modDownloadPartCountValue.GetType()));
			return nullptr;
		}

		uint32_t partCount = modDownloadPartCountValue.GetUint();

		if(error || partCount < 1 || partCount > std::numeric_limits<uint8_t>::max()) {
			spdlog::error("Mod download '{}' property value has an invalid value: '{}', expected unsigned integer 'number' between 1 and {} inclusively.", JSON_MOD_DOWNLOAD_PART_COUNT_PROPERTY_NAME, partCount, std::numeric_limits<uint8_t>::max());
			return nullptr;
		}

		newModDownload->setPartCount(static_cast<uint8_t>(partCount));
	}

	// parse the mod download part part number property
	if(modDownloadValue.HasMember(JSON_MOD_DOWNLOAD_PART_NUMBER_PROPERTY_NAME)) {
		const rapidjson::Value & modDownloadPartNumberValue = modDownloadValue[JSON_MOD_DOWNLOAD_PART_NUMBER_PROPERTY_NAME];

		if(!modDownloadPartNumberValue.IsUint()) {
			spdlog::error("Mod download has an invalid '{}' property type: '{}', expected unsigned integer 'number'.", JSON_MOD_DOWNLOAD_PART_NUMBER_PROPERTY_NAME, Utilities::typeToString(modDownloadPartNumberValue.GetType()));
			return nullptr;
		}

		uint32_t partNumber = modDownloadPartNumberValue.GetUint();

		if(error || partNumber < 1 || partNumber > std::numeric_limits<uint8_t>::max()) {
			spdlog::error("Mod download '{}' property value has an invalid value: '{}', expected unsigned integer 'number' between 1 and {} inclusively.", JSON_MOD_DOWNLOAD_PART_NUMBER_PROPERTY_NAME, partNumber, std::numeric_limits<uint8_t>::max());
			return nullptr;
		}

		if(partNumber > newModDownload->m_partCount) {
			spdlog::error("Attribute '{}' in element '{}' has an invalid value: '{}' which exceeds the '{}' value of '{}'.", XML_MOD_DOWNLOAD_PART_NUMBER_ATTRIBUTE_NAME, XML_MOD_DOWNLOAD_ELEMENT_NAME, partNumber, XML_MOD_DOWNLOAD_PART_COUNT_ATTRIBUTE_NAME, newModDownload->m_partCount);
			return nullptr;
		}

		newModDownload->setPartNumber(static_cast<uint8_t>(partNumber));
	}

	// parse the mod download version property
	if(modDownloadValue.HasMember(JSON_MOD_DOWNLOAD_VERSION_PROPERTY_NAME)) {
		const rapidjson::Value & modDownloadVersionValue = modDownloadValue[JSON_MOD_DOWNLOAD_VERSION_PROPERTY_NAME];

		if(!modDownloadVersionValue.IsString()) {
			spdlog::error("Mod download '{}' property has invalid type: '{}', expected 'string'.", JSON_MOD_DOWNLOAD_VERSION_PROPERTY_NAME, Utilities::typeToString(modDownloadVersionValue.GetType()));
			return nullptr;
		}

		newModDownload->setVersion(modDownloadVersionValue.GetString());
	}

	// parse the mod download version type property
	if(modDownloadValue.HasMember(JSON_MOD_DOWNLOAD_VERSION_TYPE_PROPERTY_NAME)) {
		const rapidjson::Value & modDownloadVersionTypeValue = modDownloadValue[JSON_MOD_DOWNLOAD_VERSION_TYPE_PROPERTY_NAME];

		if(!modDownloadVersionTypeValue.IsString()) {
			spdlog::error("Mod download '{}' property has invalid type: '{}', expected 'string'.", JSON_MOD_DOWNLOAD_VERSION_TYPE_PROPERTY_NAME, Utilities::typeToString(modDownloadVersionTypeValue.GetType()));
			return nullptr;
		}

		newModDownload->setVersionType(modDownloadVersionTypeValue.GetString());
	}

	// parse the mod download special property
	if(modDownloadValue.HasMember(JSON_MOD_DOWNLOAD_SPECIAL_PROPERTY_NAME)) {
		const rapidjson::Value & modDownloadSpecialValue = modDownloadValue[JSON_MOD_DOWNLOAD_SPECIAL_PROPERTY_NAME];

		if(!modDownloadSpecialValue.IsString()) {
			spdlog::error("Mod download '{}' property has invalid type: '{}', expected 'string'.", JSON_MOD_DOWNLOAD_SPECIAL_PROPERTY_NAME, Utilities::typeToString(modDownloadSpecialValue.GetType()));
			return nullptr;
		}

		newModDownload->setSpecial(modDownloadSpecialValue.GetString());
	}

	// parse the mod download game version property
	if(modDownloadValue.HasMember(JSON_MOD_DOWNLOAD_GAME_VERSION_PROPERTY_NAME)) {
		const rapidjson::Value & modDownloadGameVersionValue = modDownloadValue[JSON_MOD_DOWNLOAD_GAME_VERSION_PROPERTY_NAME];

		if(!modDownloadGameVersionValue.IsString()) {
			spdlog::error("Mod download '{}' property has invalid type: '{}', expected 'string'.", JSON_MOD_DOWNLOAD_GAME_VERSION_PROPERTY_NAME, Utilities::typeToString(modDownloadGameVersionValue.GetType()));
			return nullptr;
		}

		newModDownload->setGameVersionID(modDownloadGameVersionValue.GetString());
	}

	// parse the mod download converted property
	if(modDownloadValue.HasMember(JSON_MOD_DOWNLOAD_CONVERTED_PROPERTY_NAME)) {
		const rapidjson::Value & modDownloadConvertedValue = modDownloadValue[JSON_MOD_DOWNLOAD_CONVERTED_PROPERTY_NAME];

		if(!modDownloadConvertedValue.IsBool()) {
			spdlog::error("Mod download '{}' property has invalid type: '{}', expected 'boolean'.", JSON_MOD_DOWNLOAD_CONVERTED_PROPERTY_NAME, Utilities::typeToString(modDownloadConvertedValue.GetType()));
			return nullptr;
		}

		newModDownload->setConverted(modDownloadConvertedValue.GetBool());
	}

	// parse the mod download corrupted property
	if(modDownloadValue.HasMember(JSON_MOD_DOWNLOAD_CORRUPTED_PROPERTY_NAME)) {
		const rapidjson::Value & modDownloadCorruptedValue = modDownloadValue[JSON_MOD_DOWNLOAD_CORRUPTED_PROPERTY_NAME];

		if(!modDownloadCorruptedValue.IsBool()) {
			spdlog::error("Mod download '{}' property has invalid type: '{}', expected 'boolean'.", JSON_MOD_DOWNLOAD_CORRUPTED_PROPERTY_NAME, Utilities::typeToString(modDownloadCorruptedValue.GetType()));
			return nullptr;
		}

		newModDownload->setCorrupted(modDownloadCorruptedValue.GetBool());
	}

	// parse the mod download repaired property
	if(modDownloadValue.HasMember(JSON_MOD_DOWNLOAD_REPAIRED_PROPERTY_NAME)) {
		const rapidjson::Value & modDownloadRepairedValue = modDownloadValue[JSON_MOD_DOWNLOAD_REPAIRED_PROPERTY_NAME];

		if(!modDownloadRepairedValue.IsBool()) {
			spdlog::error("Mod download '{}' property has invalid type: '{}', expected 'boolean'.", JSON_MOD_DOWNLOAD_REPAIRED_PROPERTY_NAME, Utilities::typeToString(modDownloadRepairedValue.GetType()));
			return nullptr;
		}

		newModDownload->setRepaired(modDownloadRepairedValue.GetBool());
	}

	return newModDownload;
}

std::unique_ptr<ModDownload> ModDownload::parseFrom(const tinyxml2::XMLElement * modDownloadElement, bool skipFileInfoValidation) {
	if(modDownloadElement == nullptr) {
		return nullptr;
	}

	// verify element name
	if(modDownloadElement->Name() != XML_MOD_DOWNLOAD_ELEMENT_NAME) {
		spdlog::error("Invalid mod download element name: '{}', expected '{}'.", modDownloadElement->Name(), XML_MOD_DOWNLOAD_ELEMENT_NAME);
		return nullptr;
	}

	// check for unhandled mod download element attributes
	bool attributeHandled = false;
	const tinyxml2::XMLAttribute * modDownloadAttribute = modDownloadElement->FirstAttribute();

	while(true) {
		if(modDownloadAttribute == nullptr) {
			break;
		}

		attributeHandled = false;

		for(const std::string_view & attributeName : XML_MOD_DOWNLOAD_ATTRIBUTE_NAMES) {
			if(modDownloadAttribute->Name() == attributeName) {
				attributeHandled = true;
				break;
			}
		}

		if(!attributeHandled) {
			spdlog::warn("Element '{}' has unexpected attribute '{}'.", XML_MOD_DOWNLOAD_ELEMENT_NAME, modDownloadAttribute->Name());
		}

		modDownloadAttribute = modDownloadAttribute->Next();
	}

	// check for unexpected mod download element child elements
	if(modDownloadElement->FirstChildElement() != nullptr) {
		spdlog::warn("Element '{}' has an unexpected child element.", XML_MOD_DOWNLOAD_ELEMENT_NAME);
	}

	// read the mod download attributes
	const char * modDownloadFileName = modDownloadElement->Attribute(XML_MOD_DOWNLOAD_FILE_NAME_ATTRIBUTE_NAME.c_str());

	if(Utilities::stringLength(modDownloadFileName) == 0) {
		spdlog::error("Attribute '{}' is missing from '{}' element.", XML_MOD_DOWNLOAD_FILE_NAME_ATTRIBUTE_NAME, XML_MOD_DOWNLOAD_ELEMENT_NAME);
		return nullptr;
	}

	const char * modDownloadType = modDownloadElement->Attribute(XML_MOD_DOWNLOAD_TYPE_ATTRIBUTE_NAME.c_str());

	if(Utilities::stringLength(modDownloadType) == 0) {
		spdlog::error("Attribute '{}' is missing from '{}' element.", XML_MOD_DOWNLOAD_TYPE_ATTRIBUTE_NAME, XML_MOD_DOWNLOAD_ELEMENT_NAME);
		return nullptr;
	}

	const char * modDownloadSHA1 = modDownloadElement->Attribute(XML_MOD_DOWNLOAD_SHA1_ATTRIBUTE_NAME.c_str());

	if(!skipFileInfoValidation && Utilities::stringLength(modDownloadSHA1) == 0) {
		spdlog::error("Attribute '{}' is missing from '{}' element.", XML_MOD_DOWNLOAD_SHA1_ATTRIBUTE_NAME, XML_MOD_DOWNLOAD_ELEMENT_NAME);
		return nullptr;
	}

	const char * modDownloadVersion = modDownloadElement->Attribute(XML_MOD_DOWNLOAD_VERSION_ATTRIBUTE_NAME.c_str());
	const char * modDownloadVersionType = modDownloadElement->Attribute(XML_MOD_DOWNLOAD_VERSION_TYPE_ATTRIBUTE_NAME.c_str());
	const char * modDownloadSpecial = modDownloadElement->Attribute(XML_MOD_DOWNLOAD_SPECIAL_ATTRIBUTE_NAME.c_str());
	const char * modDownloadGameVersion = modDownloadElement->Attribute(XML_MOD_DOWNLOAD_GAME_VERSION_ATTRIBUTE_NAME.c_str());
	const char * modDownloadConvertedData = modDownloadElement->Attribute(XML_MOD_DOWNLOAD_CONVERTED_ATTRIBUTE_NAME.c_str());
	const char * modDownloadPartNumberData = modDownloadElement->Attribute(XML_MOD_DOWNLOAD_PART_NUMBER_ATTRIBUTE_NAME.c_str());
	const char * modDownloadPartCountData = modDownloadElement->Attribute(XML_MOD_DOWNLOAD_PART_COUNT_ATTRIBUTE_NAME.c_str());
	const char * modDownloadCorruptedData = modDownloadElement->Attribute(XML_MOD_DOWNLOAD_CORRUPTED_ATTRIBUTE_NAME.c_str());
	const char * modDownloadRepairedData = modDownloadElement->Attribute(XML_MOD_DOWNLOAD_REPAIRED_ATTRIBUTE_NAME.c_str());
	const char * modDownloadFileSizeData = modDownloadElement->Attribute(XML_MOD_DOWNLOAD_FILE_SIZE_ATTRIBUTE_NAME.c_str());

	bool error = false;
	uint64_t modDownloadFileSize = 0;

	if(Utilities::stringLength(modDownloadFileSizeData) != 0) {
		modDownloadFileSize = Utilities::parseUnsignedLong(modDownloadFileSizeData, &error);

		if(error || modDownloadFileSize == 0) {
			spdlog::error("Attribute '{}' in element '{}' has an invalid value: '{}', expected positive integer number.", XML_MOD_DOWNLOAD_FILE_SIZE_ATTRIBUTE_NAME, XML_MOD_DOWNLOAD_ELEMENT_NAME, modDownloadFileSizeData);
			return nullptr;
		}
	}
	else {
		if(!skipFileInfoValidation) {
			spdlog::error("Attribute '{}' is missing from '{}' element.", XML_MOD_DOWNLOAD_FILE_SIZE_ATTRIBUTE_NAME, XML_MOD_DOWNLOAD_ELEMENT_NAME);
			return false;
		}
	}

	// initialize the mod download
	std::unique_ptr<ModDownload> newModDownload(std::make_unique<ModDownload>(modDownloadFileName, modDownloadFileSize, modDownloadType, modDownloadSHA1 == nullptr ? "" : modDownloadSHA1));

	if(modDownloadVersion != nullptr) {
		newModDownload->setVersion(modDownloadVersion);
	}

	if(modDownloadVersionType != nullptr) {
		newModDownload->setVersionType(modDownloadVersionType);
	}

	if(modDownloadSpecial != nullptr) {
		newModDownload->setSpecial(modDownloadSpecial);
	}

	if(modDownloadGameVersion != nullptr) {
		newModDownload->setGameVersionID(modDownloadGameVersion);
	}

	if(modDownloadPartCountData != nullptr) {
		uint32_t partCount = Utilities::parseUnsignedInteger(modDownloadPartCountData, &error);

		if(error || partCount < 1 || partCount > std::numeric_limits<uint8_t>::max()) {
			spdlog::error("Attribute '{}' in element '{}' has an invalid value: '{}', expected integer number between 1 and {} inclusively.", XML_MOD_DOWNLOAD_PART_COUNT_ATTRIBUTE_NAME, XML_MOD_DOWNLOAD_ELEMENT_NAME, modDownloadPartCountData, std::numeric_limits<uint8_t>::max());
			return nullptr;
		}

		newModDownload->setPartCount(static_cast<uint8_t>(partCount));
	}

	if(modDownloadPartNumberData != nullptr) {
		uint32_t partNumber = Utilities::parseUnsignedInteger(modDownloadPartNumberData, &error);

		if(error || partNumber < 1 || partNumber > std::numeric_limits<uint8_t>::max()) {
			spdlog::error("Attribute '{}' in element '{}' has an invalid value: '{}', expected integer number between 1 and {} inclusively.", XML_MOD_DOWNLOAD_PART_NUMBER_ATTRIBUTE_NAME, XML_MOD_DOWNLOAD_ELEMENT_NAME, modDownloadPartNumberData, std::numeric_limits<uint8_t>::max());
			return nullptr;
		}

		if(partNumber > newModDownload->m_partCount) {
			spdlog::error("Attribute '{}' in element '{}' has an invalid value: '{}' which exceeds the '{}' value of '{}'.", XML_MOD_DOWNLOAD_PART_NUMBER_ATTRIBUTE_NAME, XML_MOD_DOWNLOAD_ELEMENT_NAME, partNumber, XML_MOD_DOWNLOAD_PART_COUNT_ATTRIBUTE_NAME, newModDownload->m_partCount);
			return nullptr;
		}

		newModDownload->setPartNumber(static_cast<uint8_t>(partNumber));
	}

	if(modDownloadConvertedData != nullptr) {
		if(Utilities::areStringsEqualIgnoreCase(modDownloadConvertedData, "Converted")) {
			newModDownload->setConverted(true);
		}
		else if(Utilities::areStringsEqualIgnoreCase(modDownloadConvertedData, "Native")) {
			newModDownload->setConverted(false);
		}
		else {
			spdlog::error("Attribute '{}' in element '{}' has an invalid value: '{}', expected 'Native' or 'Converted'.", XML_MOD_DOWNLOAD_CONVERTED_ATTRIBUTE_NAME, XML_MOD_DOWNLOAD_ELEMENT_NAME, modDownloadConvertedData);
			return nullptr;
		}
	}

	if(modDownloadCorruptedData != nullptr) {
		bool corrupted = Utilities::parseBoolean(modDownloadCorruptedData, &error);

		if(error) {
			spdlog::error("Attribute '{}' in element '{}' has an invalid value: '{}', expected boolean.", XML_MOD_DOWNLOAD_CORRUPTED_ATTRIBUTE_NAME, XML_MOD_DOWNLOAD_ELEMENT_NAME, modDownloadCorruptedData);
			return nullptr;
		}

		newModDownload->setCorrupted(corrupted);
	}

	if(modDownloadRepairedData != nullptr) {
		bool repaired = Utilities::parseBoolean(modDownloadRepairedData, &error);

		if(error) {
			spdlog::error("Attribute '{}' in element '{}' has an invalid value: '{}', expected boolean.", XML_MOD_DOWNLOAD_REPAIRED_ATTRIBUTE_NAME, XML_MOD_DOWNLOAD_ELEMENT_NAME, modDownloadRepairedData);
			return nullptr;
		}

		newModDownload->setRepaired(repaired);
	}

	return newModDownload;
}

bool ModDownload::isValid(bool skipFileInfoValidation) const {
	if(!skipFileInfoValidation) {
		if(m_fileSize == 0 ||
		   m_sha1.empty()) {
			return false;
		}
	}

	return !m_fileName.empty() &&
		   !m_type.empty() ||
		   m_partNumber > m_partCount;
}

bool ModDownload::isValid(const ModDownload * d, bool skipFileInfoValidation) {
	return d != nullptr && d->isValid(skipFileInfoValidation);
}

bool ModDownload::operator == (const ModDownload & d) const {
	return m_fileSize == d.m_fileSize &&
		   m_partNumber == d.m_partNumber &&
		   m_partCount == d.m_partCount &&
		   m_converted == d.m_converted &&
		   m_corrupted == d.m_corrupted &&
		   m_repaired == d.m_repaired &&
		   Utilities::areStringsEqualIgnoreCase(m_fileName, d.m_fileName) &&
		   Utilities::areStringsEqualIgnoreCase(m_version, d.m_version) &&
		   Utilities::areStringsEqualIgnoreCase(m_versionType, d.m_versionType) &&
		   Utilities::areStringsEqualIgnoreCase(m_special, d.m_special) &&
		   Utilities::areStringsEqualIgnoreCase(m_gameVersionID, d.m_gameVersionID) &&
		   Utilities::areStringsEqualIgnoreCase(m_type, d.m_type) &&
		   m_sha1 == d.m_sha1;
}

bool ModDownload::operator != (const ModDownload & d) const {
	return !operator == (d);
}
