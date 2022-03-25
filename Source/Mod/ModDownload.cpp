#include "ModDownload.h"

#include "Mod.h"

#include <Utilities/RapidJSONUtilities.h>
#include <Utilities/StringUtilities.h>

#include <fmt/core.h>
#include <tinyxml2.h>

#include <array>
#include <string_view>

static const std::string XML_MOD_DOWNLOAD_ELEMENT_NAME("download");
static const std::string XML_MOD_DOWNLOAD_FILE_NAME_ATTRIBUTE_NAME("filename");
static const std::string XML_MOD_DOWNLOAD_PART_NUMBER_ATTRIBUTE_NAME("part");
static const std::string XML_MOD_DOWNLOAD_PART_COUNT_ATTRIBUTE_NAME("numparts");
static const std::string XML_MOD_DOWNLOAD_VERSION_ATTRIBUTE_NAME("version");
static const std::string XML_MOD_DOWNLOAD_SPECIAL_ATTRIBUTE_NAME("special");
static const std::string XML_MOD_DOWNLOAD_SUBFOLDER_ATTRIBUTE_NAME("subfolder");
static const std::string XML_MOD_DOWNLOAD_GAME_VERSION_ATTRIBUTE_NAME("game");
static const std::string XML_MOD_DOWNLOAD_TYPE_ATTRIBUTE_NAME("type");
static const std::string XML_MOD_DOWNLOAD_SHA1_ATTRIBUTE_NAME("sha1");
static const std::string XML_MOD_DOWNLOAD_CONVERTED_ATTRIBUTE_NAME("state");
static const std::string XML_MOD_DOWNLOAD_CORRUPTED_ATTRIBUTE_NAME("corrupted");
static const std::string XML_MOD_DOWNLOAD_REPAIRED_ATTRIBUTE_NAME("repaired");
static const std::array<std::string_view, 12> XML_MOD_DOWNLOAD_ATTRIBUTE_NAMES = {
	XML_MOD_DOWNLOAD_FILE_NAME_ATTRIBUTE_NAME,
	XML_MOD_DOWNLOAD_PART_NUMBER_ATTRIBUTE_NAME,
	XML_MOD_DOWNLOAD_PART_COUNT_ATTRIBUTE_NAME,
	XML_MOD_DOWNLOAD_VERSION_ATTRIBUTE_NAME,
	XML_MOD_DOWNLOAD_SPECIAL_ATTRIBUTE_NAME,
	XML_MOD_DOWNLOAD_SUBFOLDER_ATTRIBUTE_NAME,
	XML_MOD_DOWNLOAD_GAME_VERSION_ATTRIBUTE_NAME,
	XML_MOD_DOWNLOAD_TYPE_ATTRIBUTE_NAME,
	XML_MOD_DOWNLOAD_SHA1_ATTRIBUTE_NAME,
	XML_MOD_DOWNLOAD_CONVERTED_ATTRIBUTE_NAME,
	XML_MOD_DOWNLOAD_CORRUPTED_ATTRIBUTE_NAME,
	XML_MOD_DOWNLOAD_REPAIRED_ATTRIBUTE_NAME
};

