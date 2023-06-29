#include "Animation.h"

#include <fmt/core.h>

Animation::Animation(const std::string & filePath)
	: GameFile(filePath) { }

Animation::Animation(Animation && animation) noexcept
	: GameFile(std::move(animation)) { }

Animation::Animation(const Animation & animation)
	: GameFile(animation) { }

Animation & Animation::operator = (Animation && animation) noexcept {
	if(this != &animation) {
		GameFile::operator = (std::move(animation));
	}

	return *this;
}

Animation & Animation::operator = (const Animation & animation) {
	GameFile::operator = (animation);

	return *this;
}

Animation::~Animation() { }

void Animation::addMetadata(std::vector<std::pair<std::string, std::string>> & metadata) const {
	metadata.push_back({ "Frame Size", fmt::format("{} x {}", getFrameWidth(), getFrameHeight()) });
	metadata.push_back({ "Number of Frames", std::to_string(numberOfFrames()) });
	metadata.push_back({ "Duration", fmt::format("{:.2f} Seconds", getDuration().count() / 1000.0) });

	std::shared_ptr<ColourTable> colourTable(getColourTable());

	if(colourTable != nullptr) {
		const std::string & colourTableName = colourTable->getName();
		const std::optional<uint8_t> & optionalTransparentColourIndex = colourTable->getTransparentColourIndex();

		if(!colourTableName.empty()) {
			metadata.push_back({ "Colour Table Name", colourTableName });
		}

		metadata.push_back({ "Number of Colours", std::to_string(colourTable->numberOfColours()) });
		metadata.push_back({ "Transparent Colour Index", optionalTransparentColourIndex.has_value() ? std::to_string(optionalTransparentColourIndex.value()) : "N/A" });
	}
}

bool Animation::isValid(bool verifyParent) const {
	if(getFrameWidth() == 0 ||
	   getFrameHeight() == 0) {
		return false;
	}

	std::shared_ptr<ColourTable> colourTable(getColourTable());

	if(colourTable != nullptr) {
		if(!colourTable->isValid()) {
			return false;
		}

		if(verifyParent) {
			if(colourTable->getParent() != this) {
				return false;
			}
		}
	}

	return true;
}

bool Animation::isValid(const Animation * animation, bool verifyParent) {
	return animation != nullptr &&
		   animation->isValid(verifyParent);
}
