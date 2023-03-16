#include "ModManagerApplication.h"

#include "Logging/LogSinkWX.h"
#include "Manager/SettingsManager.h"
#include "Project.h"
#include "WXUtilities.h"

#include <Application/ComponentRegistry.h>
#include <LibraryInformation.h>
#include <Logging/LogSystem.h>

#include <fmt/core.h>
#include <magic_enum.hpp>
#include <png.h>
#include <spdlog/spdlog.h>
#include <wx/app.h>
#include <wx/cmdline.h>
#include <wx/progdlg.h>
#include <wx/version.h>

wxDECLARE_EVENT(EVENT_INITIALIZED, ModManagerInitializedEvent);
wxDECLARE_EVENT(EVENT_INITIALIZATION_CANCELLED, ModManagerInitializationCancelledEvent);
wxDECLARE_EVENT(EVENT_INITIALIZATION_FAILED, ModManagerInitializationFailedEvent);

class ModManagerInitializedEvent final : public wxEvent {
public:
	ModManagerInitializedEvent()
		: wxEvent(0, EVENT_INITIALIZED) { }

	virtual ~ModManagerInitializedEvent() { }

	virtual wxEvent * Clone() const override {
		return new ModManagerInitializedEvent(*this);
	}

	DECLARE_DYNAMIC_CLASS(ModManagerInitializedEvent);
};

IMPLEMENT_DYNAMIC_CLASS(ModManagerInitializedEvent, wxEvent);

class ModManagerInitializationCancelledEvent final : public wxEvent {
public:
	ModManagerInitializationCancelledEvent()
		: wxEvent(0, EVENT_INITIALIZATION_CANCELLED) { }

	virtual ~ModManagerInitializationCancelledEvent() { }

	virtual wxEvent * Clone() const override {
		return new ModManagerInitializationCancelledEvent(*this);
	}

	DECLARE_DYNAMIC_CLASS(ModManagerInitializationCancelledEvent);
};

IMPLEMENT_DYNAMIC_CLASS(ModManagerInitializationCancelledEvent, wxEvent);

class ModManagerInitializationFailedEvent final : public wxEvent {
public:
	ModManagerInitializationFailedEvent()
		: wxEvent(0, EVENT_INITIALIZATION_FAILED) { }

	virtual ~ModManagerInitializationFailedEvent() { }

	virtual wxEvent * Clone() const override {
		return new ModManagerInitializationFailedEvent(*this);
	}

	DECLARE_DYNAMIC_CLASS(ModManagerInitializationFailedEvent);
};

IMPLEMENT_DYNAMIC_CLASS(ModManagerInitializationFailedEvent, wxEvent);

wxDEFINE_EVENT(EVENT_INITIALIZED, ModManagerInitializedEvent);
wxDEFINE_EVENT(EVENT_INITIALIZATION_CANCELLED, ModManagerInitializationCancelledEvent);
wxDEFINE_EVENT(EVENT_INITIALIZATION_FAILED, ModManagerInitializationFailedEvent);

ModManagerApplication::ModManagerApplication()
	: m_modManagerFrame(new ModManagerFrame())
	, m_newModManagerFrame(nullptr)
	, m_reloadRequired(false) {
	SetAppDisplayName(APPLICATION_NAME);

	ComponentRegistry::getInstance().registerGlobalComponents();
}

ModManagerApplication::~ModManagerApplication() { }

