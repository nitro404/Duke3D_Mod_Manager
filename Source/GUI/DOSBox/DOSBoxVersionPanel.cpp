#include "DOSBoxVersionPanel.h"

#include "../SettingPanel.h"
#include "../WXUtilities.h"
#include "Configuration/DOSBoxConfigurationPanel.h"
#include "DOSBox/DOSBoxVersion.h"
#include "DOSBox/DOSBoxVersionCollection.h"
#include "Project.h"

#include <Utilities/TimeUtilities.h>

#include <boost/signals2.hpp>
#include <fmt/core.h>
#include <spdlog/spdlog.h>
#include <wx/gbsizer.h>
#include <wx/wrapsizer.h>

#include <sstream>

DOSBoxVersionPanel::DOSBoxVersionPanel(std::shared_ptr<DOSBoxVersion> dosboxVersion, wxWindow * parent, wxWindowID windowID, const wxPoint & position, const wxSize & size, long style)
	: wxPanel(parent, windowID, position, size, style, "DOSBox Version")
	, m_dosboxVersion(dosboxVersion != nullptr ? dosboxVersion : std::make_shared<DOSBoxVersion>())
	, m_installedText(nullptr)
	, m_lastLaunchedText(nullptr)
	, m_dosboxIDSettingPanel(nullptr)
	, m_dosboxPathSettingPanel(nullptr)
	, m_dosboxConfigurationBox(nullptr)
	, m_dosboxConfigurationPanel(nullptr)
	, m_modified(false) {
	const DOSBoxVersion * defaultDOSBoxVersion = nullptr;

	for(const DOSBoxVersion * currentDefaultDOSBoxVersion : DOSBoxVersion::DEFAULT_DOSBOX_VERSIONS) {
		if(Utilities::areStringsEqualIgnoreCase(m_dosboxVersion->getID(), currentDefaultDOSBoxVersion->getID())) {
			defaultDOSBoxVersion = currentDefaultDOSBoxVersion;
			break;
		}
	}

	int border = 5;

	wxWrapSizer * dosboxInformationSizer = new wxWrapSizer(wxHORIZONTAL);
	wxStaticBox * dosboxInformationBox = new wxStaticBox(this, wxID_ANY, "Information", wxDefaultPosition, wxDefaultSize, wxALIGN_LEFT, "Information");
	dosboxInformationBox->SetOwnFont(dosboxInformationBox->GetFont().MakeBold());

	wxPanel * dosboxInformationPanel = new wxPanel(dosboxInformationBox, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL);

	m_settingsPanels.push_back(SettingPanel::createStringSettingPanel<void>(std::bind(&DOSBoxVersion::getID, m_dosboxVersion.get()), std::bind(&DOSBoxVersion::setID, m_dosboxVersion.get(), std::placeholders::_1), defaultDOSBoxVersion != nullptr ? defaultDOSBoxVersion->getID() : "", "DOSBox Version Identifier", dosboxInformationPanel, dosboxInformationSizer, 1));
	m_dosboxIDSettingPanel = m_settingsPanels.back();
	m_dosboxIDSettingPanel->setEditable(!m_dosboxVersion->hasID());
	m_settingsPanels.push_back(SettingPanel::createStringSettingPanel<void>(std::bind(&DOSBoxVersion::getLongName, m_dosboxVersion.get()), std::bind(&DOSBoxVersion::setLongName, m_dosboxVersion.get(), std::placeholders::_1), defaultDOSBoxVersion != nullptr ? defaultDOSBoxVersion->getLongName() : "", "DOSBox Version Long Name", dosboxInformationPanel, dosboxInformationSizer, 1));
	m_settingsPanels.push_back(SettingPanel::createStringSettingPanel<void>(std::bind(&DOSBoxVersion::getShortName, m_dosboxVersion.get()), std::bind(&DOSBoxVersion::setShortName, m_dosboxVersion.get(), std::placeholders::_1), defaultDOSBoxVersion != nullptr ? defaultDOSBoxVersion->getShortName() : "", "DOSBox Version Short Name", dosboxInformationPanel, dosboxInformationSizer, 1));
	m_settingsPanels.push_back(SettingPanel::createStringSettingPanel<void>(std::bind(&DOSBoxVersion::getWebsite, m_dosboxVersion.get()), std::bind(&DOSBoxVersion::setWebsite, m_dosboxVersion.get(), std::placeholders::_1), defaultDOSBoxVersion != nullptr ? defaultDOSBoxVersion->getWebsite() : "", "Website", dosboxInformationPanel, dosboxInformationSizer));
	m_settingsPanels.push_back(SettingPanel::createStringSettingPanel<void>(std::bind(&DOSBoxVersion::getSourceCodeURL, m_dosboxVersion.get()), std::bind(&DOSBoxVersion::setSourceCodeURL, m_dosboxVersion.get(), std::placeholders::_1), defaultDOSBoxVersion != nullptr ? defaultDOSBoxVersion->getSourceCodeURL() : "", "Source Code URL", dosboxInformationPanel, dosboxInformationSizer));

	wxPanel * installedPanel = new wxPanel(dosboxInformationPanel);

	wxStaticText * installedLabel = new wxStaticText(installedPanel, wxID_ANY, "Installed", wxDefaultPosition, wxDefaultSize, wxALIGN_LEFT);
	installedLabel->SetFont(installedLabel->GetFont().MakeBold());
	m_installedText = new wxStaticText(installedPanel, wxID_ANY, m_dosboxVersion->hasInstalledTimePoint() ? Utilities::timePointToString(*m_dosboxVersion->getInstalledTimePoint()) : "N/A", wxDefaultPosition, wxDefaultSize, wxALIGN_LEFT);

	wxBoxSizer * installedBoxSizer = new wxBoxSizer(wxVERTICAL);
	installedBoxSizer->Add(installedLabel, 1, wxEXPAND | wxHORIZONTAL, 2);
	installedBoxSizer->Add(m_installedText, 1, wxEXPAND | wxHORIZONTAL, 2);
	installedPanel->SetSizer(installedBoxSizer);
	dosboxInformationSizer->Add(installedPanel, 1, wxEXPAND | wxALL, 5);

	wxPanel * lastLaunchedPanel = new wxPanel(dosboxInformationPanel);

	wxStaticText * lastLaunchedLabel = new wxStaticText(lastLaunchedPanel, wxID_ANY, "Last Launched", wxDefaultPosition, wxDefaultSize, wxALIGN_LEFT);
	lastLaunchedLabel->SetFont(lastLaunchedLabel->GetFont().MakeBold());
	m_lastLaunchedText = new wxStaticText(lastLaunchedPanel, wxID_ANY, m_dosboxVersion->hasLastLaunchedTimePoint() ? Utilities::timePointToString(*m_dosboxVersion->getLastLaunchedTimePoint()) : "N/A", wxDefaultPosition, wxDefaultSize, wxALIGN_LEFT);

	wxBoxSizer * lastLaunchedBoxSizer = new wxBoxSizer(wxVERTICAL);
	lastLaunchedBoxSizer->Add(lastLaunchedLabel, 1, wxEXPAND | wxHORIZONTAL, 2);
	lastLaunchedBoxSizer->Add(m_lastLaunchedText, 1, wxEXPAND | wxHORIZONTAL, 2);
	lastLaunchedPanel->SetSizer(lastLaunchedBoxSizer);
	dosboxInformationSizer->Add(lastLaunchedPanel, 1, wxEXPAND | wxALL, 5);

	wxWrapSizer * dosboxOptionsSizer = new wxWrapSizer(wxHORIZONTAL);
	wxStaticBox * dosboxOptionsBox = new wxStaticBox(this, wxID_ANY, "Options", wxDefaultPosition, wxDefaultSize, wxALIGN_LEFT, "Options");
	dosboxOptionsBox->SetOwnFont(dosboxOptionsBox->GetFont().MakeBold());

	wxPanel * dosboxOptionsPanel = new wxPanel(dosboxOptionsBox, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL);

	m_settingsPanels.push_back(SettingPanel::createStringSettingPanel<void>(std::bind(&DOSBoxVersion::getDirectoryPath, m_dosboxVersion.get()), std::bind(&DOSBoxVersion::setDirectoryPath, m_dosboxVersion.get(), std::placeholders::_1), defaultDOSBoxVersion != nullptr ? defaultDOSBoxVersion->getDirectoryPath() : "", "DOSBox Path", dosboxOptionsPanel, dosboxOptionsSizer));
	m_dosboxPathSettingPanel = m_settingsPanels.back();
	m_settingsPanels.push_back(SettingPanel::createStringSettingPanel<void>(std::bind(&DOSBoxVersion::getExecutableName, m_dosboxVersion.get()), std::bind(&DOSBoxVersion::setExecutableName, m_dosboxVersion.get(), std::placeholders::_1), defaultDOSBoxVersion != nullptr ? defaultDOSBoxVersion->getExecutableName() : "", "DOSBox Executable Name", dosboxOptionsPanel, dosboxOptionsSizer, 1));
	m_settingsPanels.push_back(SettingPanel::createStringSettingPanel<void>(std::bind(&DOSBoxVersion::getLaunchArguments, m_dosboxVersion.get()), std::bind(&DOSBoxVersion::setLaunchArguments, m_dosboxVersion.get(), std::placeholders::_1), Utilities::emptyString, "Launch Arguments", dosboxOptionsPanel, dosboxOptionsSizer));

	wxStaticBox * supportedOperatingSystemsBox = new wxStaticBox(this, wxID_ANY, "Supported Operating Systems", wxDefaultPosition, wxDefaultSize, wxALIGN_LEFT, "Supported Operating Systems");
	supportedOperatingSystemsBox->SetOwnFont(supportedOperatingSystemsBox->GetFont().MakeBold());

	m_dosboxConfigurationBox = new wxStaticBox(this, wxID_ANY, (m_dosboxVersion->hasLongName() ? m_dosboxVersion->getLongName() + " " : "") + "Configuration", wxDefaultPosition, wxDefaultSize, wxALIGN_LEFT, "DOSBox Configuration");
	m_dosboxConfigurationBox->SetOwnFont(m_dosboxConfigurationBox->GetFont().MakeBold());

	SettingPanel * supportedOperatingSystemsPanel = SettingPanel::createEnumMultiChoiceSettingPanel<DeviceInformationBridge::OperatingSystemType>(std::bind(&DOSBoxVersion::getSupportedOperatingSystems, m_dosboxVersion.get()), std::bind(&DOSBoxVersion::hasSupportedOperatingSystem, m_dosboxVersion.get(), std::placeholders::_1), std::bind(&DOSBoxVersion::addSupportedOperatingSystem, m_dosboxVersion.get(), std::placeholders::_1), std::bind(static_cast<bool(DOSBoxVersion::*)(DeviceInformationBridge::OperatingSystemType)>(&DOSBoxVersion::removeSupportedOperatingSystem), m_dosboxVersion.get(), std::placeholders::_1), "Supported Operating Systems", supportedOperatingSystemsBox, 1, nullptr, defaultDOSBoxVersion != nullptr ? defaultDOSBoxVersion->getSupportedOperatingSystems() : std::vector<DeviceInformationBridge::OperatingSystemType>());
	m_settingsPanels.push_back(supportedOperatingSystemsPanel);

	m_dosboxConfigurationPanel = new DOSBoxConfigurationPanel(m_dosboxVersion->getDOSBoxConfiguration(), m_dosboxConfigurationBox);

	dosboxInformationPanel->SetSizerAndFit(dosboxInformationSizer);
	dosboxOptionsPanel->SetSizerAndFit(dosboxOptionsSizer);

	wxBoxSizer * dosboxInformationBoxSizer = new wxBoxSizer(wxHORIZONTAL);
	dosboxInformationBoxSizer->Add(dosboxInformationPanel, 1, wxEXPAND | wxALL, 18);
	dosboxInformationBox->SetSizerAndFit(dosboxInformationBoxSizer);

	wxBoxSizer * dosboxOptionsBoxSizer = new wxBoxSizer(wxHORIZONTAL);
	dosboxOptionsBoxSizer->Add(dosboxOptionsPanel, 1, wxEXPAND | wxALL, 18);
	dosboxOptionsBox->SetSizerAndFit(dosboxOptionsBoxSizer);

	wxBoxSizer * supportedOperatingSystemsBoxSizer = new wxBoxSizer(wxHORIZONTAL);
	supportedOperatingSystemsBoxSizer->Add(supportedOperatingSystemsPanel, 1, wxEXPAND | wxALL, 18);
	supportedOperatingSystemsBox->SetSizerAndFit(supportedOperatingSystemsBoxSizer);

	wxBoxSizer * dosboxConfigurationBoxSizer = new wxBoxSizer(wxHORIZONTAL);
	dosboxConfigurationBoxSizer->Add(m_dosboxConfigurationPanel, 1, wxEXPAND | wxALL, 18);
	m_dosboxConfigurationBox->SetSizerAndFit(dosboxConfigurationBoxSizer);

	wxGridBagSizer * dosboxVersionSizer = new wxGridBagSizer(border, border);
	dosboxVersionSizer->Add(dosboxInformationBox, wxGBPosition(0, 0), wxGBSpan(1, 2), wxEXPAND | wxALL, border);
	dosboxVersionSizer->Add(dosboxOptionsBox, wxGBPosition(1, 0), wxGBSpan(1, 1), wxEXPAND | wxALL, border);
	dosboxVersionSizer->Add(supportedOperatingSystemsBox, wxGBPosition(1, 1), wxGBSpan(1, 1), wxEXPAND | wxALL, border);
	dosboxVersionSizer->Add(m_dosboxConfigurationBox, wxGBPosition(2, 0), wxGBSpan(1, 2), wxEXPAND | wxALL, border);
	dosboxVersionSizer->AddGrowableRow(0, 1);
	dosboxVersionSizer->AddGrowableRow(1, 1);
	dosboxVersionSizer->AddGrowableRow(2, 3);
	dosboxVersionSizer->AddGrowableCol(0, 3);
	dosboxVersionSizer->AddGrowableCol(1, 1);
	SetSizerAndFit(dosboxVersionSizer);

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
	return m_dosboxVersion->getShortName().empty() ? "NEW DOSBOX *" : fmt::format("{}{}", m_dosboxVersion->getShortName(), m_modified ? " *" : "");
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

	m_modified = false;

	m_dosboxIDSettingPanel->setEditable(false);

	dosboxVersionSaved(*this);

	return true;
}

