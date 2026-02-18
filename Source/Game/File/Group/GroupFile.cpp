#include "Group.h"

#include <Utilities/FileUtilities.h>
#include <Utilities/StringUtilities.h>
#include <Utilities/Utilities.h>

#include <fmt/core.h>
#include <spdlog/spdlog.h>

#include <filesystem>
#include <fstream>

const uint8_t GroupFile::MAX_FILE_NAME_LENGTH = 12;
const bool GroupFile::DEFAULT_OVERWRITE_FILES = false;

GroupFile::GroupFile(const std::string & fileName)
	: m_fileName(formatFileName(fileName))
	, m_data(std::make_unique<ByteBuffer>())
	, m_modified(false)
	, m_parentGroup(nullptr) { }

GroupFile::GroupFile(const std::string & fileName, const uint8_t * data, size_t dataSize, const uint8_t * trailingData, size_t trailingDataSize)
	: m_fileName(formatFileName(fileName))
	, m_data(std::make_unique<ByteBuffer>(data, data == nullptr ? 0 : dataSize))
	, m_trailingData(trailingData == nullptr || trailingDataSize == 0 ? nullptr : std::make_unique<ByteBuffer>(trailingData, trailingDataSize))
	, m_modified(false)
	, m_parentGroup(nullptr) { }

GroupFile::GroupFile(const std::string & fileName, const std::vector<uint8_t> & data, const std::vector<uint8_t> & trailingData)
	: m_fileName(formatFileName(fileName))
	, m_data(std::make_unique<ByteBuffer>(data))
	, m_trailingData(trailingData.empty() ? nullptr : std::make_unique<ByteBuffer>(trailingData))
	, m_modified(false)
	, m_parentGroup(nullptr) { }

GroupFile::GroupFile(const std::string & fileName, const ByteBuffer & data, const ByteBuffer & trailingData)
	: m_fileName(formatFileName(fileName))
	, m_data(std::make_unique<ByteBuffer>(data))
	, m_trailingData(trailingData.isEmpty() ? nullptr : std::make_unique<ByteBuffer>(trailingData))
	, m_modified(false)
	, m_parentGroup(nullptr) { }

GroupFile::GroupFile(const std::string & fileName, std::unique_ptr<ByteBuffer> data, std::unique_ptr<ByteBuffer> trailingData)
	: m_fileName(formatFileName(fileName))
	, m_data(std::move(data))
	, m_trailingData(trailingData != nullptr ? std::move(trailingData) : std::make_unique<ByteBuffer>())
	, m_modified(false)
	, m_parentGroup(nullptr) { }

GroupFile::GroupFile(GroupFile && file) noexcept
	: m_fileName(std::move(file.m_fileName))
	, m_data(std::move(file.m_data))
	, m_trailingData(std::move(file.m_trailingData))
	, m_modified(false)
	, m_parentGroup(nullptr) { }

GroupFile::GroupFile(const GroupFile & file)
	: m_fileName(file.m_fileName)
	, m_data(std::make_unique<ByteBuffer>(file.m_data->getData()))
	, m_trailingData(file.m_trailingData != nullptr ? std::make_unique<ByteBuffer>(*file.m_trailingData) : nullptr)
	, m_modified(false)
	, m_parentGroup(nullptr) { }

GroupFile & GroupFile::operator = (GroupFile && file) noexcept {
	if(this != &file) {
		m_fileName = std::move(file.m_fileName);
		m_data = std::move(file.m_data);
		m_trailingData = std::move(file.m_trailingData);

		setModified(true);
	}

	return *this;
}

GroupFile & GroupFile::operator = (const GroupFile & file) {
	m_fileName = file.m_fileName;
	m_data->setData(file.m_data->getData());
	m_trailingData = file.m_trailingData != nullptr ? std::make_unique<ByteBuffer>(*file.m_trailingData) : nullptr;

	setModified(true);

	return *this;
}

GroupFile::~GroupFile() {
	m_parentGroup = nullptr;
}

bool GroupFile::isModified() const {
	return m_modified;
}

void GroupFile::setModified(bool value) {
	m_modified = value;

	modified(*this);
}

const std::string & GroupFile::getFileName() const {
	return m_fileName;
}

bool GroupFile::hasFileExtension(std::string_view fileExtension) const {
	if(m_fileName.length() < fileExtension.length() + 1) {
		return false;
	}

	return m_fileName[m_fileName.length() - fileExtension.length() - 1] == '.' &&
		   Utilities::areStringsEqualIgnoreCase(std::string_view(m_fileName.data() + m_fileName.length() - fileExtension.length(), fileExtension.length()), fileExtension);
}

std::string_view GroupFile::getFileExtension() const {
	return Utilities::getFileExtension(m_fileName);
}

size_t GroupFile::getSize() const {
	return m_data->getSize();
}

bool GroupFile::hasTrailingData() const {
	return m_trailingData != nullptr &&
		   m_trailingData->isNotEmpty();
}

