#include "ModManagerFrame.h"

#include "ConsolePanel.h"
#include "ModBrowserPanel.h"
#include "Project.h"
#include "Manager/SettingsManager.h"
#include "WXUtilities.h"

#include <Core.h>
#include <LibraryInformation.h>

#include <spdlog/spdlog.h>
#include <wx/bookctrl.h>

ModManagerFrame::ModManagerFrame(std::shared_ptr<ModManager> modManager)
	: wxFrame(nullptr, wxID_ANY, APPLICATION_NAME, WXUtilities::createWXPoint(SettingsManager::getInstance()->windowPosition), WXUtilities::createWXSize(SettingsManager::getInstance()->windowSize))
#if wxUSE_MENUS
	, m_resetWindowPositionMenuItem(nullptr)
	, m_resetWindowSizeMenuItem(nullptr)
#endif // wxUSE_MENUS
{
	SetIcon(wxICON(D3DMODMGR_ICON));
	SetMinSize(WXUtilities::createWXSize(SettingsManager::MINIMUM_WINDOW_SIZE));

#if wxUSE_MENUS
	wxMenu * fileMenu = new wxMenu();
	fileMenu->Append(wxID_EXIT, "E&xit\tAlt-X", "Close the application");

	wxMenu * viewMenu = new wxMenu();
	m_resetWindowPositionMenuItem = new wxMenuItem(viewMenu, wxID_ANY, "Reset Window Position", "Resets the window position", wxITEM_NORMAL);
	viewMenu->Bind(wxEVT_MENU, &ModManagerFrame::onMenuBarItemPressed, this);
	m_resetWindowSizeMenuItem = new wxMenuItem(viewMenu, wxID_ANY, "Reset Window Size", "Resets the window size", wxITEM_NORMAL);
	viewMenu->Append(m_resetWindowPositionMenuItem);
	viewMenu->Append(m_resetWindowSizeMenuItem);

	wxMenu * helpMenu = new wxMenu();
	helpMenu->Append(wxID_ABOUT, "&About\tF1", "Show application information");

	wxMenuBar * menuBar = new wxMenuBar();
	menuBar->Append(fileMenu, "&File");
	menuBar->Append(viewMenu, "&View");
	menuBar->Append(helpMenu, "&Help");

	SetMenuBar(menuBar);
#endif // wxUSE_MENUS

	wxNotebook * notebook = new wxNotebook(this, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxNB_TOP, "Main");

	if(modManager->isInitialized()) {
		ModBrowserPanel * modBrowserPanel = new ModBrowserPanel(modManager, notebook, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL);
		notebook->AddPage(modBrowserPanel, "Mod Browser");
	}

	ConsolePanel * consolePanel = new ConsolePanel(notebook, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL);
	notebook->AddPage(consolePanel, "Console");

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
	}
}

ModManagerFrame::~ModManagerFrame() {
	SettingsManager * settings = SettingsManager::getInstance();

	settings->windowPosition = WXUtilities::createPoint(GetPosition());
	settings->windowSize = WXUtilities::createDimension(GetSize());
}

#if wxUSE_MENUS
void ModManagerFrame::onMenuBarItemPressed(wxCommandEvent & event) {
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
#endif // wxUSE_MENUS

void ModManagerFrame::onQuit(wxCommandEvent& WXUNUSED(event)) {
	Close(true);
}

void ModManagerFrame::onAbout(wxCommandEvent& WXUNUSED(event)) {
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

wxBEGIN_EVENT_TABLE(ModManagerFrame, wxFrame)
	EVT_MENU(wxID_EXIT, ModManagerFrame::onQuit)
	EVT_MENU(wxID_ABOUT, ModManagerFrame::onAbout)
wxEND_EVENT_TABLE()
