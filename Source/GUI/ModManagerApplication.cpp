#include "ModManagerApplication.h"

#include "Logging/LogSinkWX.h"
#include "Manager/ModManager.h"
#include "ModManagerFrame.h"
#include "Project.h"

#include <Arguments/ArgumentParser.h>
#include <LibraryInformation.h>
#include <Logging/LogSystem.h>

#include <fmt/core.h>
#include <png.h>
#include <spdlog/spdlog.h>
#include <wx/app.h>
#include <wx/cmdline.h>
#include <wx/progdlg.h>
#include <wx/version.h>

ModManagerApplication::ModManagerApplication()
	: m_modManager(std::make_shared<ModManager>())
	, m_logSinkWX(std::make_shared<LogSinkWX>()) {
	SetAppDisplayName(APPLICATION_NAME);
	LogSystem::getInstance()->addLogSink(m_logSinkWX);

	LibraryInformation * libraryInformation = LibraryInformation::getInstance();
	libraryInformation->addLibrary("LibPNG", PNG_LIBPNG_VER_STRING);
	libraryInformation->addLibrary("wxWidgets", fmt::format("{}.{}.{}.{}", wxMAJOR_VERSION, wxMINOR_VERSION, wxRELEASE_NUMBER, wxSUBRELEASE_NUMBER));
}

ModManagerApplication::~ModManagerApplication() { }

void ModManagerApplication::displayArgumentHelp() {
	wxMessageBox(ModManager::getArgumentHelpInfo(), "Argument Information", wxOK | wxICON_INFORMATION);
}

bool ModManagerApplication::OnInit() {
	std::shared_ptr<ArgumentParser> arguments;

	if(wxAppConsole::argc != 0) {
		arguments = std::make_shared<ArgumentParser>(wxAppConsole::argc, wxAppConsole::argv);
	}

	if(arguments->hasArgument("?")) {
		displayArgumentHelp();
		return false;
	}

	std::unique_ptr<wxProgressDialog> initializingProgressDialog(std::make_unique<wxProgressDialog>(
		"Initializing",
		fmt::format("{} is initializing, please wait...", APPLICATION_NAME),
		1,
		nullptr,
		wxPD_AUTO_HIDE
	));

	initializingProgressDialog->SetIcon(wxICON(D3DMODMGR_ICON));

	if(!m_modManager->initialize(arguments)) {
		spdlog::error("{} initialization failed!", APPLICATION_NAME);
	}

	ModManagerFrame * frame = new ModManagerFrame(m_modManager);
	frame->Show(true);

	m_logSinkWX->initialize();

	initializingProgressDialog->Update(1);

	return true;
}

int ModManagerApplication::OnExit() {
	LogSystem::getInstance()->removeLogSink(m_logSinkWX);

	m_modManager->uninitialize();

	return wxApp::OnExit();
}

void ModManagerApplication::CleanUp() {
	m_logSinkWX.reset();
	m_modManager.reset();

	wxApp::CleanUp();
}

IMPLEMENT_APP(ModManagerApplication)
