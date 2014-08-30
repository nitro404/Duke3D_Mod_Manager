#include "Mod Manager/ModManager.h"

ModManager::ModManager()
	: m_initialized(false)
	, m_mode(ModManagerModes::defaultMode)
	, m_gameType(GameTypes::defaultGameType)
	, m_selectedMod(NULL)
	, m_selectedModVersionIndex(0) {
	
}

ModManager::~ModManager() {
	if(m_initialized) {
		SettingsManager::getInstance()->modManagerMode = m_mode;
		SettingsManager::getInstance()->gameType = m_gameType;
	}
}

bool ModManager::init(const ArgumentParser * args, bool start) {
	if(m_initialized) { return true; }

	if(args->hasArgument("?")) {
		displayArgumentHelp();
		return false;
	}

	m_mode = SettingsManager::getInstance()->modManagerMode;
	m_gameType = SettingsManager::getInstance()->gameType;

	if(!m_mods.load()) {
		printf("Failed to load mod list!\n");
		return false;
	}

	if(m_mods.numberOfMods() == 0) {
		printf("No mods loaded!\n");
		return false;
	}
	printf("%d mod%s loaded.\n", m_mods.numberOfMods(), m_mods.numberOfMods() == 1 ? "" : "s");

	m_favouriteMods.init(&m_mods);
	if(m_favouriteMods.numberOfFavourites() > 0) {
		printf("%d favourite mod%s loaded.\n", m_favouriteMods.numberOfFavourites(), m_favouriteMods.numberOfFavourites() == 1 ? "" : "s");
	}

	m_organizedMods.init(&m_mods, &m_favouriteMods);

	m_initialized = true;

	checkForMissingExecutables();

	checkAllModsForMissingFiles();

	checkForUnlinkedModFiles();

	printf("\n");

	Utilities::pause();
	Utilities::clear();

	if(!handleArguments(args, start)) {
		return false;
	}

	return true;
}

bool ModManager::uninit() {
	if(!m_initialized) { return false; }

	m_selectedMod = NULL;
	m_organizedMods.uninit();
	m_favouriteMods.uninit();
	m_mods.clearMods();
	m_scriptArgs.clear();

	m_initialized = false;

	return true;
}

void ModManager::run() {
	if(!m_initialized) { return; }

	runMenu();
}

ModManagerModes::ModManagerMode ModManager::getMode() const {
	return m_mode;
}

const char * ModManager::getModeName() const {
	return ModManagerModes::toString(m_mode);
}

bool ModManager::setMode(const char * modeName) {
	if(modeName == NULL) { return false; }

	return setMode(ModManagerModes::parseFrom(modeName));
}

bool ModManager::setMode(const QString & modeName) {
	return setMode(ModManagerModes::parseFrom(modeName));
}

bool ModManager::setMode(int mode) {
	if(!ModManagerModes::isValid(mode)) { return false; }

	m_mode = static_cast<ModManagerModes::ModManagerMode>(mode);

	SettingsManager::getInstance()->modManagerMode = m_mode;

	return true;
}

bool ModManager::setMode(ModManagerModes::ModManagerMode mode) {
	if(!ModManagerModes::isValid(mode)) { return false; }

	m_mode = mode;

	SettingsManager::getInstance()->modManagerMode = m_mode;

	return true;
}

GameTypes::GameType ModManager::getGameType() const {
	return m_gameType;
}

const char * ModManager::getGameTypeName() const {
	return GameTypes::toString(m_mode);
}

bool ModManager::setGameType(const char * gameTypeName) {
	if(gameTypeName == NULL) { return false; }

	return setGameType(GameTypes::parseFrom(gameTypeName));
}

bool ModManager::setGameType(const QString & gameTypeName) {
	return setGameType(GameTypes::parseFrom(gameTypeName));
}

bool ModManager::setGameType(int gameType) {
	if(!GameTypes::isValid(gameType)) { return false; }

	m_gameType = static_cast<GameTypes::GameType>(gameType);

	SettingsManager::getInstance()->gameType = m_gameType;

	return true;
}

bool ModManager::setGameType(GameTypes::GameType gameType) {
	if(!GameTypes::isValid(gameType)) { return false; }

	m_gameType = gameType;

	SettingsManager::getInstance()->gameType = m_gameType;

	return true;
}

const char * ModManager::getServerIPAddress() const {
	return SettingsManager::getInstance()->serverIPAddress;
}

void ModManager::setServerIPAddress(const char * ipAddress) {
	if(SettingsManager::getInstance()->serverIPAddress != NULL) {
		delete [] SettingsManager::getInstance()->serverIPAddress;
	}
	
	if(ipAddress == NULL || Utilities::stringLength(ipAddress) == 0) {
		SettingsManager::getInstance()->serverIPAddress = new char[1];
		SettingsManager::getInstance()->serverIPAddress[0] = '\0';
	}
	else {
		SettingsManager::getInstance()->serverIPAddress = Utilities::trimCopyString(ipAddress);
	}
}

void ModManager::setServerIPAddress(const QString & ipAddress) {
	if(SettingsManager::getInstance()->serverIPAddress != NULL) {
		delete [] SettingsManager::getInstance()->serverIPAddress;
	}

	if(ipAddress.length() == 0) {
		SettingsManager::getInstance()->serverIPAddress = new char[1];
		SettingsManager::getInstance()->serverIPAddress[0] = '\0';
	}
	else {
		QByteArray ipAddressBytes = ipAddress.toLocal8Bit();
		const char * ipAddressData = ipAddressBytes.data();

		SettingsManager::getInstance()->serverIPAddress = Utilities::trimCopyString(ipAddressData);
	}
}

const Mod * ModManager::getSelectedMod() const {
	return m_selectedMod;
}

const char * ModManager::getSelectedModName() const {
	return m_selectedMod == NULL ? NULL : m_selectedMod->getName();
}

bool ModManager::setSelectedMod(const char * name) {
	const Mod * selectedMod = m_mods.getMod(name);

	if(selectedMod != NULL) {
		m_selectedMod = selectedMod;
		return true;
	}
	return false;
}

bool ModManager::setSelectedMod(const QString & name) {
	const Mod * selectedMod = m_mods.getMod(name);

	if(selectedMod != NULL) {
		m_selectedMod = selectedMod;
		return true;
	}
	return false;
}

void ModManager::selectRandomMod() {
	if(!m_initialized) { return; }

	m_selectedMod = m_organizedMods.getMod(Utilities::randomInteger(0, m_organizedMods.numberOfMods() - 1));
}

int ModManager::searchForAndSelectMod(const char * query) {
	if(!m_initialized || query == NULL || Utilities::stringLength(query) == 0) { return -1; }

	return searchForAndSelectMod(QString(query));
}

