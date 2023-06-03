#include "Velocity3D.h"

#include <ByteBuffer.h>
#include <Utilities/RapidJSONUtilities.h>

#include <fmt/core.h>
#include <spdlog/spdlog.h>

#include <cmath>

static const std::string JSON_X_PROPERTY_NAME("x");
static const std::string JSON_Y_PROPERTY_NAME("y");
static const std::string JSON_Z_PROPERTY_NAME("z");

const Velocity3D Velocity3D::ZERO(0, 0, 0);

Velocity3D::Velocity3D()
	: x(0)
	, y(0)
	, z(0) { }

Velocity3D::Velocity3D(int16_t a, int16_t b, int16_t c)
	: x(a)
	, y(b)
	, z(c) { }

Velocity3D::Velocity3D(int16_t p[3])
	: x(p[0])
	, y(p[1])
	, z(p[2]) { }

Velocity3D::Velocity3D(const Velocity3D & p)
	: x(p.x)
	, y(p.y)
	, z(p.z) { }

Velocity3D & Velocity3D::operator = (const Velocity3D & p) {
	x = p.x;
	y = p.y;

	return *this;
}

Velocity3D::~Velocity3D() = default;

Velocity3D Velocity3D::operator  + (int16_t c)            const { return Velocity3D(x + c,   y + c,   z + c); }
Velocity3D Velocity3D::operator  + (const Velocity3D & v) const { return Velocity3D(x + v.x, y + v.y, z + v.z); }

void Velocity3D::operator       += (int16_t c)                  { x += c;   y += c;   z += c; }
void Velocity3D::operator       += (const Velocity3D & v)       { x += v.x; y += v.y; z += v.z; }

Velocity3D Velocity3D::operator  - ()                     const { return Velocity3D(-x, -y, -z); }

Velocity3D Velocity3D::operator  - (int16_t c)            const { return Velocity3D(x - c,   y - c,   z - c); }
Velocity3D Velocity3D::operator  - (const Velocity3D & v) const { return Velocity3D(x - v.x, y - v.y, z - v.z); }

void Velocity3D::operator       -= (int16_t c)                  { x -= c;   y -= c;   z -= c; }
void Velocity3D::operator       -= (const Velocity3D & v)       { x -= v.x; y -= v.y; z -= v.z; }

Velocity3D Velocity3D::operator  * (double c)             const { return Velocity3D(x * c,   y * c,   z * c); }
Velocity3D Velocity3D::operator  * (const Velocity3D & v) const { return Velocity3D(x * v.x, y * v.y, z * v.z); }

void Velocity3D::operator       *= (double c)                   { x *= c;   y *= c;   z *= c; }
void Velocity3D::operator       *= (const Velocity3D & v)       { x *= v.x; y *= v.y; z *= v.z; }

Velocity3D Velocity3D::operator  / (double c)             const { return Velocity3D(  c == 0.0 ? 0.0 : x / c,     c == 0.0 ? 0.0 : y / c,     c == 0.0 ? 0.0 : z / c); }
Velocity3D Velocity3D::operator  / (const Velocity3D & v) const { return Velocity3D(v.x == 0   ? 0   : x / v.x, v.y == 0   ? 0   : y / v.y, v.z == 0   ? 0   : z / v.z); }

void Velocity3D::operator       /= (double c)                   { x =   c == 0.0 ? 0.0 : x / c;   y =   c == 0.0 ? 0.0 : y / c;   z =   c == 0.0 ? 0.0 : z / c; }
void Velocity3D::operator       /= (const Velocity3D & v)       { x = v.x == 0   ? 0   : x / v.x; y = v.y == 0   ? 0   : y / v.y; z = v.z == 0   ? 0   : z / v.z; }

int16_t Velocity3D::operator    [] (size_t index) const {
	if(index > 2) {
		return 0;
	}

	return v[index];
}

bool Velocity3D::operator == (const Velocity3D & v) const {
	return x == v.x &&
		   y == v.y &&
		   z == v.z;
}

bool Velocity3D::operator != (const Velocity3D & v) const {
	return !operator == (v);
}

void Velocity3D::setVelocity(int16_t a, int16_t b, int16_t c) {
	x = a;
	y = b;
	z = c;
}

void Velocity3D::setVelocity(const int16_t v[3]) {
	x = v[0];
	y = v[1];
	z = v[2];
}

double Velocity3D::length() const {
	return sqrt((x * x) + (y * y) + (z * z));
}

int64_t Velocity3D::dot(const Velocity3D & v) const {
	return (x * v.x) + (y * v.y) + (z * v.z);
}

Velocity3D Velocity3D::cross(const Velocity3D & v) const {
	return Velocity3D((y * v.z) - (z * v.y), (z * v.x) - (x * v.z), (x * v.y) - (y * v.x));
}

Velocity3D Velocity3D::getFrom(const ByteBuffer & byteBuffer, size_t offset, bool * error) {
	if(offset + SIZE_BYTES > byteBuffer.getSize()) {
		if(error != nullptr) {
			*error = true;
		}

		return {};
	}

	bool internalError = false;

	Velocity3D value;

	for(size_t i = 0; i < 3; i++) {
		value.v[i] = byteBuffer.getInteger(offset + (sizeof(int16_t) * i), &internalError);

		if(internalError) {
			if(error != nullptr) {
				*error = true;
			}

			return {};
		}
	}

	return value;
}

