#include "Group.h"

#include <ByteBuffer.h>
#include <Utilities/FileUtilities.h>
#include <Utilities/StringUtilities.h>
#include <Utilities/Utilities.h>

#include <fmt/core.h>
#include <spdlog/spdlog.h>

#include <filesystem>
#include <memory>
#include <sstream>

Group::Group(const std::string & filePath)
	: GameFile(filePath) { }

Group::Group(std::vector<std::unique_ptr<GroupFile>> files, const std::string & filePath)
	: GameFile(filePath) {
	m_files.reserve(files.size());

	for(std::unique_ptr<GroupFile> & file : files) {
		m_files.push_back(std::shared_ptr<GroupFile>(file.release()));
	}

	updateParent();
	connectSignals();
}

Group::Group(Group && g) noexcept
	: GameFile(std::move(g))
	, m_files(std::move(g.m_files)) {
	updateParent();
	connectSignals();
}

Group::Group(const Group & g)
	: GameFile(g) {
	for(std::vector<std::shared_ptr<GroupFile>>::const_iterator i = g.m_files.cbegin(); i != g.m_files.cend(); ++i) {
		m_files.push_back(std::make_shared<GroupFile>(**i));
	}

	updateParent();
	connectSignals();
}

Group & Group::operator = (Group && g) noexcept {
	if(this != &g) {
		GameFile::operator = (std::move(g));

		m_files = std::move(g.m_files);

		for(boost::signals2::connection & fileConnections : m_fileConnections) {
			fileConnections.disconnect();
		}

		m_fileConnections.clear();

		updateParent();
		connectSignals();
	}

	return *this;
}

Group & Group::operator = (const Group & g) {
	GameFile::operator = (g);

	m_files.clear();

	for(boost::signals2::connection & fileConnections : m_fileConnections) {
		fileConnections.disconnect();
	}

	m_fileConnections.clear();

	for(std::vector<std::shared_ptr<GroupFile>>::const_iterator i = g.m_files.cbegin(); i != g.m_files.cend(); ++i) {
		m_files.push_back(std::make_shared<GroupFile>(**i));
	}

	updateParent();
	connectSignals();

	return *this;
}

Group::~Group() {
	for(boost::signals2::connection & fileConnections : m_fileConnections) {
		fileConnections.disconnect();
	}

	for(std::shared_ptr<GroupFile> & file : m_files) {
		file->m_parentGroup = nullptr;
	}
}

void Group::setModified(bool modified) const {
	if(!modified) {
		for(const std::shared_ptr<GroupFile> & file : m_files) {
			file->m_modified = false;
		}
	}

	GameFile::setModified(modified);
}

size_t Group::numberOfFiles() const {
	return m_files.size();
}

bool Group::hasFile(const GroupFile & file) const {
	return indexOfFile(file) != std::numeric_limits<size_t>::max();
}

bool Group::hasFileWithName(const std::string & fileName) const {
	return indexOfFileWithName(fileName) != std::numeric_limits<size_t>::max();
}

size_t Group::indexOfFileWithName(const std::string & fileName) const {
	if(fileName.empty()) {
		return std::numeric_limits<size_t>::max();
	}

	std::vector<std::shared_ptr<GroupFile>>::const_iterator fileIterator = std::find_if(m_files.cbegin(), m_files.cend(), [&fileName](const std::shared_ptr<GroupFile> & file) {
		return Utilities::areStringsEqualIgnoreCase(file->getFileName(), fileName);
	});

	if(fileIterator == m_files.cend()) {
		return std::numeric_limits<size_t>::max();
	}

	return fileIterator - m_files.cbegin();
}