static constexpr const char * JSON_MOD_DOWNLOAD_FILE_NAME_PROPERTY_NAME = "fileName";
static constexpr const char * JSON_MOD_DOWNLOAD_TYPE_PROPERTY_NAME = "type";
static constexpr const char * JSON_MOD_DOWNLOAD_SHA1_PROPERTY_NAME = "sha1";
static constexpr const char * JSON_MOD_DOWNLOAD_PART_NUMBER_PROPERTY_NAME = "partNumber";
static constexpr const char * JSON_MOD_DOWNLOAD_PART_COUNT_PROPERTY_NAME = "partCount";
static constexpr const char * JSON_MOD_DOWNLOAD_VERSION_PROPERTY_NAME = "version";
static constexpr const char * JSON_MOD_DOWNLOAD_SPECIAL_PROPERTY_NAME = "special";
static constexpr const char * JSON_MOD_DOWNLOAD_SUBFOLDER_PROPERTY_NAME = "subfolder";
static constexpr const char * JSON_MOD_DOWNLOAD_GAME_VERSION_PROPERTY_NAME = "gameVersion";
static constexpr const char * JSON_MOD_DOWNLOAD_CONVERTED_PROPERTY_NAME = "converted";
static constexpr const char * JSON_MOD_DOWNLOAD_CORRUPTED_PROPERTY_NAME = "corrupted";
static constexpr const char * JSON_MOD_DOWNLOAD_REPAIRED_PROPERTY_NAME = "repaired";
static const std::array<std::string_view, 12> JSON_MOD_DOWNLOAD_PROPERTY_NAMES = {
	JSON_MOD_DOWNLOAD_FILE_NAME_PROPERTY_NAME,
	JSON_MOD_DOWNLOAD_TYPE_PROPERTY_NAME,
	JSON_MOD_DOWNLOAD_SHA1_PROPERTY_NAME,
	JSON_MOD_DOWNLOAD_PART_NUMBER_PROPERTY_NAME,
	JSON_MOD_DOWNLOAD_PART_COUNT_PROPERTY_NAME,
	JSON_MOD_DOWNLOAD_VERSION_PROPERTY_NAME,
	JSON_MOD_DOWNLOAD_SPECIAL_PROPERTY_NAME,
	JSON_MOD_DOWNLOAD_SUBFOLDER_PROPERTY_NAME,
	JSON_MOD_DOWNLOAD_GAME_VERSION_PROPERTY_NAME,
	JSON_MOD_DOWNLOAD_CONVERTED_PROPERTY_NAME,
	JSON_MOD_DOWNLOAD_CORRUPTED_PROPERTY_NAME,
	JSON_MOD_DOWNLOAD_REPAIRED_PROPERTY_NAME
};

const std::string ModDownload::ORIGINAL_FILES_TYPE("Original Files");
const std::string ModDownload::MOD_MANAGER_FILES_TYPE("Mod Manager Files");

ModDownload::ModDownload(const std::string & fileName, const std::string & type, const std::string & sha1)
	: m_fileName(Utilities::trimString(fileName))
	, m_partNumber(1)
	, m_partCount(1)
	, m_type(Utilities::trimString(type))
	, m_sha1(Utilities::trimString(sha1))
	, m_parentMod(nullptr) { }

ModDownload::ModDownload(ModDownload && d) noexcept
	: m_fileName(std::move(d.m_fileName))
	, m_partNumber(d.m_partNumber)
	, m_partCount(d.m_partCount)
	, m_version(std::move(d.m_version))
	, m_special(std::move(d.m_special))
	, m_subfolder(std::move(d.m_subfolder))
	, m_gameVersion(std::move(d.m_gameVersion))
	, m_type(std::move(d.m_type))
	, m_sha1(std::move(d.m_sha1))
	, m_converted(d.m_converted)
	, m_corrupted(d.m_corrupted)
	, m_repaired(d.m_repaired)
	, m_parentMod(nullptr) { }

ModDownload::ModDownload(const ModDownload & d)
	: m_fileName(d.m_fileName)
	, m_partNumber(d.m_partNumber)
	, m_partCount(d.m_partCount)
	, m_version(d.m_version)
	, m_special(d.m_special)
	, m_subfolder(d.m_subfolder)
	, m_gameVersion(d.m_gameVersion)
	, m_type(d.m_type)
	, m_sha1(d.m_sha1)
	, m_converted(d.m_converted)
	, m_corrupted(d.m_corrupted)
	, m_repaired(d.m_repaired)
	, m_parentMod(nullptr) { }

