#ifndef _LOCATION_H_
#define _LOCATION_H_

#include <rapidjson/document.h>

#include <string>

namespace tinyxml2 {
	class XMLDocument;
	class XMLElement;
}

class Location final {
public:
	Location();
	Location(Location && location) noexcept;
	Location(const Location & location);
	Location & operator = (Location && location) noexcept;
	Location & operator = (const Location & location);
	~Location();

	const std::string & getTown() const;
	void setTown(const std::string & town);
	const std::string & getCity() const;
	void setCity(const std::string & city);
	const std::string & getProvince() const;
	void setProvince(const std::string & province);
	const std::string & getState() const;
	void setState(const std::string & state);
	const std::string & getProvinceOrState() const;
	const std::string & getCountry() const;
	void setCountry(const std::string & country);
	bool hasValue() const;
	std::string getDetails() const;
	void clear();

	void addToJSONObject(rapidjson::Value & locationValue, rapidjson::MemoryPoolAllocator<rapidjson::CrtAllocator> & allocator) const;
	void addToXMLElement(tinyxml2::XMLElement * locationElement) const;
	bool parseFrom(const rapidjson::Value & locationValue);
	bool parseFrom(const tinyxml2::XMLElement * locationElement);

	bool isValid() const;
	static bool isValid(const Location * location);

	bool operator == (const Location & location) const;
	bool operator != (const Location & location) const;

private:
	std::string m_town;
	std::string m_city;
	std::string m_province;
	std::string m_state;
	std::string m_country;
};

#endif // _LOCATION_H_
