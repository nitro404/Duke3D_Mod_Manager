#include "Group.h"

#include <ByteBuffer.h>
#include <Utilities/FileUtilities.h>
#include <Utilities/StringUtilities.h>
#include <Utilities/Utilities.h>

#include <fmt/core.h>

#include <filesystem>
#include <memory>
#include <sstream>

const bool Group::DEFAULT_REPLACE_FILES = true;

const Endianness Group::FILE_ENDIANNESS = Endianness::LittleEndian;

const std::string Group::HEADER_TEXT("KenSilverman");

Group::Group(const std::string & filePath)
	: m_filePath(Utilities::trimString(filePath)) { }

Group::Group(Group && g) noexcept
	: m_filePath(std::move(g.m_filePath))
	, m_files(std::move(g.m_files)) { }

Group::Group(const Group & g)
	: m_filePath(g.m_filePath) {
	for(std::vector<std::shared_ptr<GroupFile>>::const_iterator i = g.m_files.begin(); i != g.m_files.end(); ++i) {
		m_files.push_back(std::make_shared<GroupFile>(**i));
	}
}

Group & Group::operator = (Group && g) noexcept {
	if(this != &g) {
		m_filePath = std::move(g.m_filePath);
		m_files = std::move(g.m_files);
	}

	return *this;
}

Group & Group::operator = (const Group & g) {
	m_files.clear();

	m_filePath = g.m_filePath;

	for(std::vector<std::shared_ptr<GroupFile>>::const_iterator i = g.m_files.begin(); i != g.m_files.end(); ++i) {
		m_files.push_back(std::make_shared<GroupFile>(**i));
	}

	return *this;
}

Group::~Group() = default;

const std::string & Group::getFilePath() const {
	return m_filePath;
}

std::string Group::getFileName() const {
	return Utilities::getFileName(m_filePath);
}

std::string Group::getFileExtension() const {
	return Utilities::getFileExtension(m_filePath);
}

void Group::setFilePath(const std::string & filePath) {
	m_filePath = Utilities::trimString(filePath);
}

size_t Group::numberOfFiles() const {
	return m_files.size();
}

bool Group::hasFile(const std::string & fileName) const {
	if(fileName.empty()) {
		return false;
	}

	std::string formattedFileName(Utilities::trimString(fileName));

	if(formattedFileName.empty()) {
		return false;
	}

	for(std::vector<std::shared_ptr<GroupFile>>::const_iterator i = m_files.begin(); i != m_files.end(); ++i) {
		if(Utilities::compareStringsIgnoreCase((*i)->getFileName(), formattedFileName) == 0) {
			return true;
		}
	}

	return false;
}

bool Group::hasFile(const GroupFile & groupFile) const {
	if(!groupFile.isValid()) {
		return false;
	}

	for(std::vector<std::shared_ptr<GroupFile>>::const_iterator i = m_files.begin(); i != m_files.end(); ++i) {
		if(Utilities::compareStringsIgnoreCase((*i)->getFileName(), groupFile.getFileName()) == 0) {
			return true;
		}
	}

	return false;
}

size_t Group::indexOfFile(const std::string & fileName) const {
	if(fileName.empty()) {
		return std::numeric_limits<size_t>::max();
	}

	std::string formattedFileName(Utilities::trimString(fileName));

	if(formattedFileName.empty()) {
		return std::numeric_limits<size_t>::max();
	}

	for(size_t i=0;i<m_files.size();i++) {
		if(Utilities::compareStringsIgnoreCase(m_files[i]->getFileName(), formattedFileName.data()) == 0) {
			return i;
		}
	}

	return std::numeric_limits<size_t>::max();
}

size_t Group::indexOfFile(const GroupFile & groupFile) const {
	if(!groupFile.isValid()) {
		return std::numeric_limits<size_t>::max();
	}

	for(size_t i = 0; i < m_files.size(); i++) {
		if(Utilities::compareStringsIgnoreCase(m_files[i]->getFileName(), groupFile.getFileName()) == 0) {
			return i;
		}
	}

	return std::numeric_limits<size_t>::max();
}

