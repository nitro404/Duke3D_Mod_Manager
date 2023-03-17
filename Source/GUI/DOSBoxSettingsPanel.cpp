#include "DOSBoxSettingsPanel.h"

#include "DOSBox/DOSBoxManager.h"
#include "DOSBox/DOSBoxVersionCollection.h"
#include "Manager/SettingsManager.h"

#include <wx/wrapsizer.h>

#include <sstream>

DOSBoxSettingsPanel::DOSBoxSettingsPanel(std::shared_ptr<ModManager> modManager, wxWindow * parent, wxWindowID windowID, const wxPoint & position, const wxSize & size, long style, const std::string & title)
	: wxStaticBox(parent, windowID, title, position, size, style, title)
	, m_modManager(modManager)
	, m_preferredDOSBoxVersionSettingPanel(nullptr)
	, m_modified(false) {
	SettingsManager * settings = SettingsManager::getInstance();
	std::shared_ptr<DOSBoxVersionCollection> dosboxVersions(modManager->getDOSBoxVersions());

	SetOwnFont(GetFont().MakeBold());

	wxWrapSizer * dosboxSettingsSizer = new wxWrapSizer(wxHORIZONTAL);
	wxPanel * dosboxSettingsPanel = new wxPanel(this, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL);

	m_preferredDOSBoxVersionSettingPanel = SettingPanel::createStringChoiceSettingPanel([modManager = modManager.get()]() { return modManager->getPreferredDOSBoxVersion()->getName(); }, std::bind(static_cast<bool(ModManager::*)(const std::string &)>(&ModManager::setPreferredDOSBoxVersion), modManager.get(), std::placeholders::_1), SettingsManager::DEFAULT_PREFERRED_DOSBOX_VERSION, "Preferred DOSBox Version", dosboxVersions->getDOSBoxVersionDisplayNames(false), dosboxSettingsPanel, dosboxSettingsSizer);
	m_settingsPanels.push_back(m_preferredDOSBoxVersionSettingPanel);
	m_settingsPanels.push_back(SettingPanel::createStringSettingPanel(settings->dosboxVersionsListFilePath, SettingsManager::DEFAULT_DOSBOX_VERSIONS_LIST_FILE_PATH, "DOSBox Versions List File Path", dosboxSettingsPanel, dosboxSettingsSizer));
	m_settingsPanels.push_back(SettingPanel::createStringSettingPanel(settings->dosboxArguments, SettingsManager::DEFAULT_DOSBOX_ARGUMENTS, "Application Arguments", dosboxSettingsPanel, dosboxSettingsSizer));
	m_settingsPanels.push_back(SettingPanel::createBooleanSettingPanel(settings->dosboxAutoExit, SettingsManager::DEFAULT_DOSBOX_AUTO_EXIT, "Auto Exit", dosboxSettingsPanel, dosboxSettingsSizer));
	m_settingsPanels.push_back(SettingPanel::createStringSettingPanel(settings->dosboxDataDirectoryName, SettingsManager::DEFAULT_DOSBOX_DATA_DIRECTORY_NAME, "Data Directory Name", dosboxSettingsPanel, dosboxSettingsSizer));
	m_settingsPanels.push_back(SettingPanel::createStringSettingPanel<void>(std::bind(&ModManager::getDOSBoxServerIPAddress, modManager.get()), std::bind(&ModManager::setDOSBoxServerIPAddress, modManager.get(), std::placeholders::_1), SettingsManager::DEFAULT_DOSBOX_SERVER_IP_ADDRESS, "Server IP Address", dosboxSettingsPanel, dosboxSettingsSizer));
	m_settingsPanels.push_back(SettingPanel::createIntegerSettingPanel<uint16_t>(std::bind(&ModManager::getDOSBoxRemoteServerPort, modManager.get()), std::bind(&ModManager::setDOSBoxRemoteServerPort, modManager.get(), std::placeholders::_1), SettingsManager::DEFAULT_DOSBOX_REMOTE_SERVER_PORT, "Remote Server Port", dosboxSettingsPanel, dosboxSettingsSizer));
	m_settingsPanels.push_back(SettingPanel::createIntegerSettingPanel<uint16_t>(std::bind(&ModManager::getDOSBoxLocalServerPort, modManager.get()), std::bind(&ModManager::setDOSBoxLocalServerPort, modManager.get(), std::placeholders::_1), SettingsManager::DEFAULT_DOSBOX_LOCAL_SERVER_PORT, "Local Server Port", dosboxSettingsPanel, dosboxSettingsSizer));

	dosboxSettingsPanel->SetSizerAndFit(dosboxSettingsSizer);

	wxBoxSizer * dosboxSettingsBoxSizer = new wxBoxSizer(wxHORIZONTAL);
	dosboxSettingsBoxSizer->Add(dosboxSettingsPanel, 1, wxEXPAND | wxALL, 20);
	SetSizer(dosboxSettingsBoxSizer);

	m_preferredDOSBoxVersionChangedConnection = modManager->preferredDOSBoxVersionChanged.connect(std::bind(&DOSBoxSettingsPanel::onPreferredDOSBoxVersionChanged, this, std::placeholders::_1));
	m_dosboxServerIPAddressChangedConnection = modManager->dosboxServerIPAddressChanged.connect(std::bind(&DOSBoxSettingsPanel::onDOSBoxServerIPAddressChanged, this, std::placeholders::_1));
	m_dosboxLocalServerPortChangedConnection = modManager->dosboxLocalServerPortChanged.connect(std::bind(&DOSBoxSettingsPanel::onDOSBoxLocalServerPortChanged, this, std::placeholders::_1));
	m_dosboxRemoteServerPortChangedConnection = modManager->dosboxRemoteServerPortChanged.connect(std::bind(&DOSBoxSettingsPanel::onDOSBoxRemoteServerPortChanged, this, std::placeholders::_1));
	m_dosboxVersionCollectionSizeChangedConnection = dosboxVersions->sizeChanged.connect(std::bind(&DOSBoxSettingsPanel::onDOSBoxVersionCollectionSizeChanged, this, std::placeholders::_1));
	m_dosboxVersionCollectionItemModifiedConnection = dosboxVersions->itemModified.connect(std::bind(&DOSBoxSettingsPanel::onDOSBoxVersionCollectionItemModified, this, std::placeholders::_1, std::placeholders::_2));

	for(SettingPanel * settingPanel : m_settingsPanels) {
		settingPanel->addListener(*this);
	}
}

