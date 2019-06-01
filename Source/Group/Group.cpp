#include "Group/Group.h"
#include "Utilities/Serializer.h"

const bool Group::DEFAULT_REPLACE_FILES = true;

const Endian::Endianness Group::FILE_ENDIANNESS = Endian::LittleEndian;

const char * Group::HEADER_TEXT = "KenSilverman";

const int Group::HEADER_TEXT_LENGTH = 12;
const int Group::NUMBER_OF_FILES_LENGTH = 4;
const int Group::GROUP_FILE_NAME_LENGTH = 12;
const int Group::GROUP_FILE_SIZE_LENGTH = 4;

const int Group::HEADER_LENGTH = HEADER_TEXT_LENGTH + NUMBER_OF_FILES_LENGTH;
const int Group::GROUP_FILE_HEADER_LENGTH = GROUP_FILE_NAME_LENGTH + GROUP_FILE_SIZE_LENGTH;

const char Group::GROUP_HEADER_TEXT_DATA[] = { // 12 bytes (total)
	0x4b, 0x65, 0x6e, // Ken (3 bytes)
	0x53, 0x69, 0x6c, 0x76, 0x65, 0x72, 0x6d, 0x61, 0x6e // Silverman (9 bytes)
};

Group::Group(const char * filePath)
	: m_filePath(NULL) {
	setFilePath(filePath);
}

Group::Group(const QString & filePath)
	: m_filePath(NULL) {
	setFilePath(filePath);
}

Group::Group(const Group & g)
	: m_filePath(NULL) {
	for(int i=0;i<g.m_files.size();i++) {
		m_files.push_back(new GroupFile(*g.m_files[i]));
	}
}

Group & Group::operator = (const Group & g) {
	if(m_filePath != NULL) {
		delete [] m_filePath;
		m_filePath = NULL;
	}
	
	for(int i=0;i<m_files.size();i++) {
		delete m_files[i];
	}
	m_files.clear();
	
	if(g.m_filePath != NULL) {
		m_filePath = Utilities::trimCopyString(g.m_filePath);
	}
	
	for(int i=0;i<g.m_files.size();i++) {
		m_files.push_back(new GroupFile(*g.m_files[i]));
	}
	
	return *this;
}

Group::~Group() {
	if(m_filePath != NULL) { delete [] m_filePath; }
	
	for(int i=0;i<m_files.size();i++) {
		delete m_files[i];
	}
}

const char * Group::getFilePath() const {
	return m_filePath;
}

QString Group::getFileName() const {
	return Utilities::getFileNameNoPath(m_filePath);
}

const char * Group::getFileExtension() const {
	return Utilities::getFileExtensionSubPointer(m_filePath);
}

void Group::setFilePath(const char * filePath) {
	if(m_filePath != NULL) {
		delete [] m_filePath;
		m_filePath = NULL;
	}
	
	if(filePath != NULL) {
		m_filePath = Utilities::trimCopyString(filePath);
	}
}

void Group::setFilePath(const QString & filePath) {
	if(filePath.isNull()) {
		setFilePath(NULL);
	}
	else {
		QByteArray filePathBytes = filePath.toLocal8Bit();
		setFilePath(filePathBytes.data());
	}
}

int Group::numberOfFiles() const {
	return m_files.size();
}

bool Group::hasFile(const char * fileName) const {
	if(fileName == NULL) { return false; }
	
	char * formattedFileName = Utilities::trimCopyString(fileName);
	if(Utilities::stringLength(formattedFileName) == 0) {
		delete [] formattedFileName;
		return false;
	}
	
	bool hasFile = false;
	for(int i=0;i<m_files.size();i++) {
		if(Utilities::compareStringsIgnoreCase(m_files[i]->getFileName(), formattedFileName) == 0) {
			hasFile = true;
			break;
		}
	}
	
	delete [] formattedFileName;
	
	return hasFile;
}