void ModManagerApplication::initialize() {
	m_modManager = std::make_shared<ModManager>();
	m_logSinkWX = std::make_shared<LogSinkWX>();

	LibraryInformation * libraryInformation = LibraryInformation::getInstance();
	libraryInformation->addLibrary("LibPNG", PNG_LIBPNG_VER_STRING);
	libraryInformation->addLibrary("wxWidgets", fmt::format("{}.{}.{}.{}", wxMAJOR_VERSION, wxMINOR_VERSION, wxRELEASE_NUMBER, wxSUBRELEASE_NUMBER));

	LogSystem::getInstance()->addLogSink(m_logSinkWX);

	std::string baseInitializationMessage(fmt::format("{} is initializing, please wait...", APPLICATION_NAME));

	std::unique_ptr<wxProgressDialog> initializingProgressDialog(std::make_unique<wxProgressDialog>(
		"Initializing",
		baseInitializationMessage,
		m_modManager->numberOfInitializationSteps(),
		nullptr,
		wxPD_AUTO_HIDE | wxPD_CAN_ABORT
	));

	initializingProgressDialog->SetIcon(wxICON(D3DMODMGR_ICON));

	m_modManagerInitializationProgressConnection = m_modManager->initializationProgress.connect([&initializingProgressDialog, baseInitializationMessage](uint8_t initializationStep, uint8_t initializationStepCount, std::string description) {
		bool updateResult = initializingProgressDialog->Update(initializationStep, fmt::format("{}\n{}...", baseInitializationMessage, description));
		initializingProgressDialog->Fit();

		return updateResult;
	});

	m_modManagerInitializedConnection = m_modManager->initialized.connect([this]() {
		QueueEvent(new ModManagerInitializedEvent());
	});

	m_modManagerInitializationCancelledConnection = m_modManager->initializationCancelled.connect([this]() {
		QueueEvent(new ModManagerInitializationCancelledEvent());
	});

	m_modManagerInitializationFailedConnection = m_modManager->initializationFailed.connect([this]() {
		QueueEvent(new ModManagerInitializationFailedEvent());
	});

	Bind(EVENT_INITIALIZED, &ModManagerApplication::onInitialized, this);
	Bind(EVENT_INITIALIZATION_CANCELLED, &ModManagerApplication::onInitializationCancelled, this);
	Bind(EVENT_INITIALIZATION_FAILED, &ModManagerApplication::onInitializationFailed, this);

	m_modManager->initializeAsync(m_arguments);
}

void ModManagerApplication::reload() {
	m_reloadRequired = true;
	m_newModManagerFrame = new ModManagerFrame();
}

void ModManagerApplication::displayArgumentHelp() {
	wxMessageBox(ModManager::getArgumentHelpInfo(), "Argument Information", wxOK | wxICON_INFORMATION);
}

void ModManagerApplication::showWindow() {
	m_modManagerFrame->Bind(wxEVT_CLOSE_WINDOW, &ModManagerApplication::onFrameClosed, this);
	m_modManagerFrame->setListener(*this);
	m_modManagerFrame->initialize(m_modManager);
	m_modManagerFrame->Show(true);
	m_modManagerFrame->Raise();

	m_logSinkWX->initialize();
}

void ModManagerApplication::onInitialized(ModManagerInitializedEvent & event) {
	showWindow();
}

void ModManagerApplication::onInitializationCancelled(ModManagerInitializationCancelledEvent & event) {
	spdlog::error("{} initialization cancelled!", APPLICATION_NAME);

	m_modManagerFrame->Destroy();
}

void ModManagerApplication::onInitializationFailed(ModManagerInitializationFailedEvent & event) {
	spdlog::error("{} initialization failed!", APPLICATION_NAME);

	showWindow();
}

void ModManagerApplication::onFrameClosed(wxCloseEvent & event) {
	SettingsManager * settings = SettingsManager::getInstance();

	settings->windowPosition = WXUtilities::createPoint(m_modManagerFrame->GetPosition());
	settings->windowSize = WXUtilities::createDimension(m_modManagerFrame->GetSize());

	m_modManagerInitializationProgressConnection.disconnect();
	m_modManagerInitializedConnection.disconnect();
	m_modManagerInitializationCancelledConnection.disconnect();
	m_modManagerInitializationFailedConnection.disconnect();

	m_modManagerFrame->Destroy();

	if(m_reloadRequired) {
		m_reloadRequired = false;
		LogSystem::getInstance()->removeLogSink(m_logSinkWX);
		ComponentRegistry::getInstance().deleteAllComponents();
		m_modManagerFrame = m_newModManagerFrame;
		m_newModManagerFrame = nullptr;
		initialize();
	}
}

bool ModManagerApplication::OnInit() {
	if(wxAppConsole::argc != 0) {
		m_arguments = std::make_shared<ArgumentParser>(wxAppConsole::argc, wxAppConsole::argv);
	}

	if(m_arguments->hasArgument("?")) {
		displayArgumentHelp();
		return false;
	}

	initialize();

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

	ComponentRegistry::getInstance().deleteAllGlobalComponents();

	wxApp::CleanUp();
}

void ModManagerApplication::reloadRequested() {
	reload();
}

IMPLEMENT_APP(ModManagerApplication)
