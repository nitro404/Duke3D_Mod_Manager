#include "DOSBoxDownloadFile.h"

#include "DOSBoxDownload.h"
#include "DOSBoxDownloadVersion.h"

#include <Utilities/FileUtilities.h>
#include <Utilities/RapidJSONUtilities.h>
#include <Utilities/StringUtilities.h>

#include <magic_enum.hpp>
#include <spdlog/spdlog.h>

#include <array>
#include <string_view>

static constexpr const char * JSON_DOSBOX_DOWNLOAD_FILE_FILE_NAME_PROPERTY_NAME = "fileName";
static constexpr const char * JSON_DOSBOX_DOWNLOAD_FILE_FILE_SIZE_PROPERTY_NAME = "fileSize";
static constexpr const char * JSON_DOSBOX_DOWNLOAD_FILE_SHA1_PROPERTY_NAME = "sha1";
static constexpr const char * JSON_DOSBOX_DOWNLOAD_FILE_OPERATING_SYSTEM_PROPERTY_NAME = "operatingSystem";
static constexpr const char * JSON_DOSBOX_DOWNLOAD_FILE_PROCESSOR_ARCHITECTURE_PROPERTY_NAME = "processorArchitecture";
static const std::array<std::string_view, 5> JSON_DOSBOX_DOWNLOAD_FILE_PROPERTY_NAMES = {
	JSON_DOSBOX_DOWNLOAD_FILE_FILE_NAME_PROPERTY_NAME,
	JSON_DOSBOX_DOWNLOAD_FILE_FILE_SIZE_PROPERTY_NAME,
	JSON_DOSBOX_DOWNLOAD_FILE_SHA1_PROPERTY_NAME,
	JSON_DOSBOX_DOWNLOAD_FILE_OPERATING_SYSTEM_PROPERTY_NAME,
	JSON_DOSBOX_DOWNLOAD_FILE_PROCESSOR_ARCHITECTURE_PROPERTY_NAME
};

DOSBoxDownloadFile::DOSBoxDownloadFile(const std::string & fileName, uint64_t fileSize, const std::string & sha1, DeviceInformationBridge::OperatingSystemType operatingSystem, std::optional<DeviceInformationBridge::OperatingSystemArchitectureType> processorArchitecture)
	: m_fileName(Utilities::trimString(fileName))
	, m_fileSize(fileSize)
	, m_sha1(Utilities::trimString(sha1))
	, m_operatingSystem(operatingSystem)
	, m_processorArchitecture(processorArchitecture)
	, m_parentDOSBoxDownloadVersion(nullptr) { }

DOSBoxDownloadFile::DOSBoxDownloadFile(DOSBoxDownloadFile && f) noexcept
	: m_fileName(std::move(f.m_fileName))
	, m_fileSize(f.m_fileSize)
	, m_sha1(std::move(f.m_sha1))
	, m_operatingSystem(f.m_operatingSystem)
	, m_processorArchitecture(f.m_processorArchitecture)
	, m_parentDOSBoxDownloadVersion(nullptr) { }

DOSBoxDownloadFile::DOSBoxDownloadFile(const DOSBoxDownloadFile & f)
	: m_fileName(f.m_fileName)
	, m_fileSize(f.m_fileSize)
	, m_sha1(f.m_sha1)
	, m_operatingSystem(f.m_operatingSystem)
	, m_processorArchitecture(f.m_processorArchitecture)
	, m_parentDOSBoxDownloadVersion(nullptr) { }

DOSBoxDownloadFile & DOSBoxDownloadFile::operator = (DOSBoxDownloadFile && f) noexcept {
	if(this != &f) {
		m_fileName = std::move(f.m_fileName);
		m_fileSize = f.m_fileSize;
		m_sha1 = std::move(f.m_sha1);
		m_operatingSystem = f.m_operatingSystem;
		m_processorArchitecture = f.m_processorArchitecture;
	}

	return *this;
}

DOSBoxDownloadFile & DOSBoxDownloadFile::operator = (const DOSBoxDownloadFile & f) {
	m_fileName = f.m_fileName;
	m_fileSize = f.m_fileSize;
	m_sha1 = f.m_sha1;
	m_operatingSystem = f.m_operatingSystem;
	m_processorArchitecture = f.m_processorArchitecture;

	return *this;
}

