#ifndef _DOSBOX_VERSION_COLLECTION_H_
#define _DOSBOX_VERSION_COLLECTION_H_

#include "DOSBoxVersion.h"

#include <boost/signals2.hpp>
#include <rapidjson/document.h>

#include <memory>
#include <optional>
#include <string>
#include <vector>

class DOSBoxVersionCollection final {
public:
	DOSBoxVersionCollection();
	DOSBoxVersionCollection(const std::vector<DOSBoxVersion> & dosboxVersions);
	DOSBoxVersionCollection(const std::vector<const DOSBoxVersion *> & dosboxVersions);
	DOSBoxVersionCollection(const std::vector<std::shared_ptr<DOSBoxVersion>> & dosboxVersions);
	DOSBoxVersionCollection(DOSBoxVersionCollection && dosboxVersions) noexcept;
	DOSBoxVersionCollection(const DOSBoxVersionCollection & dosboxVersions);
	DOSBoxVersionCollection & operator = (DOSBoxVersionCollection && dosboxVersions) noexcept;
	DOSBoxVersionCollection & operator = (const DOSBoxVersionCollection & dosboxVersions);
	virtual ~DOSBoxVersionCollection();

	size_t numberOfDOSBoxVersions() const;
	bool hasDOSBoxVersion(const DOSBoxVersion & dosboxVersion) const;
	bool hasDOSBoxVersionWithID(const std::string & dosboxVersionID) const;
	size_t indexOfDOSBoxVersion(const DOSBoxVersion & dosboxVersion) const;
	size_t indexOfDOSBoxVersionWithID(const std::string & dosboxVersionID) const;
	std::shared_ptr<DOSBoxVersion> getDOSBoxVersion(size_t index) const;
	std::shared_ptr<DOSBoxVersion> getDOSBoxVersionWithID(const std::string & dosboxVersionID) const;
	const std::vector<std::shared_ptr<DOSBoxVersion>> & getDOSBoxVersions() const;
	std::vector<std::shared_ptr<DOSBoxVersion>> getConfiguredDOSBoxVersions() const;
	std::vector<std::shared_ptr<DOSBoxVersion>> getUnconfiguredDOSBoxVersions() const;
	std::vector<std::string> getDOSBoxVersionIdentifiers() const;
	static std::vector<std::string> getDOSBoxVersionIdentifiersFrom(const std::vector<std::shared_ptr<DOSBoxVersion>> & dosboxVersions);
	static std::vector<std::string> getDOSBoxVersionIdentifiersFrom(const std::vector<const DOSBoxVersion *> & dosboxVersions);
	std::vector<std::string> getDOSBoxVersionLongNames(bool prependItemNumber = true) const;
	static std::vector<std::string> getDOSBoxVersionLongNamesFrom(const std::vector<std::shared_ptr<DOSBoxVersion>> & dosboxVersions, bool prependItemNumber = true);
	static std::vector<std::string> getDOSBoxVersionLongNamesFrom(const std::vector<const DOSBoxVersion *> & dosboxVersions, bool prependItemNumber = true);
	std::vector<std::string> getDOSBoxVersionShortNames(bool prependItemNumber = true) const;
	static std::vector<std::string> getDOSBoxVersionShortNamesFrom(const std::vector<std::shared_ptr<DOSBoxVersion>> & dosboxVersions, bool prependItemNumber = true);
	static std::vector<std::string> getDOSBoxVersionShortNamesFrom(const std::vector<const DOSBoxVersion *> & dosboxVersions, bool prependItemNumber = true);
	bool addDOSBoxVersion(const DOSBoxVersion & dosboxVersion);
	bool addDOSBoxVersion(std::shared_ptr<DOSBoxVersion> dosboxVersion);
	size_t addDOSBoxVersions(const std::vector<DOSBoxVersion> & dosboxVersions);
	size_t addDOSBoxVersions(const std::vector<const DOSBoxVersion *> & dosboxVersions);
	size_t addDOSBoxVersions(const std::vector<std::shared_ptr<DOSBoxVersion>> & dosboxVersions);
	bool removeDOSBoxVersion(size_t index);
	bool removeDOSBoxVersion(const DOSBoxVersion & dosboxVersion);
	bool removeDOSBoxVersionWithID(const std::string & dosboxVersionID);
	size_t addMissingDefaultDOSBoxVersions();
	void setDefaultDOSBoxVersions();
	void clearDOSBoxVersions();

	size_t checkForMissingExecutables() const;
	size_t checkForMissingExecutables(const std::string & dosboxVersionID) const;

	rapidjson::Document toJSON() const;
	static std::unique_ptr<DOSBoxVersionCollection> parseFrom(const rapidjson::Value & dosboxVersionCollectionValue);

	bool loadFrom(const std::string & filePath, bool autoCreate = true);
	bool loadFromJSON(const std::string & filePath, bool autoCreate = true);
	bool saveTo(const std::string & filePath, bool overwrite = true) const;
	bool saveToJSON(const std::string & filePath, bool overwrite = true) const;

	bool loadDOSBoxConfigurationsFrom(const std::string & directoryPath, size_t * numberOfConfigurationsLoaded = nullptr);
	bool saveDOSBoxConfigurationsTo(const std::string & directoryPath, size_t * numberOfConfigurationsSaved = nullptr) const;

	bool isValid() const;
	static bool isValid(const DOSBoxVersionCollection * g);

	bool operator == (const DOSBoxVersionCollection & g) const;
	bool operator != (const DOSBoxVersionCollection & g) const;

	boost::signals2::signal<void (DOSBoxVersionCollection & /* dosboxVersions */, DOSBoxVersion & /* dosboxVersion */)> itemModified;
	boost::signals2::signal<void (DOSBoxVersionCollection & /* dosboxVersions */)> sizeChanged;

	static const std::string FILE_TYPE;
	static const uint32_t FILE_FORMAT_VERSION;

private:
	void onDOSBoxVersionModified(DOSBoxVersion & dosboxVersion);

	std::vector<std::shared_ptr<DOSBoxVersion>> m_dosboxVersions;
	std::vector<boost::signals2::connection> m_dosboxVersionConnections;
};

#endif // _DOSBOX_VERSION_COLLECTION_H_
