#include "ModManager.h"

#include "Game/GameVersion.h"
#include "Game/GameVersionCollection.h"
#include "Mod/Mod.h"
#include "Mod/ModVersion.h"
#include "Mod/ModVersionType.h"
#include "Mod/ModGameVersion.h"
#include "Mod/ModAuthorInformation.h"
#include "ModMatch.h"
#include "SettingsManager.h"

#include <Utilities/NumberUtilities.h>
#include <Utilities/StringUtilities.h>
#include <Utilities/Utilities.h>

#include <fmt/core.h>
#include <magic_enum.hpp>

#include <conio.h>
#include <iostream>
#include <optional>
#include <regex>
#include <sstream>

static const std::regex   backRegExp("^(b(ack)?)$");
static const std::regex   helpRegExp("^(\\?|help)$");

ModManager::CLI::CLI(ModManager * modManager)
	: m_modManager(modManager) { }

ModManager::CLI::~CLI() {
	m_modManager = nullptr;
}

void ModManager::CLI::runMenu() {
	if(m_modManager == nullptr || !m_modManager->isInitialized()) {
		return;
	}

	std::shared_ptr<OrganizedModCollection> organizedMods = m_modManager->getOrganizedMods();

	size_t selectedIndex = std::numeric_limits<size_t>::max();
	bool error = false;
	std::string input, data, formattedData, date;

	static const std::regex      searchRegExp("^(s(earch)?)([ ]+.*)?");
	static const std::regex      randomRegExp("^(r(andom)?)$");
	static const std::regex     vanillaRegExp("^(v(anilla)?)$");
	static const std::regex      filterRegExp("^(f(ilter)?)([ ]+.*)?");
	static const std::regex        sortRegExp("^(o|sort)([ ]+.*)?");
	static const std::regex          ipRegExp("^(ip)([ ]+.*)?");
	static const std::regex        portRegExp("^(p(ort)?)([ ]+.*)?");
	static const std::regex    gameTypeRegExp("^(t(ype)?)([ ]+.*)?");
	static const std::regex gameVersionRegExp("^(g(ame)?)([ ]+.*)?");
	static const std::regex        exitRegExp("^(x|exit|q(uit)?)$");

	size_t maxLength = 0;

	fmt::print("\n");

	pause();
	clearOutput();

	while(true) {
		clearOutput();

		if(organizedMods->shouldDisplayMods()) {
			maxLength = Utilities::unsignedLongLength(organizedMods->numberOfMods());

			for(size_t i = 0; i < organizedMods->numberOfMods(); i++) {
				std::stringstream modText;
				modText << std::string(maxLength - Utilities::unsignedLongLength(i + 1), ' ');
				modText << fmt::format("{}: {}", i + 1, organizedMods->getMod(i)->getName());

				if(organizedMods->getSortType() == OrganizedModCollection::SortType::InitialReleaseDate || organizedMods->getSortType() == OrganizedModCollection::SortType::LatestReleaseDate) {
					std::string releaseDate(organizedMods->getSortType() == OrganizedModCollection::SortType::InitialReleaseDate ? organizedMods->getMod(i)->getInitialReleaseDateAsString() : organizedMods->getMod(i)->getLatestReleaseDateAsString());

					if(!releaseDate.empty()) {
						modText << fmt::format(" ({})", releaseDate);
					}
				}
				else if(organizedMods->getSortType() == OrganizedModCollection::SortType::NumberOfVersions) {
					modText << fmt::format(" ({})", organizedMods->getMod(i)->numberOfVersions());
				}

				fmt::print("{}\n", modText.str());
			}
		}
		else if(organizedMods->shouldDisplayGameVersions()) {
			maxLength = Utilities::unsignedLongLength(organizedMods->numberOfGameVersions());

			for(size_t i = 0; i < organizedMods->numberOfGameVersions(); i++) {
				std::stringstream gameVersionText;
				gameVersionText << std::string(maxLength - Utilities::unsignedLongLength(i + 1), ' ');
				gameVersionText << fmt::format("{}: {}", i + 1, organizedMods->getGameVersion(i)->getName());

				if(organizedMods->getSortType() == OrganizedModCollection::SortType::NumberOfSupportedMods ||
				   (organizedMods->getFilterType() == OrganizedModCollection::FilterType::SupportedGameVersions && organizedMods->getSortType() != OrganizedModCollection::SortType::NumberOfCompatibleMods)) {
					gameVersionText << fmt::format(" ({})", organizedMods->getSupportedModCountForGameVersion(organizedMods->getGameVersion(i)->getName()));
				}
				else if(organizedMods->getSortType() == OrganizedModCollection::SortType::NumberOfCompatibleMods || organizedMods->getFilterType() == OrganizedModCollection::FilterType::CompatibleGameVersions) {
					gameVersionText << fmt::format(" ({})", organizedMods->getCompatibleModCountForGameVersion(organizedMods->getGameVersion(i)->getName()));
				}

				fmt::print("{}\n", gameVersionText.str());
			}
		}
		else if(organizedMods->shouldDisplayTeams()) {
			maxLength = Utilities::unsignedLongLength(organizedMods->numberOfTeams());

			for(size_t i = 0; i < organizedMods->numberOfTeams(); i++) {
				std::stringstream teamText;
				teamText << std::string(maxLength - Utilities::unsignedLongLength(i + 1), ' ');
				teamText << fmt::format("{}: {} ({})", i + 1, organizedMods->getTeamInfo(i)->getName(), organizedMods->getTeamInfo(i)->getModCount());

				fmt::print("{}\n", teamText.str());
			}
		}
		else if(organizedMods->shouldDisplayAuthors()) {
			maxLength = Utilities::unsignedLongLength(organizedMods->numberOfAuthors());

			for(size_t i = 0; i < organizedMods->numberOfAuthors(); i++) {
				std::stringstream authorText;
				authorText << std::string(maxLength - Utilities::unsignedLongLength(i + 1), ' ');
				authorText << fmt::format("{}: {} ({})", i + 1, organizedMods->getAuthorInfo(i)->getName(), organizedMods->getAuthorInfo(i)->getModCount());

				fmt::print("{}\n", authorText.str());
			}
		}

		fmt::print("> ");

		std::getline(std::cin, input);
		data = Utilities::trimString(input);
		formattedData = Utilities::toLowerCase(data);

		if(std::regex_match(formattedData, sortRegExp)) {
			runSortPrompt(getArguments(data));
			continue;
		}
		else if(std::regex_match(formattedData, searchRegExp)) {
			runSearchPrompt(getArguments(data));

			if(m_modManager->getSelectedMod() != nullptr) {
				fmt::print("\n");

				if(m_modManager->runSelectedMod()) {
					fmt::print("\n");

#if _DEBUG
					pause();
					fmt::print("\n");
#endif // _DEBUG

					break;
				}
				else {
					fmt::print("\n");
					pause();
				}
			}

			continue;
		}
		else if(std::regex_match(formattedData, randomRegExp)) {
			runSelectRandomModPrompt(false, false);

			if(m_modManager->getSelectedMod() != nullptr) {
				fmt::print("\n");

				if(m_modManager->runSelectedMod()) {
					fmt::print("\n");

#if _DEBUG
					pause();
					fmt::print("\n");
#endif // _DEBUG

					break;
				}
				else {
					fmt::print("\n");
					pause();
				}
			}

			continue;
		}
		else if(std::regex_match(formattedData, vanillaRegExp)) {
			fmt::print("\n");

			m_modManager->clearSelectedMod();
			m_modManager->runSelectedMod();

			fmt::print("\n");

#if _DEBUG
			pause();
			fmt::print("\n");
#endif // _DEBUG

			break;
		}
		else if(std::regex_match(formattedData, filterRegExp)) {
			updateFilterTypePrompt(getArguments(data));
			continue;
		}
		else if(std::regex_match(formattedData, ipRegExp)) {
			updateIPAddressPrompt(getArguments(data));
			continue;
		}
		else if(std::regex_match(formattedData, portRegExp)) {
			updatePortPrompt(getArguments(data));
			continue;
		}
		else if(std::regex_match(formattedData, gameTypeRegExp)) {
			updateGameTypePrompt(getArguments(data));
			continue;
		}
		else if(std::regex_match(formattedData, gameVersionRegExp)) {
			updatePreferredGameVersionPrompt(getArguments(data));
			continue;
		}
		else if(std::regex_match(formattedData, backRegExp)) {
			clearOutput();

			if(organizedMods->getFilterType() == OrganizedModCollection::FilterType::Teams) {
				if(organizedMods->hasSelectedTeam()) {
					organizedMods->clearSelectedTeam();

					fmt::print("Returning to team list.\n\n");
				}
				else {
					organizedMods->setFilterType(OrganizedModCollection::FilterType::None);

					fmt::print("Returning to main mod list.\n\n");
				}
			}
			else if(organizedMods->getFilterType() == OrganizedModCollection::FilterType::Authors) {
				if(organizedMods->hasSelectedAuthor()) {
					organizedMods->clearSelectedAuthor();

					fmt::print("Returning to author list.\n\n");
				}
				else {
					organizedMods->setFilterType(OrganizedModCollection::FilterType::None);

					fmt::print("Returning to main mod list.\n\n");
				}
			}
			else if(organizedMods->getFilterType() == OrganizedModCollection::FilterType::Favourites) {
				organizedMods->setFilterType(OrganizedModCollection::FilterType::None);

				fmt::print("Returning to main mod list.\n\n");
			}
			else if(organizedMods->getFilterType() == OrganizedModCollection::FilterType::SupportedGameVersions ||
					organizedMods->getFilterType() == OrganizedModCollection::FilterType::CompatibleGameVersions) {
				if(organizedMods->hasSelectedGameVersion()) {
					organizedMods->clearSelectedGameVersion();

					fmt::print("Returning to game version list.\n\n");
				}
				else {
					organizedMods->setFilterType(OrganizedModCollection::FilterType::None);

					fmt::print("Returning to main mod list.\n\n");
				}
			}
			else {
				fmt::print("Cannot go back from main mod list.\n\n");
			}

			pause();

			continue;
		}
		else if(std::regex_match(formattedData, helpRegExp)) {
			clearOutput();

			fmt::print("Command information:\n");
			fmt::print(" [s]earch <args> - searches for the specified mod, or runs a prompt.\n");
			fmt::print(" [r]andom -------- randomly select a mod from the list and run it.\n");
			fmt::print("[v]anilla -------- run game without any mods.\n");
			fmt::print(" [f]ilter <args> - changes the mod list filter.\n");
			fmt::print("   s[o]rt <args> - changes the mod list sorting options.\n");
			fmt::print("       ip <args> - changes the ip address of the DOSBox server.\n");
			fmt::print("   [p]ort <args> - changes the port of the DOSBox server.\n");
			fmt::print("   [t]ype <args> - changes the game type (game / setup / client / server).\n");
			fmt::print("   [g]ame <args> - changes the preferred game version.\n");
			fmt::print("   [b]ack -------- returns to the previous menu (if applicable).\n");
			fmt::print("        ? -------- displays this help message.\n");
			fmt::print("   [q]uit -------- closes the program.\n");
			fmt::print("   e[x]it -------- closes the program.\n");
			fmt::print("\n");

			pause();

			continue;
		}
		else if(std::regex_match(formattedData, exitRegExp)) {
			clearOutput();

			fmt::print("\n");

			return;
		}

		selectedIndex = Utilities::parseUnsignedInteger(data, &error);

		if(!error) {
			if(organizedMods->shouldDisplayMods()) {
				if(selectedIndex >= 1 && selectedIndex <= organizedMods->numberOfMods()) {
					m_modManager->setSelectedMod(organizedMods->getMod(selectedIndex - 1));

					fmt::print("\n");

					if(m_modManager->runSelectedMod()) {
						fmt::print("\n");

#if _DEBUG
						pause();
						fmt::print("\n");
#endif // _DEBUG

						break;
					}
					else {
						fmt::print("\n");
						pause();
					}
				}
				else {
					error = true;
				}
			}
			else if(organizedMods->shouldDisplayGameVersions()) {
				if(selectedIndex >= 1 && selectedIndex <= organizedMods->numberOfGameVersions()) {
					organizedMods->setSelectedGameVersion(selectedIndex - 1);
				}
				else {
					error = true;
				}
			}
			else if(organizedMods->shouldDisplayTeams()) {
				if(selectedIndex >= 1 && selectedIndex <= organizedMods->numberOfTeams()) {
					organizedMods->setSelectedTeam(selectedIndex - 1);
				}
				else {
					error = true;
				}
			}
			else if(organizedMods->shouldDisplayAuthors()) {
				if(selectedIndex >= 1 && selectedIndex <= organizedMods->numberOfAuthors()) {
					organizedMods->setSelectedAuthor(selectedIndex - 1);
				}
				else {
					error = true;
				}
			}
		}

		if(error) {
			clearOutput();
			fmt::print("Invalid selection!\n\n");
			pause();
		}
	}
}

