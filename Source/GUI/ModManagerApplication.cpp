#include "ModManagerApplication.h"

#include "Console/Logging/LogSinkWX.h"
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

#if !wxUSE_UNICODE || wxUSE_UNICODE_UTF8
	#define PCRE2_CODE_UNIT_WIDTH 8
		typedef char wxRegChar;
#elif wxUSE_UNICODE_UTF16
	#define PCRE2_CODE_UNIT_WIDTH 16
	typedef wchar_t wxRegChar;
#else
	#define PCRE2_CODE_UNIT_WIDTH 32
	typedef wchar_t wxRegChar;
#endif

#include <pcre2.h>

#include <future>

// #include <vld.h>

#define QUOTE(name) #name
#define STR(macro) QUOTE(macro)

wxDECLARE_EVENT(EVENT_INITIALIZATION_DONE, ModManagerInitializationDoneEvent);

class ModManagerInitializationDoneEvent final : public wxEvent {
public:
	ModManagerInitializationDoneEvent(bool success = true, bool aborted = false)
		: wxEvent(0, EVENT_INITIALIZATION_DONE)
		, m_success(success)
		, m_aborted(aborted) { }

	virtual ~ModManagerInitializationDoneEvent() { }

	virtual wxEvent * Clone() const override {
		return new ModManagerInitializationDoneEvent(*this);
	}

	bool wasSuccessful() const {
		return m_success;
	}

	bool wasAborted() const {
		return m_aborted;
	}

	DECLARE_DYNAMIC_CLASS(ModManagerInitializationDoneEvent);

private:
	bool m_success;
	bool m_aborted;
};

IMPLEMENT_DYNAMIC_CLASS(ModManagerInitializationDoneEvent, wxEvent);

wxDEFINE_EVENT(EVENT_INITIALIZATION_DONE, ModManagerInitializationDoneEvent);

static const std::string BASE_INITIALIZATION_MESSAGE(fmt::format("{} is initializing, please wait...", APPLICATION_NAME));

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
	libraryInformation->addLibrary("PCRE2", fmt::format("{}.{}", PCRE2_MAJOR, PCRE2_MINOR), STR(PCRE2_DATE));
	libraryInformation->addLibrary("wxWidgets", fmt::format("{}.{}.{}.{}", wxMAJOR_VERSION, wxMINOR_VERSION, wxRELEASE_NUMBER, wxSUBRELEASE_NUMBER));

	LogSystem::getInstance()->addLogSink(m_logSinkWX);

	SettingsManager * settings = SettingsManager::getInstance();

	settings->load(m_arguments.get());

	if(!settings->analyticsConfirmationAcknowledged) {
		int result = wxMessageBox("Do you consent to the collection of anonymous application usage statistics for the sole purpose of improving application functionality? This information is completely anonymous, non-intrusive, never shared, and extremely helpful to better understand if the application is used at all, what features are used most, or under utilized. Analytics can be enabled or disabled at any time from the application settings screen. Collected information includes:\n- Internal application state and version\n- Application configuration parameters\n- Generalized usage statistics\n- Hardware information\n- Operating system type and version\n- Locale and time zone\n- Coarse location accurate to city only", "Enable Analytics", wxYES_NO | wxCANCEL | wxICON_QUESTION, nullptr);

		if(result == wxYES) {
			settings->segmentAnalyticsEnabled = true;
		}
		else if(result == wxNO) {
			settings->segmentAnalyticsEnabled = false;
		}
		else if(result == wxCANCEL) {
			m_modManagerFrame->Close();
			return;
		}

		settings->analyticsConfirmationAcknowledged = true;
	}

	std::unique_ptr<wxProgressDialog> initializingProgressDialog(std::make_unique<wxProgressDialog>(
		"Initializing",
		BASE_INITIALIZATION_MESSAGE,
		m_modManager->numberOfInitializationSteps(),
		nullptr,
		wxPD_AUTO_HIDE | wxPD_CAN_ABORT
	));

	initializingProgressDialog->SetIcon(wxICON(D3DMODMGR_ICON));
	initializingProgressDialog->Fit();

	m_modManagerInitializationProgressConnection = m_modManager->initializationProgress.connect([this, &initializingProgressDialog](uint8_t initializationStep, uint8_t initializationStepCount, std::string description) {
		bool updateResult = initializingProgressDialog->Update(initializationStep, fmt::format("{}\n{}...", BASE_INITIALIZATION_MESSAGE, description));
		initializingProgressDialog->Fit();

		return updateResult;
	});

	Bind(EVENT_INITIALIZATION_DONE, &ModManagerApplication::onInitializationDone, this);

	bool aborted = false;

	std::future<bool> initializeFuture(std::async(std::launch::async, [this, &aborted]() mutable {
		return m_modManager->initialize(m_arguments, &aborted);
	}));

	initializeFuture.wait();

	bool initialized = initializeFuture.get();

	initializingProgressDialog = nullptr;

	if(aborted) {
		QueueEvent(new ModManagerInitializationDoneEvent(false, true));
	}
	else if(!initialized) {
		QueueEvent(new ModManagerInitializationDoneEvent(false, false));
	}
	else {
		QueueEvent(new ModManagerInitializationDoneEvent(true, true));
	}
}