DOSBoxSettingsPanel::~DOSBoxSettingsPanel() {
	m_preferredDOSBoxVersionChangedConnection.disconnect();
	m_dosboxServerIPAddressChangedConnection.disconnect();
	m_dosboxLocalServerPortChangedConnection.disconnect();
	m_dosboxRemoteServerPortChangedConnection.disconnect();
	m_dosboxVersionCollectionSizeChangedConnection.disconnect();
	m_dosboxVersionCollectionItemModifiedConnection.disconnect();

	for(SettingPanel * settingPanel : m_settingsPanels) {
		settingPanel->removeListener(*this);
	}
}

bool DOSBoxSettingsPanel::areSettingsModified() const {
	for(const SettingPanel * settingPanel : m_settingsPanels) {
		if(settingPanel->isModified()) {
			return true;
		}
	}

	return false;
}

bool DOSBoxSettingsPanel::areSettingsValid() const {
	for(const SettingPanel * settingPanel : m_settingsPanels) {
		if(!settingPanel->isValid()) {
			return false;
		}
	}

	return true;
}

void DOSBoxSettingsPanel::updateSettings() {
	for(SettingPanel * settingPanel : m_settingsPanels) {
		settingPanel->update();
	}
}

bool DOSBoxSettingsPanel::saveSettings() {
	size_t numberOfInvalidSettings = 0;
	std::stringstream invalidSettingPanelNames;

	for(const SettingPanel * settingPanel : m_settingsPanels) {
		if(!settingPanel->isValid()) {
			numberOfInvalidSettings++;

			if(invalidSettingPanelNames.tellp() != 0) {
				invalidSettingPanelNames << ", ";
			}

			invalidSettingPanelNames << "'" << settingPanel->getName() << "'";
		}
	}

	if(numberOfInvalidSettings != 0) {
		wxMessageBox(
			fmt::format(
				"Failed to validate general DOSBox settings values!\n"
				"\n"
				"The following setting{0} {1} invalid value{0}: {2}",
				numberOfInvalidSettings == 1 ? "" : "s",
				numberOfInvalidSettings == 1 ? "has an" : "have",
				invalidSettingPanelNames.str()
			),
			"Invalid Settings",
			wxOK | wxICON_WARNING,
			this
		);

		return false;
	}

	for(SettingPanel * settingPanel : m_settingsPanels) {
		if(!settingPanel->save()) {
			return false;
		}
	}

	SettingsManager * settings = SettingsManager::getInstance();

	if(!settings->save()) {
		wxMessageBox("Failed to save settings!", "Save Failed", wxOK | wxICON_ERROR, this);
		return false;
	}

	return true;
}