bool ModManager::CLI::updateFilterTypePrompt(const std::string & args) {
	if(m_modManager == nullptr || !m_modManager->isInitialized()) {
		return false;
	}

	std::shared_ptr<OrganizedModCollection> organizedMods = m_modManager->getOrganizedMods();
	std::optional<OrganizedModCollection::FilterType> optionalFilterType(promptForFilterType(args));

	if(!optionalFilterType.has_value() || organizedMods->getFilterType() == optionalFilterType.value()) {
		return false;
	}

	fmt::print("Changing filter type from '{}' to '{}'.\n\n", Utilities::toCapitalCase(std::string(magic_enum::enum_name(organizedMods->getFilterType()))), Utilities::toCapitalCase(std::string(magic_enum::enum_name(optionalFilterType.value()))));

	organizedMods->setFilterType(optionalFilterType.value());

	pause();

	return true;
}

std::optional<OrganizedModCollection::FilterType> ModManager::CLI::promptForFilterType(const std::string & args) const {
	if(m_modManager == nullptr || !m_modManager->isInitialized()) {
		return {};
	}

	std::shared_ptr<OrganizedModCollection> organizedMods = m_modManager->getOrganizedMods();
	std::vector<std::string> filterTypeNames;
	const auto & filterTypes = magic_enum::enum_values<OrganizedModCollection::FilterType>();

	for(size_t i = 0; i < filterTypes.size(); i++) {
		filterTypeNames.push_back(Utilities::toCapitalCase(std::string(magic_enum::enum_name(filterTypes[i]))));
	}

	std::stringstream filterTypePromptMessage;
	filterTypePromptMessage << fmt::format("Current filter type is set to: '{}' (default: '{}')\n\n", Utilities::toCapitalCase(std::string(magic_enum::enum_name(organizedMods->getFilterType()))), Utilities::toCapitalCase(std::string(magic_enum::enum_name(OrganizedModCollection::DEFAULT_FILTER_TYPE))));
	filterTypePromptMessage << "Choose a filter type:";

	size_t filterTypeIndex = getValuePrompt(filterTypeNames, filterTypePromptMessage.str(), args);

	if(filterTypeIndex == std::numeric_limits<size_t>::max()) {
		return {};
	}

	return filterTypes[filterTypeIndex];
}