size_t GroupFile::getTrailingDataSize() const {
	if(m_trailingData == nullptr) {
		return 0;
	}

	return m_trailingData->getSize();
}

std::string GroupFile::getSizeAsString() const {
	size_t size = getSize();

	if(size < 1000) {
		return fmt::format("{} B", size);
	}
	else if(size < 1000000) {
		return fmt::format("{:.2f} KB", size / 1000.0);
	}
	else {
		return fmt::format("{:.2f} MB", size / 1000000.0);
	}
}

bool GroupFile::hasData() const {
	return m_data->isNotEmpty();
}

const ByteBuffer & GroupFile::getData() const {
	m_data->setReadOffset(0);
	return *m_data;
}

ByteBuffer & GroupFile::getData() {
	m_data->setReadOffset(0);
	return *m_data;
}

std::unique_ptr<ByteBuffer> GroupFile::transferData() {
	std::unique_ptr<ByteBuffer> data(std::move(m_data));

	m_data = std::make_unique<ByteBuffer>();

	return data;
}

const ByteBuffer & GroupFile::getTrailingData() const {
	if(m_trailingData == nullptr) {
		return ByteBuffer::EMPTY_BYTE_BUFFER;
	}

	m_trailingData->setReadOffset(0);
	return *m_trailingData;
}

std::unique_ptr<ByteBuffer> GroupFile::transferTrailingData() {
	return std::move(m_trailingData);
}

bool GroupFile::setFileName(const std::string & newFileName) {
	if(newFileName.empty() || newFileName.length() > GroupFile::MAX_FILE_NAME_LENGTH) {
		return false;
	}

	if(Utilities::areStringsEqualIgnoreCase(m_fileName, newFileName)) {
		return true;
	}

	if(m_parentGroup != nullptr && m_parentGroup->hasFile(newFileName)) {
		return false;
	}

	m_fileName = Utilities::toUpperCase(newFileName);

	setModified(true);

	return true;
}

void GroupFile::setData(const uint8_t * data, size_t size) {
	m_data->setData(data, size);

	setModified(true);
}

void GroupFile::setData(const std::vector<uint8_t> & data) {
	m_data->setData(data);

	setModified(true);
}

void GroupFile::setData(const ByteBuffer & data) {
	m_data->setData(data);

	setModified(true);
}

void GroupFile::setData(std::unique_ptr<ByteBuffer> data) {
	m_data = std::move(data);

	setModified(true);
}

void GroupFile::clearData() {
	m_data->clear();

	setModified(true);
}

void GroupFile::setTrailingData(const uint8_t * trailingData, size_t size) {
	m_trailingData->setData(trailingData, size);

	setModified(true);
}

void GroupFile::setTrailingData(const std::vector<uint8_t> & trailingData) {
	m_trailingData->setData(trailingData);

	setModified(true);
}

void GroupFile::setTrailingData(const ByteBuffer & trailingData) {
	m_trailingData->setData(trailingData);

	setModified(true);
}

void GroupFile::setTrailingData(std::unique_ptr<ByteBuffer> trailingData) {
	m_trailingData = std::move(trailingData);

	setModified(true);
}

void GroupFile::clearTrailingData() {
	m_trailingData->clear();

	setModified(true);
}

Group * GroupFile::getParentGroup() const {
	return m_parentGroup;
}

bool GroupFile::isValid() const {
	return !m_fileName.empty() &&
		   m_fileName.length() <= MAX_FILE_NAME_LENGTH &&
		   m_data != nullptr;
}

bool GroupFile::isValid(const GroupFile * file) {
	return file != nullptr &&
		   file->isValid();
}

bool GroupFile::writeTo(const std::string & basePath, bool overwrite, const std::string & alternateFileName, bool createParentDirectories) const {
	if(!isValid()) {
		spdlog::error("Failed to write '{}' file, file is not valid.", m_fileName);
		return false;
	}

	std::string filePath(Utilities::joinPaths(basePath, alternateFileName.empty() ? m_fileName : alternateFileName));

	if(!overwrite && std::filesystem::exists(std::filesystem::path(filePath))) {
		spdlog::warn("File '{}' already exists, use overwrite to force write.", filePath);
		return false;
	}

	if(!m_data->writeTo(filePath, overwrite, createParentDirectories)) {
		spdlog::error("Failed to open write stream for file '{}'.", filePath);
		return false;
	}

	return true;
}

std::string GroupFile::formatFileName(std::string_view fileName) {
	return Utilities::toUpperCase(Utilities::truncateFileName(fileName, MAX_FILE_NAME_LENGTH));
}

bool GroupFile::operator == (const GroupFile & file) const {
	return Utilities::areStringsEqualIgnoreCase(m_fileName, file.m_fileName) &&
		   *m_data == *file.m_data &&
		   ((m_trailingData == nullptr && file.m_trailingData == nullptr) || (m_trailingData != nullptr && file.m_trailingData != nullptr && *m_trailingData == *file.m_trailingData) );
}

bool GroupFile::operator != (const GroupFile & file) const {
	return !operator == (file);
}