int ModManager::searchForAndSelectMod(const QString & query) {
	if(!m_initialized) { return -1; }

	QString data = query.trimmed().toLower();
	if(data.length() == 0) { return -1; }

	QString modName;
	const Mod * matchingMod = NULL;
	int numberOfMatches = 0;

	for(int i=0;i<m_mods.numberOfMods();i++) {
		modName = QString(m_mods.getMod(i)->getName()).toLower();

		if(QString::compare(modName, data, Qt::CaseSensitive) == 0) {
			matchingMod = m_mods.getMod(i);
			numberOfMatches = 1;
			break;
		}

		if(modName.contains(data)) {
			matchingMod = m_mods.getMod(i);
			numberOfMatches++;
		}
	}

	if(numberOfMatches) {
		m_selectedMod = matchingMod;
	}

	return numberOfMatches;
}

void ModManager::clearSelectedMod() {
	m_selectedMod = NULL;
}

void ModManager::runMenu() {
	if(!m_initialized) { return; }

	int selectedIndex = -1;
	bool valid = false;
	QTextStream in(stdin);
	QString input, data, date;
	QByteArray dateBytes;

	QRegExp searchRegExp =   QRegExp("^(s(earch)?)([ ]+.*)?");
	QRegExp randomRegExp =   QRegExp("^(r(andom)?)$");
	QRegExp vanillaRegExp =  QRegExp("^(v(anilla)?)$");
	QRegExp filterRegExp =   QRegExp("^(f(ilter)?)([ ]+.*)?");
	QRegExp sortRegExp =     QRegExp("^(o|sort)([ ]+.*)?");
	QRegExp ipRegExp =       QRegExp("^(ip|c(onnect)?)([ ]+.*)?");
	QRegExp gameTypeRegExp = QRegExp("^(t(ype)?)([ ]+.*)?");
	QRegExp modeRegExp =     QRegExp("^(m(ode)?)([ ]+.*)?");
	QRegExp helpRegExp =     QRegExp("^(\\?|h(elp)?)$");
	QRegExp exitRegExp =     QRegExp("^(x|exit|q(uit)?)$");

	while(true) {
		Utilities::clear();

		if( m_organizedMods.getFilterType() == ModFilterTypes::None ||
		    m_organizedMods.getFilterType() == ModFilterTypes::Favourites ||
		   (m_organizedMods.getFilterType() == ModFilterTypes::Teams && m_organizedMods.hasSelectedTeam()) ||
		   (m_organizedMods.getFilterType() == ModFilterTypes::Authors && m_organizedMods.hasSelectedAuthor())) {
			for(int i=0;i<m_organizedMods.numberOfMods();i++) {
				printf("%d: %s", (i + 1), m_organizedMods.getMod(i)->getName());
				if(m_organizedMods.getSortType() == ModSortTypes::ReleaseDate) {
					if(!m_organizedMods.getMod(i)->getReleaseDate().isNull()) {
						date = Utilities::dateToString(m_organizedMods.getMod(i)->getReleaseDate());
						dateBytes = date.toLocal8Bit();

						printf(" (%s)", dateBytes.data());
					}
				}
				if(m_organizedMods.getSortType() == ModSortTypes::Rating) {
// TODO: print rating information
				}
				printf("\n");
			}
		}
		else if(m_organizedMods.getFilterType() == ModFilterTypes::Teams && !m_organizedMods.hasSelectedTeam()) {
			for(int i=0;i<m_organizedMods.numberOfTeams();i++) {
				printf("%d: %s (%d)\n", (i + 1), m_organizedMods.getTeamInfo(i)->getName(), m_organizedMods.getTeamInfo(i)->getModCount());
			}
		}
		else if(m_organizedMods.getFilterType() == ModFilterTypes::Authors && !m_organizedMods.hasSelectedAuthor()) {
			for(int i=0;i<m_organizedMods.numberOfAuthors();i++) {
				printf("%d: %s (%d)\n", (i + 1), m_organizedMods.getAuthorInfo(i)->getName(), m_organizedMods.getAuthorInfo(i)->getModCount());
			}
		}
		printf("> ");

		input = in.readLine();
		data = input.trimmed().toLower();

		if(sortRegExp.exactMatch(data)) {
			runSortPrompt(Utilities::getArguments(data));
			continue;
		}
		else if(searchRegExp.exactMatch(data)) {
			runSearchPrompt(Utilities::getArguments(data));
			if(m_selectedMod != NULL) {
				printf("\n");
				runSelectedMod();
				break;
			}
			continue;
		}
		else if(randomRegExp.exactMatch(data)) {
			runSelectRandomModPrompt();
			if(m_selectedMod != NULL) {
				printf("\n");
				runSelectedMod();
				break;
			}
			continue;
		}
		else if(vanillaRegExp.exactMatch(data)) {
			m_selectedMod = NULL;
			runSelectedMod();
			break;
		}
		else if(filterRegExp.exactMatch(data)) {
			runFilterPrompt(Utilities::getArguments(data));
			continue;
		}
		else if(ipRegExp.exactMatch(data)) {
			runIPAddressPrompt(Utilities::getArguments(data));
			continue;
		}
		else if(gameTypeRegExp.exactMatch(data)) {
			runGameTypePrompt(Utilities::getArguments(data));
			continue;
		}
		else if(modeRegExp.exactMatch(data)) {
			runModePrompt(Utilities::getArguments(data));
			continue;
		}
		else if(helpRegExp.exactMatch(data)) {
			Utilities::clear();

			printf("Command information:\n");
			printf(" [s]earch <args> - searches for the specified mod, or runs a prompt.\n");
			printf(" [r]andom -------- randomly select a mod from the list and run it.\n");
			printf("[v]anilla -------- run game without any mods.\n");
			printf(" [f]ilter <args> - change the mod list filter.\n");
			printf("   s[o]rt <args> - change mod list sorting options.\n");
			printf("[c]onnect <args> - obtains the ip address of the host server.\n");
			printf("       ip <args> - obtains the ip address of the host server.\n");
			printf("   [t]ype <args> - change game type (single / multi player).\n");
			printf("   [m]ode <args> - change mod manager launch mode (regular / dosbox).\n");
			printf("   [h]elp -------- displays this help message.\n");
			printf("        ? -------- displays this help message.\n");
			printf("   [q]uit -------- closes the program.\n");
			printf("   e[x]it -------- closes the program.\n");
			printf("\n");

			Utilities::pause();

			continue;
		}
		else if(exitRegExp.exactMatch(data)) {
			Utilities::clear();

			printf("Goodbye!\n");
			printf("\n");

			return;
		}

		selectedIndex = input.toInt(&valid, 10);

		if(valid) {
			if( m_organizedMods.getFilterType() == ModFilterTypes::None ||
				m_organizedMods.getFilterType() == ModFilterTypes::Favourites ||
			   (m_organizedMods.getFilterType() == ModFilterTypes::Teams && m_organizedMods.hasSelectedTeam()) ||
			   (m_organizedMods.getFilterType() == ModFilterTypes::Authors && m_organizedMods.hasSelectedAuthor())) {
				if(selectedIndex >= 1 && selectedIndex <= m_organizedMods.numberOfMods()) {
					valid = true;

					m_selectedMod = m_organizedMods.getMod(selectedIndex - 1);
					runSelectedMod();

					break;
				}
			}
			else if(m_organizedMods.getFilterType() == ModFilterTypes::Teams && !m_organizedMods.hasSelectedTeam()) {
				if(selectedIndex >= 1 && selectedIndex <= m_organizedMods.numberOfTeams()) {
					valid = true;

					m_organizedMods.setSelectedTeam(selectedIndex - 1);
				}
			}
			else if(m_organizedMods.getFilterType() == ModFilterTypes::Authors && !m_organizedMods.hasSelectedAuthor()) {
				if(selectedIndex >= 1 && selectedIndex <= m_organizedMods.numberOfAuthors()) {
					valid = true;

					m_organizedMods.setSelectedAuthor(selectedIndex - 1);
				}
			}
		}

		if(!valid) {
			Utilities::clear();
			printf("Invalid selection!\n\n");
			Utilities::pause();
		}
	}
}

