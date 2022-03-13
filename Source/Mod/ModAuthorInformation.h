#ifndef _MOD_AUTHOR_INFORMATION_H_
#define _MOD_AUTHOR_INFORMATION_H_

#include <cstdint>
#include <string>

class ModAuthorInformation final {
public:
	ModAuthorInformation(const std::string & name);
	ModAuthorInformation(ModAuthorInformation && m) noexcept;
	ModAuthorInformation(const ModAuthorInformation & m);
	ModAuthorInformation & operator = (ModAuthorInformation && m) noexcept;
	ModAuthorInformation & operator = (const ModAuthorInformation & m);
	~ModAuthorInformation();

	const std::string & getName() const;
	void setName(const std::string & name);

	uint8_t getModCount() const;
	uint8_t incrementModCount();
	void setModCount(uint8_t numberOfMods);
	void resetModCount();

	bool isValid() const;
	static bool isValid(const ModAuthorInformation * m);

	bool operator == (const ModAuthorInformation & m) const;
	bool operator != (const ModAuthorInformation & m) const;

private:
	std::string m_name;
	uint8_t m_numberOfMods;
};

#endif // _MOD_AUTHOR_INFORMATION_H_
