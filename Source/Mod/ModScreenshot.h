#ifndef _MOD_SCREENSHOT_H_
#define _MOD_SCREENSHOT_H_

#include "ModImage.h"

class ModScreenshot final : public ModImage {
public:
	ModScreenshot(const std::string & fileName, uint16_t width, uint16_t height, const std::string & sha1);
	ModScreenshot(ModImage && s) noexcept;
	ModScreenshot(ModScreenshot && s) noexcept;
	ModScreenshot(const ModScreenshot & s);
	ModScreenshot & operator = (ModScreenshot && s) noexcept;
	ModScreenshot & operator = (const ModScreenshot & s);
	virtual ~ModScreenshot();

	virtual tinyxml2::XMLElement * toXML(tinyxml2::XMLDocument * document) const override;
	static std::unique_ptr<ModScreenshot> parseFrom(const rapidjson::Value & modScreenshotValue);
	static std::unique_ptr<ModScreenshot> parseFrom(const tinyxml2::XMLElement * modScreenshotElement);

	virtual bool isValid() const override;
	static bool isValid(const ModScreenshot * s);
};

#endif // _MOD_SCREENSHOT_H_