void DOSBoxSettingsPanel::discardSettings() {
	for(SettingPanel * settingPanel : m_settingsPanels) {
		settingPanel->discard();
	}
}

void DOSBoxSettingsPanel::resetSettings() {
	for(SettingPanel * settingPanel : m_settingsPanels) {
		settingPanel->reset();
	}
}

DOSBoxSettingsPanel::Listener::~Listener() { }

size_t DOSBoxSettingsPanel::numberOfListeners() const {
	return m_listeners.size();
}

bool DOSBoxSettingsPanel::hasListener(const Listener & listener) const {
	for(std::vector<Listener *>::const_iterator i = m_listeners.cbegin(); i != m_listeners.cend(); ++i) {
		if(*i == &listener) {
			return true;
		}
	}

	return false;
}

size_t DOSBoxSettingsPanel::indexOfListener(const Listener & listener) const {
	for(size_t i = 0; i < m_listeners.size(); i++) {
		if(m_listeners[i] == &listener) {
			return i;
		}
	}

	return std::numeric_limits<size_t>::max();
}

DOSBoxSettingsPanel::Listener * DOSBoxSettingsPanel::getListener(size_t index) const {
	if(index >= m_listeners.size()) {
		return nullptr;
	}

	return m_listeners[index];
}

bool DOSBoxSettingsPanel::addListener(Listener & listener) {
	if(!hasListener(listener)) {
		m_listeners.push_back(&listener);

		return true;
	}

	return false;
}

bool DOSBoxSettingsPanel::removeListener(size_t index) {
	if(index >= m_listeners.size()) {
		return false;
	}

	m_listeners.erase(m_listeners.cbegin() + index);

	return true;
}

bool DOSBoxSettingsPanel::removeListener(const Listener & listener) {
	for(std::vector<Listener *>::const_iterator i = m_listeners.cbegin(); i != m_listeners.cend(); ++i) {
		if(*i == &listener) {
			m_listeners.erase(i);

			return true;
		}
	}

	return false;
}

void DOSBoxSettingsPanel::clearListeners() {
	m_listeners.clear();
}

void DOSBoxSettingsPanel::settingModified(SettingPanel & settingPanel) {
	for(Listener * listener : m_listeners) {
		listener->dosboxSettingModified(settingPanel);
	}
}

void DOSBoxSettingsPanel::onDOSBoxVersionCollectionSizeChanged(DOSBoxVersionCollection & dosboxVersionCollection) {
	m_preferredDOSBoxVersionSettingPanel->setChoices(dosboxVersionCollection.getDOSBoxVersionDisplayNames(false));
}

void DOSBoxSettingsPanel::onDOSBoxVersionCollectionItemModified(DOSBoxVersionCollection & dosboxVersionCollection, DOSBoxVersion & dosboxVersion) {
	m_preferredDOSBoxVersionSettingPanel->setChoices(dosboxVersionCollection.getDOSBoxVersionDisplayNames(false));
}

void DOSBoxSettingsPanel::onPreferredDOSBoxVersionChanged(std::shared_ptr<DOSBoxVersion> dosboxVersion) {
	updateSettings();
}

void DOSBoxSettingsPanel::onDOSBoxServerIPAddressChanged(std::string ipAddress) {
	updateSettings();
}

void DOSBoxSettingsPanel::onDOSBoxLocalServerPortChanged(uint16_t port) {
	updateSettings();
}

void DOSBoxSettingsPanel::onDOSBoxRemoteServerPortChanged(uint16_t port) {
	updateSettings();
}