ModDownload & ModDownload::operator = (ModDownload && d) noexcept {
	if(this != &d) {
		m_fileName = std::move(d.m_fileName);
		m_partNumber = d.m_partNumber;
		m_partCount = d.m_partCount;
		m_version = std::move(d.m_version);
		m_special = std::move(d.m_special);
		m_subfolder = std::move(d.m_subfolder);
		m_gameVersion = std::move(d.m_gameVersion);
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
	m_partNumber = d.m_partNumber;
	m_partCount = d.m_partCount;
	m_version = d.m_version;
	m_special = d.m_special;
	m_subfolder = d.m_subfolder;
	m_gameVersion = d.m_gameVersion;
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

uint8_t ModDownload::getPartNumber() const {
	return m_partNumber;
}

uint8_t ModDownload::getPartCount() const {
	return m_partCount;
}

const std::string & ModDownload::getVersion() const {
	return m_version;
}

const std::string & ModDownload::getSpecial() const {
	return m_special;
}

const std::string & ModDownload::getSubfolder() const {
	return m_subfolder;
}

const std::string & ModDownload::getGameVersion() const {
	return m_gameVersion;
}

const std::string & ModDownload::getType() const {
	return m_type;
}

bool ModDownload::isOriginalFiles() const {
	return Utilities::compareStringsIgnoreCase(m_type, ORIGINAL_FILES_TYPE) == 0;
}

bool ModDownload::isModManagerFiles() const {
	return Utilities::compareStringsIgnoreCase(m_type, MOD_MANAGER_FILES_TYPE) == 0;
}

const std::string & ModDownload::getSHA1() const {
	return m_sha1;
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

void ModDownload::setFileName(const std::string & fileName) {
	m_fileName = Utilities::trimString(fileName);
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

void ModDownload::setSpecial(const std::string & special) {
	m_special = Utilities::trimString(special);
}

void ModDownload::setSubfolder(const std::string & subfolder) {
	m_subfolder = Utilities::trimString(subfolder);
}

void ModDownload::setGameVersion(const std::string & gameVersion) {
	m_gameVersion = Utilities::trimString(gameVersion);
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

	if(!m_special.empty()) {
		rapidjson::Value specialValue(m_special.c_str(), allocator);
		modDownloadValue.AddMember(rapidjson::StringRef(JSON_MOD_DOWNLOAD_SPECIAL_PROPERTY_NAME), specialValue, allocator);
	}

	if(!m_subfolder.empty()) {
		rapidjson::Value subfolderValue(m_subfolder.c_str(), allocator);
		modDownloadValue.AddMember(rapidjson::StringRef(JSON_MOD_DOWNLOAD_SUBFOLDER_PROPERTY_NAME), subfolderValue, allocator);
	}

	if(!m_gameVersion.empty()) {
		rapidjson::Value gameVersionValue(m_gameVersion.c_str(), allocator);
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

	if(!m_version.empty()) {
		modDownloadElement->SetAttribute(XML_MOD_DOWNLOAD_VERSION_ATTRIBUTE_NAME.c_str(), m_version.c_str());
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

	if(!m_subfolder.empty()) {
		modDownloadElement->SetAttribute(XML_MOD_DOWNLOAD_SUBFOLDER_ATTRIBUTE_NAME.c_str(), m_subfolder.c_str());
	}

	if(!m_gameVersion.empty()) {
		modDownloadElement->SetAttribute(XML_MOD_DOWNLOAD_GAME_VERSION_ATTRIBUTE_NAME.c_str(), m_gameVersion.c_str());
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

std::unique_ptr<ModDownload> ModDownload::parseFrom(const rapidjson::Value & modDownloadValue) {
	if(!modDownloadValue.IsObject()) {
		fmt::print("Invalid mod download type: '{}', expected 'object'.\n", Utilities::typeToString(modDownloadValue.GetType()));
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
			fmt::print("Mod download has unexpected property '{}'.\n", i->name.GetString());
			return nullptr;
		}
	}

	// parse mod download file name
	if(!modDownloadValue.HasMember(JSON_MOD_DOWNLOAD_FILE_NAME_PROPERTY_NAME)) {
		fmt::print("Mod download is missing '{}' property'.\n", JSON_MOD_DOWNLOAD_FILE_NAME_PROPERTY_NAME);
		return nullptr;
	}

	const rapidjson::Value & modDownloadFileNameValue = modDownloadValue[JSON_MOD_DOWNLOAD_FILE_NAME_PROPERTY_NAME];

	if(!modDownloadFileNameValue.IsString()) {
		fmt::print("Mod download has an invalid '{}' property type: '{}', expected 'string'.\n", JSON_MOD_DOWNLOAD_FILE_NAME_PROPERTY_NAME, Utilities::typeToString(modDownloadFileNameValue.GetType()));
		return nullptr;
	}

	std::string modDownloadFileName(Utilities::trimString(modDownloadFileNameValue.GetString()));

	if(modDownloadFileName.empty()) {
		fmt::print("Mod download '{}' property cannot be empty.\n", JSON_MOD_DOWNLOAD_FILE_NAME_PROPERTY_NAME);
		return nullptr;
	}

	// parse mod download type
	if(!modDownloadValue.HasMember(JSON_MOD_DOWNLOAD_TYPE_PROPERTY_NAME)) {
		fmt::print("Mod download is missing '{}' property'.\n", JSON_MOD_DOWNLOAD_TYPE_PROPERTY_NAME);
		return nullptr;
	}

	const rapidjson::Value & modDownloadTypeValue = modDownloadValue[JSON_MOD_DOWNLOAD_TYPE_PROPERTY_NAME];

	if(!modDownloadTypeValue.IsString()) {
		fmt::print("Mod download has an invalid '{}' property type: '{}', expected 'string'.\n", JSON_MOD_DOWNLOAD_TYPE_PROPERTY_NAME, Utilities::typeToString(modDownloadTypeValue.GetType()));
		return nullptr;
	}

	std::string modDownloadType(Utilities::trimString(modDownloadTypeValue.GetString()));

	if(modDownloadType.empty()) {
		fmt::print("Mod download '{}' property cannot be empty.\n", JSON_MOD_DOWNLOAD_TYPE_PROPERTY_NAME);
		return nullptr;
	}

	// parse the mod download sha1 property
	if(!modDownloadValue.HasMember(JSON_MOD_DOWNLOAD_SHA1_PROPERTY_NAME)) {
		fmt::print("Mod download is missing '{}' property'.\n", JSON_MOD_DOWNLOAD_SHA1_PROPERTY_NAME);
		return nullptr;
	}

	const rapidjson::Value & modDownloadSHA1Value = modDownloadValue[JSON_MOD_DOWNLOAD_SHA1_PROPERTY_NAME];

	if(!modDownloadSHA1Value.IsString()) {
		fmt::print("Mod download '{}' property has invalid type: '{}', expected 'string'.\n", JSON_MOD_DOWNLOAD_SHA1_PROPERTY_NAME, Utilities::typeToString(modDownloadSHA1Value.GetType()));
		return nullptr;
	}

	std::string modDownloadSHA1(Utilities::trimString(modDownloadSHA1Value.GetString()));

	if(modDownloadSHA1.empty()) {
		fmt::print("Mod download '{}' property cannot be empty.\n", JSON_MOD_DOWNLOAD_SHA1_PROPERTY_NAME);
		return nullptr;
	}

	// initialize the mod download
	std::unique_ptr<ModDownload> newModDownload = std::make_unique<ModDownload>(modDownloadFileName, modDownloadType, modDownloadSHA1);

	bool error = false;

	// parse the mod download part number property
	if(modDownloadValue.HasMember(JSON_MOD_DOWNLOAD_PART_COUNT_PROPERTY_NAME)) {
		const rapidjson::Value & modDownloadPartCountValue = modDownloadValue[JSON_MOD_DOWNLOAD_PART_COUNT_PROPERTY_NAME];

		if(!modDownloadPartCountValue.IsUint()) {
			fmt::print("Mod download has an invalid '{}' property type: '{}', expected unsigned integer 'number'.\n", JSON_MOD_DOWNLOAD_PART_COUNT_PROPERTY_NAME, Utilities::typeToString(modDownloadPartCountValue.GetType()));
			return nullptr;
		}

		uint32_t partCount = modDownloadPartCountValue.GetUint();

		if(error || partCount < 1 || partCount > std::numeric_limits<uint8_t>::max()) {
			fmt::print("Mod download '{}' property value has an invalid value: '{}', expected unsigned integer 'number' between 1 and {} inclusively.\n", JSON_MOD_DOWNLOAD_PART_COUNT_PROPERTY_NAME, partCount, std::numeric_limits<uint8_t>::max());
			return nullptr;
		}

		newModDownload->setPartCount(static_cast<uint8_t>(partCount));
	}

	// parse the mod download part part number property
	if(modDownloadValue.HasMember(JSON_MOD_DOWNLOAD_PART_NUMBER_PROPERTY_NAME)) {
		const rapidjson::Value & modDownloadPartNumberValue = modDownloadValue[JSON_MOD_DOWNLOAD_PART_NUMBER_PROPERTY_NAME];

		if(!modDownloadPartNumberValue.IsUint()) {
			fmt::print("Mod download has an invalid '{}' property type: '{}', expected unsigned integer 'number'.\n", JSON_MOD_DOWNLOAD_PART_NUMBER_PROPERTY_NAME, Utilities::typeToString(modDownloadPartNumberValue.GetType()));
			return nullptr;
		}

		uint32_t partNumber = modDownloadPartNumberValue.GetUint();

		if(error || partNumber < 1 || partNumber > std::numeric_limits<uint8_t>::max()) {
			fmt::print("Mod download '{}' property value has an invalid value: '{}', expected unsigned integer 'number' between 1 and {} inclusively.\n", JSON_MOD_DOWNLOAD_PART_NUMBER_PROPERTY_NAME, partNumber, std::numeric_limits<uint8_t>::max());
			return nullptr;
		}

		if(partNumber > newModDownload->m_partCount) {
			fmt::print("Attribute '{}' in element '{}' has an invalid value: '{}' which exceeds the '{}' value of '{}'.\n", XML_MOD_DOWNLOAD_PART_NUMBER_ATTRIBUTE_NAME, XML_MOD_DOWNLOAD_ELEMENT_NAME, partNumber, XML_MOD_DOWNLOAD_PART_COUNT_ATTRIBUTE_NAME, newModDownload->m_partCount);
			return nullptr;
		}

		newModDownload->setPartNumber(static_cast<uint8_t>(partNumber));
	}

	// parse the mod download version property
	if(modDownloadValue.HasMember(JSON_MOD_DOWNLOAD_VERSION_PROPERTY_NAME)) {
		const rapidjson::Value & modDownloadVersionValue = modDownloadValue[JSON_MOD_DOWNLOAD_VERSION_PROPERTY_NAME];

		if(!modDownloadVersionValue.IsString()) {
			fmt::print("Mod download '{}' property has invalid type: '{}', expected 'string'.\n", JSON_MOD_DOWNLOAD_VERSION_PROPERTY_NAME, Utilities::typeToString(modDownloadVersionValue.GetType()));
			return nullptr;
		}

		newModDownload->setVersion(modDownloadVersionValue.GetString());
	}

	// parse the mod download special property
	if(modDownloadValue.HasMember(JSON_MOD_DOWNLOAD_SPECIAL_PROPERTY_NAME)) {
		const rapidjson::Value & modDownloadSpecialValue = modDownloadValue[JSON_MOD_DOWNLOAD_SPECIAL_PROPERTY_NAME];

		if(!modDownloadSpecialValue.IsString()) {
			fmt::print("Mod download '{}' property has invalid type: '{}', expected 'string'.\n", JSON_MOD_DOWNLOAD_SPECIAL_PROPERTY_NAME, Utilities::typeToString(modDownloadSpecialValue.GetType()));
			return nullptr;
		}

		newModDownload->setSpecial(modDownloadSpecialValue.GetString());
	}

	// parse the mod download subfolder property
	if(modDownloadValue.HasMember(JSON_MOD_DOWNLOAD_SUBFOLDER_PROPERTY_NAME)) {
		const rapidjson::Value & modDownloadSubfolderValue = modDownloadValue[JSON_MOD_DOWNLOAD_SUBFOLDER_PROPERTY_NAME];

		if(!modDownloadSubfolderValue.IsString()) {
			fmt::print("Mod download '{}' property has invalid type: '{}', expected 'string'.\n", JSON_MOD_DOWNLOAD_SUBFOLDER_PROPERTY_NAME, Utilities::typeToString(modDownloadSubfolderValue.GetType()));
			return nullptr;
		}

		newModDownload->setSubfolder(modDownloadSubfolderValue.GetString());
	}

	// parse the mod download game version property
	if(modDownloadValue.HasMember(JSON_MOD_DOWNLOAD_GAME_VERSION_PROPERTY_NAME)) {
		const rapidjson::Value & modDownloadGameVersionValue = modDownloadValue[JSON_MOD_DOWNLOAD_GAME_VERSION_PROPERTY_NAME];

		if(!modDownloadGameVersionValue.IsString()) {
			fmt::print("Mod download '{}' property has invalid type: '{}', expected 'string'.\n", JSON_MOD_DOWNLOAD_GAME_VERSION_PROPERTY_NAME, Utilities::typeToString(modDownloadGameVersionValue.GetType()));
			return nullptr;
		}

		newModDownload->setGameVersion(modDownloadGameVersionValue.GetString());
	}

	// parse the mod download converted property
	if(modDownloadValue.HasMember(JSON_MOD_DOWNLOAD_CONVERTED_PROPERTY_NAME)) {
		const rapidjson::Value & modDownloadConvertedValue = modDownloadValue[JSON_MOD_DOWNLOAD_CONVERTED_PROPERTY_NAME];

		if(!modDownloadConvertedValue.IsBool()) {
			fmt::print("Mod download '{}' property has invalid type: '{}', expected 'boolean'.\n", JSON_MOD_DOWNLOAD_CONVERTED_PROPERTY_NAME, Utilities::typeToString(modDownloadConvertedValue.GetType()));
			return nullptr;
		}

		newModDownload->setConverted(modDownloadConvertedValue.GetBool());
	}

	// parse the mod download corrupted property
	if(modDownloadValue.HasMember(JSON_MOD_DOWNLOAD_CORRUPTED_PROPERTY_NAME)) {
		const rapidjson::Value & modDownloadCorruptedValue = modDownloadValue[JSON_MOD_DOWNLOAD_CORRUPTED_PROPERTY_NAME];

		if(!modDownloadCorruptedValue.IsBool()) {
			fmt::print("Mod download '{}' property has invalid type: '{}', expected 'boolean'.\n", JSON_MOD_DOWNLOAD_CORRUPTED_PROPERTY_NAME, Utilities::typeToString(modDownloadCorruptedValue.GetType()));
			return nullptr;
		}

		newModDownload->setCorrupted(modDownloadCorruptedValue.GetBool());
	}

	// parse the mod download repaired property
	if(modDownloadValue.HasMember(JSON_MOD_DOWNLOAD_REPAIRED_PROPERTY_NAME)) {
		const rapidjson::Value & modDownloadRepairedValue = modDownloadValue[JSON_MOD_DOWNLOAD_REPAIRED_PROPERTY_NAME];

		if(!modDownloadRepairedValue.IsBool()) {
			fmt::print("Mod download '{}' property has invalid type: '{}', expected 'boolean'.\n", JSON_MOD_DOWNLOAD_REPAIRED_PROPERTY_NAME, Utilities::typeToString(modDownloadRepairedValue.GetType()));
			return nullptr;
		}

		newModDownload->setRepaired(modDownloadRepairedValue.GetBool());
	}

	return newModDownload;
}

std::unique_ptr<ModDownload> ModDownload::parseFrom(const tinyxml2::XMLElement * modDownloadElement) {
	if(modDownloadElement == nullptr) {
		return nullptr;
	}

	// verify element name
	if(modDownloadElement->Name() != XML_MOD_DOWNLOAD_ELEMENT_NAME) {
		fmt::print("Invalid mod download element name: '{}', expected '{}'.\n", modDownloadElement->Name(), XML_MOD_DOWNLOAD_ELEMENT_NAME);
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
			fmt::print("Element '{}' has unexpected attribute '{}'.\n", XML_MOD_DOWNLOAD_ELEMENT_NAME, modDownloadAttribute->Name());
			return nullptr;
		}

		modDownloadAttribute = modDownloadAttribute->Next();
	}

	// check for unexpected mod download element child elements
	if(modDownloadElement->FirstChildElement() != nullptr) {
		fmt::print("Element '{}' has an unexpected child element.\n", XML_MOD_DOWNLOAD_ELEMENT_NAME);
		return nullptr;
	}

	// read the mod download attributes
	const char * modDownloadFileName = modDownloadElement->Attribute(XML_MOD_DOWNLOAD_FILE_NAME_ATTRIBUTE_NAME.c_str());

	if(modDownloadFileName == nullptr || Utilities::stringLength(modDownloadFileName) == 0) {
		fmt::print("Attribute '{}' is missing from '{}' element.\n", XML_MOD_DOWNLOAD_FILE_NAME_ATTRIBUTE_NAME, XML_MOD_DOWNLOAD_ELEMENT_NAME);
		return nullptr;
	}

	const char * modDownloadType = modDownloadElement->Attribute(XML_MOD_DOWNLOAD_TYPE_ATTRIBUTE_NAME.c_str());

	if(modDownloadType == nullptr || Utilities::stringLength(modDownloadType) == 0) {
		fmt::print("Attribute '{}' is missing from '{}' element.\n", XML_MOD_DOWNLOAD_TYPE_ATTRIBUTE_NAME, XML_MOD_DOWNLOAD_ELEMENT_NAME);
		return nullptr;
	}

	const char * modDownloadSHA1 = modDownloadElement->Attribute(XML_MOD_DOWNLOAD_SHA1_ATTRIBUTE_NAME.c_str());

	if(modDownloadSHA1 == nullptr || Utilities::stringLength(modDownloadSHA1) == 0) {
		fmt::print("Attribute '{}' is missing from '{}' element.\n", XML_MOD_DOWNLOAD_SHA1_ATTRIBUTE_NAME, XML_MOD_DOWNLOAD_ELEMENT_NAME);
		return nullptr;
	}

	const char * modDownloadVersion = modDownloadElement->Attribute(XML_MOD_DOWNLOAD_VERSION_ATTRIBUTE_NAME.c_str());
	const char * modDownloadSpecial = modDownloadElement->Attribute(XML_MOD_DOWNLOAD_SPECIAL_ATTRIBUTE_NAME.c_str());
	const char * modDownloadSubfolder = modDownloadElement->Attribute(XML_MOD_DOWNLOAD_SUBFOLDER_ATTRIBUTE_NAME.c_str());
	const char * modDownloadGameVersion = modDownloadElement->Attribute(XML_MOD_DOWNLOAD_GAME_VERSION_ATTRIBUTE_NAME.c_str());
	const char * modDownloadConvertedData = modDownloadElement->Attribute(XML_MOD_DOWNLOAD_CONVERTED_ATTRIBUTE_NAME.c_str());
	const char * modDownloadPartNumberData = modDownloadElement->Attribute(XML_MOD_DOWNLOAD_PART_NUMBER_ATTRIBUTE_NAME.c_str());
	const char * modDownloadPartCountData = modDownloadElement->Attribute(XML_MOD_DOWNLOAD_PART_COUNT_ATTRIBUTE_NAME.c_str());
	const char * modDownloadCorruptedData = modDownloadElement->Attribute(XML_MOD_DOWNLOAD_CORRUPTED_ATTRIBUTE_NAME.c_str());
	const char * modDownloadRepairedData = modDownloadElement->Attribute(XML_MOD_DOWNLOAD_REPAIRED_ATTRIBUTE_NAME.c_str());

	// initialize the mod download
	std::unique_ptr<ModDownload> newModDownload = std::make_unique<ModDownload>(modDownloadFileName, modDownloadType, modDownloadSHA1 == nullptr ? "" : modDownloadSHA1);

	if(modDownloadVersion != nullptr) {
		newModDownload->setVersion(modDownloadVersion);
	}

	if(modDownloadSpecial != nullptr) {
		newModDownload->setSpecial(modDownloadSpecial);
	}

	if(modDownloadSubfolder != nullptr) {
		newModDownload->setSubfolder(modDownloadSubfolder);
	}

	if(modDownloadGameVersion != nullptr) {
		newModDownload->setGameVersion(modDownloadGameVersion);
	}

	bool error = false;

	if(modDownloadPartCountData != nullptr) {
		uint32_t partCount = Utilities::parseUnsignedInteger(modDownloadPartCountData, &error);

		if(error || partCount < 1 || partCount > std::numeric_limits<uint8_t>::max()) {
			fmt::print("Attribute '{}' in element '{}' has an invalid value: '{}', expected integer number between 1 and {} inclusively.\n", XML_MOD_DOWNLOAD_PART_COUNT_ATTRIBUTE_NAME, XML_MOD_DOWNLOAD_ELEMENT_NAME, modDownloadPartCountData, std::numeric_limits<uint8_t>::max());
			return nullptr;
		}

		newModDownload->setPartCount(static_cast<uint8_t>(partCount));
	}

	if(modDownloadPartNumberData != nullptr) {
		uint32_t partNumber = Utilities::parseUnsignedInteger(modDownloadPartNumberData, &error);

		if(error || partNumber < 1 || partNumber > std::numeric_limits<uint8_t>::max()) {
			fmt::print("Attribute '{}' in element '{}' has an invalid value: '{}', expected integer number between 1 and {} inclusively.\n", XML_MOD_DOWNLOAD_PART_NUMBER_ATTRIBUTE_NAME, XML_MOD_DOWNLOAD_ELEMENT_NAME, modDownloadPartNumberData, std::numeric_limits<uint8_t>::max());
			return nullptr;
		}

		if(partNumber > newModDownload->m_partCount) {
			fmt::print("Attribute '{}' in element '{}' has an invalid value: '{}' which exceeds the '{}' value of '{}'.\n", XML_MOD_DOWNLOAD_PART_NUMBER_ATTRIBUTE_NAME, XML_MOD_DOWNLOAD_ELEMENT_NAME, partNumber, XML_MOD_DOWNLOAD_PART_COUNT_ATTRIBUTE_NAME, newModDownload->m_partCount);
			return nullptr;
		}

		newModDownload->setPartNumber(static_cast<uint8_t>(partNumber));
	}

	if(modDownloadConvertedData != nullptr) {
		if(Utilities::compareStringsIgnoreCase(modDownloadConvertedData, "Converted") == 0) {
			newModDownload->setConverted(true);
		}
		else if(Utilities::compareStringsIgnoreCase(modDownloadConvertedData, "Native") == 0) {
			newModDownload->setConverted(false);
		}
		else {
			fmt::print("Attribute '{}' in element '{}' has an invalid value: '{}', expected 'Native' or 'Converted'.\n", XML_MOD_DOWNLOAD_CONVERTED_ATTRIBUTE_NAME, XML_MOD_DOWNLOAD_ELEMENT_NAME, modDownloadConvertedData);
			return nullptr;
		}
	}

	if(modDownloadCorruptedData != nullptr) {
		bool corrupted = Utilities::parseBoolean(modDownloadCorruptedData, &error);

		if(error) {
			fmt::print("Attribute '{}' in element '{}' has an invalid value: '{}', expected boolean.\n", XML_MOD_DOWNLOAD_CORRUPTED_ATTRIBUTE_NAME, XML_MOD_DOWNLOAD_ELEMENT_NAME, modDownloadCorruptedData);
			return nullptr;
		}

		newModDownload->setCorrupted(corrupted);
	}

	if(modDownloadRepairedData != nullptr) {
		bool repaired = Utilities::parseBoolean(modDownloadRepairedData, &error);

		if(error) {
			fmt::print("Attribute '{}' in element '{}' has an invalid value: '{}', expected boolean.\n", XML_MOD_DOWNLOAD_REPAIRED_ATTRIBUTE_NAME, XML_MOD_DOWNLOAD_ELEMENT_NAME, modDownloadRepairedData);
			return nullptr;
		}

		newModDownload->setRepaired(repaired);
	}

	return newModDownload;
}

bool ModDownload::isValid() const {
	return !m_fileName.empty() &&
		   !m_type.empty() ||
		   m_partNumber > m_partCount &&
		   !m_sha1.empty();
}

bool ModDownload::isValid(const ModDownload * d) {
	return d != nullptr && d->isValid();
}

bool ModDownload::operator == (const ModDownload & d) const {
	return Utilities::compareStringsIgnoreCase(m_fileName, d.m_fileName) == 0 &&
		   m_partNumber == d.m_partNumber &&
		   m_partCount == d.m_partCount &&
		   m_converted == d.m_converted &&
		   m_corrupted == d.m_corrupted &&
		   m_repaired == d.m_repaired &&
		   Utilities::compareStringsIgnoreCase(m_version, d.m_version) == 0 &&
		   Utilities::compareStringsIgnoreCase(m_special, d.m_special) == 0 &&
		   Utilities::compareStringsIgnoreCase(m_subfolder, d.m_subfolder) == 0 &&
		   Utilities::compareStringsIgnoreCase(m_gameVersion, d.m_gameVersion) == 0 &&
		   Utilities::compareStringsIgnoreCase(m_type, d.m_type) == 0 &&
		   m_sha1 == d.m_sha1;
}

bool ModDownload::operator != (const ModDownload & d) const {
	return !operator == (d);
}
