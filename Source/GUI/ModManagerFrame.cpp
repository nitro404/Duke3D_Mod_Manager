#include "ModManagerFrame.h"

#include "Console/ConsolePanel.h"
#include "DOSBox/DOSBoxManagerPanel.h"
#include "Game/GameManagerPanel.h"
#include "Group/GroupEditorPanel.h"
#include "Manager/ModManager.h"
#include "Manager/SettingsManager.h"
#include "Mod/ModBrowserPanel.h"
#include "Project.h"
#include "Releases/ReleaseNotesPanel.h"
#include "Settings/SettingsManagerPanel.h"
#include "WXUtilities.h"

#include <Analytics/Segment/SegmentAnalytics.h>
#include <Core.h>
#include <LibraryInformation.h>

#include <spdlog/spdlog.h>

ModManagerFrame::ModManagerFrame()
	: wxFrame(nullptr, wxID_ANY, APPLICATION_NAME, wxDefaultPosition, wxDefaultSize)
	, m_initialized(false)
#if wxUSE_MENUS
	, m_exitMenuItem(nullptr)
	, m_resetWindowPositionMenuItem(nullptr)
	, m_resetWindowSizeMenuItem(nullptr)
	, m_aboutMenuItem(nullptr)
#endif // wxUSE_MENUS
	, m_notebook(nullptr)
	, m_settingsManagerPanel(nullptr) {
#if defined(D3DMODMGR_ICON)
	SetIcon(wxICON(D3DMODMGR_ICON));
#endif // D3DMODMGR_ICON
}

ModManagerFrame::~ModManagerFrame() {
	m_settingsManagerPanelSignalConnectionGroup.disconnect();
}

bool ModManagerFrame::isInitialized() const {
	return m_initialized;
}

bool ModManagerFrame::initialize(std::shared_ptr<ModManager> modManager) {
	if(m_initialized) {
		return true;
	}

	if(modManager == nullptr) {
		return false;
	}

	Freeze();

#if wxUSE_MENUS
	wxMenu * fileMenu = new wxMenu();
	m_exitMenuItem = new wxMenuItem(fileMenu, wxID_EXIT, "E&xit", "Close the application", wxITEM_NORMAL);
	fileMenu->Append(m_exitMenuItem);
	fileMenu->Bind(wxEVT_MENU, &ModManagerFrame::onFileMenuItemPressed, this);

	wxMenu * viewMenu = new wxMenu();
	m_resetWindowPositionMenuItem = new wxMenuItem(viewMenu, wxID_ANY, "Reset Window Position", "Resets the window position", wxITEM_NORMAL);
	m_resetWindowSizeMenuItem = new wxMenuItem(viewMenu, wxID_ANY, "Reset Window Size", "Resets the window size", wxITEM_NORMAL);
	viewMenu->Append(m_resetWindowPositionMenuItem);
	viewMenu->Append(m_resetWindowSizeMenuItem);
	viewMenu->Bind(wxEVT_MENU, &ModManagerFrame::onViewMenuItemPressed, this);

	wxMenu * helpMenu = new wxMenu();
	m_aboutMenuItem = new wxMenuItem(helpMenu, wxID_ABOUT, "&About\tF1", "Show application information", wxITEM_NORMAL);
	helpMenu->Append(m_aboutMenuItem);
	helpMenu->Bind(wxEVT_MENU, &ModManagerFrame::onHelpMenuItemPressed, this);

	wxMenuBar * menuBar = new wxMenuBar();
	menuBar->Append(fileMenu, "&File");
	menuBar->Append(viewMenu, "&View");
	menuBar->Append(helpMenu, "&Help");

	SetMenuBar(menuBar);
#endif // wxUSE_MENUS

	SetPosition(WXUtilities::createWXPoint(SettingsManager::getInstance()->windowPosition));
	SetSize(WXUtilities::createWXSize(SettingsManager::getInstance()->windowSize));
	SetMinSize(WXUtilities::createWXSize(SettingsManager::MINIMUM_WINDOW_SIZE));

	m_notebook = new wxNotebook(this, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxNB_TOP, "Main");
	m_notebook->Bind(wxEVT_NOTEBOOK_PAGE_CHANGING, &ModManagerFrame::onNotebookPageChanging, this);
	m_notebook->Bind(wxEVT_NOTEBOOK_PAGE_CHANGED, &ModManagerFrame::onNotebookPageChanged, this);

	ModBrowserPanel * modBrowserPanel = nullptr;

	if(modManager->isInitialized()) {
		modBrowserPanel = new ModBrowserPanel(modManager, m_notebook, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL);
		m_notebook->AddPage(modBrowserPanel, "Mod Browser");

		DOSBoxManagerPanel * dosboxManagerPanel = new DOSBoxManagerPanel(modManager, m_notebook, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL);
		m_notebook->AddPage(dosboxManagerPanel, "DOSBox Manager");

		if(modManager->hasPreferredDOSBoxVersion()) {
			dosboxManagerPanel->selectPanelWithDOSBoxVersion(*modManager->getPreferredDOSBoxVersion());
		}

		GameManagerPanel * gameManagerPanel = new GameManagerPanel(modManager->getGameManager(), m_notebook, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL);
		m_notebook->AddPage(gameManagerPanel, "Game Manager");

		if(modManager->hasPreferredGameVersion()) {
			gameManagerPanel->selectPanelWithGameVersion(*modManager->getPreferredGameVersion());
		}
	}

	GroupEditorPanel * groupEditorPanel = new GroupEditorPanel(m_notebook, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL);
	m_notebook->AddPage(groupEditorPanel, "Group Editor");

	m_settingsManagerPanel = new SettingsManagerPanel(modManager, m_notebook, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL);
	m_notebook->AddPage(m_settingsManagerPanel, "Settings");

	m_settingsManagerPanelSignalConnectionGroup = SignalConnectionGroup(
		m_settingsManagerPanel->settingsReset.connect(std::bind(&ModManagerFrame::onSettingsReset, this)),
		m_settingsManagerPanel->settingsSaved.connect(std::bind(&ModManagerFrame::onSettingsSaved, this))
	);

	ReleaseNotesPanel * releaseNotesPanel = new ReleaseNotesPanel(m_notebook, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL);
	m_notebook->AddPage(releaseNotesPanel, "Release Notes");

	ConsolePanel * consolePanel = new ConsolePanel(m_notebook, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL);
	m_notebook->AddPage(consolePanel, "Console");

	Thaw();

	if(!modManager->isInitialized()) {
		wxMessageBox(
			fmt::format(
				"{} initialization failed!\n"
				"\n"
				"See console for details.",
				APPLICATION_NAME
			),
			"Initialization Failed",
			wxOK | wxICON_ERROR,
			this
		);

		return false;
	}
	else if(modManager->didArgumentHandlingFail()) {
		wxMessageBox(
			fmt::format(
				"{} command line argument handling failed!\n"
				"\n"
				"See console for details.",
				APPLICATION_NAME
			),
			"Argument Handling Failure",
			wxOK | wxICON_ERROR,
			this
		);

		wxMessageBox(ModManager::getArgumentHelpInfo(), "Argument Information", wxOK | wxICON_INFORMATION);
	}

	m_initialized = true;

	if(modBrowserPanel != nullptr && modManager->shouldRunSelectedMod()) {
		modBrowserPanel->launchGame();
	}

	return true;
}

