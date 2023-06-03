#include "SectorItem.h"

#include "BuildConstants.h"

#include <Math/ExtendedMath.h>
#include <Utilities/RapidJSONUtilities.h>

#include <spdlog/spdlog.h>

static const std::string JSON_POSITION_PROPERTY_NAME("position");
static const std::string JSON_ANGLE_PROPERTY_NAME("angle");
static const std::string JSON_SECTOR_INDEX_PROPERTY_NAME("sectorIndex");

SectorItem::SectorItem()
	: m_position(Point3D::ZERO)
	, m_angle(0)
	, m_sectorIndex(0) { }

SectorItem::SectorItem(int32_t xPosition, int32_t yPosition, int32_t zPosition, int16_t angle, uint16_t sectorIndex)
	: m_position(xPosition, yPosition, zPosition)
	, m_angle(angle)
	, m_sectorIndex(sectorIndex) { }

SectorItem::SectorItem(int32_t position[3], int16_t angle, uint16_t sectorIndex)
	: m_position(position)
	, m_angle(angle)
	, m_sectorIndex(sectorIndex) { }

SectorItem::SectorItem(const Point3D & position, int16_t angle, uint16_t sectorIndex)
	: m_position(position)
	, m_angle(angle)
	, m_sectorIndex(sectorIndex) { }

SectorItem::SectorItem(SectorItem && sectorItem) noexcept
	: m_position(std::move(sectorItem.m_position))
	, m_angle(sectorItem.m_angle)
	, m_sectorIndex(sectorItem.m_sectorIndex) { }

SectorItem::SectorItem(const SectorItem & sectorItem)
	: m_position(sectorItem.m_position)
	, m_angle(sectorItem.m_angle)
	, m_sectorIndex(sectorItem.m_sectorIndex) { }

SectorItem & SectorItem::operator = (SectorItem && sectorItem) noexcept {
	if(this != &sectorItem) {
		m_position = std::move(sectorItem.m_position);
		m_angle = sectorItem.m_angle;
		m_sectorIndex = sectorItem.m_sectorIndex;
	}

	return *this;
}

SectorItem & SectorItem::operator = (const SectorItem & sectorItem) {
	m_position = sectorItem.m_position;
	m_angle = sectorItem.m_angle;
	m_sectorIndex = sectorItem.m_sectorIndex;

	return *this;
}

SectorItem::~SectorItem() { }

int32_t SectorItem::getX() const {
	return m_position.x;
}

void SectorItem::setX(int32_t x) {
	m_position.x = x;
}

int32_t SectorItem::getY() const {
	return m_position.y;
}

void SectorItem::setY(int32_t y) {
	m_position.y = y;
}

int32_t SectorItem::getZ() const {
	return m_position.z;
}

void SectorItem::setZ(int32_t z) {
	m_position.z = z;
}

const Point3D & SectorItem::getPosition() const {
	return m_position;
}

void SectorItem::setPosition(int32_t x, int32_t y, int32_t z) {
	m_position.setPoint(x, y, z);
}

void SectorItem::setPosition(int32_t position[3]) {
	m_position.setPoint(position);
}

void SectorItem::setPosition(const Point3D & position) {
	m_position = position;
}

int16_t SectorItem::getAngle() const {
	return m_angle;
}

bool SectorItem::setAngle(int16_t angle) {
	if(angle < BuildConstants::MIN_ANGLE || angle > BuildConstants::MAX_ANGLE) {
		return false;
	}

	m_angle = angle;

	return true;
}

double SectorItem::getAngleDegrees() const {
	return angleToDegrees(m_angle);
}

bool SectorItem::setAngleDegrees(double angleDegrees) {
	std::optional<int16_t> optionalAngle(angleFromDegrees(angleDegrees));

	if(!optionalAngle.has_value()) {
		return false;
	}

	m_angle = optionalAngle.value();

	return true;
}

double SectorItem::getAngleRadians() const {
	return angleToRadians(m_angle);
}

bool SectorItem::setAngleRadians(double angleRadians) {
	std::optional<int16_t> optionalAngle(angleFromRadians(angleRadians));

	if(!optionalAngle.has_value()) {
		return false;
	}

	m_angle = optionalAngle.value();

	return true;
}

uint16_t SectorItem::getSectorIndex() const {
	return m_sectorIndex;
}

void SectorItem::setSectorIndex(uint16_t sectorIndex) {
	m_sectorIndex = sectorIndex;
}

int16_t SectorItem::angleFromDegrees(double angleDegrees, bool * error) {
	int64_t angle = (angleDegrees / 360.0) * static_cast<double>(BuildConstants::MAX_ANGLE);

	if(angle < BuildConstants::MIN_ANGLE || angle > BuildConstants::MAX_ANGLE) {
		if(error != nullptr) {
			*error = true;
		}

		return 0;
	}

	return static_cast<int16_t>(angle);
}

int16_t SectorItem::angleFromRadians(double angleRadians, bool * error) {
	int64_t angle = (angleRadians / (Math::PI * 2.0)) * static_cast<double>(BuildConstants::MAX_ANGLE);

	if(angle < BuildConstants::MIN_ANGLE || angle > BuildConstants::MAX_ANGLE) {
		if(error != nullptr) {
			*error = true;
		}

		return 0;
	}

	return static_cast<int16_t>(angle);
}

std::optional<int16_t> SectorItem::angleFromDegrees(double angleDegrees) {
	bool error = false;

	uint16_t angle = angleFromDegrees(angleDegrees, &error);

	if(error) {
		return {};
	}

	return angle;
}

std::optional<int16_t> SectorItem::angleFromRadians(double angleRadians) {
	bool error = false;

	uint16_t angle = angleFromRadians(angleRadians, &error);

	if(error) {
		return {};
	}

	return angle;
}

