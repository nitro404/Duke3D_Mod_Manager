#include "GroupGRP.h"

#include <Utilities/FileUtilities.h>
#include <Utilities/StringUtilities.h>

#include <spdlog/spdlog.h>

#include <filesystem>

GroupGRP::GroupGRP(const std::string & filePath)
	: Group(filePath) { }

GroupGRP::GroupGRP(std::vector<std::unique_ptr<GroupFile>> files, const std::string & filePath)
	: Group(std::move(files), filePath) { }

GroupGRP::GroupGRP(GroupGRP && group) noexcept
	: Group(group) { }

GroupGRP::GroupGRP(const GroupGRP & group)
	: Group(group) { }

GroupGRP & GroupGRP::operator = (GroupGRP && group) noexcept {
	if(this != &group) {
		Group::operator = (group);
	}

	return *this;
}

GroupGRP & GroupGRP::operator = (const GroupGRP & group) {
	Group::operator = (group);

	return *this;
}

GroupGRP::~GroupGRP() { }

std::unique_ptr<GroupGRP> GroupGRP::readFrom(const ByteBuffer & byteBuffer) {
	byteBuffer.setEndianness(ENDIANNESS);

	bool error = false;

	// verify that the data is long enough to contain header information
	std::string headerText(byteBuffer.readString(GroupGRP::HEADER_TEXT.length(), &error));

	if(error) {
		spdlog::error("Build Engine GRP group is incomplete or corrupted: missing header text.");
		return false;
	}

	// verify that the header text is specified in the header
	if(!Utilities::areStringsEqual(headerText, HEADER_TEXT)) {
		spdlog::error("Build Engine GRP group is not a valid format, missing '{}' header text.", HEADER_TEXT);
		return false;
	}

	spdlog::trace("Verified Build Engine GRP group file header text.");

	// read and verify the number of files value
	uint32_t numberOfFiles = byteBuffer.readUnsignedInteger(&error);

	if(error) {
		spdlog::error("Build Engine GRP group is incomplete or corrupted: missing number of files value.");
		return false;
	}

	spdlog::trace("Detected {} files in group.", numberOfFiles);

	std::vector<std::string> fileNames;
	std::vector<uint32_t> fileSizes;
	std::vector<std::shared_ptr<GroupFile>> files;

	for(uint32_t i = 0; i < numberOfFiles; i++) {
		// read the file name
		fileNames.emplace_back(byteBuffer.readString(GroupFile::MAX_FILE_NAME_LENGTH, &error));

		if(error) {
			spdlog::error("Build Engine GRP group is incomplete or corrupted: missing file #{} name.", i + 1);
			return false;
		}

		// read and verify the file size
		fileSizes.push_back(byteBuffer.readUnsignedInteger(&error));

		if(error) {
			spdlog::error("Build Engine GRP group is incomplete or corrupted: missing file #{} size value.", i + 1);
			return false;
		}
	}

	spdlog::trace("All Build Engine GRP group file information parsed.");

	std::vector<std::unique_ptr<GroupFile>> groupFiles;

	for(uint32_t i = 0; i < numberOfFiles; i++) {
		if(byteBuffer.getSize() < byteBuffer.getReadOffset() + fileSizes[i]) {
			size_t numberOfMissingBytes = fileSizes[i] - (byteBuffer.getSize() - byteBuffer.getReadOffset());
			uint32_t numberOfAdditionalFiles = groupFiles.size() - i - 1;

			spdlog::error("Build Engine GRP group is corrupted: missing {} of {} byte{} for file #{} ('{}') data.{}", numberOfMissingBytes, fileSizes[i], fileSizes[i] == 1 ? "" : "s", i + 1, fileNames[i], numberOfAdditionalFiles > 0 ? fmt::format(" There is also an additional {} files that are missing data.", numberOfAdditionalFiles) : "");

			return false;
		}

		std::unique_ptr<GroupFile> file(std::make_unique<GroupFile>(fileNames[i], byteBuffer.readBytes(fileSizes[i])));

		if(error) {
			spdlog::error("Build Engine GRP group failed to read data bytes for file #{} ('{}').", i + 1, fileNames[i]);
		}

		groupFiles.push_back(std::move(file));
	}

	spdlog::trace("Build Engine GRP group parsed successfully, {} files loaded into memory.", groupFiles.size());

	return std::make_unique<GroupGRP>(std::move(groupFiles));
}

bool GroupGRP::writeTo(ByteBuffer & byteBuffer) const {
	byteBuffer.setEndianness(ENDIANNESS);

	if(!byteBuffer.writeString(HEADER_TEXT)) {
		return false;
	}

	if(!byteBuffer.writeUnsignedInteger(m_files.size())) {
		return false;
	}

	size_t currentFileNameLength = 0;

	for(size_t i = 0; i < m_files.size(); i++) {
		const std::shared_ptr<GroupFile> file(m_files.at(i));

		if(!byteBuffer.writeString(file->getFileName())) {
			return false;
		}

		currentFileNameLength = file->getFileName().length();

		if(currentFileNameLength < GroupFile::MAX_FILE_NAME_LENGTH) {
			if(!byteBuffer.skipWriteBytes(GroupFile::MAX_FILE_NAME_LENGTH - currentFileNameLength)) {
				return false;
			}
		}

		if(!byteBuffer.writeUnsignedInteger(file->getSize())) {
			return false;
		}
	}

	for(size_t i = 0; i < m_files.size(); i++) {
		const std::shared_ptr<GroupFile> file(m_files.at(i));

		if(!byteBuffer.writeBytes(file->getData())) {
			return false;
		}
	}

	return true;
}

std::unique_ptr<GroupGRP> GroupGRP::createFrom(const std::string & directoryPath) {
	return std::make_unique<GroupGRP>(createGroupFilesFromDirectory(directoryPath));
}

std::unique_ptr<GroupGRP> GroupGRP::loadFrom(const std::string & filePath) {
	if(filePath.empty() || !std::filesystem::is_regular_file(std::filesystem::path(filePath))) {
		spdlog::error("Build Engine GRP group file does not exist or is not a file: '{}'.", filePath);
		return false;
	}

	std::unique_ptr<ByteBuffer> byteBuffer(ByteBuffer::readFrom(filePath, ENDIANNESS));

	if(byteBuffer == nullptr) {
		spdlog::error("Failed to open Build Engine GRP group file: '{}'.", filePath);
		return false;
	}

	spdlog::trace("Opened Build Engine GRP group file: '{}', loaded {} bytes into memory.", filePath, byteBuffer->getSize());

	std::unique_ptr<GroupGRP> group(readFrom(*byteBuffer));

	if(group == nullptr) {
		return nullptr;
	}

	group->setFilePath(filePath);

	return group;
}

Endianness GroupGRP::getEndianness() const {
	return ENDIANNESS;
}

size_t GroupGRP::getSizeInBytes() const {
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

bool GroupGRP::operator == (const GroupGRP & group) const {
	return Group::operator == (group);
}

bool GroupGRP::operator != (const GroupGRP & group) const {
	return !operator == (group);
}
