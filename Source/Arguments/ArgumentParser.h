#ifndef _ARGUMENT_PARSER_H_
#define _ARGUMENT_PARSER_H_

#include "ArgumentCollection.h"

#include <map>
#include <optional>
#include <string>

class ArgumentParser final : public ArgumentCollection {
public:
	ArgumentParser();
	ArgumentParser(int argc, char * argv[]);
	ArgumentParser(ArgumentParser && parser) noexcept;
	ArgumentParser(const ArgumentParser & parser);
	ArgumentParser & operator = (ArgumentParser && parser) noexcept;
	ArgumentParser & operator = (const ArgumentParser & parser);
	virtual ~ArgumentParser();

	bool hasPassthroughArguments() const;
	std::optional<std::string> getPassthroughArguments() const;
	void setPassthroughArguments(const std::string & passthroughArguments);
	void clearPassthroughArguments();

	bool parseArguments(int argc, char * argv[]);

	std::string toString() const;

	bool operator == (const ArgumentParser & parser) const;
	bool operator != (const ArgumentParser & parser) const;

protected:
	static std::optional<std::string> parseArgument(const std::string & data);

private:
	std::optional<std::string> m_passthroughArguments;
};

#endif // _ARGUMENT_PARSER_H_
