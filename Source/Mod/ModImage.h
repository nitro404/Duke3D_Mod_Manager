#ifndef _MOD_IMAGE_H_
#define _MOD_IMAGE_H_

#include <rapidjson/document.h>

#include <cstdint>
#include <memory>
#include <string>

class Mod;

namespace tinyxml2 {
	class XMLDocument;
	class XMLElement;
}

class ModImage {
	friend class Mod;

public:
	ModImage(const std::string & fileName, uint64_t fileSize, uint16_t width, uint16_t height, const std::string & sha1 = std::string());
	ModImage(ModImage && i) noexcept;
	ModImage(const ModImage & i);
	ModImage & operator = (ModImage && i) noexcept;
	ModImage & operator = (const ModImage & i);
	virtual ~ModImage();

	const std::string & getFileName() const;
	uint64_t getFileSize() const;
	const std::string & getType() const;
	const std::string & getSubfolder() const;
	const std::string & getCaption() const;
	uint16_t getWidth() const;
	uint16_t getHeight() const;
	const std::string & getSHA1() const;
	const Mod * getParentMod() const;

	void setFileName(const std::string & fileName);
	void setFileSize(uint64_t fileSize);
	void setType(const std::string & type);
	void setSubfolder(const std::string & subfolder);
	void setCaption(const std::string & caption);
	void setWidth(uint16_t width);
	void setHeight(uint16_t height);
	void setSHA1(const std::string & sha1);

	rapidjson::Value toJSON(rapidjson::MemoryPoolAllocator<rapidjson::CrtAllocator> & allocator) const;
	virtual tinyxml2::XMLElement * toXML(tinyxml2::XMLDocument * document) const;
	static std::unique_ptr<ModImage> parseFrom(const rapidjson::Value & modImageValue);
	static std::unique_ptr<ModImage> parseFrom(const tinyxml2::XMLElement * modImageElement);

	virtual bool isValid() const;
	static bool isValid(const ModImage * i);

	bool operator == (const ModImage & i) const;
	bool operator != (const ModImage & i) const;

protected:
	void setParentMod(const Mod * mod);
	tinyxml2::XMLElement * toXML(tinyxml2::XMLDocument * document, const std::string & name) const;
	static std::unique_ptr<ModImage> parseFrom(const tinyxml2::XMLElement * modImageElement, const std::string & name);

private:
	std::string m_fileName;
	uint64_t m_fileSize;
	std::string m_type;
	std::string m_subfolder;
	std::string m_caption;
	uint16_t m_width;
	uint16_t m_height;
	std::string m_sha1;
	const Mod * m_parentMod;
};

#endif // _MOD_IMAGE_H_
