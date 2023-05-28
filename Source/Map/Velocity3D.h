#ifndef _VELOCITY_3D_H_
#define _VELOCITY_3D_H_

#include <rapidjson/document.h>

#include <cstdint>
#include <optional>
#include <string>

class ByteBuffer;

class Velocity3D final {
public:
	Velocity3D();
	Velocity3D(int16_t a, int16_t b, int16_t c);
	Velocity3D(int16_t v[3]);
	Velocity3D(const Velocity3D & v);
	Velocity3D & operator = (const Velocity3D & v);
	~Velocity3D();

	Velocity3D operator   + (int16_t c)            const;
	Velocity3D operator   + (const Velocity3D & v) const;

	void operator        += (int16_t c);
	void operator        += (const Velocity3D & v);

	Velocity3D operator   - ()                     const;

	Velocity3D operator   - (int16_t c)            const;
	Velocity3D operator   - (const Velocity3D & v) const;

	void operator        -= (int16_t c);
	void operator        -= (const Velocity3D & v);

	Velocity3D operator   * (double c)             const;
	Velocity3D operator   * (const Velocity3D & v) const;

	void operator        *= (double c);
	void operator        *= (const Velocity3D & v);

	Velocity3D operator   / (double c)             const;
	Velocity3D operator   / (const Velocity3D & v) const;

	void operator        /= (double c);
	void operator        /= (const Velocity3D & v);

	int16_t operator     [] (size_t index)         const;

	bool operator        == (const Velocity3D & v) const;
	bool operator        != (const Velocity3D & v) const;

	void setVelocity(int16_t a, int16_t b, int16_t c);
	void setVelocity(const int16_t v[3]);

	double length()const;
	int64_t dot(const Velocity3D & v) const;
	Velocity3D cross(const Velocity3D & v) const;

	static Velocity3D getFrom(const ByteBuffer & byteBuffer, size_t offset, bool * error);
	static std::optional<Velocity3D> getFrom(const ByteBuffer & byteBuffer, size_t offset);
	static Velocity3D readFrom(const ByteBuffer & byteBuffer, bool * error);
	static std::optional<Velocity3D> readFrom(const ByteBuffer & byteBuffer);
	bool putIn(ByteBuffer & byteBuffer, size_t offset) const;
	bool insertIn(ByteBuffer & byteBuffer, size_t offset) const;
	bool writeTo(ByteBuffer & byteBuffer) const;

	rapidjson::Value toJSON(rapidjson::MemoryPoolAllocator<rapidjson::CrtAllocator> & allocator) const;
	static Velocity3D parseFrom(const rapidjson::Value & velocityValue, bool * error);
	static std::optional<Velocity3D> parseFrom(const rapidjson::Value & velocityValue);
	std::string toString() const;

	static constexpr size_t SIZE_BYTES = (sizeof(int16_t) * 3);
	static const Velocity3D ZERO;

	union {
		struct {
			int16_t x, y, z;
		};

		int16_t v[3];
	};
};

#endif // _VELOCITY_3D_H_