size_t Group::indexOfFirstFileWithExtension(const std::string & extension) const {
	if(extension.empty()) {
		return std::numeric_limits<size_t>::max();
	}

	for(std::vector<std::shared_ptr<GroupFile>>::const_iterator i = m_files.cbegin(); i != m_files.cend(); ++i) {
		if(Utilities::areStringsEqualIgnoreCase((*i)->getFileExtension(), extension)) {
			return i - m_files.cbegin();
		}
	}

	return std::numeric_limits<size_t>::max();
}

size_t Group::indexOfLastFileWithExtension(const std::string & extension) const {
	if(extension.empty()) {
		return std::numeric_limits<size_t>::max();
	}

	for(std::vector<std::shared_ptr<GroupFile>>::const_reverse_iterator i = m_files.crbegin(); i != m_files.crend(); ++i) {
		if(Utilities::areStringsEqualIgnoreCase((*i)->getFileExtension(), extension)) {
			return m_files.crend() - i - 1;
		}
	}

	return std::numeric_limits<size_t>::max();
}

size_t Group::indexOfFile(const GroupFile & file) const {
	if(!file.isValid()) {
		return std::numeric_limits<size_t>::max();
	}

	std::vector<std::shared_ptr<GroupFile>>::const_iterator fileIterator = std::find_if(m_files.cbegin(), m_files.cend(), [&file](const std::shared_ptr<GroupFile> & currentFile) {
		return currentFile.get() == &file;
	});

	if(fileIterator == m_files.cend()) {
		return std::numeric_limits<size_t>::max();
	}

	return fileIterator - m_files.cbegin();
}

std::shared_ptr<GroupFile> Group::getFile(size_t index) const {
	if(index >= m_files.size()) {
		return nullptr;
	}

	return m_files[index];
}

std::shared_ptr<GroupFile> Group::getFileWithName(const std::string & fileName) const {
	return getFile(indexOfFileWithName(fileName));
}

std::shared_ptr<GroupFile> Group::getFirstFileWithExtension(const std::string & extension) const {
	return getFile(indexOfFirstFileWithExtension(extension));
}

std::shared_ptr<GroupFile> Group::getLastFileWithExtension(const std::string & extension) const {
	return getFile(indexOfLastFileWithExtension(extension));
}

const std::vector<std::shared_ptr<GroupFile>> & Group::getFiles() const {
	return m_files;
}

std::vector<std::shared_ptr<GroupFile>> Group::getFilesWithExtension(const std::string & extension) const {
	std::vector<std::shared_ptr<GroupFile>> files;

	if(extension.empty()) {
		return files;
	}

	for(std::vector<std::shared_ptr<GroupFile>>::const_iterator i = m_files.cbegin(); i != m_files.cend(); ++i) {
		if(Utilities::areStringsEqualIgnoreCase((*i)->getFileExtension(), extension)) {
			files.push_back(*i);
		}
	}

	return files;
}

std::vector<std::string> Group::getFileExtensions() const {
	std::vector<std::string> fileExtensions;

	std::string extension;
	bool duplicateExtension = false;

	for(std::vector<std::shared_ptr<GroupFile>>::const_iterator i = m_files.cbegin(); i != m_files.cend(); ++i) {
		extension = (*i)->getFileExtension();

		if(extension.empty()) {
			continue;
		}

		duplicateExtension = false;

		for(std::vector<std::string>::const_iterator j = fileExtensions.cbegin(); j != fileExtensions.cend(); ++j) {
			if(Utilities::areStringsEqualIgnoreCase(*j, extension)) {
				duplicateExtension = true;
				break;
			}
		}

		if(duplicateExtension) {
			continue;
		}

		fileExtensions.push_back(Utilities::toUpperCase(extension));
	}

	return fileExtensions;
}

std::string Group::getFileExtensionsAsString() const {
	std::stringstream fileExtensionsStream;
	std::vector<std::string> fileExtensions(getFileExtensions());

	for(const std::string & extension : fileExtensions) {
		if(fileExtensionsStream.tellp() != 0) {
			fileExtensionsStream << ", ";
		}

		fileExtensionsStream << extension;
	}

	return fileExtensionsStream.str();
}

