#include "GameDownloadFile.h"

#include "GameDownload.h"
#include "GameDownloadVersion.h"

#include <Utilities/FileUtilities.h>
#include <Utilities/RapidJSONUtilities.h>
#include <Utilities/StringUtilities.h>

#include <magic_enum.hpp>
#include <spdlog/spdlog.h>

#include <array>
#include <string_view>

static constexpr const char * JSON_GAME_DOWNLOAD_FILE_FILE_NAME_PROPERTY_NAME = "fileName";
static constexpr const char * JSON_GAME_DOWNLOAD_FILE_FILE_SIZE_PROPERTY_NAME = "fileSize";
static constexpr const char * JSON_GAME_DOWNLOAD_FILE_TYPE_PROPERTY_NAME = "type";
static constexpr const char * JSON_GAME_DOWNLOAD_FILE_SHA1_PROPERTY_NAME = "sha1";
static constexpr const char * JSON_GAME_DOWNLOAD_FILE_OPERATING_SYSTEM_PROPERTY_NAME = "operatingSystem";
static constexpr const char * JSON_GAME_DOWNLOAD_FILE_PROCESSOR_ARCHITECTURE_PROPERTY_NAME = "processorArchitecture";
static const std::array<std::string_view, 6> JSON_GAME_DOWNLOAD_FILE_PROPERTY_NAMES = {
	JSON_GAME_DOWNLOAD_FILE_FILE_NAME_PROPERTY_NAME,
	JSON_GAME_DOWNLOAD_FILE_FILE_SIZE_PROPERTY_NAME,
	JSON_GAME_DOWNLOAD_FILE_TYPE_PROPERTY_NAME,
	JSON_GAME_DOWNLOAD_FILE_SHA1_PROPERTY_NAME,
	JSON_GAME_DOWNLOAD_FILE_OPERATING_SYSTEM_PROPERTY_NAME,
	JSON_GAME_DOWNLOAD_FILE_PROCESSOR_ARCHITECTURE_PROPERTY_NAME
};

GameDownloadFile::GameDownloadFile(const std::string & fileName, uint64_t fileSize, Type type, const std::string & sha1, GameVersion::OperatingSystem operatingSystem, std::optional<ProcessorArchitecture> processorArchitecture)
	: m_fileName(Utilities::trimString(fileName))
	, m_fileSize(fileSize)
	, m_type(type)
	, m_sha1(Utilities::trimString(sha1))
	, m_operatingSystem(operatingSystem)
	, m_processorArchitecture(processorArchitecture)
	, m_parentGameDownloadVersion(nullptr) { }

GameDownloadFile::GameDownloadFile(GameDownloadFile && f) noexcept
	: m_fileName(std::move(f.m_fileName))
	, m_fileSize(f.m_fileSize)
	, m_type(f.m_type)
	, m_sha1(std::move(f.m_sha1))
	, m_operatingSystem(f.m_operatingSystem)
	, m_processorArchitecture(f.m_processorArchitecture)
	, m_parentGameDownloadVersion(nullptr) { }

GameDownloadFile::GameDownloadFile(const GameDownloadFile & f)
	: m_fileName(f.m_fileName)
	, m_fileSize(f.m_fileSize)
	, m_type(f.m_type)
	, m_sha1(f.m_sha1)
	, m_operatingSystem(f.m_operatingSystem)
	, m_processorArchitecture(f.m_processorArchitecture)
	, m_parentGameDownloadVersion(nullptr) { }

GameDownloadFile & GameDownloadFile::operator = (GameDownloadFile && f) noexcept {
	if(this != &f) {
		m_fileName = std::move(f.m_fileName);
		m_fileSize = f.m_fileSize;
		m_type = f.m_type;
		m_sha1 = std::move(f.m_sha1);
		m_operatingSystem = f.m_operatingSystem;
		m_processorArchitecture = f.m_processorArchitecture;
	}

	return *this;
}

GameDownloadFile & GameDownloadFile::operator = (const GameDownloadFile & f) {
	m_fileName = f.m_fileName;
	m_fileSize = f.m_fileSize;
	m_type = f.m_type;
	m_sha1 = f.m_sha1;
	m_operatingSystem = f.m_operatingSystem;
	m_processorArchitecture = f.m_processorArchitecture;

	return *this;
}

GameDownloadFile::~GameDownloadFile() {
	m_parentGameDownloadVersion = nullptr;
}

const std::string & GameDownloadFile::getFileName() const {
	return m_fileName;
}

