#include "Demo.h"

#include <spdlog/spdlog.h>

Demo::Demo(const std::string & filePath)
	: GameFile(filePath) { }

Demo::Demo(Demo && demo) noexcept
	: GameFile(demo) { }

Demo::Demo(const Demo & demo)
	: GameFile(demo) { }

Demo & Demo::operator = (Demo && demo) noexcept {
	if(this != &demo) {
		GameFile::operator = (demo);
	}

	return *this;
}

Demo & Demo::operator = (const Demo & demo) {
	GameFile::operator = (demo);

	return *this;
}

Demo::~Demo() { }

std::unique_ptr<Demo> Demo::readFrom(const ByteBuffer & byteBuffer) {
// TODO:
return nullptr;
}

std::unique_ptr<Demo> Demo::loadFrom(const std::string & filePath) {
// TODO:
return nullptr;
}

bool Demo::writeTo(ByteBuffer & byteBuffer) const {
// TODO:
return false;
}

void Demo::addMetadata(std::vector<std::pair<std::string, std::string>> & metadata) const {
	if(!isValid()) {
		return;
	}

// TODO:
}

Endianness Demo::getEndianness() const {
	return ENDIANNESS;
}

size_t Demo::getSizeInBytes() const {
// TODO:
return 0;
}

bool Demo::isValid(bool verifyParent) const {
// TODO:
return true;
}

bool Demo::isValid(const Demo * demo, bool verifyParent) {
	return demo != nullptr &&
		   demo->isValid(verifyParent);
}

bool Demo::operator == (const Demo & demo) const {
// TODO:
return this == &demo;
}

bool Demo::operator != (const Demo & demo) const {
	return !operator == (demo);
}
