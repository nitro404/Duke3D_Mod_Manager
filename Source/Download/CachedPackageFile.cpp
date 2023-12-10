#include "CachedPackageFile.h"

#include <Utilities/RapidJSONUtilities.h>
#include <Utilities/StringUtilities.h>

#include <spdlog/spdlog.h>

#include <array>

static constexpr const char * JSON_CACHED_FILE_FILE_NAME_PROPERTY_NAME = "fileName";
static constexpr const char * JSON_CACHED_FILE_FILE_SIZE_PROPERTY_NAME = "fileSize";
static constexpr const char * JSON_CACHED_FILE_SHA1_PROPERTY_NAME = "sha1";
static constexpr const char * JSON_CACHED_FILE_ETAG_PROPERTY_NAME = "eTag";
static constexpr const char * JSON_CACHED_FILE_DOWNLOADED_PROPERTY_NAME = "downloaded";
static constexpr const char * JSON_CACHED_PACKAGE_FILE_CONTENTS_PROPERTY_NAME = "contents";
static const std::array<std::string_view, 6> JSON_CACHED_PACKAGE_FILE_PROPERTY_NAMES = {
	JSON_CACHED_FILE_FILE_NAME_PROPERTY_NAME,
	JSON_CACHED_FILE_FILE_SIZE_PROPERTY_NAME,
	JSON_CACHED_FILE_SHA1_PROPERTY_NAME,
	JSON_CACHED_FILE_ETAG_PROPERTY_NAME,
	JSON_CACHED_FILE_DOWNLOADED_PROPERTY_NAME,
	JSON_CACHED_PACKAGE_FILE_CONTENTS_PROPERTY_NAME
};

CachedPackageFile::CachedPackageFile(const std::string & fileName, uint64_t fileSize, const std::string & sha1, const std::string & eTag, std::optional<std::chrono::time_point<std::chrono::system_clock>> downloadedTimePoint)
	: CachedFile(fileName, fileSize, sha1, eTag, downloadedTimePoint) { }

CachedPackageFile::CachedPackageFile(CachedPackageFile && f) noexcept
	: CachedFile(std::move(f))
	, m_cachedFiles(std::move(f.m_cachedFiles)) { }

CachedPackageFile::CachedPackageFile(CachedFile && f) noexcept
	: CachedFile(std::move(f)) { }

CachedPackageFile::CachedPackageFile(const CachedPackageFile & f)
	: CachedFile(f) {
	for(std::map<std::string, std::shared_ptr<CachedFile>>::const_iterator i = f.m_cachedFiles.begin(); i != f.m_cachedFiles.end(); ++i) {
		m_cachedFiles[i->second->getFileName()] = std::make_shared<CachedFile>(*i->second);
	}
}

CachedPackageFile & CachedPackageFile::operator = (CachedPackageFile && f) noexcept {
	if(this != &f) {
		CachedFile::operator = (std::move(f));

		m_cachedFiles = std::move(f.m_cachedFiles);
	}

	return *this;
}

CachedPackageFile & CachedPackageFile::operator = (CachedFile && f) noexcept {
	if(this != &f) {
		CachedFile::operator = (std::move(f));
	}

	return *this;
}

CachedPackageFile & CachedPackageFile::operator = (const CachedPackageFile & f) {
	CachedFile::operator = (f);

	for(std::map<std::string, std::shared_ptr<CachedFile>>::const_iterator i = f.m_cachedFiles.begin(); i != f.m_cachedFiles.end(); ++i) {
		m_cachedFiles[i->second->getFileName()] = std::make_shared<CachedFile>(*i->second);
	}

	return *this;
}

CachedPackageFile::~CachedPackageFile() { }

size_t CachedPackageFile::numberOfCachedFiles() const {
	return m_cachedFiles.size();
}

bool CachedPackageFile::hasCachedFile(const CachedFile * cachedFile) const {
	if(!CachedFile::isValid(cachedFile)) {
		return false;
	}

	return hasCachedFileWithName(cachedFile->getFileName());
}

bool CachedPackageFile::hasCachedFileWithName(const std::string & fileName) const {
	return m_cachedFiles.find(fileName) != m_cachedFiles.end();
}

std::shared_ptr<CachedFile> CachedPackageFile::getCachedFileWithName(const std::string & fileName) const {
	if(fileName.empty()) {
		return nullptr;
	}

	std::map<std::string, std::shared_ptr<CachedFile>>::const_iterator cachedFile(m_cachedFiles.find(fileName));

	if(cachedFile == m_cachedFiles.end()) {
		return nullptr;
	}

	return cachedFile->second;
}

std::vector<std::shared_ptr<CachedFile>> CachedPackageFile::getCachedFiles() const {
	std::vector<std::shared_ptr<CachedFile>> cachedFiles;

	for(std::map<std::string, std::shared_ptr<CachedFile>>::const_iterator i = m_cachedFiles.begin(); i != m_cachedFiles.end(); ++i) {
		cachedFiles.push_back(i->second);
	}

	return cachedFiles;
}

