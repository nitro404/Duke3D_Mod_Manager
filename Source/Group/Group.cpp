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

const bool Group::DEFAULT_REPLACE_FILES = true;

const Endianness Group::FILE_ENDIANNESS = Endianness::LittleEndian;

const std::string Group::HEADER_TEXT("KenSilverman");

Group::Group(std::string_view filePath)
	: m_filePath(filePath)
	, m_modified(false) { }

Group::Group(Group && g) noexcept
	: m_filePath(std::move(g.m_filePath))
	, m_files(std::move(g.m_files))
	, m_modified(false) {
	updateParentGroup();

	for(std::shared_ptr<GroupFile> & file : m_files) {
		file->addListener(*this);
	}
}

Group::Group(const Group & g)
	: m_filePath(g.m_filePath)
	, m_modified(false) {
	for(std::vector<std::shared_ptr<GroupFile>>::const_iterator i = g.m_files.cbegin(); i != g.m_files.cend(); ++i) {
		m_files.push_back(std::make_shared<GroupFile>(**i));
	}

	updateParentGroup();

	for(std::shared_ptr<GroupFile> & file : m_files) {
		file->addListener(*this);
	}
}

Group & Group::operator = (Group && g) noexcept {
	if(this != &g) {
		m_filePath = std::move(g.m_filePath);
		m_files = std::move(g.m_files);

		updateParentGroup();
		setModified(true);

		for(std::shared_ptr<GroupFile> & file : m_files) {
			file->addListener(*this);
		}
	}

	return *this;
}

Group & Group::operator = (const Group & g) {
	m_files.clear();

	m_filePath = g.m_filePath;

	for(std::vector<std::shared_ptr<GroupFile>>::const_iterator i = g.m_files.cbegin(); i != g.m_files.cend(); ++i) {
		m_files.push_back(std::make_shared<GroupFile>(**i));
	}

	updateParentGroup();
	setModified(true);

	for(std::shared_ptr<GroupFile> & file : m_files) {
		file->addListener(*this);
	}

	return *this;
}

Group::~Group() {
	for(std::shared_ptr<GroupFile> & file : m_files) {
		file->removeListener(*this);
		file->m_parentGroup = nullptr;
	}
}

bool Group::isModified() const {
	return m_modified;
}

void Group::setModified(bool modified) const {
	m_modified = modified;

	if(!m_modified) {
		for(const std::shared_ptr<GroupFile> & file : m_files) {
			file->m_modified = false;
		}
	}

	notifyGroupModified();
}

const std::string & Group::getFilePath() const {
	return m_filePath;
}

std::string_view Group::getFileName() const {
	return Utilities::getFileName(m_filePath);
}

std::string_view Group::getFileExtension() const {
	return Utilities::getFileExtension(m_filePath);
}

