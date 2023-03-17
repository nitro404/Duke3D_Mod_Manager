#include "DOSBoxVersionPanel.h"

#include "DOSBox/DOSBoxVersion.h"
#include "DOSBox/DOSBoxVersionCollection.h"
#include "Project.h"
#include "SettingPanel.h"
#include "WXUtilities.h"

#include <fmt/core.h>
#include <spdlog/spdlog.h>
#include <wx/gbsizer.h>
#include <wx/wrapsizer.h>

#include <sstream>

DOSBoxVersionPanel::DOSBoxVersionPanel(std::shared_ptr<DOSBoxVersion> dosboxVersion, wxWindow * parent, wxWindowID windowID, const wxPoint & position, const wxSize & size, long style)
	: wxPanel(parent, windowID, position, size, style, "DOSBox Version")
	, m_dosboxVersion(dosboxVersion != nullptr ? dosboxVersion : std::make_shared<DOSBoxVersion>())
	, m_dosboxPathSettingPanel(nullptr)
	, m_modified(false) {
	const DOSBoxVersion * defaultDOSBoxVersion = nullptr;

	for(const DOSBoxVersion * currentDefaultDOSBoxVersion : DOSBoxVersion::DEFAULT_DOSBOX_VERSIONS) {
		if(Utilities::areStringsEqualIgnoreCase(m_dosboxVersion->getName(), currentDefaultDOSBoxVersion->getName())) {
			defaultDOSBoxVersion = currentDefaultDOSBoxVersion;
			break;
		}
	}

	int border = 5;

	wxGridBagSizer * dosboxVersionConfigurationSizer = new wxGridBagSizer(border, border);

	wxWrapSizer * dosboxInformationSizer = new wxWrapSizer(wxHORIZONTAL);
	wxStaticBox * dosboxInformationBox = new wxStaticBox(this, wxID_ANY, "Information", wxDefaultPosition, wxDefaultSize, wxALIGN_LEFT, "Information");
	dosboxInformationBox->SetOwnFont(dosboxInformationBox->GetFont().MakeBold());

	wxPanel * dosboxInformationPanel = new wxPanel(dosboxInformationBox, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL);

	m_settingsPanels.push_back(SettingPanel::createStringSettingPanel<void>(std::bind(&DOSBoxVersion::getName, m_dosboxVersion.get()), std::bind(&DOSBoxVersion::setName, m_dosboxVersion.get(), std::placeholders::_1), defaultDOSBoxVersion != nullptr ? defaultDOSBoxVersion->getName() : "", "DOSBox Version Name", dosboxInformationPanel, dosboxInformationSizer, 1));
	m_settingsPanels.back()->setEditable(m_dosboxVersion->isRenamable());
	m_settingsPanels.push_back(SettingPanel::createStringSettingPanel<void>(std::bind(&DOSBoxVersion::getWebsite, m_dosboxVersion.get()), std::bind(&DOSBoxVersion::setWebsite, m_dosboxVersion.get(), std::placeholders::_1), defaultDOSBoxVersion != nullptr ? defaultDOSBoxVersion->getWebsite() : "", "Website", dosboxInformationPanel, dosboxInformationSizer));
	m_settingsPanels.push_back(SettingPanel::createStringSettingPanel<void>(std::bind(&DOSBoxVersion::getSourceCodeURL, m_dosboxVersion.get()), std::bind(&DOSBoxVersion::setSourceCodeURL, m_dosboxVersion.get(), std::placeholders::_1), defaultDOSBoxVersion != nullptr ? defaultDOSBoxVersion->getSourceCodeURL() : "", "Source Code URL", dosboxInformationPanel, dosboxInformationSizer));

	wxWrapSizer * dosboxConfigurationSizer = new wxWrapSizer(wxHORIZONTAL);
	wxStaticBox * dosboxConfigurationBox = new wxStaticBox(this, wxID_ANY, "Configuration", wxDefaultPosition, wxDefaultSize, wxALIGN_LEFT, "Configuration");
	dosboxConfigurationBox->SetOwnFont(dosboxConfigurationBox->GetFont().MakeBold());

	wxPanel * dosboxConfigurationPanel = new wxPanel(dosboxConfigurationBox, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL);

	m_settingsPanels.push_back(SettingPanel::createStringSettingPanel<void>(std::bind(&DOSBoxVersion::getDirectoryPath, m_dosboxVersion.get()), std::bind(&DOSBoxVersion::setDirectoryPath, m_dosboxVersion.get(), std::placeholders::_1), defaultDOSBoxVersion != nullptr ? defaultDOSBoxVersion->getDirectoryPath() : "", "DOSBox Path", dosboxConfigurationPanel, dosboxConfigurationSizer));
	m_dosboxPathSettingPanel = m_settingsPanels.back();
	m_settingsPanels.push_back(SettingPanel::createStringSettingPanel<void>(std::bind(&DOSBoxVersion::getExecutableName, m_dosboxVersion.get()), std::bind(&DOSBoxVersion::setExecutableName, m_dosboxVersion.get(), std::placeholders::_1), defaultDOSBoxVersion != nullptr ? defaultDOSBoxVersion->getExecutableName() : "", "DOSBox Executable Name", dosboxConfigurationPanel, dosboxConfigurationSizer, 1));

	wxStaticBox * supportedOperatingSystemsBox = new wxStaticBox(this, wxID_ANY, "Supported Operating Systems", wxDefaultPosition, wxDefaultSize, wxALIGN_LEFT, "Supported Operating Systems");
	supportedOperatingSystemsBox->SetOwnFont(supportedOperatingSystemsBox->GetFont().MakeBold());

	SettingPanel * supportedOperatingSystemsPanel = SettingPanel::createEnumMultiChoiceSettingPanel<DeviceInformationBridge::OperatingSystemType>(std::bind(&DOSBoxVersion::getSupportedOperatingSystems, m_dosboxVersion.get()), std::bind(&DOSBoxVersion::hasSupportedOperatingSystem, m_dosboxVersion.get(), std::placeholders::_1), std::bind(&DOSBoxVersion::addSupportedOperatingSystem, m_dosboxVersion.get(), std::placeholders::_1), std::bind(static_cast<bool(DOSBoxVersion::*)(DeviceInformationBridge::OperatingSystemType)>(&DOSBoxVersion::removeSupportedOperatingSystem), m_dosboxVersion.get(), std::placeholders::_1), "Supported Operating Systems", supportedOperatingSystemsBox, 1);
	m_settingsPanels.push_back(supportedOperatingSystemsPanel);

	dosboxInformationPanel->SetSizerAndFit(dosboxInformationSizer);
	dosboxConfigurationPanel->SetSizerAndFit(dosboxConfigurationSizer);

	wxBoxSizer * dosboxInformationBoxSizer = new wxBoxSizer(wxHORIZONTAL);
	dosboxInformationBoxSizer->Add(dosboxInformationPanel, 1, wxEXPAND | wxALL, 20);
	dosboxInformationBox->SetSizer(dosboxInformationBoxSizer);

	wxBoxSizer * dosboxConfigurationBoxSizer = new wxBoxSizer(wxHORIZONTAL);
	dosboxConfigurationBoxSizer->Add(dosboxConfigurationPanel, 1, wxEXPAND | wxALL, 20);
	dosboxConfigurationBox->SetSizer(dosboxConfigurationBoxSizer);

	wxBoxSizer * supportedOperatingSystemsBoxSizer = new wxBoxSizer(wxHORIZONTAL);
	supportedOperatingSystemsBoxSizer->Add(supportedOperatingSystemsPanel, 1, wxEXPAND | wxALL, 20);
	supportedOperatingSystemsBox->SetSizer(supportedOperatingSystemsBoxSizer);

	dosboxVersionConfigurationSizer->Add(dosboxInformationBox, wxGBPosition(0, 0), wxGBSpan(1, 1), wxEXPAND | wxALL, border);
	dosboxVersionConfigurationSizer->Add(dosboxConfigurationBox, wxGBPosition(1, 0), wxGBSpan(1, 1), wxEXPAND | wxALL, border);
	dosboxVersionConfigurationSizer->Add(supportedOperatingSystemsBox, wxGBPosition(2, 0), wxGBSpan(1, 1), wxEXPAND | wxALL, border);
	dosboxVersionConfigurationSizer->AddGrowableRow(0, 1);
	dosboxVersionConfigurationSizer->AddGrowableRow(1, 1);
	dosboxVersionConfigurationSizer->AddGrowableRow(2, 1);
	dosboxVersionConfigurationSizer->AddGrowableCol(0, 1);
	SetSizer(dosboxVersionConfigurationSizer);

	m_dosboxVersionModifiedConnection = m_dosboxVersion->modified.connect(std::bind(&DOSBoxVersionPanel::onDOSBoxVersionModified, this, std::placeholders::_1));

	for(SettingPanel * settingPanel : m_settingsPanels) {
		m_settingModifiedConnections.push_back(settingPanel->settingModified.connect(std::bind(&DOSBoxVersionPanel::onSettingModified, this, std::placeholders::_1)));
	}
}

