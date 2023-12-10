#include "GroupSSI.h"

#include <Utilities/FileUtilities.h>
#include <Utilities/StringUtilities.h>

#include <spdlog/spdlog.h>

#include <filesystem>

GroupSSI::GroupSSI(uint32_t version, const std::string & filePath)
	: Group(filePath)
	, m_version(version) { }

GroupSSI::GroupSSI(std::vector<std::unique_ptr<GroupFile>> files, uint32_t version, const std::string & title, const std::array<std::string, NUMBER_OF_DESCRIPTIONS> & descriptions, const std::string & runFile, std::unique_ptr<ByteBuffer> trailingData, const std::string & filePath)
	: Group(std::move(files), filePath)
	, m_version(version)
	, m_title(title)
	, m_descriptions(descriptions)
	, m_runFile(runFile)
	, m_trailingData(std::move(trailingData)) { }

GroupSSI::GroupSSI(std::vector<std::unique_ptr<GroupFile>> files, const std::string & title, const std::array<std::string, NUMBER_OF_DESCRIPTIONS> & descriptions, std::unique_ptr<ByteBuffer> trailingData, const std::string & filePath)
	: Group(std::move(files), filePath)
	, m_version(2)
	, m_title(title)
	, m_descriptions(descriptions)
	, m_trailingData(std::move(trailingData)) { }

GroupSSI::GroupSSI(GroupSSI && group) noexcept
	: Group(std::move(group))
	, m_version(group.m_version)
	, m_title(std::move(group.m_title))
	, m_descriptions(std::move(group.m_descriptions))
	, m_runFile(std::move(group.m_runFile)) { }

GroupSSI::GroupSSI(const GroupSSI & group)
	: Group(group)
	, m_version(group.m_version)
	, m_title(group.m_title)
	, m_descriptions(group.m_descriptions)
	, m_runFile(group.m_runFile) { }

GroupSSI & GroupSSI::operator = (GroupSSI && group) noexcept {
	if(this != &group) {
		Group::operator = (std::move(group));

		m_version = group.m_version;
		m_title = std::move(group.m_title);
		m_descriptions = std::move(group.m_descriptions);
		m_runFile = std::move(group.m_runFile);
	}

	return *this;
}

GroupSSI & GroupSSI::operator = (const GroupSSI & group) {
	Group::operator = (group);

	m_version = group.m_version;
	m_title = group.m_title;
	m_descriptions = group.m_descriptions;
	m_runFile = group.m_runFile;

	return *this;
}

GroupSSI::~GroupSSI() { }

uint32_t GroupSSI::getVersion() const {
	return m_version;
}

bool GroupSSI::setVersion(uint32_t version) {
	if(!isValidVersion(version)) {
		return false;
	}

	if(m_version == version) {
		return true;
	}

	m_version = version;

	if(!versionSupportsRunFile()) {
		m_runFile = "";
	}

	setModified(true);

	return true;
}

bool GroupSSI::hasTitle() const {
	return !m_title.empty();
}

const std::string & GroupSSI::getTitle() const {
	return m_title;
}

bool GroupSSI::setTitle(const std::string & title) {
	if(title.length() > MAX_TITLE_LENGTH) {
		return false;
	}

	if(Utilities::areStringsEqual(m_title, title)) {
		return true;
	}

	m_title = title;

	setModified(true);

	return true;
}

void GroupSSI::clearTitle() {
	if(m_title.empty()) {
		return;
	}

	m_title = "";

	setModified(true);
}

bool GroupSSI::hasAnyDescription() const {
	for(const std::string & description : m_descriptions) {
		if(!description.empty()) {
			return true;
		}
	}

	return false;
}

size_t GroupSSI::numberOfDescriptions() const {
	return m_descriptions.size();
}

const std::string & GroupSSI::getDescription(size_t index) const {
	if(index >= m_descriptions.size()) {
		return Utilities::emptyString;
	}

	return m_descriptions[index];
}

bool GroupSSI::setDescription(size_t index, const std::string description) {
	if(index >= m_descriptions.size() || description.length() > MAX_DESCRIPTION_LENGTH) {
		return false;
	}

	if(Utilities::areStringsEqual(m_descriptions[index], description)) {
		return true;
	}

	m_descriptions[index] = description;

	setModified(true);

	return true;
}