bool Group::hasFile(const QString & fileName) const {
	if(fileName.isEmpty()) { return false; }
	
	QString formattedFileName = fileName.trimmed();
	if(formattedFileName.length() == 0) { return false; }
	
	QByteArray formattedFileNameBytes = formattedFileName.toLocal8Bit();
	
	for(int i=0;i<m_files.size();i++) {
		if(Utilities::compareStringsIgnoreCase(m_files[i]->getFileName(), formattedFileNameBytes.data()) == 0) {
			return true;
		}
	}
	return false;
}

bool Group::hasFile(const GroupFile * groupFile) const {
	if(!GroupFile::isValid(groupFile)) { return false; }
	
	for(int i=0;i<m_files.size();i++) {
		if(*m_files[i] == *groupFile) {
			return true;
		}
	}
	return false;
}

int Group::indexOfFile(const char * fileName) const {
	if(fileName == NULL) { return -1; }
	
	char * formattedFileName = Utilities::trimCopyString(fileName);
	if(Utilities::stringLength(formattedFileName) == 0) {
		delete [] formattedFileName;
		return false;
	}
	
	int fileIndex = -1;
	for(int i=0;i<m_files.size();i++) {
		if(Utilities::compareStringsIgnoreCase(m_files[i]->getFileName(), formattedFileName) == 0) {
			fileIndex = i;
			break;
		}
	}
	
	delete [] formattedFileName;
	
	return fileIndex;
}

int Group::indexOfFile(const QString & fileName) const {
	if(fileName.isEmpty()) { return -1; }
	
	QString formattedFileName = fileName.trimmed();
	if(formattedFileName.length() == 0) { return -1; }
	
	QByteArray formattedFileNameBytes = formattedFileName.toLocal8Bit();
	
	for(int i=0;i<m_files.size();i++) {
		if(Utilities::compareStringsIgnoreCase(m_files[i]->getFileName(), formattedFileNameBytes.data()) == 0) {
			return i;
		}
	}
	return -1;
}

int Group::indexOfFile(const GroupFile * groupFile) const {
	if(!GroupFile::isValid(groupFile)) { return -1; }
	
	for(int i=0;i<m_files.size();i++) {
		if(*m_files[i] == *groupFile) {
			return i;
		}
	}
	return -1;
}

const GroupFile * Group::getFile(int index) const {
	if(index < 0 || index >= m_files.size()) { return NULL; }
	
	return m_files[index];
}

const GroupFile * Group::getFile(const char * fileName) const {
	if(fileName == NULL) { return NULL; }
	
	char * formattedFileName = Utilities::trimCopyString(fileName);
	if(Utilities::stringLength(formattedFileName) == 0) {
		delete [] formattedFileName;
		return false;
	}
	
	GroupFile * file = NULL;
	for(int i=0;i<m_files.size();i++) {
		if(Utilities::compareStringsIgnoreCase(m_files[i]->getFileName(), formattedFileName) == 0) {
			file = m_files[i];
			break;
		}
	}
	
	delete [] formattedFileName;
	
	return file;
}

const GroupFile * Group::getFile(const QString & fileName) const {
	if(fileName.isEmpty()) { return NULL; }
	
	QString formattedFileName = fileName.trimmed();
	if(formattedFileName.length() == 0) { return NULL; }
	
	QByteArray formattedFileNameBytes = formattedFileName.toLocal8Bit();
	
	for(int i=0;i<m_files.size();i++) {
		if(Utilities::compareStringsIgnoreCase(m_files[i]->getFileName(), formattedFileNameBytes.data()) == 0) {
			return m_files[i];
		}
	}
	return NULL;
}

QVector<GroupFile *> Group::getFilesWithExtension(const char * extension) const {
	QVector<GroupFile *> files;

	if(extension == NULL) { return files; }

	for(int i=0;i<m_files.size();i++) {
		if(Utilities::compareStringsIgnoreCase(Utilities::getFileExtensionSubPointer(m_files[i]->getFileName()), extension) == 0) {
			files.push_back(m_files[i]);
		}
	}

	return files;
}