bool ModManager::CLI::runSortPrompt(const std::string & args) {
	if(m_modManager == nullptr || !m_modManager->isInitialized()) {
		return false;
	}

	std::shared_ptr<OrganizedModCollection> organizedMods = m_modManager->getOrganizedMods();

	bool error = false;
	bool inputSortDirection = false;
	std::string input, data, formattedData;
	uint32_t value = std::numeric_limits<uint32_t>::max();
	uint32_t cancel = std::numeric_limits<uint32_t>::max();
	std::string trimmedArgs(Utilities::trimString(args));
	bool skipInput = !trimmedArgs.empty();
	std::optional<OrganizedModCollection::SortType> sortTypeOptional;
	std::optional<OrganizedModCollection::SortDirection> sortDirectionOptional;

	static const std::regex whitespaceRegExp("[ \t]+");

	std::vector<OrganizedModCollection::SortType> validSortTypes;
	const auto & sortTypes = magic_enum::enum_values<OrganizedModCollection::SortType>();

	for(size_t i = 0; i < sortTypes.size(); i++) {
		if(organizedMods->areSortOptionsValidInCurrentContext(sortTypes[i], organizedMods->getFilterType())) {
			validSortTypes.push_back(sortTypes[i]);
		}
	}

	while(true) {
		clearOutput();

		if(!skipInput) {
			if(!sortTypeOptional.has_value()) {
				std::stringstream sortTypePromptText;
				sortTypePromptText << fmt::format("Current sort type is: {}", Utilities::toCapitalCase(std::string(magic_enum::enum_name(organizedMods->getSortType()))));

				if(organizedMods->getSortType() != OrganizedModCollection::SortType::Unsorted && organizedMods->getSortType() != OrganizedModCollection::SortType::Random) {
					sortTypePromptText << fmt::format(" ({})", Utilities::toCapitalCase(std::string(magic_enum::enum_name(organizedMods->getSortDirection()))));
				}

				sortTypePromptText << fmt::format(" (default: {}", Utilities::toCapitalCase(std::string(magic_enum::enum_name(OrganizedModCollection::DEFAULT_SORT_TYPE))));

				if(OrganizedModCollection::DEFAULT_SORT_TYPE != OrganizedModCollection::SortType::Unsorted) {
					sortTypePromptText << fmt::format(" ({})", Utilities::toCapitalCase(std::string(magic_enum::enum_name(OrganizedModCollection::DEFAULT_SORT_DIRECTION))));
				}

				sortTypePromptText << fmt::format(")");

				fmt::print("{}\n\n", sortTypePromptText.str());
				fmt::print("Choose a sort type:\n");

				for(size_t i = 0; i < validSortTypes.size(); i++) {
					fmt::print("{}: {}\n", i + 1, Utilities::toCapitalCase(std::string(magic_enum::enum_name(validSortTypes[i]))));
				}

				cancel = validSortTypes.size() + 1;
			}
			else {
				fmt::print("Current sort type will be changed from '{}' to '{}'.\n\n", Utilities::toCapitalCase(std::string(magic_enum::enum_name(organizedMods->getSortType()))), Utilities::toCapitalCase(std::string(magic_enum::enum_name(sortTypeOptional.value()))));

				if(sortTypeOptional.value() != OrganizedModCollection::SortType::Random) {
					fmt::print("Current sort direction is: '{}' (default: '{}').\n\n", Utilities::toCapitalCase(std::string(magic_enum::enum_name(organizedMods->getSortDirection()))), Utilities::toCapitalCase(std::string(magic_enum::enum_name(OrganizedModCollection::DEFAULT_SORT_DIRECTION))));
					fmt::print("Choose a sort direction:\n");

					const auto& sortDirections = magic_enum::enum_values<OrganizedModCollection::SortDirection>();

					for(size_t i = 0; i < sortDirections.size(); i++) {
						fmt::print("{}: {}\n", i + 1, Utilities::toCapitalCase(std::string(magic_enum::enum_name(sortDirections[i]))));
					}

					cancel = sortDirections.size() + 1;

					inputSortDirection = true;
				}
			}

			fmt::print("{}: Cancel\n", cancel);
			fmt::print("> ");

			std::getline(std::cin, input);
			data = Utilities::trimString(input);

			if(data.empty()) {
				return false;
			}

			formattedData = Utilities::toLowerCase(data);

			fmt::print("\n");

			if(std::regex_match(formattedData, helpRegExp)) {
				clearOutput();

				fmt::print("<enter> - returns to previous menu.\n");
				fmt::print("      ? - displays this help message.\n");
				fmt::print("\n");

				pause();

				continue;
			}
		}

		if(skipInput) {
			data.clear();

			std::vector<std::string> parts = Utilities::regularExpressionStringSplit(trimmedArgs, whitespaceRegExp);

			sortDirectionOptional = magic_enum::enum_cast<OrganizedModCollection::SortDirection>(Utilities::toPascalCase(parts[parts.size() - 1]));

			if(parts.size() > 1) {
				if(!sortDirectionOptional.has_value()) {
					for(size_t i = 0; i < parts.size() - 1; i++) {
						data.append(parts[i]);

						if(i < parts.size() - 1) {
							data.append(" ");
						}
					}
				}
				else {
					sortDirectionOptional = magic_enum::enum_cast<OrganizedModCollection::SortDirection>(Utilities::toPascalCase(parts[0]));

					if(sortDirectionOptional.has_value()) {
						for(size_t i = 1; i < parts.size(); i++) {
							data.append(parts[i]);

							if(i < parts.size() - 1) {
								data.append(" ");
							}
						}
					}
					else {
						data = trimmedArgs;
					}
				}
			}
			else {
				if(!sortDirectionOptional.has_value()) {
					data = trimmedArgs;
				}
			}

			if(!data.empty()) {
				sortTypeOptional = magic_enum::enum_cast<OrganizedModCollection::SortType>(Utilities::toPascalCase(data));

				if(!sortTypeOptional.has_value()) {
					sortDirectionOptional.reset();
				}
			}
		}

		if(!skipInput) {
			if(!sortTypeOptional.has_value()) {
				if(!data.empty()) {
					sortTypeOptional = magic_enum::enum_cast<OrganizedModCollection::SortType>(Utilities::toPascalCase(data));

					if(!sortTypeOptional.has_value()) {
						if(Utilities::isValidWholeNumber(data)) {
							value = Utilities::parseUnsignedInteger(data, &error);

							if(!error) {
								if(value == cancel) {
									break;
								}
								else if(value >= 1 && value <= validSortTypes.size()) {
									sortTypeOptional = validSortTypes[value - 1];
								}
							}
						}
					}
				}
			}
			else {
				if(!data.empty()) {
					sortDirectionOptional = magic_enum::enum_cast<OrganizedModCollection::SortDirection>(Utilities::toPascalCase(data));

					if(!sortDirectionOptional.has_value()) {
						if(Utilities::isValidWholeNumber(data)) {
							value = Utilities::parseUnsignedInteger(data, &error);

							if(!error) {
								if(value == cancel) {
									break;
								}
								else if(value >= 1 && value <= magic_enum::enum_count<OrganizedModCollection::SortDirection>()) {
									sortDirectionOptional = magic_enum::enum_value<OrganizedModCollection::SortDirection>(value - 1);
								}
							}
						}
					}
				}
			}
		}

		if(skipInput) {
			if(sortTypeOptional.has_value() || sortDirectionOptional.has_value()) {
				if(!sortTypeOptional.has_value()) {
					sortTypeOptional = organizedMods->getSortType();
				}

				if(!sortDirectionOptional.has_value()) {
					sortDirectionOptional = organizedMods->getSortDirection();
				}
			}
			else {
				sortTypeOptional.reset();
				sortDirectionOptional.reset();

				fmt::print("Invalid sort type argument: '{}'.\n\n", trimmedArgs);
			}
		}

		if(sortTypeOptional.has_value() && (sortDirectionOptional.has_value() || (sortTypeOptional.value() == OrganizedModCollection::SortType::Unsorted || sortTypeOptional.value() == OrganizedModCollection::SortType::Random))) {
			if(sortTypeOptional.value() == OrganizedModCollection::SortType::Unsorted || sortTypeOptional.value() == OrganizedModCollection::SortType::Random) {
				sortDirectionOptional = OrganizedModCollection::DEFAULT_SORT_DIRECTION;
			}

			if(organizedMods->areSortOptionsValidInCurrentContext(sortTypeOptional.value(), organizedMods->getFilterType())) {
				if(organizedMods->getSortType() != sortTypeOptional.value() || organizedMods->getSortDirection() != sortDirectionOptional.value()) {
					std::stringstream sortTypeChangedText;
					sortTypeChangedText << fmt::format("Changing sort type from {}", Utilities::toCapitalCase(std::string(magic_enum::enum_name(organizedMods->getSortType()))));

					if(organizedMods->getSortType() != OrganizedModCollection::SortType::Unsorted && organizedMods->getSortType() != OrganizedModCollection::SortType::Random) {
						sortTypeChangedText << fmt::format(" ({})", Utilities::toCapitalCase(std::string(magic_enum::enum_name(organizedMods->getSortDirection()))));
					}

					sortTypeChangedText << fmt::format(" to {}", Utilities::toCapitalCase(std::string(magic_enum::enum_name(sortTypeOptional.value()))));

					if(sortTypeOptional.value() != OrganizedModCollection::SortType::Unsorted && sortTypeOptional.value() != OrganizedModCollection::SortType::Random) {
						sortTypeChangedText << fmt::format(" ({})", Utilities::toCapitalCase(std::string(magic_enum::enum_name(sortDirectionOptional.value()))));
					}

					sortTypeChangedText << fmt::format(".");

					fmt::print("{}\n\n", sortTypeChangedText.str());

					organizedMods->setSortOptions(sortTypeOptional.value(), sortDirectionOptional.value());

					pause();
				}

				break;
			}
			else {
				std::stringstream invalidSortTypeText;
				invalidSortTypeText << fmt::format("Sort type {}", Utilities::toCapitalCase(std::string(magic_enum::enum_name(sortTypeOptional.value()))));

				if(sortTypeOptional.value() != OrganizedModCollection::SortType::Unsorted && sortTypeOptional.value() != OrganizedModCollection::SortType::Random) {
					invalidSortTypeText << fmt::format(" ({})", Utilities::toCapitalCase(std::string(magic_enum::enum_name(sortDirectionOptional.value()))));
				}

				invalidSortTypeText << fmt::format(" is not valid with filter type: '{}'.", Utilities::toCapitalCase(std::string(magic_enum::enum_name(organizedMods->getFilterType()))));

				fmt::print("{}\n\n", invalidSortTypeText.str());

				sortTypeOptional.reset();
				sortDirectionOptional.reset();

				pause();
			}
		}
		else if(!sortTypeOptional.has_value() && !skipInput) {
			fmt::print("Invalid sort type: '{}'.\n\n", data);

			pause();
		}
		else if(!sortDirectionOptional.has_value() && inputSortDirection && !skipInput) {
			fmt::print("Invalid sort direction: '{}'.\n\n", data);

			pause();
		}

		if(skipInput) {
			skipInput = false;
		}
	}

	return false;
}