bool GroupSSI::clearDescription(size_t index) {
	if(index >= m_descriptions.size()) {
		return false;
	}

	if(m_descriptions[index].empty()) {
		return true;
	}

	m_descriptions[index] = "";

	setModified(true);

	return true;
}

void GroupSSI::clearAllDescriptions() {
	bool anyDescriptionsCleared = false;

	for(std::string & description : m_descriptions) {
		if(description.empty()) {
			continue;
		}

		description = "";
		anyDescriptionsCleared = true;
	}

	if(anyDescriptionsCleared) {
		setModified(true);
	}
}

bool GroupSSI::versionSupportsRunFile() const {
	return versionSupportsRunFile(m_version);
}

bool GroupSSI::versionSupportsRunFile(uint32_t version) {
	return version >= 2;
}

bool GroupSSI::hasRunFile() const {
	return !m_runFile.empty();
}

const std::string & GroupSSI::getRunFile() const {
	return m_runFile;
}

bool GroupSSI::setRunFile(const std::string & runFile) {
	if(!versionSupportsRunFile() || runFile.length() > MAX_RUN_FILE_LENGTH) {
		return false;
	}

	if(Utilities::areStringsEqual(m_runFile, runFile)) {
		return true;
	}

	m_runFile = runFile;

	setModified(true);

	return true;
}

void GroupSSI::clearRunFile() {
	if(m_runFile.empty()) {
		return;
	}

	m_runFile = "";

	setModified(true);
}

bool GroupSSI::hasTrailingData() const {
	return m_trailingData != nullptr &&
		   m_trailingData->isNotEmpty();
}

const ByteBuffer & GroupSSI::getTrailingData() const {
	if(m_trailingData == nullptr) {
		return ByteBuffer::EMPTY_BYTE_BUFFER;
	}

	return *m_trailingData;
}

bool GroupSSI::setTrailingData(std::vector<uint8_t> & trailingData) {
	if(trailingData.size() != TRAILING_FILE_DATA_LENGTH) {
		return false;
	}

	if(m_trailingData == nullptr) {
		m_trailingData = std::make_unique<ByteBuffer>(trailingData);
	}
	else {
		m_trailingData->setData(trailingData);
	}

	setModified(true);

	return true;
}

bool GroupSSI::setTrailingData(std::unique_ptr<ByteBuffer> trailingData) {
	if(trailingData == nullptr || trailingData->isEmpty()) {
		m_trailingData = nullptr;
		return true;
	}
	else if(trailingData->getSize() != TRAILING_FILE_DATA_LENGTH) {
		return false;
	}

	m_trailingData = std::move(trailingData);

	setModified(true);

	return true;
}

bool GroupSSI::setTrailingData(const ByteBuffer & trailingData) {
	if(trailingData.isEmpty()) {
		m_trailingData = nullptr;
		return true;
	}
	else if(trailingData.getSize() != TRAILING_FILE_DATA_LENGTH) {
		return false;
	}

	if(m_trailingData == nullptr) {
		m_trailingData = std::make_unique<ByteBuffer>(trailingData);
	}
	else {
		m_trailingData->setData(trailingData);
	}

	setModified(true);

	return true;
}

void GroupSSI::clearTrailingData() {
	m_trailingData = nullptr;
}