std::shared_ptr<GroupFile> Group::extractFile(size_t index, const std::string & directory, bool overwrite) const {
	return index < m_files.size() && m_files[index]->writeTo(directory, overwrite) ? m_files[index] : nullptr;
}

std::shared_ptr<GroupFile> Group::extractFileWithName(const std::string & fileName, const std::string & directory, bool overwrite) const {
	return extractFile(indexOfFileWithName(fileName), directory, overwrite);
}

std::shared_ptr<GroupFile> Group::extractFirstFileWithExtension(const std::string & extension, const std::string & directory, bool overwrite) const {
	return extractFile(indexOfFirstFileWithExtension(extension), directory, overwrite);
}

std::shared_ptr<GroupFile> Group::extractLastFileWithExtension(const std::string & extension, const std::string & directory, bool overwrite) const {
	return extractFile(indexOfLastFileWithExtension(extension), directory, overwrite);
}

std::vector<std::shared_ptr<GroupFile>> Group::extractAllFilesWithExtension(const std::string & extension, const std::string & directory, bool overwrite) const {
	if(extension.empty()) {
		return {};
	}

	std::vector<std::shared_ptr<GroupFile>> extractedFiles;

	for(size_t i = 0; i < m_files.size(); i++) {
		if(Utilities::areStringsEqualIgnoreCase(m_files[i]->getFileExtension(), extension)) {
			if(extractFile(i, directory, overwrite)) {
				extractedFiles.push_back(m_files[i]);
			}
		}
	}

	return extractedFiles;
}

std::vector<std::shared_ptr<GroupFile>> Group::extractAllFiles(const std::string & directory, bool overwrite) const {
	std::vector<std::shared_ptr<GroupFile>> extractedFiles;

	for(size_t i = 0; i < m_files.size(); i++) {
		if(extractFile(i, directory, overwrite)) {
			extractedFiles.push_back(m_files[i]);
		}
	}

	return extractedFiles;
}

bool Group::addFile(const std::string & filePath, bool replace) {
	std::unique_ptr<ByteBuffer> fileData(ByteBuffer::readFrom(filePath));

	if(fileData == nullptr) {
		return false;
	}

	return addFile(std::make_unique<GroupFile>(Utilities::toUpperCase(Utilities::truncateFileName(Utilities::getFileName(filePath), GroupFile::MAX_FILE_NAME_LENGTH)), std::move(fileData)), replace);
}

bool Group::addFile(std::unique_ptr<GroupFile> file, bool replace) {
	if(!GroupFile::isValid(file.get())) {
		return false;
	}

	size_t fileIndex = indexOfFileWithName(file->getFileName());

	if(fileIndex == std::numeric_limits<size_t>::max()) {
		m_files.emplace_back(file.release());
		m_files.back()->m_parentGroup = this;
		m_fileConnections.push_back(m_files.back()->modified.connect(std::bind(&Group::onGroupFileModified, this, std::placeholders::_1)));

		setModified(true);

		return true;
	}
	else {
		if(replace) {
			m_files[fileIndex] = std::shared_ptr<GroupFile>(file.release());
			m_files[fileIndex]->m_parentGroup = this;
			m_fileConnections[fileIndex].disconnect();
			m_fileConnections[fileIndex] = m_files[fileIndex]->modified.connect(std::bind(&Group::onGroupFileModified, this, std::placeholders::_1));

			setModified(true);

			return true;
		}
	}

	return false;
}

bool Group::addFile(const GroupFile & file, bool replace) {
	if(!file.isValid()) {
		return false;
	}

	return addFile(std::make_unique<GroupFile>(file), replace);
}

bool Group::addFiles(const std::vector<std::string> & filePaths, bool replace) {
	size_t numberOfFilesAdded = 0;

	for(std::vector<std::string>::const_iterator i = filePaths.cbegin(); i != filePaths.cend(); ++i) {
		if(addFile(*i, replace)) {
			numberOfFilesAdded++;
		}
	}

	return numberOfFilesAdded;
}

