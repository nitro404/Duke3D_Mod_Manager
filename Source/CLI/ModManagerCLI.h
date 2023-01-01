#include "Game/GameType.h"
#include "Mod/OrganizedModCollection.h"

#include <memory>
#include <optional>
#include <string>
#include <vector>

class GameVersion;
class ModGameVersion;
class ModManager;
class ModVersionType;

class ModManagerCLI final {
public:
	ModManagerCLI(ModManager * modManager);
	~ModManagerCLI();

	void runMenu();
	bool runSelectedMod();
	bool updateFilterTypePrompt(const std::string & args = "");
	std::optional<OrganizedModCollection::FilterType> promptForFilterType(const std::string & args = "") const;
	bool runSortPrompt(const std::string & args = "");
	bool updateGameTypePrompt(const std::string & args = "");
	std::optional<GameType> promptForGameType(const std::string & args = "") const;
	bool updatePreferredGameVersionPrompt(const std::string & args = "");
	std::shared_ptr<GameVersion> promptForPreferredGameVersion(const std::string & args = "") const;
	std::optional<std::pair<std::shared_ptr<GameVersion>, std::shared_ptr<ModGameVersion>>> runUnsupportedModVersionTypePrompt(std::shared_ptr<ModVersionType> modVersionType, const std::string & promptMessage = "") const;
	bool updateIPAddressPrompt(const std::string & args = "");
	std::string promptForIPAddress(const std::string & args = "") const;
	bool updatePortPrompt(const std::string & args = "");
	std::optional<uint16_t> promptForPort(const std::string & args = "") const;
	bool runSelectRandomModPrompt(bool selectPreferredVersion, bool selectFirstVersionType);
	bool runSearchPrompt(const std::string & args = "");
	bool updateSelectedModVersionPrompt();
	bool updateSelectedModVersionTypePrompt();

private:
	static size_t getValuePrompt(const std::vector<std::string> & values, const std::string & promptMessage = "", const std::string & args = "");
	static std::string getArguments(const std::string & data);
	static void printSpacing(size_t length);
	static void pause();
	static void clearOutput();

	ModManager * m_modManager;

	ModManagerCLI(const ModManagerCLI &) = delete;
	ModManagerCLI(ModManagerCLI &&) noexcept = delete;
	const ModManagerCLI & operator = (const ModManagerCLI &) = delete;
	const ModManagerCLI & operator = (ModManagerCLI &&) noexcept = delete;
};