bool ModManager::CLI::updateGameTypePrompt(const std::string & args) {
	if(m_modManager == nullptr || !m_modManager->isInitialized()) {
		return false;
	}

	std::optional<GameType> optionalGameType(promptForGameType(args));

	if(!optionalGameType.has_value() || m_modManager->getSettings()->gameType == optionalGameType.value()) {
		return false;
	}

	fmt::print("Changing game type from '{}' to '{}'.\n\n", Utilities::toCapitalCase(std::string(magic_enum::enum_name(m_modManager->getSettings()->gameType))), Utilities::toCapitalCase(std::string(magic_enum::enum_name(optionalGameType.value()))));

	m_modManager->setGameType(optionalGameType.value());

	pause();

	return true;
}

std::optional<GameType> ModManager::CLI::promptForGameType(const std::string & args) const {
	if(m_modManager == nullptr || !m_modManager->isInitialized()) {
		return {};
	}

	std::vector<std::string> gameTypeNames;
	const auto & gameTypes = magic_enum::enum_values<GameType>();

	for(size_t i = 0; i < gameTypes.size(); i++) {
		gameTypeNames.push_back(Utilities::toCapitalCase(std::string(magic_enum::enum_name(gameTypes[i]))));
	}

	std::string promptMessage(fmt::format("Current game type is set to: '{}' (default: '{}')\n\nChoose a game type:", Utilities::toCapitalCase(std::string(magic_enum::enum_name(m_modManager->getSettings()->gameType))), Utilities::toCapitalCase(std::string(magic_enum::enum_name(ModManager::DEFAULT_GAME_TYPE)))));

	size_t gameTypeIndex = getValuePrompt(gameTypeNames, promptMessage, args);

	if(gameTypeIndex == std::numeric_limits<size_t>::max()) {
		return {};
	}

	return gameTypes[gameTypeIndex];
}