std::unique_ptr<GroupSSI> GroupSSI::readFrom(const ByteBuffer & byteBuffer) {
	byteBuffer.setEndianness(ENDIANNESS);

	bool error = false;

	uint32_t version = byteBuffer.readUnsignedInteger(&error);

	if(error) {
		spdlog::error("Failed to read Sunstorm Interactive SSI group version.");
		return nullptr;
	}

	if(!isValidVersion(version)) {
		spdlog::error("Invalid Sunstorm Interactive SSI group version: {}.", version);
		return nullptr;
	}

	uint32_t fileCount = byteBuffer.readUnsignedInteger(&error);

	if(error) {
		spdlog::error("Failed to read Sunstorm Interactive SSI group file count.");
		return nullptr;
	}

	uint8_t titleLength = byteBuffer.readUnsignedByte(&error);

	if(error) {
		spdlog::error("Failed to read Sunstorm Interactive SSI group title length.");
		return nullptr;
	}

	if(titleLength > MAX_TITLE_LENGTH) {
		spdlog::error("Sunstorm Interactive SSI group title length {} exceeds limit of {} characters.", titleLength, MAX_TITLE_LENGTH);
		return nullptr;
	}

	std::string title(byteBuffer.readFixedLengthString(titleLength, MAX_TITLE_LENGTH, &error));

	if(error) {
		spdlog::error("Failed to read Sunstorm Interactive SSI group title.");
		return nullptr;
	}

	std::string runFile;

	if(versionSupportsRunFile(version)) {
		uint8_t runFileLength = byteBuffer.readUnsignedByte(&error);

		if(error) {
			spdlog::error("Failed to read Sunstorm Interactive SSI group run file length.");
			return nullptr;
		}

		if(runFileLength > MAX_RUN_FILE_LENGTH) {
			spdlog::error("Sunstorm Interactive SSI group run file length {} exceeds limit of {} characters.", runFileLength, MAX_RUN_FILE_LENGTH);
			return nullptr;
		}

		runFile = byteBuffer.readFixedLengthString(runFileLength, MAX_RUN_FILE_LENGTH, &error);

		if(error) {
			spdlog::error("Failed to read Sunstorm Interactive SSI group run file.");
			return nullptr;
		}
	}

	uint8_t descriptionLength = 0;
	std::array<std::string, NUMBER_OF_DESCRIPTIONS> descriptions;

	for(uint8_t i = 0; i < NUMBER_OF_DESCRIPTIONS; i++) {
		descriptionLength = byteBuffer.readUnsignedByte(&error);

		if(error) {
			spdlog::error("Failed to read Sunstorm Interactive SSI group description #{} length.", i + 1);
			return nullptr;
		}

		if(descriptionLength > MAX_DESCRIPTION_LENGTH) {
			spdlog::error("Sunstorm Interactive SSI group description #{} length of {} exceeds limit of {} characters.", i + 1, descriptionLength, MAX_DESCRIPTION_LENGTH);
			return nullptr;
		}

		descriptions[i] = byteBuffer.readFixedLengthString(descriptionLength, MAX_DESCRIPTION_LENGTH, &error);

		if(error) {
			spdlog::error("Failed to read Sunstorm Interactive SSI group description #{}.", i + 1);
			return nullptr;
		}
	}

	uint8_t fileNameLength = 0;
	std::string fileName;
	std::vector<std::unique_ptr<GroupFile>> groupFiles;
	groupFiles.reserve(fileCount);
	std::vector<uint32_t> fileSizes;
	fileSizes.reserve(fileCount);

	for(uint32_t i = 0; i < fileCount; i++) {
		fileNameLength = byteBuffer.readUnsignedByte(&error);

		if(error) {
			spdlog::error("Failed to read Sunstorm Interactive SSI group file name #{} length.", i + 1);
			return nullptr;
		}

		if(fileNameLength > GroupFile::MAX_FILE_NAME_LENGTH) {
			spdlog::error("Sunstorm Interactive SSI group file name #{} length of {} exceeds limit of {} characters.", i + 1, fileNameLength, GroupFile::MAX_FILE_NAME_LENGTH);
			return nullptr;
		}

		fileName = byteBuffer.readFixedLengthString(fileNameLength, GroupFile::MAX_FILE_NAME_LENGTH, &error);

		if(error) {
			spdlog::error("Failed to read Sunstorm Interactive SSI group file name #{}.", i + 1);
			return nullptr;
		}

		fileSizes.push_back(byteBuffer.readUnsignedInteger(&error));

		if(error) {
			spdlog::error("Failed to read Sunstorm Interactive SSI group file #{} size.", i + 1);
			return nullptr;
		}

		groupFiles.emplace_back(std::make_unique<GroupFile>(fileName, nullptr, std::make_unique<ByteBuffer>(byteBuffer.readBytes(TRAILING_FILE_DATA_LENGTH))));
	}

	for(uint32_t i = 0; i < fileCount; i++) {
		groupFiles[i]->setData(std::make_unique<ByteBuffer>(byteBuffer.readBytes(fileSizes[i])));

		if(fileSizes[i] != 0 && !groupFiles[i]->hasData()) {
			spdlog::error("Failed to read Sunstorm Interactive SSI group file #{} data.", i + 1);
			return nullptr;
		}
	}

	return std::make_unique<GroupSSI>(std::move(groupFiles), version, title, descriptions, runFile, byteBuffer.getRemainingBytes());
}

