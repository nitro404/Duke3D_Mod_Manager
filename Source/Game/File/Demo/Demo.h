#ifndef _DEMO_H_
#define _DEMO_H_

#include "../GameFile.h"

#include <ByteBuffer.h>
#include <Endianness.h>

#include <memory>
#include <vector>
#include <string>

class Demo final : public GameFile {
public:
	Demo(const std::string & filePath = {});
	Demo(Demo && demo) noexcept;
	Demo(const Demo & demo);
	Demo & operator = (Demo && demo) noexcept;
	Demo & operator = (const Demo & demo);
	virtual ~Demo();

	static std::unique_ptr<Demo> readFrom(const ByteBuffer & byteBuffer);
	static std::unique_ptr<Demo> loadFrom(const std::string & filePath);

	virtual bool writeTo(ByteBuffer & byteBuffer) const override;
	virtual void addMetadata(std::vector<std::pair<std::string, std::string>> & metadata) const override;
	virtual Endianness getEndianness() const override;
	virtual size_t getSizeInBytes() const override;
	virtual bool isValid(bool verifyParent = true) const override;
	static bool isValid(const Demo * demo, bool verifyParent = true);

	bool operator == (const Demo & demo) const;
	bool operator != (const Demo & demo) const;

	static constexpr Endianness ENDIANNESS = Endianness::LittleEndian;

};

#endif // _DEMO_H_