std::optional<Velocity3D> Velocity3D::getFrom(const ByteBuffer & byteBuffer, size_t offset) {
	bool error = false;

	Velocity3D value(getFrom(byteBuffer, offset, &error));

	if(error) {
		return {};
	}

	return value;
}

Velocity3D Velocity3D::readFrom(const ByteBuffer & byteBuffer, bool * error) {
	bool internalError = false;

	Velocity3D value(getFrom(byteBuffer, byteBuffer.getReadOffset(), error));

	if(internalError) {
		if(error != nullptr) {
			*error = true;
		}
	}
	else {
		byteBuffer.skipReadBytes(SIZE_BYTES);
	}

	return value;
}

std::optional<Velocity3D> Velocity3D::readFrom(const ByteBuffer & byteBuffer) {
	bool error = false;

	Velocity3D value(readFrom(byteBuffer, &error));

	if(error) {
		return {};
	}

	return value;
}

bool Velocity3D::putIn(ByteBuffer & byteBuffer, size_t offset) const {
	for(size_t i = 0; i < 3; i++) {
		if(!byteBuffer.putInteger(v[i], offset + (sizeof(int16_t) * i))) {
			return false;
		}
	}

	return true;
}

bool Velocity3D::insertIn(ByteBuffer & byteBuffer, size_t offset) const {
	for(size_t i = 0; i < 3; i++) {
		if(!byteBuffer.insertInteger(v[i], offset + (sizeof(int16_t) * i))) {
			return false;
		}
	}

	return true;
}

bool Velocity3D::writeTo(ByteBuffer & byteBuffer) const {
	for(size_t i = 0; i < 3; i++) {
		if(!byteBuffer.putUnsignedLong(v[i], byteBuffer.getWriteOffset())) {
			return false;
		}

		byteBuffer.skipWriteBytes(sizeof(int16_t));
	}

	return true;
}

rapidjson::Value Velocity3D::toJSON(rapidjson::MemoryPoolAllocator<rapidjson::CrtAllocator> & allocator) const {
	rapidjson::Value velocityValue(rapidjson::kObjectType);

	velocityValue.AddMember(rapidjson::StringRef(JSON_X_PROPERTY_NAME.c_str()), rapidjson::Value(x), allocator);
	velocityValue.AddMember(rapidjson::StringRef(JSON_Y_PROPERTY_NAME.c_str()), rapidjson::Value(y), allocator);
	velocityValue.AddMember(rapidjson::StringRef(JSON_Z_PROPERTY_NAME.c_str()), rapidjson::Value(z), allocator);

	return velocityValue;
}

Velocity3D Velocity3D::parseFrom(const rapidjson::Value & velocityValue, bool * error) {
	if(!velocityValue.IsObject()) {
		spdlog::error("Invalid point type: '{}', expected 'object'.", Utilities::typeToString(velocityValue.GetType()));

		if(error != nullptr) {
			*error = true;
		}

		return {};
	}

	// parse x value
	if(!velocityValue.HasMember(JSON_X_PROPERTY_NAME.c_str())) {
		spdlog::error("Point is missing '{}' property.", JSON_X_PROPERTY_NAME);

		if(error != nullptr) {
			*error = true;
		}

		return {};
	}

	const rapidjson::Value & xValue = velocityValue[JSON_X_PROPERTY_NAME.c_str()];

	if(!xValue.IsInt()) {
		spdlog::error("Point has an invalid '{}' property type: '{}', expected integer 'number'.", JSON_X_PROPERTY_NAME, Utilities::typeToString(xValue.GetType()));

		if(error != nullptr) {
			*error = true;
		}

		return {};
	}

	// parse y value
	if(!velocityValue.HasMember(JSON_Y_PROPERTY_NAME.c_str())) {
		spdlog::error("Point is missing '{}' property.", JSON_Y_PROPERTY_NAME);

		if(error != nullptr) {
			*error = true;
		}

		return {};
	}

	const rapidjson::Value & yValue = velocityValue[JSON_Y_PROPERTY_NAME.c_str()];

	if(!yValue.IsInt()) {
		spdlog::error("Point has an invalid '{}' property type: '{}', expected integer 'number'.", JSON_Y_PROPERTY_NAME, Utilities::typeToString(yValue.GetType()));

		if(error != nullptr) {
			*error = true;
		}

		return {};
	}

	// parse z value
	if(!velocityValue.HasMember(JSON_Z_PROPERTY_NAME.c_str())) {
		spdlog::error("Point is missing '{}' property.", JSON_Z_PROPERTY_NAME);

		if(error != nullptr) {
			*error = true;
		}

		return {};
	}

	const rapidjson::Value & zValue = velocityValue[JSON_Z_PROPERTY_NAME.c_str()];

	if(!zValue.IsInt()) {
		spdlog::error("Point has an invalid '{}' property type: '{}', expected integer 'number'.", JSON_Z_PROPERTY_NAME, Utilities::typeToString(zValue.GetType()));

		if(error != nullptr) {
			*error = true;
		}

		return {};
	}

	return Velocity3D(xValue.GetInt(), yValue.GetInt(), zValue.GetInt());
}

std::optional<Velocity3D> Velocity3D::parseFrom(const rapidjson::Value & velocityValue) {
	bool error = false;

	Velocity3D value(parseFrom(velocityValue, &error));

	if(error) {
		return {};
	}

	return value;
}

std::string Velocity3D::toString() const {
	return fmt::format("{}, {}, {}", x, y, z);
}