#if wxUSE_MENUS

void ModManagerFrame::onFileMenuItemPressed(wxCommandEvent & event) {
	if(event.GetId() == m_exitMenuItem->GetId()) {
		Close();
	}
}

void ModManagerFrame::onViewMenuItemPressed(wxCommandEvent & event) {
	SettingsManager * settings = SettingsManager::getInstance();

	if(event.GetId() == m_resetWindowPositionMenuItem->GetId()) {
		settings->windowPosition.x = 0;
		settings->windowPosition.y = 0;
		SetPosition(WXUtilities::createWXPoint(settings->windowPosition));
	}
	else if(event.GetId() == m_resetWindowSizeMenuItem->GetId()) {
		settings->windowSize = SettingsManager::DEFAULT_WINDOW_SIZE;
		SetSize(WXUtilities::createWXSize(settings->windowSize));
	}
}

void ModManagerFrame::onHelpMenuItemPressed(wxCommandEvent & event) {
	if(event.GetId() == m_aboutMenuItem->GetId()) {
		wxMessageBox(
			fmt::format(
				"{} {} ({})\n"
				"Created by: Kevin Scroggins\n"
				"\n"
				"Library Information:\n"
				"{}",
				APPLICATION_NAME,
				APPLICATION_VERSION,
				APPLICATION_COMMIT_HASH,
				LibraryInformation::getInstance()->getLibraryInformationString()
			),
			"About",
			wxOK | wxICON_INFORMATION,
			this
		);
	}
}

#endif // wxUSE_MENUS

void ModManagerFrame::onNotebookPageChanging(wxBookCtrlEvent & event) {
	wxWindow * currentPage = m_notebook->GetPage(m_notebook->GetSelection());

	if(dynamic_cast<SettingsManagerPanel *>(currentPage) != nullptr) {
		SettingsManagerPanel * settingsManagerPanel = static_cast<SettingsManagerPanel *>(currentPage);

		if(!settingsManagerPanel->isModified()) {
			return;
		}

		int result = wxMessageBox("You have unsaved settings modifications.\nWould you like to save settings and re-load the application, or discard your changes?", "Save Settings", wxYES_NO | wxCANCEL | wxICON_INFORMATION, this);

		if(result == wxYES) {
			settingsManagerPanel->save();
			event.Veto();
		}
		else if(result == wxNO) {
			static_cast<SettingsManagerPanel *>(currentPage)->discard();
		}
		else if(result == wxCANCEL) {
			event.Veto();
		}
	}
}

void ModManagerFrame::onNotebookPageChanged(wxBookCtrlEvent & event) {
	wxWindow * currentPage = m_notebook->GetPage(m_notebook->GetSelection());

	if(dynamic_cast<SettingsManagerPanel *>(currentPage) != nullptr) {
		static_cast<SettingsManagerPanel *>(currentPage)->discard();
	}
	else if(dynamic_cast<ReleaseNotesPanel *>(currentPage) != nullptr) {
		static_cast<ReleaseNotesPanel *>(currentPage)->load();
	}

	if(SettingsManager::getInstance()->segmentAnalyticsEnabled) {
		SegmentAnalytics::getInstance()->screen(std::string(m_notebook->GetPageText(m_notebook->GetSelection()).mb_str()), "Main");
	}
}

void ModManagerFrame::onSettingsReset() {
	requestReload();
}

void ModManagerFrame::onSettingsSaved() {
	requestReload();
}

void ModManagerFrame::requestReload() {
	reloadRequested();

	Close();
}