void Group::setFilePath(const std::string & filePath) {
	m_filePath = filePath;
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

bool Group::extractFile(size_t index, const std::string & directory, bool overwrite) const {
	if(index >= m_files.size()) {
		return false;
	}

	return m_files[index]->writeTo(directory, overwrite);
}

bool Group::extractFileWithName(const std::string & fileName, const std::string & directory, bool overwrite) const {
	return extractFile(indexOfFileWithName(fileName), directory, overwrite);
}

bool Group::extractFirstFileWithExtension(const std::string & extension, const std::string & directory, bool overwrite) const {
	return extractFile(indexOfFirstFileWithExtension(extension), directory, overwrite);
}

bool Group::extractLastFileWithExtension(const std::string & extension, const std::string & directory, bool overwrite) const {
	return extractFile(indexOfLastFileWithExtension(extension), directory, overwrite);
}

size_t Group::extractAllFilesWithExtension(const std::string & extension, const std::string & directory, bool overwrite) const {
	if(extension.empty()) {
		return 0;
	}

	size_t numberOfExtractedFiles = 0;

	for(size_t i = 0; i < m_files.size(); i++) {
		if(Utilities::areStringsEqualIgnoreCase(m_files[i]->getFileExtension(), extension)) {
			if(extractFile(i, directory, overwrite)) {
				numberOfExtractedFiles++;
			}
		}
	}

	return numberOfExtractedFiles;
}

size_t Group::extractAllFiles(const std::string & directory, bool overwrite) const {
	size_t numberOfExtractedFiles = 0;

	for(size_t i = 0; i < m_files.size(); i++) {
		if(extractFile(i, directory, overwrite)) {
			numberOfExtractedFiles++;
		}
	}

	return numberOfExtractedFiles;
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
		m_files.back()->addListener(*this);

		setModified(true);

		return true;
	}
	else {
		if(replace) {
			m_files[fileIndex] = std::shared_ptr<GroupFile>(file.release());
			m_files[fileIndex]->m_parentGroup = this;
			m_files[fileIndex]->addListener(*this);

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

	m_files[index]->removeListener(*this);
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
	for(std::shared_ptr<GroupFile> & file : m_files) {
		file->removeListener(*this);
		file->m_parentGroup = nullptr;
	}

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

bool Group::isValid(const Group * g, bool verifyParent) {
	return g != nullptr && g->isValid();
}

std::unique_ptr<Group> Group::createFrom(const std::string & directoryPath) {
	if(directoryPath.empty()) {
		return nullptr;
	}

	std::filesystem::path directory(directoryPath);

	if(!std::filesystem::is_directory(directory)) {
		return nullptr;
	}

	std::unique_ptr<Group> group(std::make_unique<Group>());

	for(const std::filesystem::directory_entry & entry : std::filesystem::directory_iterator(directory)) {
		if(!entry.is_regular_file()) {
			continue;
		}

		if(!group->addFile(entry.path().string())) {
			spdlog::warn("Failed to read and add file '{}' to group file from directory: '{}'.", Utilities::getFileName(entry.path().string()), directoryPath);
		}
	}

	group->setModified(false);

	return group;
}

std::unique_ptr<Group> Group::loadFrom(const std::string & filePath) {
	// verify that the file has a path
	if(filePath.empty()) {
		spdlog::error("Group has no file name.");
		return false;
	}

	// verify that the file exists and is a file
	if(!std::filesystem::is_regular_file(std::filesystem::path(filePath))) {
		spdlog::error("Group file does not exist or is not a file: '{}'.", filePath);
		return false;
	}

	// open the file and read it into memory
	std::unique_ptr<ByteBuffer> buffer(ByteBuffer::readFrom(filePath, Group::FILE_ENDIANNESS));

	if(buffer == nullptr) {
		spdlog::error("Failed to open group file: '{}'.", filePath);
		return false;
	}

	spdlog::debug("Opened group file: '{}', loaded {} bytes into memory.", filePath, buffer->getSize());

	bool error = false;

	// verify that the data is long enough to contain header information
	std::string headerText(buffer->readString(Group::HEADER_TEXT.length(), &error));

	if(error) {
		spdlog::error("Group file '{}' is incomplete or corrupted: missing header text.", filePath);
		return false;
	}

	// verify that the header text is specified in the header
	if(headerText != HEADER_TEXT) {
		spdlog::error("Group file '{}' is not a valid format, missing '{}' header text.", filePath, HEADER_TEXT);
		return false;
	}

	spdlog::debug("Verified group file header text.");

	// read and verify the number of files value
	uint32_t numberOfFiles = buffer->readUnsignedInteger(&error);

	if(error) {
		spdlog::error("Group file '{}' is incomplete or corrupted: missing number of files value.", filePath);
		return false;
	}

	spdlog::debug("Detected {} files in group '{}'.", numberOfFiles, filePath);

	std::vector<std::string> fileNames;
	std::vector<uint32_t> fileSizes;
	std::vector<std::shared_ptr<GroupFile>> files;

	for(uint32_t i = 0; i < numberOfFiles; i++) {
		// read the file name
		fileNames.emplace_back(buffer->readString(GroupFile::MAX_FILE_NAME_LENGTH, &error));

		if(error) {
			spdlog::error("Group file '{}' is incomplete or corrupted: missing file #{} name.", filePath, i + 1);
			return false;
		}

		// read and verify the file size
		fileSizes.push_back(buffer->readUnsignedInteger(&error));

		if(error) {
			spdlog::error("Group file '{}' is incomplete or corrupted: missing file #{} size value.", filePath, i + 1);
			return false;
		}
	}

	spdlog::debug("All group file information parsed.");

	std::unique_ptr<Group> group(std::make_unique<Group>(filePath));

	for(uint32_t i = 0; i < numberOfFiles; i++) {
		if(buffer->getSize() < buffer->getReadOffset() + fileSizes[i]) {
			size_t numberOfMissingBytes = fileSizes[i] - (buffer->getSize() - buffer->getReadOffset());
			uint32_t numberOfAdditionalFiles = group->m_files.size() - i - 1;

			spdlog::error("Group file '{}' is corrupted: missing {} of {} byte{} for file #{} ('{}') data.{}", filePath, numberOfMissingBytes, fileSizes[i], fileSizes[i] == 1 ? "" : "s", i + 1, fileNames[i], numberOfAdditionalFiles > 0 ? fmt::format(" There is also an additional {} files that are missing data.", numberOfAdditionalFiles) : "");

			return false;
		}

		std::shared_ptr<GroupFile> file(std::make_shared<GroupFile>(fileNames[i], buffer->readBytes(fileSizes[i], &error)));

		if(error) {
			spdlog::error("Group file '{}' failed to read data bytes for file #{} ('{}').", filePath, i + 1, fileNames[i]);
		}

		file->m_parentGroup = group.get();
		file->addListener(*group);
		group->m_files.push_back(file);
	}

	spdlog::debug("Group file parsed successfully, {} files loaded into memory.", group->m_files.size());

	return group;
}

bool Group::save(bool overwrite) const {
	// verify that the file has a path
	if(m_filePath.empty()) {
		spdlog::error("Group has no file name.");
		return false;
	}

	// check if the file already exists and verify that overwrite is specified to continue
	if(!overwrite && std::filesystem::is_regular_file(std::filesystem::path(m_filePath))) {
		spdlog::warn("Group file already exists, must specify overwrite flag to save.", m_filePath);
		return false;
	}

	ByteBuffer buffer(getGroupSize(), Group::FILE_ENDIANNESS);

	if(!buffer.writeString(HEADER_TEXT)) {
		return false;
	}

	if(!buffer.writeUnsignedInteger(m_files.size())) {
		return false;
	}

	size_t currentFileNameLength = 0;

	for(size_t i = 0; i < m_files.size(); i++) {
		const std::shared_ptr<GroupFile> file(m_files.at(i));

		if(!buffer.writeString(file->getFileName())) {
			return false;
		}

		currentFileNameLength = file->getFileName().length();

		if(currentFileNameLength < GroupFile::MAX_FILE_NAME_LENGTH) {
			if(!buffer.skipWriteBytes(GroupFile::MAX_FILE_NAME_LENGTH - currentFileNameLength)) {
				return false;
			}
		}

		if(!buffer.writeUnsignedInteger(file->getSize())) {
			return false;
		}
	}

	for(size_t i = 0; i < m_files.size(); i++) {
		const std::shared_ptr<GroupFile> file(m_files.at(i));

		if(!buffer.writeBytes(file->getData())) {
			return false;
		}
	}

	if(!buffer.writeTo(m_filePath, overwrite)) {
		return false;
	}

	setModified(false);

	return true;
}

size_t Group::getGroupSize() const {
	static constexpr size_t NUMBER_OF_FILES_LENGTH = sizeof(uint32_t);
	static constexpr size_t GROUP_FILE_SIZE_LENGTH = sizeof(uint32_t);
	static const size_t HEADER_LENGTH = HEADER_TEXT.length() + NUMBER_OF_FILES_LENGTH;
	static const size_t GROUP_FILE_HEADER_LENGTH = GroupFile::MAX_FILE_NAME_LENGTH + GROUP_FILE_SIZE_LENGTH;

	size_t size = HEADER_LENGTH;

	for(std::vector<std::shared_ptr<GroupFile>>::const_iterator i = m_files.cbegin(); i != m_files.cend(); ++i) {
		size += GROUP_FILE_HEADER_LENGTH + (*i)->getSize();
	}

	return size;
}

std::string Group::getGroupSizeAsString() const {
	size_t size = getGroupSize();

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

void Group::updateParentGroup() {
	for(std::shared_ptr<GroupFile> & file : m_files) {
		file->m_parentGroup = this;
	}
}

Group::Listener::~Listener() { }

size_t Group::numberOfListeners() const {
	return m_listeners.size();
}

bool Group::hasListener(const Listener & listener) const {
	for(std::vector<Listener *>::const_iterator i = m_listeners.cbegin(); i != m_listeners.cend(); ++i) {
		if(*i == &listener) {
			return true;
		}
	}

	return false;
}

size_t Group::indexOfListener(const Listener & listener) const {
	for(size_t i = 0; i < m_listeners.size(); i++) {
		if(m_listeners[i] == &listener) {
			return i;
		}
	}

	return std::numeric_limits<size_t>::max();
}

Group::Listener * Group::getListener(size_t index) const {
	if(index >= m_listeners.size()) {
		return nullptr;
	}

	return m_listeners[index];
}

bool Group::addListener(Listener & listener) {
	if(!hasListener(listener)) {
		m_listeners.push_back(&listener);

		return true;
	}

	return false;
}

bool Group::removeListener(size_t index) {
	if(index >= m_listeners.size()) {
		return false;
	}

	m_listeners.erase(m_listeners.cbegin() + index);

	return true;
}

bool Group::removeListener(const Listener & listener) {
	for(std::vector<Listener *>::const_iterator i = m_listeners.cbegin(); i != m_listeners.cend(); ++i) {
		if(*i == &listener) {
			m_listeners.erase(i);

			return true;
		}
	}

	return false;
}

void Group::clearListeners() {
	m_listeners.clear();
}

void Group::notifyGroupModified() const {
	for(Listener * listener : m_listeners) {
		listener->groupModified(this, m_modified);
	}
}

void Group::groupFileModified(const GroupFile * groupFile, bool modified) {
	if(modified) {
		setModified(true);
	}
}

bool Group::operator == (const Group & g) const {
	if(m_files.size() != g.m_files.size()) {
		return false;
	}

	for(std::vector<std::shared_ptr<GroupFile>>::const_iterator i = m_files.cbegin(); i != m_files.cend(); ++i) {
		if(!g.hasFile(**i)) {
			return false;
		}
	}

	return true;
}

bool Group::operator != (const Group & g) const {
	return !operator == (g);
}
