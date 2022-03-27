#include "CachedFile.h"

#include <ByteBuffer.h>
#include <Utilities/FileUtilities.h>
#include <Utilities/RapidJSONUtilities.h>
#include <Utilities/StringUtilities.h>

#include <fmt/core.h>

#include <filesystem>
#include <optional>

static constexpr const char * JSON_CACHED_FILE_ID_PROPERTY_NAME = "id";
static constexpr const char * JSON_CACHED_FILE_FILE_NAME_PROPERTY_NAME = "fileName";
static constexpr const char * JSON_CACHED_FILE_FILE_SIZE_PROPERTY_NAME = "fileSize";
static constexpr const char * JSON_CACHED_FILE_SHA1_PROPERTY_NAME = "sha1";
static constexpr const char * JSON_CACHED_FILE_ETAG_PROPERTY_NAME = "eTag";

CachedFile::CachedFile(uint64_t id, const std::string & fileName, uint64_t fileSize, const std::string & sha1, const std::string & eTag)
	: m_id(id)
	, m_fileName(fileName)
	, m_fileSize(fileSize)
	, m_sha1(sha1)
	, m_eTag(eTag) { }

CachedFile::CachedFile(CachedFile && f) noexcept
	: m_id(f.m_id)
	, m_fileName(std::move(f.m_fileName))
	, m_fileSize(f.m_fileSize)
	, m_sha1(std::move(f.m_sha1))
	, m_eTag(std::move(f.m_eTag)) { }

CachedFile::CachedFile(const CachedFile & f)
	: m_id(f.m_id)
	, m_fileName(f.m_fileName)
	, m_fileSize(f.m_fileSize)
	, m_sha1(f.m_sha1)
	, m_eTag(f.m_eTag) { }

CachedFile & CachedFile::operator = (CachedFile && f) noexcept {
	if(this != &f) {
		m_id = f.m_id;
		m_fileName = std::move(f.m_fileName);
		m_fileSize = f.m_fileSize;
		m_sha1 = std::move(f.m_sha1);
		m_eTag = std::move(f.m_eTag);
	}

	return *this;
}

CachedFile & CachedFile::operator = (const CachedFile & f) {
	m_id = f.m_id;
	m_fileName = f.m_fileName;
	m_fileSize = f.m_fileSize;
	m_sha1 = f.m_sha1;
	m_eTag = f.m_eTag;

	return *this;
}

CachedFile::~CachedFile() { }

uint64_t CachedFile::getID() const {
	return m_id;
}

const std::string & CachedFile::getFileName() const {
	return m_fileName;
}

bool CachedFile::setFileName(const std::string & fileName) {
	if(fileName.empty()) {
		return false;
	}

	m_fileName = fileName;

	return true;
}

uint64_t CachedFile::getFileSize() const {
	return m_fileSize;
}

void CachedFile::setFileSize(uint64_t fileSize) {
	m_fileSize = fileSize;
}

const std::string & CachedFile::getSHA1() const {
	return m_sha1;
}

bool CachedFile::setSHA1(const std::string & sha1) {
	if(sha1.empty()) {
		return false;
	}

	m_sha1 = sha1;

	return true;
}

bool CachedFile::hasETag() const {
	return !m_eTag.empty();
}

const std::string & CachedFile::getETag() const {
	return m_eTag;
}

bool CachedFile::setETag(const std::string & eTag) {
	if(eTag.empty()) {
		return false;
	}

	m_eTag = eTag;

	return true;
}

rapidjson::Value CachedFile::toJSON(rapidjson::MemoryPoolAllocator<rapidjson::CrtAllocator> & allocator) const {
	rapidjson::Value cachedFileValue(rapidjson::kObjectType);

	cachedFileValue.AddMember(rapidjson::StringRef(JSON_CACHED_FILE_ID_PROPERTY_NAME), rapidjson::Value(m_id), allocator);

	rapidjson::Value fileNameValue(m_fileName.c_str(), allocator);
	cachedFileValue.AddMember(rapidjson::StringRef(JSON_CACHED_FILE_FILE_NAME_PROPERTY_NAME), fileNameValue, allocator);

	cachedFileValue.AddMember(rapidjson::StringRef(JSON_CACHED_FILE_FILE_SIZE_PROPERTY_NAME), rapidjson::Value(m_fileSize), allocator);

	rapidjson::Value sha1Value(m_sha1.c_str(), allocator);
	cachedFileValue.AddMember(rapidjson::StringRef(JSON_CACHED_FILE_SHA1_PROPERTY_NAME), sha1Value, allocator);

	if(!m_eTag.empty()) {
		rapidjson::Value eTagValue(m_eTag.c_str(), allocator);
		cachedFileValue.AddMember(rapidjson::StringRef(JSON_CACHED_FILE_ETAG_PROPERTY_NAME), eTagValue, allocator);
	}

	return cachedFileValue;
}