DOSBoxVersionPanel::~DOSBoxVersionPanel() {
	m_dosboxVersionModifiedConnection.disconnect();

	for(boost::signals2::connection & settingModifiedConnection : m_settingModifiedConnections) {
		settingModifiedConnection.disconnect();
	}
}

bool DOSBoxVersionPanel::isModified() const {
	return m_modified;
}

std::string DOSBoxVersionPanel::getPanelName() const {
	return m_dosboxVersion->getName().empty() ? "NEW DOSBOX *" : fmt::format("{}{}", m_dosboxVersion->getName(), m_modified ? " *" : "");
}

std::shared_ptr<DOSBoxVersion> DOSBoxVersionPanel::getDOSBoxVersion() const {
	return m_dosboxVersion;
}

bool DOSBoxVersionPanel::isValid() const {
	for(const SettingPanel * settingPanel : m_settingsPanels) {
		if(!settingPanel->isValid()) {
			return false;
		}
	}

	return true;
}

bool DOSBoxVersionPanel::save() {
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
				"Failed to validate DOSBox version properties!\n"
				"\n"
				"The following dosbox {0} {1} invalid value{2}: {3}",
				numberOfInvalidSettings == 1 ? "property" : "properties",
				numberOfInvalidSettings == 1 ? "has an" : "have",
				numberOfInvalidSettings == 1 ? "" : "s",
				invalidSettingPanelNames.str()
			),
			"Invalid DOSBox Properties",
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

	notifyDOSBoxVersionSaved();

	return true;
}

