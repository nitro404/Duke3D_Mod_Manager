#ifndef MOD_DOWNLOAD_H
#define MOD_DOWNLOAD_H

#include <QString.h>
#include "Utilities/Utilities.h"
#include "Mod Collection/GameVersion.h"
#include "Mod Collection/ModState.h"

class ModDownload {
public:
	ModDownload(const char * fileName, const char * type);
	ModDownload(const QString & fileName, const QString & type);
	ModDownload(const ModDownload & d);
	ModDownload & operator = (const ModDownload & d);
	virtual ~ModDownload();

	const char * getFileName() const;
	int getPartNumber() const;
	int getPartCount() const;
	const char * getVersion() const;
	const char * getSpecial() const;
	const char * getSubfolder() const;
	GameVersions::GameVersion getGameVersion() const;
	const char * getType() const;
	const char * getDescription() const;
	ModStates::ModState getState() const;

	void setFileName(const char * fileName);
	void setFileName(const QString & fileName);
	void setPartNumber(int partNumber);
	void setPartNumber(const char * partNumber);
	void setPartNumber(const QString & partNumber);
	void setPartCount(int partCount);
	void setPartCount(const char * partCount);
	void setPartCount(const QString & partCount);
	void setVersion(const char * version);
	void setVersion(const QString & version);
	void setSpecial(const char * special);
	void setSpecial(const QString & special);
	void setSubfolder(const char * subfolder);
	void setSubfolder(const QString & subfolder);
	bool setGameVersion(int gameVersion);
	bool setGameVersion(GameVersions::GameVersion gameVersion);
	bool setGameVersion(const char * data);
	bool setGameVersion(const QString & data);
	void setType(const char * type);
	void setType(const QString & type);
	void setDescription(const char * description);
	void setDescription(const QString & description);
	bool setState(int state);
	bool setState(ModStates::ModState state);
	bool setState(const char * data);
	bool setState(const QString & data);

	bool operator == (const ModDownload & d) const;
	bool operator != (const ModDownload & d) const;

private:
	char * m_fileName;
	int m_partNumber;
	int m_partCount;
	char * m_version;
	char * m_special;
	char * m_subfolder;
	GameVersions::GameVersion m_gameVersion;
	char * m_type;
	char * m_description;
	ModStates::ModState m_state;
};

#endif // MOD_DOWNLOAD_H