QVector<GroupFile *> Group::getFilesWithExtension(const QString & extension) const {
	if(extension.isNull()) { return QVector<GroupFile *>(); }
	
	QByteArray extensionBytes = extension.toLocal8Bit();
	return getFilesWithExtension(extensionBytes.data());
}

QVector<QString> Group::getFileExtensions() const {
	QVector<QString> fileExtensions;

	const char * extension = NULL;
	bool duplicateExtension = false;

	for(int i=0;i<m_files.size();i++) {
		extension = Utilities::getFileExtensionSubPointer(m_files[i]->getFileName());
		if(Utilities::stringLength(extension) == 0) { continue; }

		duplicateExtension = false;
		for(int j=0;j<m_files.size();j++) {
			if(Utilities::compareStringsIgnoreCase(fileExtensions[i].toLocal8Bit().data(), extension) == 0) {
				duplicateExtension = true;
				break;
			}
		}
		if(duplicateExtension) { continue; }

		fileExtensions.push_back(QString(extension).toUpper());
	}

	return fileExtensions;
}

bool Group::extractFile(int index, const char * directory, bool overwrite) const {
	if(index < 0 || index >= m_files.size()) { return false; }
	
	return m_files[index]->writeTo(directory, overwrite);
}

bool Group::extractFile(int index, const QString & directory, bool overwrite) const {
	QByteArray directoryBytes = directory.toLocal8Bit().data();

	return extractFile(index, directoryBytes.data(), overwrite);
}

bool Group::extractFile(const char * fileName, const char * directory, bool overwrite) const {
	return extractFile(indexOfFile(fileName), directory, overwrite);
}

bool Group::extractFile(const QString & fileName, const char * directory, bool overwrite) const {
	return extractFile(indexOfFile(fileName), directory, overwrite);
}

bool Group::extractFile(const char * fileName, const QString & directory, bool overwrite) const {
	QByteArray directoryBytes = directory.toLocal8Bit();

	return extractFile(indexOfFile(fileName), directoryBytes.data(), overwrite);
}

bool Group::extractFile(const QString & fileName, const QString & directory, bool overwrite) const {
	QByteArray directoryBytes = directory.toLocal8Bit();

	return extractFile(indexOfFile(fileName), directoryBytes.data(), overwrite);
}

int Group::extractAllFilesWithExtension(const char * extension, const char * directory, bool overwrite) const {
	if(Utilities::stringLength(extension) == 0) { return 0; }

	int numberOfExtractedFiles = 0;
	
	for(int i=0;i<m_files.size();i++) {
		if(Utilities::compareStringsIgnoreCase(Utilities::getFileExtensionSubPointer(m_files[i]->getFileName()), extension) == 0) {
			if(extractFile(i, directory, overwrite)) {
				numberOfExtractedFiles++;
			}
		}
	}

	return numberOfExtractedFiles;
}

int Group::extractAllFilesWithExtension(const  QString & extension, const char * directory, bool overwrite) const {
	QByteArray extensionBytes = extension.toLocal8Bit();

	return extractAllFilesWithExtension(extensionBytes.data(), directory, overwrite);
}

int Group::extractAllFilesWithExtension(const char * extension, const QString & directory, bool overwrite) const {
	QByteArray directoryBytes = directory.toLocal8Bit();

	return extractAllFilesWithExtension(extension, directoryBytes.data(), overwrite);
}

int Group::extractAllFilesWithExtension(const QString & extension, const QString & directory, bool overwrite) const {
	QByteArray extensionBytes = extension.toLocal8Bit();
	QByteArray directoryBytes = directory.toLocal8Bit();

	return extractAllFilesWithExtension(extensionBytes.data(), directoryBytes.data(), overwrite);
}

int Group::extractAllFiles(const char * directory, bool overwrite) const {
	int numberOfExtractedFiles = 0;
	
	for(int i=0;i<m_files.size();i++) {
		if(extractFile(i, directory, overwrite)) {
			numberOfExtractedFiles++;
		}
	}
	
	return numberOfExtractedFiles;
}