bool ModManager::CLI::updatePreferredGameVersionPrompt(const std::string & args) {
	if(m_modManager == nullptr || !m_modManager->isInitialized()) {
		return false;
	}

	std::shared_ptr<GameVersion> gameVersion(promptForPreferredGameVersion(args));

	if(gameVersion == nullptr || (m_modManager->hasPreferredGameVersion() && *m_modManager->getPreferredGameVersion() == *gameVersion)) {
		return false;
	}

	fmt::print("Changing preferred game version from '{}' to '{}'.\n\n", m_modManager->hasPreferredGameVersion() ? m_modManager->getPreferredGameVersion()->getName() : "None", gameVersion->getName());

	m_modManager->setPreferredGameVersion(gameVersion);

	pause();

	return true;
}

std::shared_ptr<GameVersion> ModManager::CLI::promptForPreferredGameVersion(const std::string & args) const {
	if(m_modManager == nullptr || !m_modManager->isInitialized()) {
		return nullptr;
	}

	std::shared_ptr<GameVersion> gameVersion;
	std::shared_ptr<GameVersionCollection> gameVersions(m_modManager->getGameVersions());
	std::vector<std::string> gameVersionNames;

	for(size_t i = 0; i < gameVersions->numberOfGameVersions(); i++) {
		gameVersionNames.push_back(gameVersions->getGameVersion(i)->getName());
	}

	std::stringstream preferredGameVersionPromptMessage;
	preferredGameVersionPromptMessage << fmt::format("Current preferred game version is set to: '{}' (default: '{}')\n\n", m_modManager->getSettings()->preferredGameVersion, SettingsManager::DEFAULT_PREFERRED_GAME_VERSION);
	preferredGameVersionPromptMessage << "Choose a preferred game version:";

	size_t gameVersionIndex = getValuePrompt(gameVersionNames, preferredGameVersionPromptMessage.str(), args);

	if(gameVersionIndex == std::numeric_limits<size_t>::max()) {
		return {};
	}

	return gameVersions->getGameVersion(gameVersionIndex);
}

std::optional<std::pair<std::shared_ptr<GameVersion>, std::shared_ptr<ModGameVersion>>> ModManager::CLI::runUnsupportedModVersionTypePrompt(std::shared_ptr<ModVersionType> modVersionType, const std::string & promptMessage) const {
	if(m_modManager == nullptr || !m_modManager->isInitialized() || !ModVersionType::isValid(modVersionType.get())) {
		return {};
	}

	std::shared_ptr<GameVersion> selectedGameVersion;
	std::shared_ptr<ModGameVersion> selectedModGameVersion;
	std::vector<std::shared_ptr<ModGameVersion>> * modGameVersions = nullptr;
	std::vector<std::pair<std::shared_ptr<GameVersion>, std::vector<std::shared_ptr<ModGameVersion>>>> compatibleGameVersions(m_modManager->getGameVersions()->getGameVersionsCompatibleWith(modVersionType->getGameVersions(), true));

	if(compatibleGameVersions.empty()) {
		return {};
	}
	else if(compatibleGameVersions.size() == 1) {
		selectedGameVersion = compatibleGameVersions[0].first;
		modGameVersions = &compatibleGameVersions[0].second;
	}
	else {
		std::vector<std::string> compatibleGameVersionNames;

		for(std::vector<std::pair<std::shared_ptr<GameVersion>, std::vector<std::shared_ptr<ModGameVersion>>>>::const_iterator i = compatibleGameVersions.begin(); i != compatibleGameVersions.end(); ++i) {
			compatibleGameVersionNames.push_back((*i).first->getName());
		}

		std::stringstream fullPromptMessage;

		if(!promptMessage.empty()) {
			fullPromptMessage << promptMessage + "\n\n";
		}

		fullPromptMessage << "Choose a compatible game type to run:";

		size_t selectedCompatibleGameVersionIndex = getValuePrompt(compatibleGameVersionNames, fullPromptMessage.str());

		if(selectedCompatibleGameVersionIndex == std::numeric_limits<size_t>::max()) {
			return {};
		}

		selectedGameVersion = compatibleGameVersions[selectedCompatibleGameVersionIndex].first;
		modGameVersions = &compatibleGameVersions[selectedCompatibleGameVersionIndex].second;
	}

	if(modGameVersions->empty()) {
		return {};
	}
	else if(modGameVersions->size() == 1) {
		selectedModGameVersion = (*modGameVersions)[0];
	}
	else {
		std::vector<std::string> modGameVersionNames;

		for(std::vector<std::shared_ptr<ModGameVersion>>::const_iterator i = modGameVersions->begin(); i != modGameVersions->end(); ++i) {
			modGameVersionNames.push_back((*i)->getFullName());
		}

		size_t selectedModGameVersionIndex = getValuePrompt(modGameVersionNames, "Choose a mod game version to run:");

		if(selectedModGameVersionIndex == std::numeric_limits<size_t>::max()) {
			return {};
		}

		selectedModGameVersion = (*modGameVersions)[selectedModGameVersionIndex];
	}

	return std::make_pair(selectedGameVersion, selectedModGameVersion);
}