bool CachedPackageFile::addCachedFile(std::unique_ptr<CachedFile> cachedFile) {
	if(!CachedFile::isValid(cachedFile.get()) || hasCachedFileWithName(cachedFile->getFileName())) {
		return false;
	}

	std::string cachedFileName(cachedFile->getFileName());

	m_cachedFiles[cachedFileName] = std::move(cachedFile);

	return true;
}

rapidjson::Value CachedPackageFile::toJSON(rapidjson::MemoryPoolAllocator<rapidjson::CrtAllocator> & allocator) const {
	rapidjson::Value cachedPackageFileValue(CachedFile::toJSON(allocator));

	rapidjson::Value contentsValue(rapidjson::kArrayType);
	contentsValue.Reserve(m_cachedFiles.size(), allocator);

	for(std::map<std::string, std::shared_ptr<CachedFile>>::const_iterator i = m_cachedFiles.begin(); i != m_cachedFiles.end(); ++i) {
		contentsValue.PushBack(i->second->toJSON(allocator), allocator);
	}

	cachedPackageFileValue.AddMember(rapidjson::StringRef(JSON_CACHED_PACKAGE_FILE_CONTENTS_PROPERTY_NAME), contentsValue, allocator);

	return cachedPackageFileValue;
}

std::unique_ptr<CachedPackageFile> CachedPackageFile::parseFrom(const rapidjson::Value & cachedPackageFileValue) {
	if(!cachedPackageFileValue.IsObject()) {
		spdlog::error("Invalid cached package file type: '{}', expected 'object'.", Utilities::typeToString(cachedPackageFileValue.GetType()));
		return nullptr;
	}

	std::unique_ptr<CachedPackageFile> newCachedPackageFile(std::make_unique<CachedPackageFile>(std::move(*std::move(CachedFile::parseFrom(cachedPackageFileValue)))));

	// verify that the cached package file has an ETag
	if(newCachedPackageFile->getETag().empty()) {
		spdlog::error("Cached package file '{}' property cannot be empty.", JSON_CACHED_FILE_ETAG_PROPERTY_NAME);
		return nullptr;
	}

	// check for unhandled cached package file properties
	bool propertyHandled = false;

	for(rapidjson::Value::ConstMemberIterator i = cachedPackageFileValue.MemberBegin(); i != cachedPackageFileValue.MemberEnd(); ++i) {
		propertyHandled = false;

		for(const std::string_view propertyName : JSON_CACHED_PACKAGE_FILE_PROPERTY_NAMES) {
			if(i->name.GetString() == propertyName) {
				propertyHandled = true;
				break;
			}
		}

		if(!propertyHandled) {
			spdlog::warn("Cached package file has unexpected property '{}'.", i->name.GetString());
		}
	}

	// parse the cached package file contents property
	if(!cachedPackageFileValue.HasMember(JSON_CACHED_PACKAGE_FILE_CONTENTS_PROPERTY_NAME)) {
		spdlog::error("Cached package file is missing '{}' property.", JSON_CACHED_PACKAGE_FILE_CONTENTS_PROPERTY_NAME);
		return nullptr;
	}

	const rapidjson::Value & cachedPackageFileContentsValue = cachedPackageFileValue[JSON_CACHED_PACKAGE_FILE_CONTENTS_PROPERTY_NAME];

	if(!cachedPackageFileContentsValue.IsArray()) {
		spdlog::error("Cached package file '{}' property has invalid type: '{}', expected 'array'.", JSON_CACHED_PACKAGE_FILE_CONTENTS_PROPERTY_NAME, Utilities::typeToString(cachedPackageFileContentsValue.GetType()));
		return nullptr;
	}

	std::shared_ptr<CachedFile> newCachedFile;

	for(rapidjson::Value::ConstValueIterator i = cachedPackageFileContentsValue.Begin(); i != cachedPackageFileContentsValue.End(); ++i) {
		newCachedFile = CachedFile::parseFrom(*i);

		if(!CachedFile::isValid(newCachedFile.get())) {
			spdlog::error("Failed to parse cached package file cached file #{}.", newCachedPackageFile->m_cachedFiles.size() + 1);
			return nullptr;
		}

		if(newCachedPackageFile->hasCachedFileWithName(newCachedFile->getFileName())) {
			spdlog::error("Encountered duplicate cached package file cached file #{}.", newCachedPackageFile->m_cachedFiles.size() + 1);
			return nullptr;
		}

		newCachedPackageFile->m_cachedFiles[newCachedFile->getFileName()] = newCachedFile;
	}

	if(newCachedPackageFile->m_cachedFiles.empty()) {
		spdlog::error("Failed to parse package file, cached file list is empty.");
		return nullptr;
	}

	return newCachedPackageFile;
}

bool CachedPackageFile::isValid() const {
	return CachedFile::isValid() &&
		   !getETag().empty();
}

bool CachedPackageFile::isValid(const CachedPackageFile * f) {
	return f != nullptr &&
		   f->isValid();
}

bool CachedPackageFile::operator == (const CachedPackageFile & f) const {
	return Utilities::areStringsEqual(getFileName(), f.getFileName());
}

bool CachedPackageFile::operator != (const CachedPackageFile & f) const {
	return !operator == (f);
}