std::shared_ptr<GroupFile> Group::getFile(size_t index) const {
	if(index >= m_files.size()) {
		return nullptr;
	}

	return m_files[index];
}

std::shared_ptr<GroupFile> Group::getFile(const std::string & fileName) const {
	if(fileName.empty()) {
		return nullptr;
	}

	std::string formattedFileName(Utilities::trimString(fileName));

	if(formattedFileName.empty()) {
		return nullptr;
	}

	for(std::vector<std::shared_ptr<GroupFile>>::const_iterator i = m_files.begin(); i != m_files.end(); ++i) {
		if(Utilities::compareStringsIgnoreCase((*i)->getFileName(), formattedFileName.data()) == 0) {
			return *i;
		}
	}

	return nullptr;
}

std::vector<std::shared_ptr<GroupFile>> Group::getFilesWithExtension(const std::string & extension) const {
	std::vector<std::shared_ptr<GroupFile>> files;

	if(extension.empty()) {
		return files;
	}

	for(std::vector<std::shared_ptr<GroupFile>>::const_iterator i = m_files.begin(); i != m_files.end(); ++i) {
		if(Utilities::compareStringsIgnoreCase((*i)->getFileExtension(), extension) == 0) {
			files.push_back(*i);
		}
	}

	return files;
}

