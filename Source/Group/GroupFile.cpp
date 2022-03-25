#include "GroupFile.h"

#include <Utilities/FileUtilities.h>
#include <Utilities/StringUtilities.h>
#include <Utilities/Utilities.h>

#include <fmt/core.h>

#include <filesystem>
#include <fstream>

const uint8_t GroupFile::MAX_FILE_NAME_LENGTH = 12;
const bool GroupFile::DEFAULT_OVERWRITE_FILES = false;

GroupFile::GroupFile(const std::string & fileName)
	: m_fileName(Utilities::trimString(fileName)) { }

GroupFile::GroupFile(const std::string & fileName, const uint8_t * data, size_t size)
	: m_fileName(Utilities::trimString(fileName))
	, m_data(data, data == nullptr ? 0 : size) { }

GroupFile::GroupFile(const std::string & fileName, const std::vector<uint8_t> & data)
	: m_fileName(Utilities::trimString(fileName))
	, m_data(data) { }

GroupFile::GroupFile(const std::string & fileName, ByteBuffer && data) noexcept
	: m_fileName(Utilities::trimString(fileName))
	, m_data(std::move(data)) { }

GroupFile::GroupFile(const std::string & fileName, const ByteBuffer & data)
	: m_fileName(Utilities::trimString(fileName))
	, m_data(data) { }

GroupFile::GroupFile(GroupFile && g) noexcept
	: m_fileName(std::move(g.m_fileName))
	, m_data(std::move(g.m_data)) { }

GroupFile::GroupFile(const GroupFile & g)
	: m_fileName(g.m_fileName)
	, m_data(g.m_data) { }

GroupFile & GroupFile::operator = (GroupFile && g) noexcept {
	if(this != &g) {
		m_fileName = std::move(g.m_fileName);
		m_data = std::move(g.m_data);
	}

	return *this;
}

GroupFile & GroupFile::operator = (const GroupFile & g) {
	m_fileName = g.m_fileName;
	m_data = g.m_data;

	return *this;
}

GroupFile::~GroupFile() = default;

const std::string & GroupFile::getFileName() const {
	return m_fileName;
}

std::string_view GroupFile::getFileExtension() const {
	return Utilities::getFileExtension(m_fileName);
}

size_t GroupFile::getSize() const {
	return m_data.getSize();
}

const ByteBuffer & GroupFile::getData() const {
	return m_data;
}

void GroupFile::setFileName(const std::string & fileName) {
	m_fileName = Utilities::trimString(fileName);
}

void GroupFile::setData(const uint8_t * data, size_t size) {
	m_data.clear();

	if(data == nullptr || size == 0) {
		return;
	}

	m_data.resize(size, 0);
	memcpy(m_data.getRawData(), data, size);
}

void GroupFile::setData(std::vector<uint8_t> && data) noexcept {
	m_data = std::move(data);
}

void GroupFile::setData(const std::vector<uint8_t> & data) {
	m_data = data;
}

void GroupFile::setData(const ByteBuffer & data) {
	m_data = data;
}

void GroupFile::clearData() {
	m_data.clear();
}

std::string GroupFile::toString() const {
	return fmt::format("{0} ({1} byte{2})", m_fileName, m_data.getSize(), m_data.getSize() == 1 ? "" : "s");
}

bool GroupFile::isValid() const {
	return !m_fileName.empty() && m_fileName.length() <= MAX_FILE_NAME_LENGTH;
}

bool GroupFile::isValid(const GroupFile * g) {
	return g != nullptr && g->isValid();
}

bool GroupFile::writeTo(const std::string & basePath, bool overwrite, const std::string & alternateFileName) const {
	if(!isValid()) {
		fmt::print("Failed to write '{}' file, file is not valid.\n", m_fileName);
		return false;
	}

	std::string filePath(Utilities::joinPaths(basePath, alternateFileName.empty() ? m_fileName : alternateFileName));

	if(!overwrite && std::filesystem::exists(std::filesystem::path(filePath))) {
		fmt::print("File '{}' already exists, use overwrite to force write.\n", filePath);
		return false;
	}

	if(!basePath.empty()) {
		if(!std::filesystem::exists(std::filesystem::path(basePath))) {
			std::error_code errorCode;
			std::filesystem::create_directories(basePath, errorCode);

			if(errorCode) {
				fmt::print("Failed to create directory structure for base path '{}': {}\n", basePath, errorCode.message());
				return false;
			}
		}
	}

	std::ofstream fileStream(filePath, std::ios::binary);

	if(!fileStream.is_open()) {
		fmt::print("Failed to open write stream for file '{}'.\n", filePath);
		return false;
	}

	fileStream.write(reinterpret_cast<const char *>(m_data.getRawData()), m_data.getSize());

	fileStream.close();

	return true;
}

bool GroupFile::operator == (const GroupFile & g) const {
	return Utilities::compareStringsIgnoreCase(m_fileName, g.m_fileName) == 0 && m_data == g.m_data;
}

bool GroupFile::operator != (const GroupFile & g) const {
	return !operator == (g);
}