int Group::extractAllFiles(const QString & directory, bool overwrite) const {
	QByteArray directoryBytes = directory.toLocal8Bit();

	return extractAllFiles(directoryBytes.data(), overwrite);
}

bool Group::addFile(const GroupFile * groupFile, bool replace) {
	if(!GroupFile::isValid(groupFile)) { return false; }
	
	int fileIndex = indexOfFile(groupFile);
	
	if(fileIndex == -1) {
		m_files.push_back(new GroupFile(*groupFile));
		return true;
	}
	else {
		if(replace) {
			m_files.replace(fileIndex, new GroupFile(*groupFile));
			return true;
		}
	}
	return false;
}

bool Group::addFiles(const QVector<GroupFile *> & files, bool replace) {
	if(files.size() == 0) { return 0; }

	int numberOfFilesAdded = 0;
		
	for(int i=0;i<files.size();i++) {
		if(addFile(files[i], replace)) {
			numberOfFilesAdded++;
		}
	}
		
	return numberOfFilesAdded;
}

bool Group::addFiles(const Group * group, bool replace) {
	if(group == NULL || group->m_files.size() == 0) { return 0; }

	int numberOfFilesAdded = 0;
		
	for(int i=0;i<group->m_files.size();i++) {
		if(addFile(group->m_files[i], replace)) {
			numberOfFilesAdded++;
		}
	}
		
	return numberOfFilesAdded;
}

bool Group::removeFile(int index) {
	if(index < 0 || index >= m_files.size()) { return false; }
	
	delete m_files[index];
	m_files.remove(index);
	
	return true;
}

bool Group::removeFile(const char * fileName) {
	if(fileName == NULL) { return false; }
	
	char * formattedFileName = Utilities::trimCopyString(fileName);
	if(Utilities::stringLength(formattedFileName) == 0) {
		delete [] formattedFileName;
		return false;
	}
	
	bool fileRemoved = false;
	for(int i=0;i<m_files.size();i++) {
		if(Utilities::compareStringsIgnoreCase(m_files[i]->getFileName(), formattedFileName) == 0) {
			delete m_files[i];
			m_files.remove(i);

			fileRemoved = true;
			break;
		}
	}
	
	delete [] formattedFileName;
	
	return fileRemoved;
}

bool Group::removeFile(const QString & fileName) {
	if(fileName.isEmpty()) { return false; }
	
	QString formattedFileName = fileName.trimmed();
	if(formattedFileName.length() == 0) { return false; }
	
	QByteArray formattedFileNameBytes = formattedFileName.toLocal8Bit();
	
	for(int i=0;i<m_files.size();i++) {
		if(Utilities::compareStringsIgnoreCase(m_files[i]->getFileName(), formattedFileNameBytes.data()) == 0) {
			delete m_files[i];
			m_files.remove(i);

			return true;
		}
	}
	return false;
}

bool Group::removeFile(const GroupFile * groupFile) {
	if(!GroupFile::isValid(groupFile)) { return false; }
	
	for(int i=0;i<m_files.size();i++) {
		if(*m_files[i] == *groupFile) {
			delete m_files[i];
			m_files.remove(i);

			return true;
		}
	}
	return false;
}

int Group::removeFiles(const QVector<QString> & fileNames) {
	if(fileNames.size() == 0) { return 0; }

	int numberOfFilesRemoved = 0;
		
	for(int i=0;i<fileNames.size();i++) {
		if(removeFile(fileNames[i])) {
			numberOfFilesRemoved++;
		}
	}
		
	return numberOfFilesRemoved;
}

int Group::removeFiles(const QVector<const GroupFile *> & files) {
	if(files.size() == 0) { return 0; }

	int numberOfFilesRemoved = 0;
		
	for(int i=0;i<files.size();i++) {
		if(removeFile(files[i])) {
			numberOfFilesRemoved++;
		}
	}
		
	return numberOfFilesRemoved;
}