DOSBoxDownloadFile::~DOSBoxDownloadFile() {
	m_parentDOSBoxDownloadVersion = nullptr;
}

const std::string & DOSBoxDownloadFile::getFileName() const {
	return m_fileName;
}

std::string_view DOSBoxDownloadFile::getFileExtension() const {
	return Utilities::getFileExtension(m_fileName);
}

uint64_t DOSBoxDownloadFile::getFileSize() const {
	return m_fileSize;
}

const std::string & DOSBoxDownloadFile::getSHA1() const {
	return m_sha1;
}

DeviceInformationBridge::OperatingSystemType DOSBoxDownloadFile::getOperatingSystem() const {
	return m_operatingSystem;
}

bool DOSBoxDownloadFile::hasProcessorArchitecture() const {
	return m_processorArchitecture.has_value();
}

std::optional<DeviceInformationBridge::OperatingSystemArchitectureType> DOSBoxDownloadFile::getProcessorArchitecture() const {
	return m_processorArchitecture;
}

const DOSBoxDownload * DOSBoxDownloadFile::getParentDOSBoxDownload() const {
	if(m_parentDOSBoxDownloadVersion == nullptr) {
		return nullptr;
	}

	return m_parentDOSBoxDownloadVersion->getParentDOSBoxDownload();
}

const DOSBoxDownloadVersion * DOSBoxDownloadFile::getParentDOSBoxDownloadVersion() const {
	return m_parentDOSBoxDownloadVersion;
}

void DOSBoxDownloadFile::setParentDOSBoxDownloadVersion(const DOSBoxDownloadVersion * dosboxDownloadVersion) {
	m_parentDOSBoxDownloadVersion = dosboxDownloadVersion;
}

rapidjson::Value DOSBoxDownloadFile::toJSON(rapidjson::MemoryPoolAllocator<rapidjson::CrtAllocator> & allocator) const {
	rapidjson::Value dosboxDownloadFileValue(rapidjson::kObjectType);

	rapidjson::Value fileNameValue(m_fileName.c_str(), allocator);
	dosboxDownloadFileValue.AddMember(rapidjson::StringRef(JSON_DOSBOX_DOWNLOAD_FILE_FILE_NAME_PROPERTY_NAME), fileNameValue, allocator);

	dosboxDownloadFileValue.AddMember(rapidjson::StringRef(JSON_DOSBOX_DOWNLOAD_FILE_FILE_SIZE_PROPERTY_NAME), rapidjson::Value(m_fileSize), allocator);

	rapidjson::Value sha1Value(m_sha1.c_str(), allocator);
	dosboxDownloadFileValue.AddMember(rapidjson::StringRef(JSON_DOSBOX_DOWNLOAD_FILE_SHA1_PROPERTY_NAME), sha1Value, allocator);

	rapidjson::Value operatingSystemValue(std::string(magic_enum::enum_name(m_operatingSystem)).c_str(), allocator);
	dosboxDownloadFileValue.AddMember(rapidjson::StringRef(JSON_DOSBOX_DOWNLOAD_FILE_OPERATING_SYSTEM_PROPERTY_NAME), operatingSystemValue, allocator);

	if(m_processorArchitecture.has_value()) {
		rapidjson::Value processorArchitectureValue(std::string(magic_enum::enum_name(m_processorArchitecture.value())).c_str(), allocator);
		dosboxDownloadFileValue.AddMember(rapidjson::StringRef(JSON_DOSBOX_DOWNLOAD_FILE_PROCESSOR_ARCHITECTURE_PROPERTY_NAME), processorArchitectureValue, allocator);
	}

	return dosboxDownloadFileValue;
}