bool GroupSSI::writeTo(ByteBuffer & byteBuffer) const {
	if(!isValid(false)) {
		spdlog::error("Failed to write invalid Sunstorm Interactive SSI group.");
		return false;
	}

	byteBuffer.setEndianness(ENDIANNESS);

	if(!byteBuffer.writeUnsignedInteger(m_version)) {
		spdlog::error("Failed to write Sunstorm Interactive SSI group version.");
		return false;
	}

	if(!byteBuffer.writeUnsignedInteger(static_cast<uint32_t>(m_files.size()))) {
		spdlog::error("Failed to write Sunstorm Interactive SSI group file count.");
		return false;
	}

	if(!byteBuffer.writeUnsignedByte(static_cast<uint8_t>(m_title.length()))) {
		spdlog::error("Failed to write Sunstorm Interactive SSI group title length.");
		return false;
	}

	if(!byteBuffer.writeFixedLengthString(m_title, MAX_TITLE_LENGTH)) {
		spdlog::error("Failed to write Sunstorm Interactive SSI group title.");
		return false;
	}

	if(versionSupportsRunFile()) {
		if(!byteBuffer.writeUnsignedByte(static_cast<uint8_t>(m_runFile.length()))) {
			spdlog::error("Failed to write Sunstorm Interactive SSI group run file length.");
			return false;
		}

		if(!byteBuffer.writeFixedLengthString(m_runFile, MAX_RUN_FILE_LENGTH)) {
			spdlog::error("Failed to write Sunstorm Interactive SSI group run file.");
			return false;
		}
	}

	for(uint8_t i = 0; i < NUMBER_OF_DESCRIPTIONS; i++) {
		if(!byteBuffer.writeUnsignedByte(static_cast<uint8_t>(m_descriptions[i].length()))) {
			spdlog::error("Failed to write Sunstorm Interactive SSI group description #{} length.", i + 1);
			return false;
		}

		if(!byteBuffer.writeFixedLengthString(m_descriptions[i], MAX_DESCRIPTION_LENGTH)) {
			spdlog::error("Failed to write Sunstorm Interactive SSI group description #{}.", i + 1);
			return false;
		}
	}

	for(size_t i = 0; i < m_files.size(); i++) {
		if(!byteBuffer.writeUnsignedByte(static_cast<uint8_t>(m_files[i]->getFileName().length()))) {
			spdlog::error("Failed to write Sunstorm Interactive SSI group file #{} name length.", i + 1);
			return false;
		}

		if(!byteBuffer.writeFixedLengthString(m_files[i]->getFileName(), GroupFile::MAX_FILE_NAME_LENGTH)) {
			spdlog::error("Failed to write Sunstorm Interactive SSI group file #{} name.", i + 1);
			return false;
		}

		if(!byteBuffer.writeUnsignedInteger(static_cast<uint32_t>(m_files[i]->getSize()))) {
			spdlog::error("Failed to write Sunstorm Interactive SSI group file #{} size.", i + 1);
			return false;
		}

		if(m_files[i]->hasTrailingData()) {
			if(!byteBuffer.writeBytes(m_files[i]->getTrailingData())) {
				return false;
			}
		}
		else {
			if(!byteBuffer.skipWriteBytes(TRAILING_FILE_DATA_LENGTH)) {
				return false;
			}
		}
	}

	for(size_t i = 0; i < m_files.size(); i++) {
		if(m_files[i]->hasData() && !byteBuffer.writeBytes(m_files[i]->getData())) {
			spdlog::error("Failed to write Sunstorm Interactive SSI group file #{} data.", i + 1);
			return false;
		}
	}

	return true;
}

std::unique_ptr<GroupSSI> GroupSSI::createFrom(const std::string & directoryPath) {
	return std::make_unique<GroupSSI>(createGroupFilesFromDirectory(directoryPath));
}