void ModManager::runFilterPrompt(const QString & args) {
	if(!m_initialized) { return; }

	bool valid = false;
	QTextStream in(stdin);
	QString input, data;
	int value = -1;
	int cancel = -1;
	QString trimmedArgs = args.trimmed();
	bool skipInput = !trimmedArgs.isEmpty();
	ModFilterTypes::ModFilterType filterType = ModFilterTypes::Invalid;
	QRegExp integerRegExp(QString("^[0-9]+$"));

	while(true) {
		Utilities::clear();

		if(skipInput) {
			data = trimmedArgs;
			skipInput = false;
		}
		else {
			printf("Current filter type is set to: \"%s\" (default: \"%s\")\n\n", ModFilterTypes::toString(m_organizedMods.getFilterType()), ModFilterTypes::toString(ModFilterTypes::defaultFilterType));
			printf("Choose a filter type:\n");

			for(int i=static_cast<int>(ModFilterTypes::Invalid) + 1;i<static_cast<int>(ModFilterTypes::NumberOfFilterTypes);i++) {
				printf("%d: %s\n", i, ModFilterTypes::filterTypeStrings[i]);

				cancel = i + 1;
			}

			printf("%d: Cancel\n", cancel);

			printf("> ");

			input = in.readLine();
			data = input.trimmed();

			printf("\n");
		}

		valid = false;
		filterType = ModFilterTypes::Invalid;

		if(data.length() > 0) {
			filterType = ModFilterTypes::parseFrom(data);
			if(!ModFilterTypes::isValid(filterType)) {
				if(!trimmedArgs.isEmpty()) {
					trimmedArgs.clear();
				}
				else if(integerRegExp.exactMatch(data)) {
					value = data.toInt(&valid, 10);
					if(valid) {
						if(value == cancel) {
							break;
						}
						else if(ModFilterTypes::isValid(value)) {
							filterType = static_cast<ModFilterTypes::ModFilterType>(value);
						}
					}
				}
			}
		}
		
		if(ModFilterTypes::isValid(filterType)) {
			printf("Changing filter type from \"%s\" to \"%s\".\n\n", ModFilterTypes::toString(m_organizedMods.getFilterType()), ModFilterTypes::toString(filterType));

			m_organizedMods.setFilterType(filterType);

			Utilities::pause();

			break;
		}
		else {
			QByteArray dataBytes = data.toLocal8Bit();

			printf("Invalid filter type: %s\n\n", dataBytes.data());

			Utilities::pause();
		}
	}
}