double SectorItem::angleToDegrees(int16_t angle) {
	return (static_cast<double>(angle) / static_cast<double>(BuildConstants::MAX_ANGLE)) * 360.0;
}

double SectorItem::angleToRadians(int16_t angle) {
	return (static_cast<double>(angle) / static_cast<double>(BuildConstants::MAX_ANGLE)) * Math::PI * 2.0;
}

rapidjson::Value SectorItem::toJSON(rapidjson::MemoryPoolAllocator<rapidjson::CrtAllocator> & allocator) const {
	rapidjson::Value sectorItemValue(rapidjson::kObjectType);

	addToJSONObject(sectorItemValue, allocator);

	return sectorItemValue;
}

bool SectorItem::addToJSONObject(rapidjson::Value & value, rapidjson::MemoryPoolAllocator<rapidjson::CrtAllocator> & allocator) const {
	if(!value.IsObject()) {
		return false;
	}

	rapidjson::Value positionValue(m_position.toJSON(allocator));
	value.AddMember(rapidjson::StringRef(JSON_POSITION_PROPERTY_NAME.c_str()), positionValue, allocator);

	value.AddMember(rapidjson::StringRef(JSON_ANGLE_PROPERTY_NAME.c_str()), rapidjson::Value(m_angle), allocator);

	value.AddMember(rapidjson::StringRef(JSON_SECTOR_INDEX_PROPERTY_NAME.c_str()), rapidjson::Value(m_sectorIndex), allocator);

	return true;
}

SectorItem SectorItem::parseFrom(const rapidjson::Value & sectorItemValue, bool * error) {
	if(!sectorItemValue.IsObject()) {
		spdlog::error("Invalid sector item type: '{}', expected 'object'.", Utilities::typeToString(sectorItemValue.GetType()));

		if(error != nullptr) {
			*error = true;
		}

		return {};
	}

	// parse position
	if(!sectorItemValue.HasMember(JSON_POSITION_PROPERTY_NAME.c_str())) {
		spdlog::error("Sector item is missing '{}' property.", JSON_POSITION_PROPERTY_NAME);

		if(error != nullptr) {
			*error = true;
		}

		return {};
	}

	std::optional<Point3D> optionalPosition(Point3D::parseFrom(sectorItemValue[JSON_POSITION_PROPERTY_NAME.c_str()]));

	if(!optionalPosition.has_value()) {
		if(error != nullptr) {
			*error = true;
		}

		return {};
	}

	// parse angle
	if(!sectorItemValue.HasMember(JSON_ANGLE_PROPERTY_NAME.c_str())) {
		spdlog::error("Sector item is missing '{}' property.", JSON_ANGLE_PROPERTY_NAME);

		if(error != nullptr) {
			*error = true;
		}

		return {};
	}

	const rapidjson::Value & angleValue = sectorItemValue[JSON_ANGLE_PROPERTY_NAME.c_str()];

	if(!angleValue.IsInt()) {
		spdlog::error("Sector item has an invalid '{}' property type: '{}', expected integer 'number'.", JSON_ANGLE_PROPERTY_NAME, Utilities::typeToString(angleValue.GetType()));

		if(error != nullptr) {
			*error = true;
		}

		return {};
	}

	int32_t angle = angleValue.GetInt();

	if(angle < BuildConstants::MIN_ANGLE || angle > BuildConstants::MAX_ANGLE) {
		spdlog::error("Invalid sector item angle: {}, expected a value between {} and {}, inclusively.", angle, BuildConstants::MIN_ANGLE, BuildConstants::MAX_ANGLE);

		if(error != nullptr) {
			*error = true;
		}

		return {};
	}

	// parse sector index
	if(!sectorItemValue.HasMember(JSON_SECTOR_INDEX_PROPERTY_NAME.c_str())) {
		spdlog::error("Sector item is missing '{}' property.", JSON_SECTOR_INDEX_PROPERTY_NAME);

		if(error != nullptr) {
			*error = true;
		}

		return {};
	}

	const rapidjson::Value & sectorIndexValue = sectorItemValue[JSON_SECTOR_INDEX_PROPERTY_NAME.c_str()];

	if(!sectorIndexValue.IsUint()) {
		spdlog::error("Sector item has an invalid '{}' property type: '{}', expected unsigned integer 'number'.", JSON_SECTOR_INDEX_PROPERTY_NAME, Utilities::typeToString(sectorIndexValue.GetType()));

		if(error != nullptr) {
			*error = true;
		}

		return {};
	}

	uint32_t sectorIndex = sectorIndexValue.GetUint();

	if(sectorIndex > std::numeric_limits<uint16_t>::max()) {
		spdlog::error("Invalid sector item sector index: {}, expected a value between 0 and {}, inclusively.", sectorIndex, std::numeric_limits<uint16_t>::max());

		if(error != nullptr) {
			*error = true;
		}

		return {};
	}

	return SectorItem(std::move(optionalPosition.value()), static_cast<int16_t>(angle), static_cast<uint16_t>(sectorIndex));
}

std::optional<SectorItem> SectorItem::parseFrom(const rapidjson::Value & sectorItemValue) {
	bool error = false;

	SectorItem value(parseFrom(sectorItemValue, &error));

	if(error) {
		return {};
	}

	return std::move(value);
}

bool SectorItem::operator == (const SectorItem & sectorItem) const {
	return m_position == sectorItem.m_position &&
		   m_angle == sectorItem.m_angle &&
		   m_sectorIndex == sectorItem.m_sectorIndex;
}

bool SectorItem::operator != (const SectorItem & sectorItem) const {
	return !operator == (sectorItem);
}
