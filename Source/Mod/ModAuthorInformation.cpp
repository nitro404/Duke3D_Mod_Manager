#include "ModAuthorInformation.h"

#include <Utilities/StringUtilities.h>

ModAuthorInformation::ModAuthorInformation(const std::string & name)
	: m_name(Utilities::trimString(name))
	, m_numberOfMods(1) { }


ModAuthorInformation::ModAuthorInformation(ModAuthorInformation && m) noexcept
	: m_name(std::move(m.m_name))
	, m_numberOfMods(m.m_numberOfMods) { }

ModAuthorInformation::ModAuthorInformation(const ModAuthorInformation & m)
	: m_name(m.m_name)
	, m_numberOfMods(m.m_numberOfMods) { }

ModAuthorInformation & ModAuthorInformation::operator = (ModAuthorInformation && m) noexcept {
	if(this != &m) {
		m_name = std::move(m.m_name);
		m_numberOfMods = m.m_numberOfMods;
	}

	return *this;
}

ModAuthorInformation & ModAuthorInformation::operator = (const ModAuthorInformation & m) {
	m_name = m.m_name;
	m_numberOfMods = m.m_numberOfMods;

	return *this;
}

ModAuthorInformation::~ModAuthorInformation() = default;

const std::string & ModAuthorInformation::getName() const {
	return m_name;
}

void ModAuthorInformation::setName(const std::string & name) {
	m_name = Utilities::trimString(name);
}

uint8_t ModAuthorInformation::getModCount() const {
	return m_numberOfMods;
}

uint8_t ModAuthorInformation::incrementModCount() {
	return m_numberOfMods++;
}

void ModAuthorInformation::setModCount(uint8_t numberOfMods) {
	if(numberOfMods < 0) { return; }

	m_numberOfMods = numberOfMods;
}

void ModAuthorInformation::resetModCount() {
	m_numberOfMods = 0;
}

bool ModAuthorInformation::isValid() const {
	return !m_name.empty();
}

bool ModAuthorInformation::isValid(const ModAuthorInformation * m) {
	return m != nullptr && m->isValid();
}

bool ModAuthorInformation::operator == (const ModAuthorInformation & m) const {
	return Utilities::compareStringsIgnoreCase(m_name, m.m_name) == 0 &&
		   m_numberOfMods == m.m_numberOfMods;
}

bool ModAuthorInformation::operator != (const ModAuthorInformation & m) const {
	return !operator == (m);
}