std::unique_ptr<DOSBoxDownloadFile> DOSBoxDownloadFile::parseFrom(const rapidjson::Value & dosboxDownloadFileValue) {
	if(!dosboxDownloadFileValue.IsObject()) {
		spdlog::error("Invalid dosbox download file type: '{}', expected 'object'.", Utilities::typeToString(dosboxDownloadFileValue.GetType()));
		return nullptr;
	}

	// check for unhandled dosbox download file properties
	bool propertyHandled = false;

	for(rapidjson::Value::ConstMemberIterator i = dosboxDownloadFileValue.MemberBegin(); i != dosboxDownloadFileValue.MemberEnd(); ++i) {
		propertyHandled = false;

		for(const std::string_view propertyName : JSON_DOSBOX_DOWNLOAD_FILE_PROPERTY_NAMES) {
			if(i->name.GetString() == propertyName) {
				propertyHandled = true;
				break;
			}
		}

		if(!propertyHandled) {
			spdlog::warn("DOSBox download file has unexpected property '{}'.", i->name.GetString());
		}
	}

	// parse dosbox download file name
	if(!dosboxDownloadFileValue.HasMember(JSON_DOSBOX_DOWNLOAD_FILE_FILE_NAME_PROPERTY_NAME)) {
		spdlog::error("DOSBox download file is missing '{}' property.", JSON_DOSBOX_DOWNLOAD_FILE_FILE_NAME_PROPERTY_NAME);
		return nullptr;
	}

	const rapidjson::Value & fileNameValue = dosboxDownloadFileValue[JSON_DOSBOX_DOWNLOAD_FILE_FILE_NAME_PROPERTY_NAME];

	if(!fileNameValue.IsString()) {
		spdlog::error("DOSBox download file has an invalid '{}' property type: '{}', expected 'string'.", JSON_DOSBOX_DOWNLOAD_FILE_FILE_NAME_PROPERTY_NAME, Utilities::typeToString(fileNameValue.GetType()));
		return nullptr;
	}

	std::string fileName(Utilities::trimString(fileNameValue.GetString()));

	if(fileName.empty()) {
		spdlog::error("DOSBox download file '{}' property cannot be empty.", JSON_DOSBOX_DOWNLOAD_FILE_FILE_NAME_PROPERTY_NAME);
		return nullptr;
	}

	// parse dosbox download file size
	if(!dosboxDownloadFileValue.HasMember(JSON_DOSBOX_DOWNLOAD_FILE_FILE_SIZE_PROPERTY_NAME)) {
		spdlog::error("DOSBox download file is missing '{}' property.", JSON_DOSBOX_DOWNLOAD_FILE_FILE_SIZE_PROPERTY_NAME);
		return nullptr;
	}

	const rapidjson::Value & fileSizeValue = dosboxDownloadFileValue[JSON_DOSBOX_DOWNLOAD_FILE_FILE_SIZE_PROPERTY_NAME];

	if(!fileSizeValue.IsUint64()) {
		spdlog::error("DOSBox download file has an invalid '{}' property type: '{}', expected unsigned integer 'number'.", JSON_DOSBOX_DOWNLOAD_FILE_FILE_SIZE_PROPERTY_NAME, Utilities::typeToString(fileSizeValue.GetType()));
		return nullptr;
	}

	uint64_t fileSize = fileSizeValue.GetUint64();

	// parse the dosbox download file sha1 property
	if(!dosboxDownloadFileValue.HasMember(JSON_DOSBOX_DOWNLOAD_FILE_SHA1_PROPERTY_NAME)) {
		spdlog::error("DOSBox download file is missing '{}' property.", JSON_DOSBOX_DOWNLOAD_FILE_SHA1_PROPERTY_NAME);
		return nullptr;
	}

	const rapidjson::Value & fileSHA1Value = dosboxDownloadFileValue[JSON_DOSBOX_DOWNLOAD_FILE_SHA1_PROPERTY_NAME];

	if(!fileSHA1Value.IsString()) {
		spdlog::error("DOSBox download file '{}' property has invalid type: '{}', expected 'string'.", JSON_DOSBOX_DOWNLOAD_FILE_SHA1_PROPERTY_NAME, Utilities::typeToString(fileSHA1Value.GetType()));
		return nullptr;
	}

	std::string fileSHA1(Utilities::trimString(fileSHA1Value.GetString()));

	if(fileSHA1.empty()) {
		spdlog::error("DOSBox download file '{}' property cannot be empty.", JSON_DOSBOX_DOWNLOAD_FILE_SHA1_PROPERTY_NAME);
		return nullptr;
	}

	// parse the dosbox download file operating system
	if(!dosboxDownloadFileValue.HasMember(JSON_DOSBOX_DOWNLOAD_FILE_OPERATING_SYSTEM_PROPERTY_NAME)) {
		spdlog::error("DOSBox download file is missing '{}' property.", JSON_DOSBOX_DOWNLOAD_FILE_OPERATING_SYSTEM_PROPERTY_NAME);
		return nullptr;
	}

	const rapidjson::Value & operatingSystemNameValue = dosboxDownloadFileValue[JSON_DOSBOX_DOWNLOAD_FILE_OPERATING_SYSTEM_PROPERTY_NAME];

	if(!operatingSystemNameValue.IsString()) {
		spdlog::error("DOSBox download file has an invalid '{}' property type: '{}', expected 'string'.", JSON_DOSBOX_DOWNLOAD_FILE_OPERATING_SYSTEM_PROPERTY_NAME, Utilities::typeToString(operatingSystemNameValue.GetType()));
		return nullptr;
	}

	std::string operatingSystemName(Utilities::trimString(operatingSystemNameValue.GetString()));
	std::optional<DeviceInformationBridge::OperatingSystemType> optionalOperatingSystem(magic_enum::enum_cast<DeviceInformationBridge::OperatingSystemType>(operatingSystemName));

	if(!optionalOperatingSystem.has_value()) {
		spdlog::error("DOSBox download file has an invalid '{}' property value: '{}'.", JSON_DOSBOX_DOWNLOAD_FILE_OPERATING_SYSTEM_PROPERTY_NAME, operatingSystemName);
		return nullptr;
	}

	// parse the dosbox download file processor architecture
	std::optional<DeviceInformationBridge::OperatingSystemArchitectureType> optionalProcessorArchitecture;

	if(dosboxDownloadFileValue.HasMember(JSON_DOSBOX_DOWNLOAD_FILE_PROCESSOR_ARCHITECTURE_PROPERTY_NAME)) {
		const rapidjson::Value & processorArchitectureNameValue = dosboxDownloadFileValue[JSON_DOSBOX_DOWNLOAD_FILE_PROCESSOR_ARCHITECTURE_PROPERTY_NAME];

		if(!processorArchitectureNameValue.IsString()) {
			spdlog::error("DOSBox download file has an invalid '{}' property type: '{}', expected 'string'.", JSON_DOSBOX_DOWNLOAD_FILE_PROCESSOR_ARCHITECTURE_PROPERTY_NAME, Utilities::typeToString(processorArchitectureNameValue.GetType()));
			return nullptr;
		}

		std::string processorArchitectureName(Utilities::trimString(processorArchitectureNameValue.GetString()));
		optionalProcessorArchitecture = magic_enum::enum_cast<DeviceInformationBridge::OperatingSystemArchitectureType>(processorArchitectureName);

		if(!optionalProcessorArchitecture.has_value()) {
			spdlog::error("DOSBox download file has an invalid '{}' property value: '{}'.", JSON_DOSBOX_DOWNLOAD_FILE_PROCESSOR_ARCHITECTURE_PROPERTY_NAME, processorArchitectureName);
			return nullptr;
		}
	}

	// initialize the dosbox download file
	return std::make_unique<DOSBoxDownloadFile>(fileName, fileSize, fileSHA1, optionalOperatingSystem.value(), optionalProcessorArchitecture);
}

bool DOSBoxDownloadFile::isValid() const {
	return !m_fileName.empty() &&
		   m_fileSize != 0 &&
		   !m_sha1.empty();
}

bool DOSBoxDownloadFile::isValid(const DOSBoxDownloadFile * f) {
	return f != nullptr && f->isValid();
}

bool DOSBoxDownloadFile::operator == (const DOSBoxDownloadFile & f) const {
	return Utilities::areStringsEqualIgnoreCase(m_fileName, f.m_fileName) &&
		   m_fileSize == f.m_fileSize &&
		   m_sha1 == f.m_sha1 &&
		   m_operatingSystem == f.m_operatingSystem &&
		   m_processorArchitecture == f.m_processorArchitecture;
}

bool DOSBoxDownloadFile::operator != (const DOSBoxDownloadFile & f) const {
	return !operator == (f);
}
