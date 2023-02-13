#ifndef _MOD_VIDEO_H_
#define _MOD_VIDEO_H_

#include <rapidjson/document.h>

#include <cstdint>
#include <memory>
#include <string>

class Mod;

namespace tinyxml2 {
	class XMLDocument;
	class XMLElement;
}

class ModVideo final {
	friend class Mod;

public:
	ModVideo(const std::string & url, const std::string & title, uint16_t width = 0, uint16_t height = 0);
	ModVideo(ModVideo && v) noexcept;
	ModVideo(const ModVideo & v);
	ModVideo & operator = (ModVideo && v) noexcept;
	ModVideo & operator = (const ModVideo & v);
	~ModVideo();

	const std::string & getURL() const;
	const std::string & getTitle() const;
	uint16_t getWidth() const;
	uint16_t getHeight() const;
	const Mod * getParentMod() const;

	void setURL(const std::string & url);
	void setTitle(const std::string & title);
	void setWidth(uint16_t width);
	void setHeight(uint16_t height);

	rapidjson::Value toJSON(rapidjson::MemoryPoolAllocator<rapidjson::CrtAllocator> & allocator) const;
	tinyxml2::XMLElement * toXML(tinyxml2::XMLDocument * document) const;
	static std::unique_ptr<ModVideo> parseFrom(const rapidjson::Value & modVideoValue);
	static std::unique_ptr<ModVideo> parseFrom(const tinyxml2::XMLElement * modVideoElement);

	bool isValid() const;
	static bool isValid(const ModVideo * v);

	bool operator == (const ModVideo & v) const;
	bool operator != (const ModVideo & v) const;

protected:
	void setParentMod(const Mod * mod);

private:
	std::string m_url;
	std::string m_title;
	uint16_t m_width;
	uint16_t m_height;
	const Mod * m_parentMod;
};

#endif // _MOD_VIDEO_H_