bool ModManager::CLI::updateIPAddressPrompt(const std::string & args) {
	if(m_modManager == nullptr || !m_modManager->isInitialized()) {
		return false;
	}

	std::string ipAddress(promptForIPAddress(args));

	if(ipAddress.empty()) {
		return false;
	}

	m_modManager->getSettings()->dosboxServerIPAddress = ipAddress;

	fmt::print("Server IP address changed to: '{}'.\n\n", ipAddress);

	pause();

	return true;
}

std::string ModManager::CLI::promptForIPAddress(const std::string & args) const {
	if(m_modManager == nullptr || !m_modManager->isInitialized()) {
		return "";
	}

	std::string input, data, formattedData;
	std::string trimmedArgs(Utilities::trimString(args));
	bool skipInput = !trimmedArgs.empty();

	while(true) {
		clearOutput();

		if(skipInput) {
			data = trimmedArgs;
			skipInput = false;
		}
		else {
			fmt::print("Current server IP Address: '{}'.\n\n", m_modManager->getSettings()->dosboxServerIPAddress);
			fmt::print("Enter new server IP Address:\n");
			fmt::print("> ");

			std::getline(std::cin, input);
			data = Utilities::trimString(input);

			if(data.empty()) {
				return "";
			}

			formattedData = Utilities::toLowerCase(data);

			fmt::print("\n");
		}

		if(std::regex_match(data, helpRegExp)) {
			clearOutput();

			fmt::print("<enter> - returns to previous menu.\n");
			fmt::print("      ? - displays this help message.\n");
			fmt::print("\n");

			pause();

			continue;
		}

		if(!data.empty() && Utilities::isIPV4Address(data)) {
			return data;
		}

		fmt::print("Invalid server IP Address: '{}'.\n\n", data);

		pause();
	}

	return "";
}

bool ModManager::CLI::updatePortPrompt(const std::string & args) {
	if(m_modManager == nullptr || !m_modManager->isInitialized()) {
		return false;
	}

	std::optional<uint16_t> optionalServerPort(promptForPort(args));

	if(!optionalServerPort.has_value()) {
		return false;
	}

	if(m_modManager->getGameType() == GameType::Server) {
		m_modManager->getSettings()->dosboxLocalServerPort = optionalServerPort.value();
	}
	else {
		m_modManager->getSettings()->dosboxRemoteServerPort = optionalServerPort.value();
	}

	fmt::print("{} server port changed to: {}\n\n", m_modManager->getGameType() == GameType::Server ? "local" : "remote", optionalServerPort.value());

	pause();

	return true;
}

std::optional<uint16_t> ModManager::CLI::promptForPort(const std::string & args) const {
	if(m_modManager == nullptr || !m_modManager->isInitialized()) {
		return {};
	}

	bool error = false;
	std::string input, data, formattedData;
	std::string trimmedArgs(Utilities::trimString(args));
	bool skipInput = !trimmedArgs.empty();
	uint16_t currentPort = 0;
	std::string portType(m_modManager->getGameType() == GameType::Server ? "local" : "remote");

	while(true) {
		clearOutput();

		currentPort = m_modManager->getGameType() == GameType::Server ? m_modManager->getSettings()->dosboxLocalServerPort : m_modManager->getSettings()->dosboxRemoteServerPort;

		if(skipInput) {
			data = trimmedArgs;
			skipInput = false;
		}
		else {
			fmt::print("Current {} port: '{}'.\n\n", portType, currentPort);
			fmt::print("Enter {} server port:\n", portType);
			fmt::print("> ");

			std::getline(std::cin, input);
			data = Utilities::trimString(input);

			if(data.empty()) {
				return {};
			}

			formattedData = Utilities::toLowerCase(data);

			fmt::print("\n");
		}

		if(std::regex_match(data, helpRegExp)) {
			clearOutput();

			fmt::print("<enter> - displays this help message.\n");
			fmt::print("      ? - displays this help message.\n");
			fmt::print("\n");

			pause();

			continue;
		}

		error = false;

		if(data.empty() || data.find_first_of(" \t") != std::string::npos) {
			error = true;
		}

		if(!error) {
			uint16_t port = static_cast<uint16_t>(Utilities::parseUnsignedInteger(data, &error));

			if(port == 0) {
				error = true;
			}
			else {
				return port;
			}
		}

		if(error) {
			fmt::print("Invalid {} server port: {}\n\n", m_modManager->getGameType() == GameType::Server ? "local" : "remote", data);

			pause();
		}
	}

	return {};
}