std::string_view GameDownloadFile::getFileExtension() const {
	return Utilities::getFileExtension(m_fileName);
}

uint64_t GameDownloadFile::getFileSize() const {
	return m_fileSize;
}

GameDownloadFile::Type GameDownloadFile::getType() const {
	return m_type;
}

const std::string & GameDownloadFile::getSHA1() const {
	return m_sha1;
}

GameVersion::OperatingSystem GameDownloadFile::getOperatingSystem() const {
	return m_operatingSystem;
}

bool GameDownloadFile::hasProcessorArchitecture() const {
	return m_processorArchitecture.has_value();
}

std::optional<GameDownloadFile::ProcessorArchitecture> GameDownloadFile::getProcessorArchitecture() const {
	return m_processorArchitecture;
}

const GameDownload * GameDownloadFile::getParentGameDownload() const {
	if(m_parentGameDownloadVersion == nullptr) {
		return nullptr;
	}

	return m_parentGameDownloadVersion->getParentGameDownload();
}

const GameDownloadVersion * GameDownloadFile::getParentGameDownloadVersion() const {
	return m_parentGameDownloadVersion;
}

void GameDownloadFile::setParentGameDownloadVersion(const GameDownloadVersion * gameDownloadVersion) {
	m_parentGameDownloadVersion = gameDownloadVersion;
}

rapidjson::Value GameDownloadFile::toJSON(rapidjson::MemoryPoolAllocator<rapidjson::CrtAllocator> & allocator) const {
	rapidjson::Value gameDownloadFileValue(rapidjson::kObjectType);

	rapidjson::Value fileNameValue(m_fileName.c_str(), allocator);
	gameDownloadFileValue.AddMember(rapidjson::StringRef(JSON_GAME_DOWNLOAD_FILE_FILE_NAME_PROPERTY_NAME), fileNameValue, allocator);

	gameDownloadFileValue.AddMember(rapidjson::StringRef(JSON_GAME_DOWNLOAD_FILE_FILE_SIZE_PROPERTY_NAME), rapidjson::Value(m_fileSize), allocator);

	rapidjson::Value typeValue(std::string(magic_enum::enum_name(m_type)).c_str(), allocator);
	gameDownloadFileValue.AddMember(rapidjson::StringRef(JSON_GAME_DOWNLOAD_FILE_TYPE_PROPERTY_NAME), typeValue, allocator);

	rapidjson::Value sha1Value(m_sha1.c_str(), allocator);
	gameDownloadFileValue.AddMember(rapidjson::StringRef(JSON_GAME_DOWNLOAD_FILE_SHA1_PROPERTY_NAME), sha1Value, allocator);

	rapidjson::Value operatingSystemValue(std::string(magic_enum::enum_name(m_operatingSystem)).c_str(), allocator);
	gameDownloadFileValue.AddMember(rapidjson::StringRef(JSON_GAME_DOWNLOAD_FILE_OPERATING_SYSTEM_PROPERTY_NAME), operatingSystemValue, allocator);

	if(m_processorArchitecture.has_value()) {
		rapidjson::Value processorArchitectureValue(std::string(magic_enum::enum_name(m_processorArchitecture.value())).c_str(), allocator);
		gameDownloadFileValue.AddMember(rapidjson::StringRef(JSON_GAME_DOWNLOAD_FILE_PROCESSOR_ARCHITECTURE_PROPERTY_NAME), processorArchitectureValue, allocator);
	}

	return gameDownloadFileValue;
}