void DOSBoxVersionPanel::discard() {
	for(SettingPanel * settingPanel : m_settingsPanels) {
		settingPanel->discard();
	}

	m_modified = false;

	dosboxVersionChangesDiscarded(*this);
}

void DOSBoxVersionPanel::reset() {
	for(SettingPanel * settingPanel : m_settingsPanels) {
		settingPanel->reset();
	}

	m_modified = true;

	dosboxVersionReset(*this);
}

void DOSBoxVersionPanel::discardDOSBoxPathChanges() {
	m_dosboxPathSettingPanel->discard();
}

void DOSBoxVersionPanel::update() {
	for(SettingPanel * settingPanel : m_settingsPanels) {
		settingPanel->update();
	}

	m_installedText->SetLabelText(m_dosboxVersion->hasInstalledTimePoint() ? Utilities::timePointToString(*m_dosboxVersion->getInstalledTimePoint()) : "N/A");
	m_lastLaunchedText->SetLabelText(m_dosboxVersion->hasLastLaunchedTimePoint() ? Utilities::timePointToString(*m_dosboxVersion->getLastLaunchedTimePoint()) : "N/A");

	m_dosboxConfigurationBox->SetLabelText((m_dosboxVersion->hasLongName() ? m_dosboxVersion->getLongName() + " " : "") + "Configuration");

	Layout();
}

void DOSBoxVersionPanel::onDOSBoxVersionModified(DOSBoxVersion & dosboxVersion) {
	update();
}

void DOSBoxVersionPanel::onSettingModified(SettingPanel & settingPanel) {
	if(settingPanel.isModified()) {
		m_modified = true;

		dosboxVersionSettingChanged(*this, settingPanel);
	}
}