void Group::clearFiles() {
	for(int i=0;i<m_files.size();i++) {
		delete m_files[i];
	}
	m_files.clear();
}

QString Group::toString() const {
	QString groupString;
	
	if(m_filePath != NULL) {
		char * fileName = Utilities::getFileNameNoPath(m_filePath);

		groupString.append(QString("\"%1\"").arg(fileName));

		delete [] fileName;
	}

	groupString.append(QString("Files (%1)%2").arg(m_files.size()).arg(m_files.size() == 0 ? "" : ": "));

	for(int i=0;i<m_files.size();i++) {
		if(i > 0) {
			groupString.append(", ");
		}

		groupString.append(m_files[i]->getFileName());
	}

	return groupString;
}

bool Group::load() {
	// verify that the file has a path
	if(m_filePath == NULL) {
		printf("Group file has no file name.\n");
		return false;
	}

	QFileInfo fileInfo(m_filePath);

	// verify that the file exists and is a file
	if(!fileInfo.exists() || !fileInfo.isFile()) {
		printf("Group file does not exist or is not a file: \"%s\".\n", m_filePath);
		return false;
	}
	
	// check to make sure that the file is not too big to be stored in memory
	if(fileInfo.size() > LLONG_MAX) {
		printf("Group file \"%s\" is too large to store in memory.\n", m_filePath);
		return false;
	}
	
	// open the file and read it into memory
	QFile file(m_filePath);
	if(!file.open(QIODevice::ReadOnly)) {
		printf("Failed to open group file: \"%s\".\n", m_filePath);
		return false;
	};
	QByteArray data = file.readAll();
	file.close();

	printf("Opened group file: \"%s\", loaded %d bytes into memory.\n", m_filePath, data.size());
	
	int offset = 0;
	
	// verify that the data is long enough to contain header information
	if(data.size() < offset + HEADER_LENGTH) {
		printf("Group file \"%s\" is incomplete or corrupted: missing header text.\n", m_filePath);
		return false;
	}
	
	// verify that the header text is specified in the header
	char * headerData = Utilities::byteArrayCopyOfRange(data, offset, offset + HEADER_TEXT_LENGTH);
	QString headerText = Serializer::deserializeByteString(headerData, HEADER_TEXT_LENGTH);
	delete [] headerData;

	if(Utilities::compareStrings(headerText.toLocal8Bit().data(), HEADER_TEXT) != 0) {
		printf("Group file \"%s\" is not a valid format, missing %s specification in header.\n", m_filePath, HEADER_TEXT);
		return false;
	}
	
	offset += HEADER_TEXT_LENGTH;
	
	printf("Verified group file header text.\n");
	
	// verify that the data is long enough to contain the number of files specification
	if(data.size() < offset + NUMBER_OF_FILES_LENGTH) {
		printf("Group file \"%s\" is incomplete or corrupted: missing number of files value.\n", m_filePath);
		return false;
	}
	
	// read and verify the number of files
	char * numberOfFilesData = Utilities::byteArrayCopyOfRange(data, offset, offset + NUMBER_OF_FILES_LENGTH);
	int numberOfFiles = Serializer::deserializeInteger(numberOfFilesData, NUMBER_OF_FILES_LENGTH, FILE_ENDIANNESS);
	delete [] numberOfFilesData;

	if(numberOfFiles < 0) {
		printf("Group file \"%s\" has a negative number of files value.\n", m_filePath);
		return false;
	}
	
	printf("Number of files detected in group: %d\n", numberOfFiles);
	
	offset += NUMBER_OF_FILES_LENGTH;

	char * fileNameData = NULL;
	char * fileSizeData = NULL;
	char * fileData = NULL;
	QString fileName;
	int fileSize = -1;
	GroupFile * g = NULL;
	QVector<GroupFile *> groupFiles;
	for(int i=0;i<numberOfFiles;i++) {
		// verify that the data is long enough to contain the file name
		if(data.size() < offset + GROUP_FILE_NAME_LENGTH) {
			printf("Group file \"%s\" is incomplete or corrupted: missing file #%d name.\n", m_filePath, i + 1);
			
			for(int i=0;i<groupFiles.size();i++) {
				delete groupFiles[i];
			}

			return false;
		}
		
		// read the file name
		fileNameData = Utilities::byteArrayCopyOfRange(data, offset, offset + GROUP_FILE_NAME_LENGTH);
		fileName = Serializer::deserializeByteString(fileNameData, GROUP_FILE_NAME_LENGTH);
		delete [] fileNameData;
		
		offset += GROUP_FILE_NAME_LENGTH;
		
		// verify that the data is long enough to contain the file size value
		if(data.size() < offset + GROUP_FILE_SIZE_LENGTH) {
			printf("Group file \"%s\" is incomplete or corrupted: missing file #%d size value.\n", m_filePath, i + 1);
			
			for(int i=0;i<groupFiles.size();i++) {
				delete groupFiles[i];
			}

			return false;
		}
		
		// read and verify the file size
		fileSizeData = Utilities::byteArrayCopyOfRange(data, offset, offset + GROUP_FILE_SIZE_LENGTH);
		fileSize = Serializer::deserializeInteger(fileSizeData, GROUP_FILE_SIZE_LENGTH, FILE_ENDIANNESS);
		delete [] fileSizeData;
		
		if(fileSize < 0) {
			printf("Group file \"%s\" file #%d size can not be negative.\n", m_filePath, i + 1);
			
			for(int i=0;i<groupFiles.size();i++) {
				delete groupFiles[i];
			}

			return false;
		}
		
		offset += GROUP_FILE_SIZE_LENGTH;
		
		groupFiles.push_back(new GroupFile(fileName, fileSize));
	}
	
	printf("All group file information parsed.\n");

	for(int i=0;i<numberOfFiles;i++) {
		g = groupFiles[i];
		
		if(data.size() < offset + g->getFileSize()) {
			int numberOfMissingBytes = g->getFileSize() - (data.size() - offset);
			int numberOfAdditionalFiles = groupFiles.size() - i - 1;
			printf("Group file \"%s\" is corrupted: missing %d of %d byte%s for file #%d (\"%s\") data.", m_filePath, numberOfMissingBytes, g->getFileSize(), g->getFileSize() == 1 ? "" : "s", i + 1, g->getFileName());
			if(numberOfAdditionalFiles > 0) {
				printf(" There is also an additional %d files that are missing data.", numberOfAdditionalFiles);
			}
			printf("\n");
			
			for(int i=0;i<groupFiles.size();i++) {
				delete groupFiles[i];
			}

			return false;
		}
		
		fileData = Utilities::byteArrayCopyOfRange(data, offset, offset + g->getFileSize());
		g->setData(fileData, g->getFileSize());
		delete [] fileData;
		
		offset += g->getFileSize();
	}
	
	addFiles(groupFiles);

	for(int i=0;i<groupFiles.size();i++) {
		delete groupFiles[i];
	}
	
	printf("Group file parsed successfully, %d files loaded into memory.\n", groupFiles.size());
	
	return true;
}

int Group::getGroupFileSize() const {
	int fileSize = 0;
	for(int i=0;i<m_files.size();i++) {
		fileSize += m_files[i]->getDataSize();
	}
	return fileSize;
}

bool Group::verifyAllFiles() const {
	for(int i=0;i<m_files.size();i++) {
		if(!m_files[i]->isValid()) {
			return false;
		}
	}
	return true;
}

bool Group::operator == (const Group & g) const {
	if(m_files.size() != g.m_files.size()) {
		return false;
	}
	
	for(int i=0;i<m_files.size();i++) {
		if(!g.hasFile(m_files[i])) {
			return false;
		}
	}
	return true;
}

bool Group::operator != (const Group & g) const {
	return !operator == (g);
}