std::unique_ptr<GameDownloadFile> GameDownloadFile::parseFrom(const rapidjson::Value & gameDownloadFileValue) {
	if(!gameDownloadFileValue.IsObject()) {
		spdlog::error("Invalid game download file type: '{}', expected 'object'.", Utilities::typeToString(gameDownloadFileValue.GetType()));
		return nullptr;
	}

	// check for unhandled game download file properties
	bool propertyHandled = false;

	for(rapidjson::Value::ConstMemberIterator i = gameDownloadFileValue.MemberBegin(); i != gameDownloadFileValue.MemberEnd(); ++i) {
		propertyHandled = false;

		for(const std::string_view propertyName : JSON_GAME_DOWNLOAD_FILE_PROPERTY_NAMES) {
			if(i->name.GetString() == propertyName) {
				propertyHandled = true;
				break;
			}
		}

		if(!propertyHandled) {
			spdlog::warn("Game download file has unexpected property '{}'.", i->name.GetString());
		}
	}

	// parse game download file name
	if(!gameDownloadFileValue.HasMember(JSON_GAME_DOWNLOAD_FILE_FILE_NAME_PROPERTY_NAME)) {
		spdlog::error("Game download file is missing '{}' property'.", JSON_GAME_DOWNLOAD_FILE_FILE_NAME_PROPERTY_NAME);
		return nullptr;
	}

	const rapidjson::Value & fileNameValue = gameDownloadFileValue[JSON_GAME_DOWNLOAD_FILE_FILE_NAME_PROPERTY_NAME];

	if(!fileNameValue.IsString()) {
		spdlog::error("Game download file has an invalid '{}' property type: '{}', expected 'string'.", JSON_GAME_DOWNLOAD_FILE_FILE_NAME_PROPERTY_NAME, Utilities::typeToString(fileNameValue.GetType()));
		return nullptr;
	}

	std::string fileName(Utilities::trimString(fileNameValue.GetString()));

	if(fileName.empty()) {
		spdlog::error("Game download file '{}' property cannot be empty.", JSON_GAME_DOWNLOAD_FILE_FILE_NAME_PROPERTY_NAME);
		return nullptr;
	}

	// parse game download file size
	if(!gameDownloadFileValue.HasMember(JSON_GAME_DOWNLOAD_FILE_FILE_SIZE_PROPERTY_NAME)) {
		spdlog::error("Game download file is missing '{}' property'.", JSON_GAME_DOWNLOAD_FILE_FILE_SIZE_PROPERTY_NAME);
		return nullptr;
	}

	const rapidjson::Value & fileSizeValue = gameDownloadFileValue[JSON_GAME_DOWNLOAD_FILE_FILE_SIZE_PROPERTY_NAME];

	if(!fileSizeValue.IsUint64()) {
		spdlog::error("Game download file has an invalid '{}' property type: '{}', expected unsigned integer 'number'.", JSON_GAME_DOWNLOAD_FILE_FILE_SIZE_PROPERTY_NAME, Utilities::typeToString(fileSizeValue.GetType()));
		return nullptr;
	}

	uint64_t fileSize = fileSizeValue.GetUint64();

	// parse the game download file type
	if(!gameDownloadFileValue.HasMember(JSON_GAME_DOWNLOAD_FILE_TYPE_PROPERTY_NAME)) {
		spdlog::error("Game download file is missing '{}' property'.", JSON_GAME_DOWNLOAD_FILE_TYPE_PROPERTY_NAME);
		return nullptr;
	}

	const rapidjson::Value & typeNameValue = gameDownloadFileValue[JSON_GAME_DOWNLOAD_FILE_TYPE_PROPERTY_NAME];

	if(!typeNameValue.IsString()) {
		spdlog::error("Game download file has an invalid '{}' property type: '{}', expected 'string'.", JSON_GAME_DOWNLOAD_FILE_TYPE_PROPERTY_NAME, Utilities::typeToString(typeNameValue.GetType()));
		return nullptr;
	}

	std::string typeName(Utilities::trimString(typeNameValue.GetString()));
	std::optional<Type> optionalType(magic_enum::enum_cast<Type>(typeName));

	if(!optionalType.has_value()) {
		spdlog::error("Game download file has an invalid '{}' property value: '{}'.", JSON_GAME_DOWNLOAD_FILE_TYPE_PROPERTY_NAME, typeName);
		return nullptr;
	}

	// parse the game download file sha1 property
	if(!gameDownloadFileValue.HasMember(JSON_GAME_DOWNLOAD_FILE_SHA1_PROPERTY_NAME)) {
		spdlog::error("Game download file is missing '{}' property'.", JSON_GAME_DOWNLOAD_FILE_SHA1_PROPERTY_NAME);
		return nullptr;
	}

	const rapidjson::Value & fileSHA1Value = gameDownloadFileValue[JSON_GAME_DOWNLOAD_FILE_SHA1_PROPERTY_NAME];

	if(!fileSHA1Value.IsString()) {
		spdlog::error("Game download file '{}' property has invalid type: '{}', expected 'string'.", JSON_GAME_DOWNLOAD_FILE_SHA1_PROPERTY_NAME, Utilities::typeToString(fileSHA1Value.GetType()));
		return nullptr;
	}

	std::string fileSHA1(Utilities::trimString(fileSHA1Value.GetString()));

	if(fileSHA1.empty()) {
		spdlog::error("Game download file '{}' property cannot be empty.", JSON_GAME_DOWNLOAD_FILE_SHA1_PROPERTY_NAME);
		return nullptr;
	}

	// parse the game download file operating system
	if(!gameDownloadFileValue.HasMember(JSON_GAME_DOWNLOAD_FILE_OPERATING_SYSTEM_PROPERTY_NAME)) {
		spdlog::error("Game download file is missing '{}' property'.", JSON_GAME_DOWNLOAD_FILE_OPERATING_SYSTEM_PROPERTY_NAME);
		return nullptr;
	}

	const rapidjson::Value & operatingSystemNameValue = gameDownloadFileValue[JSON_GAME_DOWNLOAD_FILE_OPERATING_SYSTEM_PROPERTY_NAME];

	if(!operatingSystemNameValue.IsString()) {
		spdlog::error("Game download file has an invalid '{}' property type: '{}', expected 'string'.", JSON_GAME_DOWNLOAD_FILE_OPERATING_SYSTEM_PROPERTY_NAME, Utilities::typeToString(operatingSystemNameValue.GetType()));
		return nullptr;
	}

	std::string operatingSystemName(Utilities::trimString(operatingSystemNameValue.GetString()));
	std::optional<GameVersion::OperatingSystem> optionalOperatingSystem(magic_enum::enum_cast<GameVersion::OperatingSystem>(operatingSystemName));

	if(!optionalOperatingSystem.has_value()) {
		spdlog::error("Game download file has an invalid '{}' property value: '{}'.", JSON_GAME_DOWNLOAD_FILE_OPERATING_SYSTEM_PROPERTY_NAME, operatingSystemName);
		return nullptr;
	}

	// parse the game download file processor architecture
	std::optional<ProcessorArchitecture> optionalProcessorArchitecture;

	if(gameDownloadFileValue.HasMember(JSON_GAME_DOWNLOAD_FILE_PROCESSOR_ARCHITECTURE_PROPERTY_NAME)) {
		const rapidjson::Value & processorArchitectureNameValue = gameDownloadFileValue[JSON_GAME_DOWNLOAD_FILE_PROCESSOR_ARCHITECTURE_PROPERTY_NAME];

		if(!processorArchitectureNameValue.IsString()) {
			spdlog::error("Game download file has an invalid '{}' property type: '{}', expected 'string'.", JSON_GAME_DOWNLOAD_FILE_PROCESSOR_ARCHITECTURE_PROPERTY_NAME, Utilities::typeToString(processorArchitectureNameValue.GetType()));
			return nullptr;
		}

		std::string processorArchitectureName(Utilities::trimString(processorArchitectureNameValue.GetString()));
		optionalProcessorArchitecture = magic_enum::enum_cast<ProcessorArchitecture>(processorArchitectureName);

		if(!optionalProcessorArchitecture.has_value()) {
			spdlog::error("Game download file has an invalid '{}' property value: '{}'.", JSON_GAME_DOWNLOAD_FILE_PROCESSOR_ARCHITECTURE_PROPERTY_NAME, processorArchitectureName);
			return nullptr;
		}
	}

	// initialize the game download file
	return std::make_unique<GameDownloadFile>(fileName, fileSize, optionalType.value(), fileSHA1, optionalOperatingSystem.value(), optionalProcessorArchitecture);
}

