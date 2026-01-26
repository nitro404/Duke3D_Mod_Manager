#include "Zip.h"

#include <Archive/Zip/ZipArchive.h>
#include <Utilities/FileUtilities.h>
#include <Utilities/TimeUtilities.h>

#include <fmt/core.h>
#include <magic_enum/magic_enum.hpp>

Zip::Zip(const std::string & filePath)
	: GameFile(filePath)
	, m_zipArchive(ZipArchive::createNew(filePath)) { }

Zip::Zip(std::unique_ptr<ZipArchive> zipArchive)
	: GameFile()
	, m_zipArchive(std::move(zipArchive)) {
	if(m_zipArchive != nullptr) {
		setFilePath(m_zipArchive->getFilePath());
	}
}

Zip::Zip(Zip && zip) noexcept
	: GameFile(zip) 
	, m_zipArchive(std::move(zip.m_zipArchive)) { }

Zip::Zip(const Zip & zip)
	: GameFile(std::move(zip)) {
	if(zip.m_zipArchive != nullptr) {
		const ByteBuffer * zipArchiveData = zip.m_zipArchive->getData();

		if(zipArchiveData != nullptr) {
			m_zipArchive = ZipArchive::createFrom(std::make_unique<ByteBuffer>(*zipArchiveData), zip.m_zipArchive->getPassword(), false, zip.m_zipArchive->getFilePath());
			setFilePath(m_zipArchive->getFilePath());
		}
	}
}

Zip & Zip::operator = (Zip && zip) noexcept {
	if(this != &zip) {
		GameFile::operator = (std::move(zip));

		m_zipArchive = std::move(m_zipArchive);
	}

	return *this;
}

Zip & Zip::operator = (const Zip & zip) {
	m_zipArchive.reset();
	m_filePath = "";

	GameFile::operator = (zip);

	if(zip.m_zipArchive != nullptr) {
		const ByteBuffer * zipArchiveData = zip.m_zipArchive->getData();

		if(zipArchiveData != nullptr) {
			m_zipArchive = ZipArchive::createFrom(std::make_unique<ByteBuffer>(*zipArchiveData), zip.m_zipArchive->getPassword(), false, zip.m_zipArchive->getFilePath());
		}
	}

	return *this;
}

Zip::~Zip() { }

std::unique_ptr<Zip> Zip::readFrom(const ByteBuffer & byteBuffer) {
	std::unique_ptr<ZipArchive> zipArchive(ZipArchive::createFrom(std::make_unique<ByteBuffer>(byteBuffer)));

	if(zipArchive == nullptr) {
		return nullptr;
	}

	return std::make_unique<Zip>(std::move(zipArchive));
}

std::unique_ptr<Zip> Zip::loadFrom(const std::string & filePath) {
	std::unique_ptr<ZipArchive> zipArchive(ZipArchive::readFrom(filePath));

	if(zipArchive == nullptr) {
		return nullptr;
	}

	return std::make_unique<Zip>(std::move(zipArchive));
}

bool Zip::writeTo(ByteBuffer & byteBuffer) const {
	if(m_zipArchive == nullptr) {
		return false;
	}

	const ByteBuffer * zipArchiveData = m_zipArchive->getData();

	if(zipArchiveData == nullptr) {
		return false;
	}

	return byteBuffer.writeBytes(*zipArchiveData);
}

void Zip::addMetadata(std::vector<std::pair<std::string, std::string>> & metadata) const {
	if(!isValid()) {
		return;
	}

	metadata.push_back({ "Number of Entries", std::to_string(m_zipArchive->numberOfEntries()) });
	metadata.push_back({ "Number of Files", std::to_string(m_zipArchive->numberOfFiles()) });
	metadata.push_back({ "Number of Directories", std::to_string(m_zipArchive->numberOfDirectories()) });
	metadata.push_back({ "Has Comment", m_zipArchive->hasComment() ? "Yes" : "No" });
	metadata.push_back({ "Passworded", m_zipArchive->hasPassword() ? "Yes" : "No" });
	metadata.push_back({ "Compressed", m_zipArchive->isCompressed() ? "Yes" : "No" });
	metadata.push_back({ "Uncompressed Size", Utilities::fileSizeToString(m_zipArchive->getUncompressedSize()) });

	if(m_zipArchive->isCompressed()) {
		metadata.push_back({ "Compressed Size", Utilities::fileSizeToString(m_zipArchive->getCompressedSize()) });
		metadata.push_back({ "Compression Method", std::string(magic_enum::enum_name(m_zipArchive->getCompressionMethod())) });
	}

	metadata.push_back({ "Encrypted", m_zipArchive->isEncrypted() ? "Yes" : "No" });

	if(m_zipArchive->isEncrypted()) {
		metadata.push_back({ "Encryption Method", std::string(magic_enum::enum_name(m_zipArchive->getEncryptionMethod())) });
	}

	std::vector<std::shared_ptr<ArchiveEntry>> zipArchiveEntries(m_zipArchive->getEntries());

	for(const std::shared_ptr<ArchiveEntry> & archiveEntry : zipArchiveEntries) {
		metadata.push_back({ fmt::format("Entry #{}", archiveEntry->getIndex() + 1), fmt::format("'{}' Size: {} CRC32: {}", archiveEntry->getPath(), Utilities::fileSizeToString(archiveEntry->getUncompressedSize()), archiveEntry->getCRC32()) });
	}
}

Endianness Zip::getEndianness() const {
	return Endianness::LittleEndian;
}

size_t Zip::getSizeInBytes() const {
	if(m_zipArchive == nullptr) {
		return 0;
	}

	return m_zipArchive->getCompressedSize();
}

bool Zip::isValid(bool verifyParent) const {
	return m_zipArchive != nullptr;
}