bool ModManager::CLI::runSelectRandomModPrompt(bool selectPreferredVersion, bool selectFirstVersionType) {
	if(m_modManager == nullptr || !m_modManager->isInitialized()) {
		return false;
	}

	std::shared_ptr<OrganizedModCollection> organizedMods = m_modManager->getOrganizedMods();

	std::string input, data, formattedData;
	bool skipRandomSelection = false;
	bool randomMods = organizedMods->shouldDisplayMods();
	bool randomGameVersions = organizedMods->shouldDisplayGameVersions();
	bool randomTeams = organizedMods->shouldDisplayTeams();
	bool randomAuthors = organizedMods->shouldDisplayAuthors();
	const char * searchType = randomMods ? "mod" : (randomTeams ? "team" : (randomAuthors ? "author" : "item"));
	int selectedIndex = std::numeric_limits<size_t>::max();

	static const std::regex randomRegExp("^(r(andom)?)$");

	while(true) {
		clearOutput();

		std::string selectedItemName;

		if(!skipRandomSelection) {
			if(organizedMods->shouldDisplayMods()) {
				m_modManager->selectRandomMod(selectPreferredVersion, selectFirstVersionType);
				std::shared_ptr<Mod> selectedMod(m_modManager->getSelectedMod());

				if(selectedMod != nullptr) {
					selectedItemName = selectedMod->getName();
				}
			}
			else if(organizedMods->shouldDisplayGameVersions()) {
				selectedIndex = organizedMods->numberOfGameVersions() == 0 ? std::numeric_limits<size_t>::max() : Utilities::randomInteger(0, organizedMods->numberOfGameVersions() - 1);
				std::shared_ptr<GameVersion> selectedGameVersion(organizedMods->getGameVersion(selectedIndex));

				if(selectedGameVersion != nullptr) {
					selectedItemName = selectedGameVersion->getName();
				}
			}
			else if(organizedMods->shouldDisplayTeams()) {
				selectedIndex = organizedMods->numberOfTeams() == 0 ? std::numeric_limits<size_t>::max() : Utilities::randomInteger(0, organizedMods->numberOfTeams() - 1);
				std::shared_ptr<ModAuthorInformation> selectedTeamInfo(organizedMods->getTeamInfo(selectedIndex));

				if(selectedTeamInfo != nullptr) {
					selectedItemName = selectedTeamInfo->getName();
				}
			}
			else if(organizedMods->shouldDisplayAuthors()) {
				selectedIndex = organizedMods->numberOfAuthors() == 0 ? std::numeric_limits<size_t>::max() : Utilities::randomInteger(0, organizedMods->numberOfAuthors() - 1);
				std::shared_ptr<ModAuthorInformation> selectedAuthorInfo(organizedMods->getAuthorInfo(selectedIndex));

				if(selectedAuthorInfo != nullptr) {
					selectedItemName = selectedAuthorInfo->getName();
				}
			}
		}
		else {
			skipRandomSelection = false;
		}

		fmt::print("Randomly selected {}: '{}'.\n\n", searchType, selectedItemName);
		fmt::print("Press enter to continue, or type r to select a new {}, or b to go back.\n\n", searchType);
		fmt::print("> ");

		std::getline(std::cin, input);
		data = Utilities::trimString(input);

		if(data.empty()) {
			if(randomGameVersions) {
				organizedMods->setSelectedGameVersion(selectedIndex);
			}
			else if(randomTeams) {
				organizedMods->setSelectedTeam(selectedIndex);
			}
			else if(randomAuthors) {
				organizedMods->setSelectedAuthor(selectedIndex);
			}

			return true;
		}

		formattedData = Utilities::toLowerCase(data);

		if(std::regex_match(formattedData, randomRegExp)) {
			continue;
		}
		else if(std::regex_match(formattedData, backRegExp)) {
			if(randomMods) {
				m_modManager->clearSelectedMod();
			}
			else if(randomGameVersions) {
				organizedMods->clearSelectedGameVersion();
			}
			else if(randomTeams) {
				organizedMods->clearSelectedTeam();
			}
			else if(randomAuthors) {
				organizedMods->clearSelectedAuthor();
			}

			break;
		}
		else {
			skipRandomSelection = true;

			clearOutput();

			fmt::print("Invalid input, please either leave input empty by pressing enter to select the mod, or use one of the following commands:\n.");
			fmt::print("[r]andom - randomly select a new {}.\n", searchType);
			fmt::print("  [b]ack - returns to previous menu.\n");
			fmt::print("       ? - displays this help message.\n");
			fmt::print("\n");

			pause();
		}
	}

	return false;
}

bool ModManager::CLI::runSearchPrompt(const std::string & args) {
	if(m_modManager == nullptr || !m_modManager->isInitialized()) {
		return false;
	}

	std::shared_ptr<OrganizedModCollection> organizedMods = m_modManager->getOrganizedMods();

	std::string input, data, formattedData;
	std::string trimmedArgs(Utilities::trimString(args));
	bool skipInput = !trimmedArgs.empty();
	bool searchMods = organizedMods->shouldDisplayMods();
	bool searchGameVersions = organizedMods->shouldDisplayGameVersions();
	bool searchTeams = organizedMods->shouldDisplayTeams();
	bool searchAuthors = organizedMods->shouldDisplayAuthors();
	const char * searchType = searchMods ? "mod" : (searchTeams ? "team" : (searchAuthors ? "author" : ""));

	while(true) {
		clearOutput();

		if(skipInput) {
			data = trimmedArgs;

			fmt::print("Searching for{}: {}\n", searchType, data);
			fmt::print("\n");
		}
		else {
			fmt::print("Enter search query:\n");
			fmt::print("> ");

			std::getline(std::cin, input);
			data = Utilities::trimString(input);

			if(data.empty()) {
				return false;
			}

			formattedData = Utilities::toLowerCase(data);

			fmt::print("\n");
		}

		if(!skipInput && data.empty()) {
			fmt::print("Please enter a valid search query.\n\n");

			pause();

			continue;
		}

		if(!skipInput && std::regex_match(formattedData, backRegExp)) {
			if(searchMods) {
				m_modManager->clearSelectedMod();
			}
			else if(searchGameVersions) {
				organizedMods->clearSelectedGameVersion();
			}
			else if(searchTeams) {
				organizedMods->clearSelectedTeam();
			}
			else if(searchAuthors) {
				organizedMods->clearSelectedAuthor();
			}

			break;
		}
		else if(!skipInput && std::regex_match(formattedData, helpRegExp)) {
			clearOutput();

			fmt::print("[b]ack - returns to previous menu.\n");
			fmt::print("     ? - displays this help message.\n");
			fmt::print("\n");

			pause();

			continue;
		}
		else {
			std::vector<ModMatch> modMatches;
			ModMatch * modMatch;
			std::vector<std::shared_ptr<GameVersion>> matchingGameVersions;
			std::vector<std::shared_ptr<ModAuthorInformation>> matchingTeamsOrAuthors;
			size_t numberOfMatches = std::numeric_limits<size_t>::max();

			if(searchMods) {
				modMatches = ModManager::searchForMod(organizedMods->getOrganizedMods(), data);
				numberOfMatches = modMatches.size();
			}
			else if(searchGameVersions) {
				numberOfMatches = m_modManager->searchForAndSelectGameVersion(data, &matchingGameVersions);
			}
			else if(searchTeams) {
				numberOfMatches = m_modManager->searchForAndSelectTeam(data, &matchingTeamsOrAuthors);
			}
			else if(searchAuthors) {
				numberOfMatches = m_modManager->searchForAndSelectAuthor(data, &matchingTeamsOrAuthors);
			}

			if(numberOfMatches == std::numeric_limits<size_t>::max()) {
				fmt::print("Invalid or empty search query.\n");
			}
			else if(numberOfMatches == 0) {
				fmt::print("No matches found.\n");
			}
			else if(numberOfMatches == 1) {
				std::string selectedItemName;

				if(searchMods) {
					modMatch = &modMatches[0];

					m_modManager->setSelectedMod(modMatch->getMod());
					m_modManager->setSelectedModVersionIndex(modMatch->getModVersionIndex());
					m_modManager->setSelectedModVersionTypeIndex(modMatch->getModVersionTypeIndex());

					selectedItemName = modMatch->toString();
				}
				else if(searchGameVersions) {
					selectedItemName = organizedMods->getSelectedGameVersion()->getName();
				}
				else if(searchTeams) {
					selectedItemName = organizedMods->getSelectedTeam()->getName();
				}
				else if(searchAuthors) {
					selectedItemName = organizedMods->getSelectedAuthor()->getName();
				}

				fmt::print("Selected {}: {}\n", searchMods ? Utilities::toCapitalCase(std::string(magic_enum::enum_name(modMatch->getMatchType()))) : searchType, selectedItemName);

				return true;
			}
			else {
				if(numberOfMatches > 20) {
					fmt::print("Found {} matches, please refine your search query.\n", numberOfMatches);
				}
				else {
					fmt::print("Found {} matches:\n", numberOfMatches);

					std::string currentMatchName;

					for(size_t i = 0; i < numberOfMatches; i++) {
						printSpacing(Utilities::unsignedLongLength(numberOfMatches) - Utilities::unsignedLongLength(i + 1));

						if(searchMods) {
							currentMatchName = modMatches[i].toString();
						}
						else if(searchGameVersions) {
							currentMatchName = matchingGameVersions[i]->getName();
						}
						else if(searchTeams || searchAuthors) {
							currentMatchName = matchingTeamsOrAuthors[i]->getName();
						}

						fmt::print("{}. {}\n", i + 1, currentMatchName);
					}

					fmt::print("\n");
					fmt::print("Please refine your search query.\n", numberOfMatches);
				}
			}

			fmt::print("\n");

			pause();
		}

		if(skipInput) {
			skipInput = false;
		}
	}

	return false;
}

