#ifndef MOD_GAME_VERSION_H
#define MOD_GAME_VERSION_H

#include <QString.h>
#include <QVector.h>
#include "Utilities/Utilities.h"
#include "Mod Collection/ModVersionFile.h"
#include "Mod Collection/ModState.h"
#include "Mod Collection/GameVersion.h"

class ModGameVersion {
public:
	ModGameVersion(GameVersions::GameVersion gameVersion, ModStates::ModState state);
	ModGameVersion(const ModGameVersion & m);
	ModGameVersion & operator = (const ModGameVersion & m);
	virtual ~ModGameVersion();
	
	GameVersions::GameVersion getGameVersion() const;
	bool setGameVersion(int gameVersion);
	bool setGameVersion(GameVersions::GameVersion gameVersion);
	bool setGameVersion(const char * data);
	bool setGameVersion(const QString & data);
	
	ModStates::ModState getState() const;
	bool setState(ModStates::ModState state);

	int numberOfFiles() const;
	bool hasFile(const ModVersionFile & file) const;
	bool hasFile(const char * fileName) const;
	bool hasFile(const QString & fileName) const;
	bool hasFileOfType(const char * fileType) const;
	bool hasFileOfType(const QString & fileType) const;
	int indexOfFile(const ModVersionFile & file) const;
	int indexOfFile(const char * fileName) const;
	int indexOfFile(const QString & fileName) const;
	int indexOfFileByType(const char * fileType) const;
	int indexOfFileByType(const QString & fileType) const;
	const ModVersionFile * getFile(int index) const;
	const ModVersionFile * getFile(const char * fileName) const;
	const ModVersionFile * getFile(const QString & fileName) const;
	const ModVersionFile * getFileByType(const char * fileType) const;
	const ModVersionFile * getFileByType(const QString & fileType) const;
	const char * getFileNameByType(const char * fileType) const;
	const char * getFileNameByType(const QString & fileType) const;
	bool addFile(ModVersionFile * file);
	bool removeFile(int index);
	bool removeFile(const ModVersionFile & file);
	bool removeFile(const char * fileName);
	bool removeFile(const QString & fileName);
	bool removeFileByType(const char * fileType);
	bool removeFileByType(const QString & fileType);
	void clearFiles();

	bool isValid() const;
	static bool isValid(const ModGameVersion * m);
	
	bool operator == (const ModGameVersion & m) const;
	bool operator != (const ModGameVersion & m) const;
	
private:
	GameVersions::GameVersion m_gameVersion;
	ModStates::ModState m_state;
	QVector<ModVersionFile *> m_files;
};

#endif // MOD_GAME_VERSION_H
