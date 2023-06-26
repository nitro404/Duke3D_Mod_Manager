#ifndef _ANIMATION_H_
#define _ANIMATION_H_

#include "../GameFile.h"
#include "../Palette/ColourTable.h"

#include <chrono>
#include <memory>
#include <string>
#include <vector>

class Animation : public GameFile {
public:
	Animation(Animation && animation) noexcept;
	Animation(const Animation & animation);
	Animation & operator = (Animation && animation) noexcept;
	Animation & operator = (const Animation & animation);
	virtual ~Animation();

	virtual uint16_t getFrameWidth() const = 0;
	virtual uint16_t getFrameHeight() const = 0;
	virtual uint32_t numberOfFrames() const = 0;
	virtual std::chrono::milliseconds getDuration() const = 0;
	virtual std::shared_ptr<ColourTable> getColourTable() const = 0;

	virtual void addMetadata(std::vector<std::pair<std::string, std::string>> & metadata) const override;
	virtual bool isValid(bool verifyParent = true) const override;
	static bool isValid(const Animation * animation, bool verifyParent = true);

protected:
	Animation(const std::string & filePath = {});
};

#endif // _ANIMATION_H_