bool ModManager::CLI::updateSelectedModVersionPrompt() {
	if(m_modManager == nullptr || !m_modManager->isInitialized()) {
		return false;
	}

	std::shared_ptr<Mod> selectedMod(m_modManager->getSelectedMod());

	if(selectedMod == nullptr || selectedMod->numberOfVersions() == 0) {
		return false;
	}

	if(selectedMod->numberOfVersions() == 1) {
		m_modManager->setSelectedModVersionIndex(0);

		return true;
	}

	std::vector<std::string> modVersionNames;

	for(size_t i = 0; i < selectedMod->numberOfVersions(); i++) {
		modVersionNames.push_back(selectedMod->getFullName(i));
	}

	size_t modVersionIndex = getValuePrompt(modVersionNames, "Please select a version of the mod to run:");

	if(modVersionIndex == std::numeric_limits<size_t>::max()) {
		return {};
	}

	return m_modManager->setSelectedModVersionIndex(modVersionIndex);
}

bool ModManager::CLI::updateSelectedModVersionTypePrompt() {
	if(m_modManager == nullptr || !m_modManager->isInitialized()) {
		return false;
	}

	std::shared_ptr<Mod> selectedMod(m_modManager->getSelectedMod());

	if(selectedMod == nullptr || selectedMod->numberOfVersions() == 0) {
		return false;
	}

	std::shared_ptr<ModVersion> selectedModVersion(m_modManager->getSelectedModVersion());

	if(selectedModVersion == nullptr || selectedModVersion->numberOfTypes() == 0) {
		return false;
	}

	if(selectedModVersion->numberOfTypes() == 1) {
		m_modManager->setSelectedModVersionTypeIndex(0);

		return true;
	}

	std::vector<std::string> modVersionTypeNames;

	for(size_t i = 0; i < selectedModVersion->numberOfTypes(); i++) {
		modVersionTypeNames.push_back(selectedMod->getFullName(m_modManager->getSelectedModVersionIndex(), i));
	}

	size_t modVersionTypeIndex = getValuePrompt(modVersionTypeNames, "Please select a version type of the mod to run:");

	if(modVersionTypeIndex == std::numeric_limits<size_t>::max()) {
		return {};
	}

	return m_modManager->setSelectedModVersionTypeIndex(modVersionTypeIndex);
}

size_t ModManager::CLI::getValuePrompt(const std::vector<std::string> & values, const std::string & promptMessage, const std::string & args) {
	std::string input, data, formattedData;
	uint32_t cancel = std::numeric_limits<uint32_t>::max();
	std::string trimmedArgs = Utilities::trimString(args);
	bool skipInput = !trimmedArgs.empty();

	while(true) {
		clearOutput();

		if(skipInput) {
			data = trimmedArgs;
			skipInput = false;
		}
		else {
			if(!promptMessage.empty()) {
				fmt::print("{}\n", promptMessage);
			}

			for(size_t i = 0; i < values.size(); i++) {
				fmt::print("{}: {}\n", i + 1, values[i]);
			}

			cancel = values.size() + 1;

			fmt::print("{}: Cancel\n", cancel);

			fmt::print("> ");

			std::getline(std::cin, input);
			data = Utilities::trimString(input);

			if(data.empty()) {
				return std::numeric_limits<size_t>::max();
			}

			formattedData = Utilities::toLowerCase(data);

			fmt::print("\n");

			if(std::regex_match(formattedData, helpRegExp)) {
				clearOutput();

				fmt::print("<enter> - returns to previous menu.\n");
				fmt::print("      ? - displays this help message.\n");
				fmt::print("\n");

				pause();

				continue;
			}
		}

		if(!data.empty()) {
			for(size_t i = 0; i < values.size(); i++) {
				if(Utilities::areStringsEqualIgnoreCase(values[i], data)) {
					return i;
				}
			}

			if(!trimmedArgs.empty()) {
				trimmedArgs.clear();
			}
			else {
				std::optional<uint32_t> optionalSelection = Utilities::parseUnsignedInteger(data);

				if(optionalSelection.has_value()) {
					if(optionalSelection.value() == cancel) {
						return std::numeric_limits<size_t>::max();
					}
					else if(optionalSelection.value() >= 1 && optionalSelection.value() <= values.size()) {
						return optionalSelection.value() - 1;
					}
				}
			}
		}

		fmt::print("Invalid selection: '{}'.\n\n", data);

		pause();
	}

	return std::numeric_limits<size_t>::max();
}

std::string ModManager::CLI::getArguments(const std::string & data) {
	std::string formattedData(Utilities::trimString(data));

	size_t separatorIndex = formattedData.find_first_of(" \t");

	if(separatorIndex == std::string::npos) {
		return std::string();
	}

	return Utilities::trimString(formattedData.substr(separatorIndex + 1, formattedData.length() - (separatorIndex + 1)));
}

void ModManager::CLI::pause() {
	static constexpr const char * pauseMessage = "Press any key to continue...\n";

	fmt::print(pauseMessage);

	getch();
}

void ModManager::CLI::clearOutput() {
	static constexpr const char * clearLine = "\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n";

	fmt::print(clearLine);
}
