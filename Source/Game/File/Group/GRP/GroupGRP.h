#ifndef _GROUP_GRP_H_
#define _GROUP_GRP_H_

#include "../Group.h"

class GroupGRP final : public Group {
public:
	GroupGRP(const std::string & filePath = {});
	GroupGRP(std::vector<std::unique_ptr<GroupFile>> groupFiles, const std::string & filePath = {});
	GroupGRP(GroupGRP && group) noexcept;
	GroupGRP(const GroupGRP & group);
	GroupGRP & operator = (GroupGRP && group) noexcept;
	GroupGRP & operator = (const GroupGRP & group);
	virtual ~GroupGRP();

	static std::unique_ptr<GroupGRP> readFrom(const ByteBuffer & byteBuffer);
	virtual bool writeTo(ByteBuffer & byteBuffer) const override;

	static std::unique_ptr<GroupGRP> createFrom(const std::string & directoryPath);
	static std::unique_ptr<GroupGRP> loadFrom(const std::string & filePath);

	virtual Endianness getEndianness() const override;
	virtual size_t getSizeInBytes() const override;

	bool operator == (const GroupGRP & group) const;
	bool operator != (const GroupGRP & group) const;

	static constexpr Endianness ENDIANNESS = Endianness::LittleEndian;
	static inline const std::string HEADER_TEXT = "KenSilverman";
	static inline const std::string DUKE_NUKEM_3D_GROUP_FILE_NAME = "DUKE3D.GRP";
	static inline const std::string DUKE_NUKEM_3D_BETA_VERSION_GROUP_SHA1_FILE_HASH = "a6341c16bc1170b43be7f28b5a91c080f9ce3409";
	static inline const std::string DUKE_NUKEM_3D_REGULAR_VERSION_GROUP_SHA1_FILE_HASH = "3d508eaf3360605b0204301c259bd898717cf468";
	static inline const std::string DUKE_NUKEM_3D_PLUTONIUM_PAK_GROUP_SHA1_FILE_HASH = "61e70f883df9552395406bf3d64f887f3c709438";
	static inline const std::string DUKE_NUKEM_3D_ATOMIC_EDITION_GROUP_SHA1_FILE_HASH = "4fdef8559e2d35b1727fe92f021df9c148cf696c";
	static inline const std::string DUKE_NUKEM_3D_WORLD_TOUR_GROUP_SHA1_FILE_HASH = "d745396afc3e734029ec2b9bd8b20bdb3a11b3a2";

private:

};

#endif // _GROUP_GRP_H_