std::unique_ptr<GroupSSI> GroupSSI::loadFrom(const std::string & filePath) {
	if(filePath.empty() || !std::filesystem::is_regular_file(std::filesystem::path(filePath))) {
		spdlog::error("Sunstorm Interactive SSI group does not exist or is not a file: '{}'.", filePath);
		return nullptr;
	}

	std::unique_ptr<ByteBuffer> byteBuffer(ByteBuffer::readFrom(filePath, ENDIANNESS));

	if(byteBuffer == nullptr) {
		spdlog::error("Failed to open group: '{}'.", filePath);
		return nullptr;
	}

	spdlog::trace("Opened Sunstorm Interactive SSI group '{}', loaded {} bytes into memory.", filePath, byteBuffer->getSize());

	std::unique_ptr<GroupSSI> group(readFrom(*byteBuffer));

	if(group == nullptr) {
		return nullptr;
	}

	group->setFilePath(filePath);

	return group;
}

void GroupSSI::addMetadata(std::vector<std::pair<std::string, std::string>> & metadata) const {
	Group::addMetadata(metadata);

	metadata.push_back({ "Version", std::to_string(m_version) });
	metadata.push_back({ "Title", m_title });

	if(versionSupportsRunFile()) {
		metadata.push_back({ "Run File", m_runFile });
	}

	if(hasTrailingData()) {
		metadata.push_back({ "Number of Trailing Bytes", std::to_string(m_trailingData->getSize()) });
	}

	for(uint8_t i = 0; i < NUMBER_OF_DESCRIPTIONS; i++) {
		metadata.push_back({ fmt::format("Description #{}", i + 1), m_descriptions[i] });
	}
}

Endianness GroupSSI::getEndianness() const {
	return ENDIANNESS;
}

size_t GroupSSI::getSizeInBytes() const {
	size_t sizeBytes = (sizeof(uint32_t) * 2) + ((MAX_TITLE_LENGTH + 1) * sizeof(uint8_t)) + ((MAX_DESCRIPTION_LENGTH + 1) * 3 * sizeof(uint8_t)) + ((((GroupFile::MAX_FILE_NAME_LENGTH + 1) * sizeof(uint8_t)) + sizeof(uint32_t) + TRAILING_FILE_DATA_LENGTH) * m_files.size()) + (m_trailingData != nullptr ? m_trailingData->getSize() : 0);

	if(versionSupportsRunFile()) {
		sizeBytes += (MAX_RUN_FILE_LENGTH + 1) * sizeof(uint8_t);
	}

	for(const std::shared_ptr<GroupFile> & file : m_files) {
		sizeBytes += file->getSize();
	}

	return sizeBytes;
}

bool GroupSSI::isValid(bool verifyParent) const {
	if(!Group::isValid(verifyParent) ||
	   !isValidVersion(m_version) ||
	   m_title.length() > MAX_TITLE_LENGTH ||
	   m_runFile.length() > MAX_RUN_FILE_LENGTH ||
	   (!versionSupportsRunFile() && !m_runFile.empty())) {
		return false;
	}

	for(uint8_t i = 0; i < NUMBER_OF_DESCRIPTIONS; i++) {
		if(m_descriptions[i].length() > MAX_DESCRIPTION_LENGTH) {
			return false;
		}
	}

	for(const std::shared_ptr<GroupFile> & file : m_files) {
		if(file->getSize() > std::numeric_limits<uint32_t>::max()) {
			return false;
		}

		if(file->hasTrailingData() && file->getTrailingDataSize() != TRAILING_FILE_DATA_LENGTH) {
			return false;
		}
	}

	return true;
}

bool GroupSSI::isValidVersion(uint32_t version) {
	return version >= 1 &&
		   version <= 2;
}

bool GroupSSI::operator == (const GroupSSI & group) const {
	if(Group::operator != (group) ||
	   m_version != group.m_version ||
	   !Utilities::areStringsEqual(m_title, group.m_title) ||
	   versionSupportsRunFile() && !Utilities::areStringsEqual(m_runFile, group.m_runFile) ||
	   !(m_trailingData == nullptr && group.m_trailingData == nullptr || (m_trailingData != nullptr && group.m_trailingData != nullptr && *m_trailingData == *group.m_trailingData))) {
		return false;
	}

	for(uint8_t i = 0; i < NUMBER_OF_DESCRIPTIONS; i++) {
		if(!Utilities::areStringsEqual(m_descriptions[i], group.m_descriptions[i])) {
			return false;
		}
	}

	return true;
}

bool GroupSSI::operator != (const GroupSSI & group) const {
	return !operator == (group);
}
