#ifndef _INSTALLED_MOD_INFO_H_
#define _INSTALLED_MOD_INFO_H_

#include <rapidjson/document.h>

#include <chrono>
#include <memory>
#include <string>
#include <vector>

class GameVersion;
class ModVersion;

class InstalledModInfo final {
public:
	InstalledModInfo(const ModVersion * modVersion = nullptr, const std::vector<std::string> & originalFiles = {}, const std::vector<std::string> & modFiles = {});
	InstalledModInfo(InstalledModInfo && i) noexcept;
	InstalledModInfo(const InstalledModInfo & i);
	InstalledModInfo & operator = (InstalledModInfo && i) noexcept;
	InstalledModInfo & operator = (const InstalledModInfo & i);
	~InstalledModInfo();

	const std::string & getModID() const;
	const std::string & getModName() const;
	const std::string & getModVersion() const;
	std::string getFullModName() const;
	std::chrono::time_point<std::chrono::system_clock> getInstalledTimestamp() const;
	bool isEmpty() const;

	size_t numberOfOriginalFiles() const;
	bool hasOriginalFile(const std::string & filePath) const;
	size_t indexOfOriginalFile(const std::string & filePath) const;
	std::string getOriginalFile(size_t index) const;
	const std::vector<std::string> getOriginalFiles() const;
	bool addOriginalFile(const std::string & filePath);
	bool removeOriginalFile(size_t index);
	bool removeOriginalFile(const std::string filePath);
	void clearOriginalFiles();

	size_t numberOfModFiles() const;
	bool hasModFile(const std::string & filePath) const;
	size_t indexOfModFile(const std::string & filePath) const;
	std::string getModFile(size_t index) const;
	const std::vector<std::string> getModFiles() const;
	bool addModFile(const std::string & filePath);
	bool removeModFile(size_t index);
	bool removeModFile(const std::string filePath);
	void clearModFiles();

	rapidjson::Document toJSON() const;
	static std::unique_ptr<InstalledModInfo> parseFrom(const rapidjson::Value & installedModInfoValue);

	static std::unique_ptr<InstalledModInfo> loadFrom(const GameVersion & gameVersion);
	static std::unique_ptr<InstalledModInfo> loadFrom(const std::string & filePath);
	static std::unique_ptr<InstalledModInfo> loadFromJSON(const std::string & filePath);
	bool saveTo(const GameVersion & gameVersion);
	bool saveTo(const std::string & filePath, bool overwrite = true) const;
	bool saveToJSON(const std::string & filePath, bool overwrite = true) const;

	bool isValid() const;
	static bool isValid(const InstalledModInfo * i);

	bool operator == (const InstalledModInfo & i) const;
	bool operator != (const InstalledModInfo & i) const;

	static const std::string FILE_TYPE;
	static const std::string FILE_FORMAT_VERSION;
	static const std::string DEFAULT_FILE_NAME;

private:
	InstalledModInfo(const std::string & modID, const std::string & modName, const std::string & modVersion, std::chrono::time_point<std::chrono::system_clock> installedTimestamp, const std::vector<std::string> & originalFiles, const std::vector<std::string> & modFiles);

	std::string m_modID;
	std::string m_modName;
	std::string m_modVersion;
	std::chrono::time_point<std::chrono::system_clock> m_installedTimestamp;
	std::vector<std::string> m_originalFiles;
	std::vector<std::string> m_modFiles;
};

#endif // _INSTALLED_MOD_INFO_H_
