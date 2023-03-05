#ifndef _MOD_SCREENSHOT_H_
#define _MOD_SCREENSHOT_H_

#include "ModImage.h"

class ModScreenshot final : public ModImage {
public:
	ModScreenshot(const std::string & fileName, uint64_t fileSize, uint16_t width, uint16_t height, const std::string & sha1);
	ModScreenshot(ModImage && s) noexcept;
	ModScreenshot(ModScreenshot && s) noexcept;
	ModScreenshot(const ModScreenshot & s);
	ModScreenshot & operator = (ModScreenshot && s) noexcept;
	ModScreenshot & operator = (const ModScreenshot & s);
	virtual ~ModScreenshot();

	virtual tinyxml2::XMLElement * toXML(tinyxml2::XMLDocument * document) const override;
	static std::unique_ptr<ModScreenshot> parseFrom(const rapidjson::Value & modScreenshotValue, bool skipFileInfoValidation = false);
	static std::unique_ptr<ModScreenshot> parseFrom(const tinyxml2::XMLElement * modScreenshotElement, bool skipFileInfoValidation = false);

	virtual bool isValid(bool skipFileInfoValidation = false) const override;
	static bool isValid(const ModScreenshot * s, bool skipFileInfoValidation = false);
};

#endif // _MOD_SCREENSHOT_H_