std::unique_ptr<CachedFile> CachedFile::parseFrom(const rapidjson::Value & cachedFileValue) {
	if(!cachedFileValue.IsObject()) {
		fmt::print("Invalid cached file type: '{}', expected 'object'.\n", Utilities::typeToString(cachedFileValue.GetType()));
		return nullptr;
	}

	// parse cached file ID
	if(!cachedFileValue.HasMember(JSON_CACHED_FILE_ID_PROPERTY_NAME)) {
		fmt::print("Cached file is missing '{}' property'.\n", JSON_CACHED_FILE_ID_PROPERTY_NAME);
		return nullptr;
	}

	const rapidjson::Value & cachedFileIDValue = cachedFileValue[JSON_CACHED_FILE_ID_PROPERTY_NAME];

	if(!cachedFileIDValue.IsUint64()) {
		fmt::print("Cached file has an invalid '{}' property type: '{}', expected unsigned integer 'number'.\n", JSON_CACHED_FILE_ID_PROPERTY_NAME, Utilities::typeToString(cachedFileIDValue.GetType()));
		return nullptr;
	}

	uint64_t id = cachedFileIDValue.GetUint64();

	if(id == 0) {
		fmt::print("Cached file has an invalid '{}' property value: '{}', expected positive integer greater than 0.\n", JSON_CACHED_FILE_ID_PROPERTY_NAME, Utilities::typeToString(cachedFileIDValue.GetType()));
		return nullptr;
	}

	// parse cached file name
	if(!cachedFileValue.HasMember(JSON_CACHED_FILE_FILE_NAME_PROPERTY_NAME)) {
		fmt::print("Cached file is missing '{}' property'.\n", JSON_CACHED_FILE_FILE_NAME_PROPERTY_NAME);
		return nullptr;
	}

	const rapidjson::Value & cachedFileNameValue = cachedFileValue[JSON_CACHED_FILE_FILE_NAME_PROPERTY_NAME];

	if(!cachedFileNameValue.IsString()) {
		fmt::print("Cached file has an invalid '{}' property type: '{}', expected 'string'.\n", JSON_CACHED_FILE_FILE_NAME_PROPERTY_NAME, Utilities::typeToString(cachedFileNameValue.GetType()));
		return nullptr;
	}

	std::string fileName(Utilities::trimString(cachedFileNameValue.GetString()));

	if(fileName.empty()) {
		fmt::print("Cached file '{}' property cannot be empty.\n", JSON_CACHED_FILE_FILE_NAME_PROPERTY_NAME);
		return nullptr;
	}

	// parse cached file size
	if(!cachedFileValue.HasMember(JSON_CACHED_FILE_FILE_SIZE_PROPERTY_NAME)) {
		fmt::print("Cached file is missing '{}' property'.\n", JSON_CACHED_FILE_FILE_SIZE_PROPERTY_NAME);
		return nullptr;
	}

	const rapidjson::Value & cachedFileSizeValue = cachedFileValue[JSON_CACHED_FILE_FILE_SIZE_PROPERTY_NAME];

	if(!cachedFileSizeValue.IsUint64()) {
		fmt::print("Cached file has an invalid '{}' property type: '{}', expected unsigned integer 'number'.\n", JSON_CACHED_FILE_FILE_SIZE_PROPERTY_NAME, Utilities::typeToString(cachedFileSizeValue.GetType()));
		return nullptr;
	}

	uint64_t fileSize = cachedFileSizeValue.GetUint64();

	// parse cached file SHA1
	if(!cachedFileValue.HasMember(JSON_CACHED_FILE_SHA1_PROPERTY_NAME)) {
		fmt::print("Cached file is missing '{}' property'.\n", JSON_CACHED_FILE_SHA1_PROPERTY_NAME);
		return nullptr;
	}

	const rapidjson::Value & cachedFileSHA1Value = cachedFileValue[JSON_CACHED_FILE_SHA1_PROPERTY_NAME];

	if(!cachedFileSHA1Value.IsString()) {
		fmt::print("Cached file has an invalid '{}' property type: '{}', expected 'string'.\n", JSON_CACHED_FILE_SHA1_PROPERTY_NAME, Utilities::typeToString(cachedFileSHA1Value.GetType()));
		return nullptr;
	}

	std::string sha1(Utilities::trimString(cachedFileSHA1Value.GetString()));

	if(sha1.empty()) {
		fmt::print("Cached file '{}' property cannot be empty.\n", JSON_CACHED_FILE_SHA1_PROPERTY_NAME);
		return nullptr;
	}

	// parse cached file ETag

	std::string eTag;

	if(cachedFileValue.HasMember(JSON_CACHED_FILE_ETAG_PROPERTY_NAME)) {
		const rapidjson::Value & cachedFileETagValue = cachedFileValue[JSON_CACHED_FILE_ETAG_PROPERTY_NAME];

		if(!cachedFileETagValue.IsString()) {
			fmt::print("Cached file has an invalid '{}' property type: '{}', expected 'string'.\n", JSON_CACHED_FILE_ETAG_PROPERTY_NAME, Utilities::typeToString(cachedFileETagValue.GetType()));
			return nullptr;
		}

		eTag = Utilities::trimString(cachedFileETagValue.GetString());
	}

	return std::make_unique<CachedFile>(id, fileName, fileSize, sha1, eTag);
}

bool CachedFile::isValid() const {
	return m_id > 0 &&
		   !m_fileName.empty() &&
		   !m_sha1.empty();
}

bool CachedFile::isValid(const CachedFile * f) {
	return f != nullptr &&
		   f->isValid();
}

bool CachedFile::operator == (const CachedFile & f) const {
	return m_id == f.m_id;
}

bool CachedFile::operator != (const CachedFile & f) const {
	return !operator == (f);
}