void DOSBoxVersionPanel::discard() {
	for(SettingPanel * settingPanel : m_settingsPanels) {
		settingPanel->discard();
	}

	m_modified = false;

	notifyDOSBoxVersionChangesDiscarded();
}

void DOSBoxVersionPanel::reset() {
	for(SettingPanel * settingPanel : m_settingsPanels) {
		settingPanel->reset();
	}

	m_modified = true;

	notifyDOSBoxVersionReset();
}

void DOSBoxVersionPanel::discardDOSBoxPathChanges() {
	m_dosboxPathSettingPanel->discard();
}

void DOSBoxVersionPanel::update() {
	for(SettingPanel * settingPanel : m_settingsPanels) {
		settingPanel->update();
	}
}

DOSBoxVersionPanel::Listener::~Listener() { }

size_t DOSBoxVersionPanel::numberOfListeners() const {
	return m_listeners.size();
}

bool DOSBoxVersionPanel::hasListener(const Listener & listener) const {
	for(std::vector<Listener *>::const_iterator i = m_listeners.cbegin(); i != m_listeners.cend(); ++i) {
		if(*i == &listener) {
			return true;
		}
	}

	return false;
}

size_t DOSBoxVersionPanel::indexOfListener(const Listener & listener) const {
	for(size_t i = 0; i < m_listeners.size(); i++) {
		if(m_listeners[i] == &listener) {
			return i;
		}
	}

	return std::numeric_limits<size_t>::max();
}

DOSBoxVersionPanel::Listener * DOSBoxVersionPanel::getListener(size_t index) const {
	if(index >= m_listeners.size()) {
		return nullptr;
	}

	return m_listeners[index];
}

bool DOSBoxVersionPanel::addListener(Listener & listener) {
	if(!hasListener(listener)) {
		m_listeners.push_back(&listener);

		return true;
	}

	return false;
}

bool DOSBoxVersionPanel::removeListener(size_t index) {
	if(index >= m_listeners.size()) {
		return false;
	}

	m_listeners.erase(m_listeners.cbegin() + index);

	return true;
}

bool DOSBoxVersionPanel::removeListener(const Listener & listener) {
	for(std::vector<Listener *>::const_iterator i = m_listeners.cbegin(); i != m_listeners.cend(); ++i) {
		if(*i == &listener) {
			m_listeners.erase(i);

			return true;
		}
	}

	return false;
}

void DOSBoxVersionPanel::clearListeners() {
	m_listeners.clear();
}

void DOSBoxVersionPanel::notifyDOSBoxVersionSettingChanged(SettingPanel & settingPanel) {
	for(Listener * listener : m_listeners) {
		listener->dosboxVersionSettingChanged(*this, settingPanel);
	}
}

void DOSBoxVersionPanel::notifyDOSBoxVersionChangesDiscarded() {
	for(Listener * listener : m_listeners) {
		listener->dosboxVersionChangesDiscarded(*this);
	}
}

void DOSBoxVersionPanel::notifyDOSBoxVersionReset() {
	m_modified = true;

	for(Listener * listener : m_listeners) {
		listener->dosboxVersionReset(*this);
	}
}

void DOSBoxVersionPanel::notifyDOSBoxVersionSaved() {
	m_modified = false;

	for(Listener * listener : m_listeners) {
		listener->dosboxVersionSaved(*this);
	}
}

void DOSBoxVersionPanel::onDOSBoxVersionModified(DOSBoxVersion & dosboxVersion) {
	update();
}

void DOSBoxVersionPanel::onSettingModified(SettingPanel & settingPanel) {
	if(settingPanel.isModified()) {
		m_modified = true;

		notifyDOSBoxVersionSettingChanged(settingPanel);
	}
}
