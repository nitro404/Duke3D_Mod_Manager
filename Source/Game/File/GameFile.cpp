#include "GameFile.h"

#include <ByteBuffer.h>
#include <Utilities/FileUtilities.h>
#include <Utilities/StringUtilities.h>

#include <magic_enum.hpp>
#include <spdlog/spdlog.h>

#include <filesystem>

GameFile::GameFile(const std::string & filePath)
	: m_filePath(filePath)
	, m_modified(false) { }


GameFile::GameFile(GameFile && gameFile) noexcept
	: m_filePath(std::move(gameFile.m_filePath))
	, m_modified(false) { }

GameFile::GameFile(const GameFile & gameFile)
	: m_filePath(gameFile.m_filePath)
	, m_modified(false) { }

GameFile & GameFile::operator = (GameFile && gameFile) noexcept {
	if(this != &gameFile) {
		m_filePath = std::move(gameFile.m_filePath);

		setModified(true);
	}

	return *this;
}

GameFile & GameFile::operator = (const GameFile & gameFile) {
	m_filePath = gameFile.m_filePath;

	setModified(true);

	return *this;
}

GameFile::~GameFile() { }

const std::string & GameFile::getFilePath() const {
	return m_filePath;
}

std::string_view GameFile::getFileName() const {
	return Utilities::getFileName(m_filePath);
}

std::string_view GameFile::getFileExtension() const {
	return Utilities::getFileExtension(m_filePath);
}

void GameFile::setFilePath(const std::string & filePath) {
	m_filePath = filePath;
}

bool GameFile::isModified() const {
	return m_modified;
}

void GameFile::setModified(bool value) const {
	m_modified = value;

	modified(*this);
}

std::unique_ptr<ByteBuffer> GameFile::getData() const {
	std::unique_ptr<ByteBuffer> data(std::make_unique<ByteBuffer>(getEndianness()));

	if(!writeTo(*data)) {
		return nullptr;
	}

	return data;
}

bool GameFile::save(bool overwrite) {
	if(m_filePath.empty()) {
		spdlog::error("Game file has no file path.");
		return false;
	}

	return saveTo(m_filePath, overwrite);
}

bool GameFile::saveTo(const std::string & filePath, bool overwrite) const {
	if(!overwrite && std::filesystem::is_regular_file(std::filesystem::path(filePath))) {
		spdlog::warn("File '{}' already exists, use overwrite to force write.", filePath);
		return false;
	}

	ByteBuffer byteBuffer(getEndianness());

	if(!writeTo(byteBuffer) || !byteBuffer.writeTo(filePath, overwrite)) {
		return false;
	}

	setModified(false);

	return true;
}

std::vector<std::pair<std::string, std::string>> GameFile::getMetadata() const {
	std::vector<std::pair<std::string, std::string>> metadata;

	std::string_view fileName(Utilities::getFileName(m_filePath));

	if(!fileName.empty()) {
		metadata.push_back({ "File Name", std::string(fileName) });
	}

	metadata.push_back({ "File Size", Utilities::fileSizeToString(getSizeInBytes()) });
	metadata.push_back({ "Endianness", Utilities::toCapitalCase(magic_enum::enum_name(getEndianness())) });

	addMetadata(metadata);

	return metadata;
}
