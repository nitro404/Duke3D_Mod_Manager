#ifndef _DOSBOX_VERSION_COLLECTION_H_
#define _DOSBOX_VERSION_COLLECTION_H_

#include "DOSBoxVersion.h"

#include <rapidjson/document.h>

#include <memory>
#include <optional>
#include <string>
#include <vector>

class DOSBoxVersionCollection final : public DOSBoxVersion::Listener {
public:
	class Listener {
	public:
		virtual ~Listener();

		virtual void dosboxVersionCollectionSizeChanged(DOSBoxVersionCollection & dosboxVersionCollection) = 0;
		virtual void dosboxVersionCollectionItemModified(DOSBoxVersionCollection & dosboxVersionCollection, DOSBoxVersion & dosboxVersion) = 0;
	};

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
	bool hasDOSBoxVersionWithName(const std::string & name) const;
	size_t indexOfDOSBoxVersion(const DOSBoxVersion & dosboxVersion) const;
	size_t indexOfDOSBoxVersionWithName(const std::string & name) const;
	std::shared_ptr<DOSBoxVersion> getDOSBoxVersion(size_t index) const;
	std::shared_ptr<DOSBoxVersion> getDOSBoxVersionWithName(const std::string & name) const;
	const std::vector<std::shared_ptr<DOSBoxVersion>> & getDOSBoxVersions() const;
	std::vector<std::shared_ptr<DOSBoxVersion>> getConfiguredDOSBoxVersions() const;
	std::vector<std::shared_ptr<DOSBoxVersion>> getUnconfiguredDOSBoxVersions() const;
	std::vector<std::string> getDOSBoxVersionDisplayNames(bool prependItemNumber = true) const;
	static std::vector<std::string> getDOSBoxVersionDisplayNamesFrom(const std::vector<std::shared_ptr<DOSBoxVersion>> & dosboxVersions, bool prependItemNumber = true);
	static std::vector<std::string> getDOSBoxVersionDisplayNamesFrom(const std::vector<const DOSBoxVersion *> & dosboxVersions, bool prependItemNumber = true);
	bool addDOSBoxVersion(const DOSBoxVersion & dosboxVersion);
	bool addDOSBoxVersion(std::shared_ptr<DOSBoxVersion> dosboxVersion);
	size_t addDOSBoxVersions(const std::vector<DOSBoxVersion> & dosboxVersions);
	size_t addDOSBoxVersions(const std::vector<const DOSBoxVersion *> & dosboxVersions);
	size_t addDOSBoxVersions(const std::vector<std::shared_ptr<DOSBoxVersion>> & dosboxVersions);
	bool removeDOSBoxVersion(size_t index);
	bool removeDOSBoxVersion(const DOSBoxVersion & dosboxVersion);
	bool removeDOSBoxVersionWithName(const std::string & name);
	size_t addMissingDefaultDOSBoxVersions();
	void setDefaultDOSBoxVersions();
	void clearDOSBoxVersions();

	size_t checkForMissingExecutables() const;
	size_t checkForMissingExecutables(const std::string & name) const;

	rapidjson::Document toJSON() const;
	static std::unique_ptr<DOSBoxVersionCollection> parseFrom(const rapidjson::Value & dosboxVersionCollectionValue);

	bool loadFrom(const std::string & filePath, bool autoCreate = true);
	bool loadFromJSON(const std::string & filePath, bool autoCreate = true);
	bool saveTo(const std::string & filePath, bool overwrite = true) const;
	bool saveToJSON(const std::string & filePath, bool overwrite = true) const;

	size_t numberOfListeners() const;
	bool hasListener(const Listener & listener) const;
	size_t indexOfListener(const Listener & listener) const;
	Listener * getListener(size_t index) const;
	bool addListener(Listener & listener);
	bool removeListener(size_t index);
	bool removeListener(const Listener & listener);
	void clearListeners();

	bool isValid() const;
	static bool isValid(const DOSBoxVersionCollection * g);

	bool operator == (const DOSBoxVersionCollection & g) const;
	bool operator != (const DOSBoxVersionCollection & g) const;

	static const std::string FILE_FORMAT_VERSION;

	// DOSBoxVersion::Listener Virtuals
	virtual void dosboxVersionModified(DOSBoxVersion & dosboxVersion) override;

private:
	void notifyCollectionSizeChanged();
	void notifyDOSBoxVersionModified(DOSBoxVersion & dosboxVersion);

	std::vector<std::shared_ptr<DOSBoxVersion>> m_dosboxVersions;
	std::vector<Listener *> m_listeners;
};

#endif // _DOSBOX_VERSION_COLLECTION_H_