bool Group::addFiles(const std::vector<std::shared_ptr<GroupFile>> & files, bool replace) {
	size_t numberOfFilesAdded = 0;

	for(std::vector<std::shared_ptr<GroupFile>>::const_iterator i = files.cbegin(); i != files.cend(); ++i) {
		if(*i == nullptr) {
			continue;
		}

		if(addFile(**i, replace)) {
			numberOfFilesAdded++;
		}
	}

	return numberOfFilesAdded;
}

bool Group::addFiles(const Group & group, bool replace) {
	size_t numberOfFilesAdded = 0;

	for(std::vector<std::shared_ptr<GroupFile>>::const_iterator i = group.m_files.cbegin(); i != group.m_files.cend(); ++i) {
		if(addFile(**i, replace)) {
			numberOfFilesAdded++;
		}
	}

	return numberOfFilesAdded;
}

bool Group::renameFile(size_t index, const std::string & newFileName) {
	if(index >= m_files.size()) {
		return false;
	}

	return m_files[index]->setFileName(newFileName);
}

bool Group::renameFile(const GroupFile & file, const std::string & newFileName) {
	return renameFile(indexOfFile(file), newFileName);
}

bool Group::renameFile(const std::string & oldFileName, const std::string & newFileName) {
	return renameFile(indexOfFileWithName(oldFileName), newFileName);
}

bool Group::replaceFile(size_t index, const std::string & newFilePath, bool keepExistingFileName) {
	if(index >= m_files.size()) {
		return false;
	}

	std::string formattedNewFileName(Utilities::toUpperCase(Utilities::truncateFileName(Utilities::getFileName(newFilePath), GroupFile::MAX_FILE_NAME_LENGTH)));

	if(!keepExistingFileName && !Utilities::areStringsEqualIgnoreCase(m_files[index]->getFileName(), formattedNewFileName) && hasFileWithName(formattedNewFileName)) {
		return false;
	}

	std::unique_ptr<ByteBuffer> data(ByteBuffer::readFrom(newFilePath));

	if(data == nullptr) {
		return false;
	}

	if(keepExistingFileName) {
		return replaceFile(index, std::move(data));
	}
	else {
		return replaceFile(index, std::make_unique<GroupFile>(formattedNewFileName, std::move(data)), keepExistingFileName);
	}
}

bool Group::replaceFile(size_t index, std::unique_ptr<GroupFile> file, bool keepExistingFileName) {
	if(index >= m_files.size() || !GroupFile::isValid(file.get())) {
		return false;
	}

	if(!keepExistingFileName && !Utilities::areStringsEqualIgnoreCase(m_files[index]->getFileName(), file->getFileName()) && hasFileWithName(file->getFileName())) {
		return false;
	}

	if(!keepExistingFileName) {
		m_files[index]->setFileName(file->getFileName());
	}

	m_files[index]->setData(std::move(file->transferData()));

	return true;
}

bool Group::replaceFile(size_t index, const GroupFile & file, bool keepExistingFileName) {
	if(index >= m_files.size() || !GroupFile::isValid(&file)) {
		return false;
	}

	if(!keepExistingFileName && !Utilities::areStringsEqualIgnoreCase(m_files[index]->getFileName(), file.getFileName()) && hasFileWithName(file.getFileName())) {
		return false;
	}

	if(!keepExistingFileName) {
		m_files[index]->setFileName(file.getFileName());
	}

	m_files[index]->setData(file.getData());

	return true;
}

bool Group::replaceFile(size_t index, std::unique_ptr<ByteBuffer> data) {
	if(index >= m_files.size()) {
		return false;
	}

	m_files[index]->setData(std::move(data));

	return true;
}