void ModManager::runSortPrompt(const QString & args) {
	if(!m_initialized) { return; }

	bool valid = false;
	bool inputSortDirection = false;
	QTextStream in(stdin);
	QString input, data;
	int value = -1;
	int cancel = -1;
	QString trimmedArgs = args.trimmed();
	bool skipInput = !trimmedArgs.isEmpty();
	ModSortTypes::ModSortType sortType = ModSortTypes::Invalid;
	ModSortDirections::ModSortDirection sortDirection = ModSortDirections::Invalid;
	QRegExp integerRegExp(QString("^[0-9]+$"));
	QRegExp whitespaceRegExp = QRegExp("[ \t]+");

	QVector<ModSortTypes::ModSortType> validSortTypes;
	for(int i=ModSortTypes::Invalid+1;i<ModSortTypes::NumberOfSortTypes;i++) {
		if(ModSortTypes::isValidInContext(i, m_organizedMods.getFilterType(), m_organizedMods.hasSelectedTeam(), m_organizedMods.hasSelectedAuthor())) {
			validSortTypes.push_back(static_cast<ModSortTypes::ModSortType>(i));
		}
	}

	while(true) {
		Utilities::clear();

		if(!skipInput) {
			if(!ModSortTypes::isValid(sortType)) {
				printf("Current sort type is: \"%s", ModSortTypes::toString(m_organizedMods.getSortType()));
				if(m_organizedMods.getSortType() != ModSortTypes::Unsorted) {
					printf(" (%s)", ModSortDirections::toString(m_organizedMods.getSortDirection()));
				}
				printf("\" (default: \"%s", ModSortTypes::toString(ModSortTypes::defaultSortType));
				if(ModSortTypes::defaultSortType != ModSortTypes::Unsorted) {
					printf(" (%s)", ModSortDirections::toString(ModSortDirections::defaultSortDirection));
				}
				printf("\")\n\n");
				printf("Choose a sort type:\n");
				for(int i=0;i<validSortTypes.size();i++) {
					printf("%d: %s\n", i + 1, ModSortTypes::sortTypeStrings[static_cast<int>(validSortTypes[i])]);

					cancel = i + 2;
				}
				printf("%d: Cancel\n", cancel);
				printf("> ");

				input = in.readLine();
				data = input.trimmed();

				printf("\n");
			}
			else {
				printf("Current sort type will be changed from \"%s\" to \"%s\".\n\n", ModSortTypes::toString(m_organizedMods.getSortType()), ModSortTypes::toString(sortType));
				printf("Current sort direction is: \"%s\" (default: \"%s\").\n\n", ModSortDirections::toString(m_organizedMods.getSortDirection()), ModSortDirections::toString(ModSortDirections::defaultSortDirection));
				printf("Choose a sort direction:\n");
				for(int i=static_cast<int>(ModSortDirections::Invalid) + 1;i<static_cast<int>(ModSortDirections::NumberOfSortDirections);i++) {
					printf("%d: %s\n", i, ModSortDirections::sortDirectionStrings[i]);

					cancel = i + 1;
				}
				printf("%d: Cancel\n", cancel);
				printf("> ");

				input = in.readLine();
				data = input.trimmed();

				printf("\n");

				inputSortDirection = true;
			}
		}

		if(skipInput) {
			data.clear();
			QStringList parts = trimmedArgs.split(whitespaceRegExp, QString::SkipEmptyParts);

			sortDirection = ModSortDirections::parseFrom(parts[parts.size() - 1]);

			if(parts.size() > 1) {
				if(ModSortDirections::isValid(sortDirection)) {
					for(int i=0;i<parts.size()-1;i++) {
						data.append(parts[i]);
						if(i < parts.size() - 1) {
							data.append(" ");
						}
					}
				}
				else {
					sortDirection = ModSortDirections::parseFrom(parts[0]);

					if(ModSortDirections::isValid(sortDirection)) {
						for(int i=1;i<parts.size();i++) {
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
				if(!ModSortDirections::isValid(sortDirection)) {
					data = trimmedArgs;
				}
			}

			if(!data.isEmpty()) {
				sortType = ModSortTypes::parseFrom(data);

				if(!ModSortTypes::isValid(sortType)) {
					sortDirection = ModSortDirections::Invalid;
				}
			}
		}

		if(!skipInput) {
			if(!ModSortTypes::isValid(sortType)) {
				if(data.length() > 0) {
					sortType = ModSortTypes::parseFrom(data);
					if(!ModSortTypes::isValid(sortType)) {
						if(integerRegExp.exactMatch(data)) {
							value = data.toInt(&valid, 10);
							if(valid) {
								if(value == cancel) {
									break;
								}
								else if(value >= 1 && value <= validSortTypes.size() && ModSortTypes::isValid(validSortTypes[value - 1])) {
									sortType = static_cast<ModSortTypes::ModSortType>(validSortTypes[value - 1]);
								}
							}
						}
					}
				}
			}
			else {
				if(data.length() > 0) {
					sortDirection = ModSortDirections::parseFrom(data);
					if(!ModSortDirections::isValid(sortDirection)) {
						if(integerRegExp.exactMatch(data)) {
							value = data.toInt(&valid, 10);
							if(valid) {
								if(value == cancel) {
									break;
								}
								else if(ModSortDirections::isValid(value)) {
									sortDirection = static_cast<ModSortDirections::ModSortDirection>(value);
								}
							}
						}
					}
				}
			}
		}

		if(skipInput) {
			if(ModSortTypes::isValid(sortType) || ModSortDirections::isValid(sortDirection)) {
				if(!ModSortTypes::isValid(sortType)) {
					sortType = m_organizedMods.getSortType();
				}

				if(!ModSortDirections::isValid(sortDirection)) {
					sortDirection = m_organizedMods.getSortDirection();
				}
			}
			else {
				sortType = ModSortTypes::Invalid;
				sortDirection = ModSortDirections::Invalid;

				QByteArray argumentBytes = trimmedArgs.toLocal8Bit();

				printf("Invalid sort type argument: %s\n\n", argumentBytes.data());
			}
		}
		
		if(ModSortTypes::isValid(sortType) && (ModSortDirections::isValid(sortDirection) || sortType == ModSortTypes::Unsorted)) {
			if(sortType == ModSortTypes::Unsorted) {
				sortDirection = ModSortDirections::defaultSortDirection;
			}

			if(ModSortTypes::isValidInContext(sortType, m_organizedMods.getFilterType(), m_organizedMods.hasSelectedTeam(), m_organizedMods.hasSelectedAuthor())) {
				printf("Changing sort type from \"%s", ModSortTypes::toString(m_organizedMods.getSortType()));
				if(m_organizedMods.getSortType() != ModSortTypes::Unsorted) {
					printf(" (%s)", ModSortDirections::toString(m_organizedMods.getSortDirection()));
				}
				printf("\" to \"%s", ModSortTypes::toString(sortType));
				if(sortType != ModSortTypes::Unsorted) {
					printf(" (%s)", ModSortDirections::toString(sortDirection));
				}
				printf("\".\n\n");

				m_organizedMods.setSortOptions(sortType, sortDirection);

				Utilities::pause();

				break;
			}
			else {
				printf("Sort type \"%s", ModSortTypes::toString(sortType));
				if(sortType != ModSortTypes::Unsorted) {
					printf(" (%s)", ModSortDirections::toString(sortDirection));
				}
				printf("\" is not valid with filter type: \"%s\".\n\n", ModFilterTypes::toString(m_organizedMods.getFilterType()));

				sortType = ModSortTypes::Invalid;
				sortDirection = ModSortDirections::Invalid;

				Utilities::pause();
			}
		}
		else if(!ModSortTypes::isValid(sortType) && !skipInput) {
			QByteArray dataBytes = data.toLocal8Bit();

			printf("Invalid sort type: %s\n\n", dataBytes.data());

			Utilities::pause();
		}
		else if(!ModSortDirections::isValid(sortDirection) && inputSortDirection && !skipInput) {
			QByteArray dataBytes = data.toLocal8Bit();

			printf("Invalid sort direction: %s\n\n", dataBytes.data());

			Utilities::pause();
		}

		if(skipInput) {
			skipInput = false;
		}
	}
}

void ModManager::runGameTypePrompt(const QString & args) {
	if(!m_initialized) { return; }

	bool valid = false;
	QTextStream in(stdin);
	QString input, data;
	int value = -1;
	int cancel = -1;
	QString trimmedArgs = args.trimmed();
	bool skipInput = !trimmedArgs.isEmpty();
	GameTypes::GameType gameType = GameTypes::Invalid;
	QRegExp integerRegExp(QString("^[0-9]+$"));

	while(true) {
		Utilities::clear();

		if(skipInput) {
			data = trimmedArgs;
			skipInput = false;
		}
		else {
			printf("Current game type is set to: \"%s\" (default: \"%s\")\n\n", GameTypes::toString(SettingsManager::getInstance()->gameType), GameTypes::toString(SettingsManager::getInstance()->defaultGameType));
			printf("Choose a game type:\n");

			for(int i=static_cast<int>(GameTypes::Invalid) + 1;i<static_cast<int>(GameTypes::NumberOfGameTypes);i++) {
				printf("%d: %s\n", i, GameTypes::gameTypeStrings[i]);

				cancel = i + 1;
			}

			printf("%d: Cancel\n", cancel);

			printf("> ");

			input = in.readLine();
			data = input.trimmed();

			printf("\n");
		}

		gameType = GameTypes::Invalid;

		if(data.length() > 0) {
			gameType = GameTypes::parseFrom(data);
			if(!GameTypes::isValid(gameType)) {
				if(!trimmedArgs.isEmpty()) {
					trimmedArgs.clear();
				}
				else if(integerRegExp.exactMatch(data)) {
					value = data.toInt(&valid, 10);
					if(valid) {
						if(value == cancel) {
							break;
						}
						else if(GameTypes::isValid(value)) {
							gameType = static_cast<GameTypes::GameType>(value);
						}
					}
				}
			}
		}
		
		if(GameTypes::isValid(gameType)) {
			printf("Changing game type from \"%s\" to \"%s\".\n\n", GameTypes::toString(SettingsManager::getInstance()->gameType), GameTypes::toString(gameType));

			setGameType(gameType);

			Utilities::pause();

			break;
		}
		else {
			QByteArray dataBytes = data.toLocal8Bit();

			printf("Invalid game type: %s\n\n", dataBytes.data());

			Utilities::pause();
		}
	}
}

void ModManager::runModePrompt(const QString & args) {
	if(!m_initialized) { return; }

	bool valid = false;
	QTextStream in(stdin);
	QString input, data;
	int value = -1;
	int cancel = -1;
	QString trimmedArgs = args.trimmed();
	bool skipInput = !trimmedArgs.isEmpty();
	ModManagerModes::ModManagerMode mode = ModManagerModes::Invalid;
	QRegExp integerRegExp(QString("^[0-9]+$"));

	while(true) {
		Utilities::clear();

		if(skipInput) {
			data = trimmedArgs;
			skipInput = false;
		}
		else {
			printf("Current launch mode is set to: \"%s\" (default: \"%s\")\n\n", ModManagerModes::toString(SettingsManager::getInstance()->modManagerMode), ModManagerModes::toString(SettingsManager::getInstance()->defaultModManagerMode));
			printf("Choose a game launch mode:\n");

			for(int i=static_cast<int>(ModManagerModes::Invalid) + 1;i<static_cast<int>(ModManagerModes::NumberOfModes);i++) {
				printf("%d: %s\n", i, ModManagerModes::modeStrings[i]);

				cancel = i + 1;
			}

			printf("%d: Cancel\n", cancel);

			printf("> ");

			input = in.readLine();
			data = input.trimmed();

			printf("\n");
		}

		mode = ModManagerModes::Invalid;

		if(data.length() > 0) {
			mode = ModManagerModes::parseFrom(data);
			if(!ModManagerModes::isValid(mode)) {
				if(!trimmedArgs.isEmpty()) {
					trimmedArgs.clear();
				}
				else if(integerRegExp.exactMatch(data)) {
					value = data.toInt(&valid, 10);
					if(valid) {
						if(value == cancel) {
							break;
						}
						else if(ModManagerModes::isValid(value)) {
							mode = static_cast<ModManagerModes::ModManagerMode>(value);
						}
					}
				}
			}
		}
		
		if(ModManagerModes::isValid(mode)) {
			printf("Changing game launch mode from \"%s\" to \"%s\".\n\n", ModManagerModes::toString(SettingsManager::getInstance()->modManagerMode), ModManagerModes::toString(mode));

			setMode(mode);

			Utilities::pause();

			break;
		}
		else {
			QByteArray dataBytes = data.toLocal8Bit();

			printf("Invalid game launch mode: %s\n\n", dataBytes.data());

			Utilities::pause();
		}
	}
}

void ModManager::runIPAddressPrompt(const QString & args) {
	if(!m_initialized) { return; }

	bool valid = false;
	QTextStream in(stdin);
	QString input, data;
	QString trimmedArgs = args.trimmed();
	bool skipInput = !trimmedArgs.isEmpty();

	while(true) {
		Utilities::clear();

		if(skipInput) {
			data = trimmedArgs;
			skipInput = false;
		}
		else {
			printf("Enter host IP Address:\n");
			printf("> ");

			input = in.readLine();
			data = input.trimmed();

			printf("\n");
		}

		valid = true;

		if(data.length() == 0) { valid = false; }

		for(int i=0;i<data.length();i++) {
			if(data[i] == ' ' || data[i] == '\t') {
				valid = false;
			}
		}

		QByteArray dataBytes = data.toLocal8Bit();
		if(valid) {
			if(SettingsManager::getInstance()->serverIPAddress != NULL) { delete [] SettingsManager::getInstance()->serverIPAddress; }
			SettingsManager::getInstance()->serverIPAddress = new char[Utilities::stringLength(dataBytes.data()) + 1];
			Utilities::copyString(SettingsManager::getInstance()->serverIPAddress, Utilities::stringLength(dataBytes.data()) + 1, dataBytes.data());

			printf("Host IP Address changed to: %s\n\n", dataBytes.data());

			Utilities::pause();

			break;
		}
		else {
			printf("Invalid IP Address: %s\n\n", dataBytes.data());
			Utilities::pause();
		}
	}
}

void ModManager::runSelectRandomModPrompt() {
	if(!m_initialized) { return; }

	QTextStream in(stdin);
	QString input, data;
	bool skipRandomSelection = false;

	QRegExp randomRegExp = QRegExp("^(r(andom)?)$");
	QRegExp cancelRegExp = QRegExp("^(a(bort)?)|(b(ack)?)|(c(ancel)?)$");

	while(true) {
		Utilities::clear();

		if(!skipRandomSelection) {
			selectRandomMod();
		}
		else {
			skipRandomSelection = false;
		}

		printf("Randomly selected mod: %s\n\n", m_selectedMod->getName());
		printf("Press enter to continue, or type r to select a new mod, or b to go back.\n\n");
		printf("> ");

		input = in.readLine();
		data = input.trimmed().toLower();

		if(data.isEmpty()) { break; }

		if(randomRegExp.exactMatch(data)) {
			continue;
		}
		else if(cancelRegExp.exactMatch(data)) {
			m_selectedMod = NULL;
			return;
		}
		else {
			skipRandomSelection = true;

			Utilities::clear();

			printf("Invalid input, please either leave input empty by pressing enter to select the mod, or use one of the following commands:\n.");
			printf(" [r]andom - randomly select a new mod.\n");
			printf("  [a]bort - returns to previous menu.\n");
			printf("   [b]ack - returns to previous menu.\n");
			printf(" [c]ancel - returns to previous menu.\n");
			printf("\n");

			Utilities::pause();
		}
	}
}

void ModManager::runSearchPrompt(const QString & args) {
	if(!m_initialized) { return; }

	QTextStream in(stdin);
	QString input, data, formattedData;
	QString trimmedArgs = args.trimmed();
	bool skipInput = !trimmedArgs.isEmpty();

	QRegExp cancelRegExp = QRegExp("^(a(bort)?)|(b(ack)?)|(c(ancel)?)$");
	QRegExp helpRegExp =   QRegExp("^(\\?|h(elp)?)$");

	while(true) {
		Utilities::clear();

		if(skipInput) {
			data = trimmedArgs;

			QByteArray dataBytes = data.toLocal8Bit();

			printf("Searching for: %s\n", dataBytes.data());
			printf("\n");
		}
		else {
			printf("Enter search query:\n");
			printf("> ");

			input = in.readLine();
			data = input.trimmed();
			formattedData = data.toLower();

			printf("\n");
		}

		if(!skipInput && data.isEmpty()) {
			printf("Please enter a valid search query.\n\n");

			Utilities::pause();

			continue;
		}

		if(!skipInput && cancelRegExp.exactMatch(formattedData)) {
			m_selectedMod = NULL;
			break;
		}
		else if(!skipInput && helpRegExp.exactMatch(formattedData)) {
			Utilities::clear();

			printf(" [a]bort - returns to previous menu.\n");
			printf("  [b]ack - returns to previous menu.\n");
			printf("[c]ancel - returns to previous menu.\n");
			printf("  [h]elp - displays this help message.\n");
			printf("       ? - displays this help message.\n");
			printf("\n");

			Utilities::pause();

			continue;
		}
		else {
			int numberOfMatches = searchForAndSelectMod(data);

			if(numberOfMatches == -1) {
				printf("Invalid or empty search query.\n");
			}
			else if(numberOfMatches == 0) {
				printf("No matches found.\n");
			}
			else if(numberOfMatches == 1) {
				printf("Selected mod: %s\n", m_selectedMod->getName());
				break;
			}
			else {
				printf("%d matches found, please refine your search query.\n", numberOfMatches);
			}

			printf("\n");

			Utilities::pause();
		}

		if(skipInput) {
			skipInput = false;
		}
	}
}

bool ModManager::runModVersionSelectionPrompt() {
	if(!m_initialized || m_selectedMod == NULL || m_selectedMod->numberOfVersions() == 0) {
		return false;
	}
	
	m_selectedModVersionIndex = 0;

	if(m_selectedMod->numberOfVersions() == 1) {
		return true;
	}

	int selectedModVersion = -1;
	bool valid = false;
	QTextStream in(stdin);
	QString input, data;

	while(true) {
		printf("Please select a version of the mod to run:\n");
		for(int i=0;i<m_selectedMod->numberOfVersions();i++) {
			printf("%d: %s\n", (i + 1), m_selectedMod->getFullName(i).toLocal8Bit().data());
		}
		printf("> ");

		input = in.readLine();
		data = input.trimmed().toLower();

		if(QRegExp("^(a(bort)?|c(ancel)?)$").exactMatch(data)) {
			return false;
		}

		selectedModVersion = input.toInt(&valid, 10);

		valid = valid && selectedModVersion >= 1 && selectedModVersion <= m_selectedMod->numberOfVersions();

		if(valid) {
			m_selectedModVersionIndex = selectedModVersion - 1;
			return true;
		}
		else {
			printf("Invalid selection! To cancel, type cancel or abort.\n\n");
			Utilities::pause();
		}
	}

	return false;
}

void ModManager::runSelectedMod(const ArgumentParser * args) {
	if(!m_initialized) { return; }

	if(m_selectedMod != NULL) {
		if(checkModForMissingFiles(*m_selectedMod) > 0) {
			printf("Mod is missing files, aborting execution.\n");
			return;
		}
		
		m_selectedModVersionIndex = 0;
		if(m_selectedMod->numberOfVersions() > 1) {
			if(!runModVersionSelectionPrompt()) {
				return;
			}

			printf("\n");
		}
	}

	Script script;

	if(!updateScriptArgs()) {
		return;
	}

	bool customMod = false;
	if(args != NULL) {
		if((args->hasArgument("g") && args->getValue("g").length() > 0) || (args->hasArgument("x") && args->getValue("x").length() > 0)) {
			m_scriptArgs.setArgument("GROUP", args->getValue("g"));
			m_scriptArgs.setArgument("CON", args->getValue("x"));

			customMod = true;
		}
	}

	if(!customMod && m_selectedMod != NULL) {
		printf("Running mod \"%s\" in %s mode.\n\n", m_selectedMod->getFullName(m_selectedModVersionIndex).toLocal8Bit().data(), GameTypes::toString(m_gameType));
	}

	if(m_selectedMod != NULL) {
		Utilities::renameFiles("DMO", "DMO_");
	}

	if(m_mode == ModManagerModes::DOSBox) {
		bool scriptLoaded = false;
		const char * scriptFileName;

		if(m_gameType == GameTypes::Game) {
			scriptLoaded = script.readFrom(SettingsManager::getInstance()->DOSBoxGameScriptFileName);

			scriptFileName = SettingsManager::getInstance()->DOSBoxGameScriptFileName;
		}
		else if(m_gameType == GameTypes::Setup) {
			scriptLoaded = script.readFrom(SettingsManager::getInstance()->DOSBoxSetupScriptFileName);

			scriptFileName = SettingsManager::getInstance()->DOSBoxSetupScriptFileName;
		}
		else if(m_gameType == GameTypes::Client) {
			scriptLoaded = script.readFrom(SettingsManager::getInstance()->DOSBoxClientScriptFileName);

			scriptFileName = SettingsManager::getInstance()->DOSBoxClientScriptFileName;
		}
		else if(m_gameType == GameTypes::Server) {
			scriptLoaded = script.readFrom(SettingsManager::getInstance()->DOSBoxServerScriptFileName);

			scriptFileName = SettingsManager::getInstance()->DOSBoxServerScriptFileName;
		}
		else {	
			return;
		}

		if(!scriptLoaded) {
			printf("Failed to load DOSBox script file: %s\n", scriptFileName);
			return;
		}

		QString DOSBoxCommand = script.generateDOSBoxCommand(m_scriptArgs);
		QByteArray DOSBoxCommandBytes = DOSBoxCommand.toLocal8Bit();
		const char * DOSBoxCommandData = DOSBoxCommandBytes.data();

#if _DEBUG
		printf("%s\n", DOSBoxCommandData);
#endif // _DEBUG

		system(DOSBoxCommandData);
	}
	else if(m_mode == ModManagerModes::Windows) {
		bool scriptLoaded = false;
		const char * scriptFileName;

		if(m_gameType == GameTypes::Game) {
			scriptLoaded = script.readFrom(SettingsManager::getInstance()->windowsGameScriptFileName);

			scriptFileName = SettingsManager::getInstance()->windowsGameScriptFileName;
		}
		else if(m_gameType == GameTypes::Setup || m_gameType == GameTypes::Client || m_gameType == GameTypes::Server) {
			scriptLoaded = script.readFrom(SettingsManager::getInstance()->windowsSetupScriptFileName);

			scriptFileName = SettingsManager::getInstance()->windowsSetupScriptFileName;
		}
		else {
			return;
		}

		if(!scriptLoaded) {
			printf("Failed to load Windows script file: %s\n", scriptFileName);
			return;
		}

		for(int i=0;i<script.numberOfCommands();i++) {
			QString windowsCommand = script.generateWindowsCommand(m_scriptArgs, i);
			QByteArray windowsCommandBytes = windowsCommand.toLocal8Bit();
			const char * windowsCommandData = windowsCommandBytes.data();

#if _DEBUG
			printf("%s\n", windowsCommandData);
#endif // _DEBUG

			system(windowsCommandData);
		}
	}

	if(m_selectedMod != NULL) {
		Utilities::deleteFiles("DMO");
	
		Utilities::renameFiles("DMO_", "DMO");
	}
}

bool ModManager::updateScriptArgs() {
	if(!m_initialized) { return false; }

	m_scriptArgs.setArgument("KEXTRACT", SettingsManager::getInstance()->kextractFileName);
	m_scriptArgs.setArgument("DUKE3D", SettingsManager::getInstance()->gameFileName);
	m_scriptArgs.setArgument("SETUP", SettingsManager::getInstance()->setupFileName);
	m_scriptArgs.setArgument("MODDIR", SettingsManager::getInstance()->modsDirectoryName);
	if(m_selectedMod != NULL && m_selectedMod->getVersion(m_selectedModVersionIndex) != NULL) {
		m_scriptArgs.setArgument("CON", m_selectedMod->getVersion(m_selectedModVersionIndex)->getFileNameByType("con"));
		m_scriptArgs.setArgument("GROUP", m_selectedMod->getVersion(m_selectedModVersionIndex)->getFileNameByType("grp"));
	}
	m_scriptArgs.setArgument("IP", SettingsManager::getInstance()->serverIPAddress);

	return true;
}

bool ModManager::handleArguments(const ArgumentParser * args, bool start) {
	if(args == NULL) { return false; }

	if(args->hasArgument("m")) {
		ModManagerModes::ModManagerMode newMode = ModManagerModes::parseFrom(args->getValue("m"));
			
		if(isValid(newMode)) {
			m_mode = newMode;
		}
		else {
			printf("Invalid mode argument, please specify one of the following: %s / %s\n", ModManagerModes::toString(ModManagerModes::DOSBox), ModManagerModes::toString(ModManagerModes::Windows));
			return false;
		}
	}

	if(args->hasArgument("t")) {
		GameTypes::GameType newGameType = GameTypes::parseFrom(args->getValue("t"));

		if(isValid(newGameType)) {
			m_gameType = newGameType;

			printf("Setting game type to: %s\n", GameTypes::toString(m_gameType));

			if(m_gameType == GameTypes::Client) {
				QString ipAddress;

				if(args->hasArgument("ip")) {
					ipAddress = args->getValue("ip").trimmed();

					bool valid = true;
					for(int i=0;i<ipAddress.length();i++) {
						if(ipAddress[i] == ' ' || ipAddress[i] == '\t') {
							valid = false;
						}
					}

					if(ipAddress.length() == 0) { valid = false; }

					QByteArray ipAddressBytes = ipAddress.toLocal8Bit();
					const char * ipAddressData = ipAddressBytes.data();

					if(valid) {
						if(SettingsManager::getInstance()->serverIPAddress != NULL) { delete [] SettingsManager::getInstance()->serverIPAddress; }
						SettingsManager::getInstance()->serverIPAddress = new char[Utilities::stringLength(ipAddressData) + 1];
						Utilities::copyString(SettingsManager::getInstance()->serverIPAddress, Utilities::stringLength(ipAddressData) + 1, ipAddressData);
					}
					else {
						printf("\nInvalid IP Address entered in arguments: %s\n\n", ipAddressData);

						runIPAddressPrompt();
					}
				}
			}
		}
		else {
			printf("Invalid game type, please specify one of the following: %s / %s / %s / %s\n", GameTypes::toString(GameTypes::Game), GameTypes::toString(GameTypes::Setup), GameTypes::toString(GameTypes::Client), GameTypes::toString(GameTypes::Server));
			return false;
		}
	}

	if(args->hasArgument("s")) {
		if(args->hasArgument("r") || args->hasArgument("g") || args->hasArgument("x") || args->hasArgument("v")) {
			printf("Redundant arguments specified, please specify either s OR r OR v OR (x AND/OR g).\n");
			return false;
		}

		int numberOfMatches = searchForAndSelectMod(args->getValue("s"));

		if(numberOfMatches == -1) {
			printf("Invalid or empty search query.\n");
			return false;
		}
		else if(numberOfMatches == 0) {
			printf("No matches found for specified search query.\n\n");
			return false;
		}
		else if(numberOfMatches == 1) {
			printf("Selected mod from search query: %s\n", m_selectedMod->getName());

			runSelectedMod();

			return true;
		}
		else {
			printf("%d matches found, please refine your search query.\n", numberOfMatches);
			return false;
		}
	}
	else if(args->hasArgument("r")) {
		if(args->hasArgument("g") || args->hasArgument("x") || args->hasArgument("v")) {
			printf("Redundant arguments specified, please specify either s OR r OR v OR (x AND/OR g).\n");
			return false;
		}

		selectRandomMod();

		printf("Selected random mod: %s\n", m_selectedMod->getName());
		printf("\n");

		runSelectedMod();

		return true;
	}
	else if(args->hasArgument("v")) {
		if(args->hasArgument("g") || args->hasArgument("x")) {
			printf("Redundant arguments specified, please specify either s OR r OR v OR (x AND/OR g).\n");
			return false;
		}

		m_selectedMod = NULL;
		runSelectedMod();

		return true;
	}
	else if(args->hasArgument("g") || args->hasArgument("x")) {
		runSelectedMod(args);
		return true;
	}
	
	if(start) {
		run();
	}

	return true;
}

int ModManager::checkForUnlinkedModFiles() const {
	if(!m_initialized) { return false; }

	QMap<QString, int> linkedModFiles;

	QDir dir(SettingsManager::getInstance()->modsDirectoryName);
	QFileInfoList files = dir.entryInfoList();
	for(int i=0;i<files.size();i++) {
		if(files[i].isFile()) {
			linkedModFiles[files[i].fileName().toUpper()] = 0;
		}
	}

	int numberOfMultipleLinkedModFiles = 0;
	const ModVersion * modVersion = NULL;
	const char * fileName = NULL;
	QString key;
	for(int i=0;i<m_mods.numberOfMods();i++) {
		for(int j=0;j<m_mods.getMod(i)->numberOfVersions();j++) {

			fileName = m_mods.getMod(i)->getVersion(j)->getFileNameByType("grp");
			if(fileName != NULL) {
				key = QString(fileName).toUpper();
				if(linkedModFiles[key] == 1) {
					numberOfMultipleLinkedModFiles++;

					QByteArray keyBytes = key.toLocal8Bit();
					const char * keyData = keyBytes.data();

					printf("Mod file linked multiple times: \"%s\"\n", keyData);
				}
				linkedModFiles[key] = 1;
			}

			fileName = m_mods.getMod(i)->getVersion(j)->getFileNameByType("con");
			if(fileName != NULL) {
				key = QString(fileName).toUpper();
				if(linkedModFiles[key] == 1) {
					numberOfMultipleLinkedModFiles++;

					QByteArray keyBytes = key.toLocal8Bit();
					const char * keyData = keyBytes.data();

					printf("Mod file linked multiple times: \"%s\"\n", keyData);
				}
				linkedModFiles[key] = 1;
			}
		}
	}

	int numberOfUnlinkedModFiles = 0;
	QList<QString> keys = linkedModFiles.keys();
	for(int i=0;i<keys.size();i++) {
		if(linkedModFiles[keys[i]] == 0) {
			numberOfUnlinkedModFiles++;
			
			QByteArray fileNameBytes = keys[i].toLocal8Bit();
			const char * fileNameData = fileNameBytes.data();
			
			printf("Unlinked file %d: \"%s\"\n", numberOfUnlinkedModFiles, fileNameData);
		}
	}
	if(numberOfUnlinkedModFiles > 0) {
		printf("Found %d unlinked mod file%s in mods directory.\n", numberOfUnlinkedModFiles, numberOfUnlinkedModFiles == 1 ? "" : "s");
	}
	if(numberOfMultipleLinkedModFiles > 0) {
		printf("Found %d multiple linked mod file%s in mods directory.\n", numberOfMultipleLinkedModFiles, numberOfMultipleLinkedModFiles == 1 ? "" : "s");
	}

	return numberOfUnlinkedModFiles;
}

int ModManager::checkModForMissingFiles(const char * modName, int versionIndex) const {
	if(!m_initialized) { return 0; }

	if(modName == NULL || Utilities::stringLength(modName) == 0) { return 0; }
	
	const Mod * mod = m_mods.getMod(modName);
	if(mod == NULL) { return 0; }

	return checkModForMissingFiles(*mod, versionIndex);
}

int ModManager::checkModForMissingFiles(const QString & modName, int versionIndex) const {
	if(!m_initialized) { return 0; }

	const Mod * mod = m_mods.getMod(modName);
	if(mod == NULL) { return 0; }

	return checkModForMissingFiles(*mod, versionIndex);
}

int ModManager::checkModForMissingFiles(const Mod & mod, int versionIndex) const {
	if(!m_initialized || mod.numberOfVersions() == 0 || versionIndex >= mod.numberOfVersions()) { return 0; }

	int numberOfMissingFiles = 0;
	const char * fileName = NULL;

	for(int i=(versionIndex < 0 ? 0 : versionIndex);i<(versionIndex < 0 ? mod.numberOfVersions() : versionIndex + 1);i++) {

		fileName = mod.getVersion(i)->getFileNameByType("con");

		if(fileName != NULL) {
			QFileInfo con(QString("%1/%2").arg(SettingsManager::getInstance()->modsDirectoryName).arg(fileName));

			if(!con.isFile() || !con.exists()) {
				printf("Missing mod con file: %s\n", fileName);
				
				numberOfMissingFiles++;
			}
		}

		fileName = mod.getVersion(i)->getFileNameByType("grp");

		if(fileName != NULL) {
			QFileInfo group(QString("%1/%2").arg(SettingsManager::getInstance()->modsDirectoryName).arg(fileName));

			if(!group.isFile() || !group.exists()) {
				printf("Missing mod group file: %s\n", fileName);
				
				numberOfMissingFiles++;
			}
		}
	}

	return numberOfMissingFiles;
}

int ModManager::checkAllModsForMissingFiles() const {
	if(!m_initialized) { return 0; }
	
	int numberOfMissingModFiles = 0;

	for(int i=0;i<m_mods.numberOfMods();i++) {
		numberOfMissingModFiles += checkModForMissingFiles(*m_mods.getMod(i));
	}

	if(numberOfMissingModFiles > 0) {
		printf("Found %d missing mod file%s in mods directory.\n", numberOfMissingModFiles, numberOfMissingModFiles == 1 ? "" : "s");
	}

	return numberOfMissingModFiles;
}

int ModManager::checkForMissingExecutables() {
	int numberOfMissingExecutables = 0;

	QFileInfo DOSBoxExe(QString(SettingsManager::getInstance()->DOSBoxPath));
	QFileInfo gameExe(QString(SettingsManager::getInstance()->gameFileName));
	QFileInfo setupExe(QString(SettingsManager::getInstance()->setupFileName));
	QFileInfo kextractExe(QString(SettingsManager::getInstance()->kextractFileName));

	if(!DOSBoxExe.isFile() || !DOSBoxExe.exists()) {
		numberOfMissingExecutables++;
		printf("Missing DOSBox executable: %s\n", SettingsManager::getInstance()->DOSBoxPath);
	}

	if(!gameExe.isFile() || !gameExe.exists()) {
		numberOfMissingExecutables++;
		printf("Missing game executable: %s\n", SettingsManager::getInstance()->gameFileName);
	}

	if(!setupExe.isFile() || !setupExe.exists()) {
		numberOfMissingExecutables++;
		printf("Missing setup executable: %s\n", SettingsManager::getInstance()->setupFileName);
	}

	if(!kextractExe.isFile() || !kextractExe.exists()) {
		numberOfMissingExecutables++;
		printf("Missing kextract executable: %s\n", SettingsManager::getInstance()->kextractFileName);
	}

	return numberOfMissingExecutables++;
}

void ModManager::displayArgumentHelp() {
	printf("Duke Nukem 3D Mod Manager arguments:\n");
	printf(" -f \"Settings.ini\" - specifies an alternate settings file to use.\n");
	printf(" -m DOSBox/Windows - specifies mod manager mode, default: DOSBox.\n");
	printf(" -t Game/Setup/Client/Server - specifies game type, default: Game.\n");
	printf(" -ip 127.0.0.1 - specifies host ip address if running in client mode.\n");
	printf(" -g Mod.grp - manually specifies a group file to use.\n");
	printf(" -x Mod.con - manually specifies a game con file to use.\n");
	printf(" -s \"Mod Name\" - searches for and selects the matching mod, if there is one.\n");
	printf(" -r - randomly selects a mod to run.\n");
	printf(" -v - runs vanilla Duke Nukem 3D without any mods.\n");
	printf(" -? - displays this help message.\n");
}