void ModManagerApplication::reload() {
	m_reloadRequired = true;
	m_newModManagerFrame = new ModManagerFrame();
}

void ModManagerApplication::displayArgumentHelp() {
	wxMessageBox(ModManager::getArgumentHelpInfo(), "Argument Information", wxOK | wxICON_INFORMATION);
}

void ModManagerApplication::showWindow() {
	std::unique_ptr<wxProgressDialog> windowCreationProgressDialog(std::make_unique<wxProgressDialog>(
		"Initializing",
		BASE_INITIALIZATION_MESSAGE + "\nInitializing window...",
		m_modManager->numberOfInitializationSteps() + 2,
		nullptr,
		wxPD_AUTO_HIDE | wxPD_CAN_ABORT
	));

	windowCreationProgressDialog->SetIcon(wxICON(D3DMODMGR_ICON));
	windowCreationProgressDialog->Fit();
	windowCreationProgressDialog->Update(m_modManager->numberOfInitializationSteps(), windowCreationProgressDialog->GetMessage());

	m_modManagerFrameReloadRequestedConnection = m_modManagerFrame->reloadRequested.connect(std::bind(&ModManagerApplication::onReloadRequested, this));
	m_modManagerFrame->Bind(wxEVT_CLOSE_WINDOW, &ModManagerApplication::onFrameClosed, this);
	m_modManagerFrame->initialize(m_modManager);

	windowCreationProgressDialog->Update(windowCreationProgressDialog->GetValue() + 1, BASE_INITIALIZATION_MESSAGE + "\nApplication initialized!");
	windowCreationProgressDialog->Fit();

	m_modManagerFrame->Show(true);
	m_modManagerFrame->Raise();

	m_logSinkWX->initialize();

	windowCreationProgressDialog->Update(windowCreationProgressDialog->GetValue() + 1, windowCreationProgressDialog->GetMessage());
}

void ModManagerApplication::onInitializationDone(ModManagerInitializationDoneEvent & event) {
	if(event.wasSuccessful()) {
		showWindow();
	}
	else if(event.wasAborted()) {
		spdlog::error("{} initialization cancelled!", APPLICATION_NAME);

		m_modManagerFrame->Destroy();
	}
	else {
		spdlog::error("{} initialization failed!", APPLICATION_NAME);

		showWindow();
	}
}

void ModManagerApplication::onFrameClosed(wxCloseEvent & event) {
	SettingsManager * settings = SettingsManager::getInstance();

	settings->windowPosition = WXUtilities::createPoint(m_modManagerFrame->GetPosition());
	settings->windowSize = WXUtilities::createDimension(m_modManagerFrame->GetSize());

	m_modManagerInitializationProgressConnection.disconnect();
	m_modManagerFrameReloadRequestedConnection.disconnect();

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

void ModManagerApplication::onReloadRequested() {
	reload();
}

IMPLEMENT_APP(ModManagerApplication)