bool Group::replaceFile(size_t index, const std::vector<uint8_t> & data) {
	if(index >= m_files.size()) {
		return false;
	}

	m_files[index]->setData(data);

	return true;
}

bool Group::replaceFile(const GroupFile & file, const std::string & newFilePath, bool keepExistingFileName) {
	return replaceFile(indexOfFile(file), newFilePath, keepExistingFileName);
}

bool Group::replaceFile(const GroupFile & file, std::unique_ptr<GroupFile> newFile, bool keepExistingFileName) {
	return replaceFile(indexOfFile(file), std::move(file), keepExistingFileName);
}

bool Group::replaceFile(const GroupFile & file, const GroupFile & newFile, bool keepExistingFileName) {
	return replaceFile(indexOfFile(file), file, keepExistingFileName);
}

bool Group::replaceFile(const GroupFile & file, std::unique_ptr<ByteBuffer> data) {
	return replaceFile(indexOfFile(file), std::move(data));
}

bool Group::replaceFile(const GroupFile & file, const std::vector<uint8_t> & data) {
	return replaceFile(indexOfFile(file), data);
}

bool Group::replaceFileWithName(const std::string & fileName, const std::string & newFilePath, bool keepExistingFileName) {
	return replaceFile(indexOfFileWithName(fileName), newFilePath, keepExistingFileName);
}

bool Group::replaceFileWithName(const std::string & fileName, std::unique_ptr<GroupFile> newFile, bool keepExistingFileName) {
	return replaceFile(indexOfFileWithName(fileName), std::move(newFile), keepExistingFileName);
}

bool Group::replaceFileWithName(const std::string & fileName, const GroupFile & newFile, bool keepExistingFileName) {
	return replaceFile(indexOfFileWithName(fileName), newFile, keepExistingFileName);
}

bool Group::replaceFileWithName(const std::string & fileName, std::unique_ptr<ByteBuffer> data) {
	return replaceFile(indexOfFileWithName(fileName), std::move(data));
}

bool Group::replaceFileWithName(const std::string & fileName, const std::vector<uint8_t> & data) {
	return replaceFile(indexOfFileWithName(fileName), data);
}

bool Group::removeFile(size_t index) {
	if(index >= m_files.size()) {
		return false;
	}

	m_fileConnections[index].disconnect();
	m_fileConnections.erase(m_fileConnections.cbegin() + index);
	m_files[index]->m_parentGroup = nullptr;
	m_files.erase(m_files.cbegin() + index);

	setModified(true);

	return true;
}

bool Group::removeFile(const GroupFile & file) {
	return removeFile(indexOfFile(file));
}

bool Group::removeFileWithName(const std::string & fileName) {
	return removeFile(indexOfFileWithName(fileName));
}
size_t Group::removeFiles(const std::vector<std::shared_ptr<GroupFile>> & files) {
	if(files.empty()) {
		return 0;
	}

	size_t numberOfFilesRemoved = 0;

	for(std::vector<std::shared_ptr<GroupFile>>::const_iterator i = files.cbegin(); i != files.cend(); ++i) {
		if(*i == nullptr) {
			continue;
		}

		if(removeFile(**i)) {
			numberOfFilesRemoved++;
		}
	}

	return numberOfFilesRemoved;
}

size_t Group::removeFilesByName(const std::vector<std::string> & fileNames) {
	if(fileNames.empty()) {
		return 0;
	}

	size_t numberOfFilesRemoved = 0;

	for(std::vector<std::string>::const_iterator i = fileNames.cbegin(); i != fileNames.cend(); ++i) {
		if(removeFile(*i)) {
			numberOfFilesRemoved++;
		}
	}

	return numberOfFilesRemoved;
}

void Group::clearFiles() {
	for(boost::signals2::connection & fileConnection : m_fileConnections) {
		fileConnection.disconnect();
	}

	for(std::shared_ptr<GroupFile> & file : m_files) {
		file->m_parentGroup = nullptr;
	}

	m_fileConnections.clear();
	m_files.clear();

	setModified(true);
}

