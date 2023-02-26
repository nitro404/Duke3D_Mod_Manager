#include "ModScreenshot.h"

static const std::string XML_MOD_SCREENSHOT_ELEMENT_NAME("screenshot");

ModScreenshot::ModScreenshot(const std::string & fileName, uint64_t fileSize, uint16_t width, uint16_t height, const std::string & sha1)
	: ModImage(fileName, fileSize, width, height, sha1) { }

ModScreenshot::ModScreenshot(ModImage && i) noexcept
	: ModImage(std::move(i)) { }

ModScreenshot::ModScreenshot(ModScreenshot && s) noexcept
	: ModImage(std::move(s)) { }

ModScreenshot::ModScreenshot(const ModScreenshot & s)
	: ModImage(s) { }

ModScreenshot & ModScreenshot::operator = (ModScreenshot && s) noexcept {
	ModImage::operator = (s);

	return *this;
}

ModScreenshot & ModScreenshot::operator = (const ModScreenshot & s) {
	ModImage::operator = (s);

	return *this;
}

ModScreenshot::~ModScreenshot() { }

tinyxml2::XMLElement * ModScreenshot::toXML(tinyxml2::XMLDocument * document) const {
	return ModImage::toXML(document, XML_MOD_SCREENSHOT_ELEMENT_NAME);
}

std::unique_ptr<ModScreenshot> ModScreenshot::parseFrom(const rapidjson::Value & modScreenshotValue) {
	return std::make_unique<ModScreenshot>(std::move(*std::move(ModImage::parseFrom(modScreenshotValue))));
}

std::unique_ptr<ModScreenshot> ModScreenshot::parseFrom(const tinyxml2::XMLElement * modScreenshotElement) {
	return std::make_unique<ModScreenshot>(std::move(*std::move(ModImage::parseFrom(modScreenshotElement, XML_MOD_SCREENSHOT_ELEMENT_NAME))));
}

bool ModScreenshot::isValid() const {
	return ModImage::isValid();
}

bool ModScreenshot::isValid(const ModScreenshot * s) {
	return ModImage::isValid(s);
}