std::vector<std::string> Group::getFileExtensions() const {
	std::vector<std::string> fileExtensions;

	std::string extension;
	bool duplicateExtension = false;

	for(std::vector<std::shared_ptr<GroupFile>>::const_iterator i = m_files.begin(); i != m_files.end(); ++i) {
		extension = (*i)->getFileExtension();

		if(extension.empty()) {
			continue;
		}

		duplicateExtension = false;

		for(std::vector<std::string>::const_iterator j = fileExtensions.begin(); j != fileExtensions.end(); ++j) {
			if(Utilities::compareStringsIgnoreCase(*j, extension) == 0) {
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

bool Group::extractFile(size_t index, const std::string & directory, bool overwrite) const {
	if(index >= m_files.size()) {
		return false;
	}

	return m_files[index]->writeTo(directory, overwrite);
}

bool Group::extractFile(const std::string & fileName, const std::string & directory, bool overwrite) const {
	return extractFile(indexOfFile(fileName), directory, overwrite);
}

size_t Group::extractAllFilesWithExtension(const std::string & extension, const std::string & directory, bool overwrite) const {
	if(extension.empty()) {
		return 0;
	}

	size_t numberOfExtractedFiles = 0;

	for(size_t i = 0; i < m_files.size(); i++) {
		if(Utilities::compareStringsIgnoreCase(m_files[i]->getFileExtension(), extension) == 0) {
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

bool Group::addFile(const GroupFile & groupFile, bool replace) {
	if(!groupFile.isValid()) {
		return false;
	}

	size_t fileIndex = indexOfFile(groupFile);

	if(fileIndex == std::numeric_limits<size_t>::max()) {
		m_files.push_back(std::make_shared<GroupFile>(groupFile));
		return true;
	}
	else {
		if(replace) {
			m_files[fileIndex] = std::make_shared<GroupFile>(groupFile);
			return true;
		}
	}

	return false;
}

bool Group::addFiles(const std::vector<std::shared_ptr<GroupFile>> & files, bool replace) {
	if(files.empty()) {
		return 0;
	}

	size_t numberOfFilesAdded = 0;

	for(std::vector<std::shared_ptr<GroupFile>>::const_iterator i = files.begin(); i != files.end(); ++i) {
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
	if(group.m_files.empty()) {
		return 0;
	}

	size_t numberOfFilesAdded = 0;

	for(std::vector<std::shared_ptr<GroupFile>>::const_iterator i = group.m_files.begin(); i != group.m_files.end(); ++i) {
		if(addFile(**i, replace)) {
			numberOfFilesAdded++;
		}
	}

	return numberOfFilesAdded;
}

bool Group::removeFile(size_t index) {
	if(index >= m_files.size()) {
		return false;
	}

	m_files.erase(m_files.begin() + index);

	return true;
}

bool Group::removeFile(const std::string & fileName) {
	if(fileName.empty()) {
		return false;
	}

	std::string formattedFileName(Utilities::trimString(fileName));

	if(formattedFileName.empty()) {
		return false;
	}

	for(std::vector<std::shared_ptr<GroupFile>>::const_iterator i = m_files.begin(); i != m_files.end(); ++i) {
		if(Utilities::compareStringsIgnoreCase((*i)->getFileName(), formattedFileName) == 0) {
			m_files.erase(i);

			return true;
		}
	}

	return false;
}

bool Group::removeFile(const GroupFile & groupFile) {
	if(!groupFile.isValid()) {
		return false;
	}

	for(std::vector<std::shared_ptr<GroupFile>>::const_iterator i = m_files.begin(); i != m_files.end(); ++i) {
		if(Utilities::compareStringsIgnoreCase((*i)->getFileName(), groupFile.getFileName()) == 0) {
			m_files.erase(i);

			return true;
		}
	}

	return false;
}

size_t Group::removeFiles(const std::vector<std::string> & fileNames) {
	if(fileNames.empty()) {
		return 0;
	}

	size_t numberOfFilesRemoved = 0;

	for(std::vector<std::string>::const_iterator i = fileNames.begin(); i != fileNames.end(); ++i) {
		if(removeFile(*i)) {
			numberOfFilesRemoved++;
		}
	}

	return numberOfFilesRemoved;
}

size_t Group::removeFiles(const std::vector<std::shared_ptr<GroupFile>> & files) {
	if(files.empty()) {
		return 0;
	}

	size_t numberOfFilesRemoved = 0;

	for(std::vector<std::shared_ptr<GroupFile>>::const_iterator i = files.begin(); i != files.end(); ++i) {
		if(*i == nullptr) {
			continue;
		}

		if(removeFile(**i)) {
			numberOfFilesRemoved++;
		}
	}

	return numberOfFilesRemoved;
}

void Group::clearFiles() {
	m_files.clear();
}

std::string Group::toString() const {
	std::stringstream groupString;

	if(!m_filePath.empty()) {
		groupString << fmt::format("\"{0}\" ", Utilities::getFileName(m_filePath));
	}

	groupString << fmt::format("Files ({0}){1}", m_files.size(), m_files.empty() ? "" : ": ");

	for(std::vector<std::shared_ptr<GroupFile>>::const_iterator i = m_files.begin(); i != m_files.end(); ++i) {
		if(i != m_files.begin()) {
			groupString << ", ";
		}

		groupString << (*i)->getFileName();
	}

	return groupString.str();
}

bool Group::load() {
	// verify that the file has a path
	if(m_filePath.empty()) {
		fmt::print("Group has no file name.\n");
		return false;
	}

	// verify that the file exists and is a file
	if(!std::filesystem::is_regular_file(std::filesystem::path(m_filePath))) {
		fmt::print("Group file does not exist or is not a file: '{}'.\n", m_filePath);
		return false;
	}

	// open the file and read it into memory
	std::unique_ptr<ByteBuffer> buffer = ByteBuffer::readFrom(m_filePath, Group::FILE_ENDIANNESS);

	if(buffer == nullptr) {
		fmt::print("Failed to open group file: '{}'.\n", m_filePath);
		return false;
	}

	fmt::print("Opened group file: '{}', loaded {} bytes into memory.\n", m_filePath, buffer->getSize());

	bool error = false;

	// verify that the data is long enough to contain header information
	std::string headerText(buffer->readString(Group::HEADER_TEXT.length(), &error));

	if(error) {
		fmt::print("Group file '{}' is incomplete or corrupted: missing header text.\n", m_filePath);
		return false;
	}

	// verify that the header text is specified in the header
	if(headerText != HEADER_TEXT) {
		fmt::print("Group file '{}' is not a valid format, missing '{}' header text.\n", m_filePath, HEADER_TEXT);
		return false;
	}

	fmt::print("Verified group file header text.\n");

	// read and verify the number of files value
	uint32_t numberOfFiles = buffer->readUnsignedInteger(&error);

	if(error) {
		fmt::print("Group file '{}' is incomplete or corrupted: missing number of files value.\n", m_filePath);
		return false;
	}

	fmt::print("Detected {} files in group '{}'.\n", numberOfFiles, m_filePath);

	std::vector<std::string> fileNames;
	std::vector<uint32_t> fileSizes;
	std::vector<std::shared_ptr<GroupFile>> groupFiles;

	for(uint32_t i = 0; i < numberOfFiles; i++) {
		// read the file name
		fileNames.emplace_back(buffer->readString(GroupFile::MAX_FILE_NAME_LENGTH, &error));

		if(error) {
			fmt::print("Group file '{}' is incomplete or corrupted: missing file #{} name.\n", m_filePath, i + 1);
			return false;
		}

		// read and verify the file size
		fileSizes.push_back(buffer->readUnsignedInteger(&error));

		if(error) {
			fmt::print("Group file '{}' is incomplete or corrupted: missing file #{} size value.\n", m_filePath, i + 1);
			return false;
		}
	}

	fmt::print("All group file information parsed.\n");

	for(uint32_t i = 0; i < numberOfFiles; i++) {
		if(buffer->getSize() < buffer->getReadOffset() + fileSizes[i]) {
			size_t numberOfMissingBytes = fileSizes[i] - (buffer->getSize() - buffer->getReadOffset());
			uint32_t numberOfAdditionalFiles = groupFiles.size() - i - 1;

			fmt::print("Group file '{}' is corrupted: missing {} of {} byte{} for file #{} ('{}') data.{}\n", m_filePath, numberOfMissingBytes, fileSizes[i], fileSizes[i] == 1 ? "" : "s", i + 1, fileNames[i], numberOfAdditionalFiles > 0 ? fmt::format(" There is also an additional {} files that are missing data.", numberOfAdditionalFiles) : "");

			return false;
		}

		std::shared_ptr<GroupFile> groupFile = std::make_shared<GroupFile>(fileNames[i], buffer->readBytes(fileSizes[i], &error));

		if(error) {
			fmt::print("Group file '{}' failed to read data bytes for file #{} ('{}').\n", m_filePath, i + 1, fileNames[i]);
		}

		groupFiles.push_back(groupFile);
	}

	m_files.clear();

	for(std::vector<std::shared_ptr<GroupFile>>::const_iterator i = groupFiles.begin(); i != groupFiles.end(); ++i) {
		m_files.push_back(*i);
	}

	fmt::print("Group file parsed successfully, {} files loaded into memory.\n", groupFiles.size());

	return true;
}

size_t Group::getGroupSize() const {
	size_t size = 0;

	for(std::vector<std::shared_ptr<GroupFile>>::const_iterator i = m_files.begin(); i != m_files.end(); ++i) {
		size += (*i)->getSize();
	}

	return size;
}

bool Group::verifyAllFiles() const {
	for(std::vector<std::shared_ptr<GroupFile>>::const_iterator i = m_files.begin(); i != m_files.end(); ++i) {
		if(!(*i)->isValid()) {
			return false;
		}
	}

	return true;
}

bool Group::operator == (const Group & g) const {
	if(m_files.size() != g.m_files.size()) {
		return false;
	}

	for(std::vector<std::shared_ptr<GroupFile>>::const_iterator i = m_files.begin(); i != m_files.end(); ++i) {
		if(!g.hasFile(**i)) {
			return false;
		}
	}

	return true;
}

bool Group::operator != (const Group & g) const {
	return !operator == (g);
}
