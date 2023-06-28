#ifndef _SOUND_H_
#define _SOUND_H_

#include "../GameFile.h"

#include <ByteBuffer.h>
#include <Endianness.h>

#include <sndfile.h>

#include <chrono>
#include <memory>
#include <vector>
#include <string>

class Sound : public GameFile {
public:
	enum class PropertyType : uint8_t {
		Title = SF_STR_TITLE,
		Copyright = SF_STR_COPYRIGHT,
		Software = SF_STR_SOFTWARE,
		Artist = SF_STR_ARTIST,
		Comment = SF_STR_COMMENT,
		Date = SF_STR_DATE,
		Album = SF_STR_ALBUM,
		License = SF_STR_LICENSE,
		TrackNumber = SF_STR_TRACKNUMBER,
		Genre = SF_STR_GENRE
	};

	Sound(int format, uint32_t sampleRate, uint16_t numberOfChannels, const std::string & filePath = {});
	Sound(Sound && sound) noexcept;
	Sound & operator = (Sound && sound) noexcept;
	virtual ~Sound();

	std::chrono::milliseconds getDuration() const;
	uint32_t getBitrate() const;
	int numberOfChannels() const;
	int numberOfFrames() const;
	int numberOfSections() const;
	int getSampleRate() const;
	bool isSeekable() const;
	const SF_INFO & getInfo() const;
	const ByteBuffer * getData() const;
	std::string getFormatName() const;
	std::string getTypeName() const;
	std::string getSubTypeName() const;
	std::string getTitle() const;
	std::string getCopyright() const;
	std::string getSoftware() const;
	std::string getArtist() const;
	std::string getComment() const;
	std::string getDate() const;
	std::string getAlbum() const;
	std::string getLicense() const;
	std::string getTrackNumber() const;
	std::string getGenre() const;
	std::string getProperty(PropertyType propertyType) const;
	bool setFormatName(std::string value) const;
	bool setTypeName(std::string value) const;
	bool setSubTypeName(std::string value) const;
	bool setTitle(std::string value) const;
	bool setCopyright(std::string value) const;
	bool setSoftware(std::string value) const;
	bool setArtist(std::string value) const;
	bool setComment(std::string value) const;
	bool setDate(std::string value) const;
	bool setAlbum(std::string value) const;
	bool setLicense(std::string value) const;
	bool setTrackNumber(std::string value) const;
	bool setGenre(std::string value) const;
	bool setProperty(PropertyType propertyType, std::string propertyValue) const;

	virtual bool writeTo(ByteBuffer & byteBuffer) const override;
	virtual void addMetadata(std::vector<std::pair<std::string, std::string>> & metadata) const override;
	virtual size_t getSizeInBytes() const override;
	virtual bool isValid(bool verifyParent = true) const override;
	static bool isValid(const Sound * sound, bool verifyParent = true);

	static std::string getFormatName(int format);
	static std::string getTypeName(int format);
	static std::string getSubTypeName(int format);

	bool operator == (const Sound & sound) const;
	bool operator != (const Sound & sound) const;

protected:
	using FileHandle = std::unique_ptr<SNDFILE, std::function<void (SNDFILE *)>>;

	Sound(FileHandle fileHandle, SF_INFO && info, std::unique_ptr<ByteBuffer> data = nullptr);

	static FileHandle createSoundFileHandle(SNDFILE * soundFile);
	static FileHandle readFrom(const ByteBuffer & byteBuffer, SF_INFO & info);
	static FileHandle loadFrom(const std::string & filePath, SF_INFO & info);
	static bool isSuccess(int errorCode, const std::string & errorMessage = {});

	std::unique_ptr<ByteBuffer> m_data;
	SF_INFO m_info;
	FileHandle m_fileHandle;

private:
	Sound(const Sound &) = delete;
	Sound & operator = (const Sound &) = delete;
};

#endif // _SOUND_H_
