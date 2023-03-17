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

GroupFile::GroupFile(const std::string & fileName, const uint8_t * data, size_t size)
	: m_fileName(formatFileName(fileName))
	, m_data(std::make_unique<ByteBuffer>(data, data == nullptr ? 0 : size))
	, m_modified(false)
	, m_parentGroup(nullptr) { }

GroupFile::GroupFile(const std::string & fileName, const std::vector<uint8_t> & data)
	: m_fileName(formatFileName(fileName))
	, m_data(std::make_unique<ByteBuffer>(data))
	, m_modified(false)
	, m_parentGroup(nullptr) { }

GroupFile::GroupFile(const std::string & fileName, const ByteBuffer & data)
	: m_fileName(formatFileName(fileName))
	, m_data(std::make_unique<ByteBuffer>(data))
	, m_modified(false)
	, m_parentGroup(nullptr) { }

GroupFile::GroupFile(const std::string & fileName, std::unique_ptr<ByteBuffer> data)
	: m_fileName(formatFileName(fileName))
	, m_data(std::move(data))
	, m_modified(false)
	, m_parentGroup(nullptr) { }

GroupFile::GroupFile(GroupFile && f) noexcept
	: m_fileName(std::move(f.m_fileName))
	, m_data(std::move(f.m_data))
	, m_modified(false)
	, m_parentGroup(nullptr) { }

GroupFile::GroupFile(const GroupFile & f)
	: m_fileName(f.m_fileName)
	, m_data(std::make_unique<ByteBuffer>(f.m_data->getData()))
	, m_modified(false)
	, m_parentGroup(nullptr) { }

GroupFile & GroupFile::operator = (GroupFile && f) noexcept {
	if(this != &f) {
		m_fileName = std::move(f.m_fileName);
		m_data = std::move(f.m_data);

		setModified(true);
	}

	return *this;
}

GroupFile & GroupFile::operator = (const GroupFile & f) {
	m_fileName = f.m_fileName;
	m_data->setData(f.m_data->getData());

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

std::string_view GroupFile::getFileExtension() const {
	return Utilities::getFileExtension(m_fileName);
}

size_t GroupFile::getSize() const {
	return m_data->getSize();
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

const ByteBuffer & GroupFile::getData() const {
	return *m_data;
}

std::unique_ptr<ByteBuffer> GroupFile::transferData() {
	std::unique_ptr<ByteBuffer> data(std::move(m_data));

	m_data = std::make_unique<ByteBuffer>();

	return std::move(data);
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

Group * GroupFile::getParentGroup() const {
	return m_parentGroup;
}

bool GroupFile::isValid() const {
	return !m_fileName.empty() &&
		   m_fileName.length() <= MAX_FILE_NAME_LENGTH &&
		   m_data != nullptr;
}

bool GroupFile::isValid(const GroupFile * f) {
	return f != nullptr &&
		   f->isValid();
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

bool GroupFile::operator == (const GroupFile & f) const {
	return Utilities::areStringsEqualIgnoreCase(m_fileName, f.m_fileName) &&
		   *m_data == *f.m_data;
}

bool GroupFile::operator != (const GroupFile & f) const {
	return !operator == (f);
}
