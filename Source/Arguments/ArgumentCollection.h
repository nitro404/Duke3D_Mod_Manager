#ifndef _ARGUMENT_COLLECTION_H_
#define _ARGUMENT_COLLECTION_H_

#include <cstdint>
#include <map>
#include <string>
#include <vector>

class ArgumentCollection {
public:
	enum class ArgumentCase {
		OriginalCase,
		UpperCase,
		LowerCase
	};

	ArgumentCollection(ArgumentCase caseType = DEFAULT_CASE);
	ArgumentCollection(ArgumentCollection && argumentCollection) noexcept;
	ArgumentCollection(const ArgumentCollection & argumentCollection);
	ArgumentCollection & operator = (ArgumentCollection && argumentCollection) noexcept;
	ArgumentCollection & operator = (const ArgumentCollection & argumentCollection);
	virtual ~ArgumentCollection();

	ArgumentCase getCase() const;
	void setCase(ArgumentCase caseType);

	size_t numberOfArguments() const;
	size_t numberOfArguments(const std::string & name) const;
	bool hasArgument(const std::string & name) const;
	std::string getFirstValue(const std::string & name) const;
	std::vector<std::string> getValues(const std::string & name) const;
	bool addArgument(const std::string & name, const std::string & value);
	void removeArgument(const std::string & name);
	void clear();

	std::string toString() const;

	bool operator == (const ArgumentCollection & argumentCollection) const;
	bool operator != (const ArgumentCollection & argumentCollection) const;

private:
	std::string formatArgument(const std::string & data) const;

public:
	static const ArgumentCase DEFAULT_CASE;

protected:
	ArgumentCase m_case;
	std::multimap<std::string, std::string> m_arguments;
};

#endif // _ARGUMENT_COLLECTION_H_