std::string Group::toString() const {
	std::stringstream groupString;

	if(!m_filePath.empty()) {
		groupString << fmt::format("\"{0}\" ", Utilities::getFileName(m_filePath));
	}

	groupString << fmt::format("Files ({0}){1}", m_files.size(), m_files.empty() ? "" : ": ");

	for(std::vector<std::shared_ptr<GroupFile>>::const_iterator i = m_files.cbegin(); i != m_files.cend(); ++i) {
		if(i != m_files.cbegin()) {
			groupString << ", ";
		}

		groupString << (*i)->getFileName();
	}

	return groupString.str();
}

bool Group::isValid(bool verifyParent) const {
	for(const std::shared_ptr<GroupFile> & file : m_files) {
		if(!file->isValid()) {
			return false;
		}

		if(verifyParent && file->m_parentGroup != this) {
			return false;
		}
	}

	return true;
}

bool Group::isValid(const Group * group, bool verifyParent) {
	return group != nullptr &&
		   group->isValid(verifyParent);
}

std::vector<std::unique_ptr<GroupFile>> Group::createGroupFilesFromDirectory(const std::string & directoryPath) {
	if(directoryPath.empty() || !std::filesystem::is_directory(std::filesystem::path(directoryPath))) {
		spdlog::error("Cannot create group files from invalid or missing directory path: '{}'.", directoryPath);
		return {};
	}

	std::vector<std::unique_ptr<GroupFile>> groupFiles;

	for(const std::filesystem::directory_entry & entry : std::filesystem::directory_iterator(std::filesystem::path(directoryPath))) {
		if(!entry.is_regular_file()) {
			continue;
		}

		std::string filePath(entry.path().string());
		std::unique_ptr<ByteBuffer> fileData(ByteBuffer::readFrom(filePath));

		if(fileData == nullptr) {
			spdlog::warn("Failed to read file data from: '{}'.", filePath);
			continue;
		}

		groupFiles.emplace_back(std::make_unique<GroupFile>(Utilities::toUpperCase(Utilities::truncateFileName(Utilities::getFileName(filePath), GroupFile::MAX_FILE_NAME_LENGTH)), std::move(fileData)));
	}

	return groupFiles;
}

void Group::addMetadata(std::vector<std::pair<std::string, std::string>> & metadata) const {
	metadata.push_back({ "Number of Files", std::to_string(m_files.size()) });

	if(!m_files.empty()) {
		metadata.push_back({ "File Extensions", getFileExtensionsAsString() });
	}

	for(std::vector<std::shared_ptr<GroupFile>>::const_iterator i = m_files.cbegin(); i != m_files.cend(); ++i) {
		metadata.push_back({ fmt::format("Entry #{}", i - m_files.cbegin() + 1), fmt::format("'{}' Size: {}", (*i)->getFileName(), (*i)->getSize()) });
	}
}

void Group::connectSignals() {
	for(std::shared_ptr<GroupFile> & file : m_files) {
		m_fileConnections.push_back(file->modified.connect(std::bind(&Group::onGroupFileModified, this, std::placeholders::_1)));
	}
}

void Group::updateParent() {
	for(std::shared_ptr<GroupFile> & file : m_files) {
		file->m_parentGroup = this;
	}
}

void Group::onGroupFileModified(GroupFile & groupFile) {
	if(groupFile.isModified()) {
		setModified(true);
	}
}

bool Group::operator == (const Group & group) const {
	if(m_files.size() != group.m_files.size()) {
		return false;
	}

	for(size_t i = 0; i < m_files.size(); i++) {
		if(*m_files[i] != *group.m_files[i]) {
			return false;
		}
	}

	return true;
}

bool Group::operator != (const Group & group) const {
	return !operator == (group);
}