std::optional<GameDownloadFile::ProcessorArchitecture> GameDownloadFile::convertOperatingSystemArchitectureType(DeviceInformationBridge::OperatingSystemArchitectureType operatingSystemArchitectureType) {
	switch(operatingSystemArchitectureType) {
		case DeviceInformationBridge::OperatingSystemArchitectureType::x86: {
			return ProcessorArchitecture::x86;
		}
		case DeviceInformationBridge::OperatingSystemArchitectureType::x64: {
			return ProcessorArchitecture::x64;
		}
	}

	return {};
}

bool GameDownloadFile::isValid() const {
	return !m_fileName.empty() &&
		   m_fileSize != 0 &&
		   !m_sha1.empty();
}

bool GameDownloadFile::isValid(const GameDownloadFile * f) {
	return f != nullptr && f->isValid();
}

bool GameDownloadFile::operator == (const GameDownloadFile & f) const {
	return Utilities::areStringsEqualIgnoreCase(m_fileName, f.m_fileName) &&
		   m_fileSize == f.m_fileSize &&
		   m_type == f.m_type &&
		   m_sha1 == f.m_sha1 &&
		   m_operatingSystem == f.m_operatingSystem &&
		   m_processorArchitecture == f.m_processorArchitecture;
}

bool GameDownloadFile::operator != (const GameDownloadFile & f) const {
	return !operator == (f);
}
