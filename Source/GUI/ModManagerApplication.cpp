#include "ModManagerApplication.h"

#include "Console/Logging/LogSinkWX.h"
#include "Manager/SettingsManager.h"
#include "Project.h"
#include "WXUtilities.h"

#include <Application/ComponentRegistry.h>
#include <LibraryInformation.h>
#include <Logging/LogSystem.h>
#include <Utilities/FileUtilities.h>

#include <expat.h>
#include <fmt/core.h>
#include <jdksmidi/version.h>
#include <jpeg/jversion.h>
#include <magic_enum/magic_enum.hpp>
#include <png.h>
#include <sndfile.h>
#include <spdlog/spdlog.h>
#include <tiffio.h>
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
#define TOSTRING(macro) QUOTE(macro)

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

#include "Game/File/Group/Group.h"
#include "Game/File/Group/GRP/GroupGRP.h"
#include "Game/File/GameFileFactoryRegistry.h"

#include <Utilities/CDIOUtilities.h>

#include <cdio/cdio.h>
#include <cdio/cd_types.h>
#include <cdio/iso9660.h>
#include <cdio/logging.h>
#include <cdio++/cdio.hpp>
#include <cdio++/iso9660.hpp>

void ModManagerApplication::initialize() {
	m_modManager = std::make_shared<ModManager>();
	m_logSinkWX = std::make_shared<LogSinkWX>();

	LibraryInformation * libraryInformation = LibraryInformation::getInstance();
	XML_Expat_Version expatVersion = XML_ExpatVersionInfo();
	libraryInformation->addLibrary("JDKSMIDI", jdksmidi::LibraryVersion);
	libraryInformation->addLibrary("LibExpat", fmt::format("{}.{}.{}", expatVersion.major, expatVersion.minor, expatVersion.micro));
	libraryInformation->addLibrary("LibJPEG", JVERSION);
	libraryInformation->addLibrary("LibPNG", PNG_LIBPNG_VER_STRING);
	libraryInformation->addLibrary("LibSndFile", sf_version_string());
	libraryInformation->addLibrary("LibTIFF", TIFFLIB_VERSION_STR_MAJ_MIN_MIC);
	libraryInformation->addLibrary("NanoSVG", NANOSVG_VERSION);
	libraryInformation->addLibrary("PCRE2", fmt::format("{}.{}", PCRE2_MAJOR, PCRE2_MINOR), TOSTRING(PCRE2_DATE));
	libraryInformation->addLibrary("WebP", WEBP_VERSION);
	libraryInformation->addLibrary("wxWidgets", fmt::format("{}.{}.{}.{}", wxMAJOR_VERSION, wxMINOR_VERSION, wxRELEASE_NUMBER, wxSUBRELEASE_NUMBER));

	LogSystem::getInstance()->addLogSink(m_logSinkWX);

	//const std::string cueFileName("Duke - Nuclear Winter (USA).cue");
	//const std::string cueFileName("Duke Caribbean - Life's a Beach (USA).cue");
	//const std::string cueFileName("Duke It Out in D.C. (USA).cue");
	const std::string cueFileName("");

	if(!cueFileName.empty()) {
		//const std::string directoryPath("/gamedata/");
		//const std::string directoryPath("/vacation/");
		const std::string directoryPath("/dukedc/");

		std::unique_ptr<ISO9660::FS> isoFileSystem(CDIOUtilities::readDiscImage(cueFileName));

		if(isoFileSystem == nullptr) {
			return;
		}

		std::unique_ptr<ISO9660::PVD> primaryVolumeDescriptor(isoFileSystem->read_pvd());

		if(primaryVolumeDescriptor) {
			spdlog::info("Application: {}", CDIOUtilities::getApplication(*primaryVolumeDescriptor));
			spdlog::info("Preparer: {}", CDIOUtilities::getPreparer(*primaryVolumeDescriptor));
			spdlog::info("Publisher: {}", CDIOUtilities::getPublisher(*primaryVolumeDescriptor));
			spdlog::info("ID: {}", CDIOUtilities::getID(*primaryVolumeDescriptor));
			spdlog::info("System: {}", CDIOUtilities::getSystem(*primaryVolumeDescriptor));
			spdlog::info("Volume: {}", CDIOUtilities::getVolume(*primaryVolumeDescriptor));
			spdlog::info("Volume Set: {}", CDIOUtilities::getVolumeSet(*primaryVolumeDescriptor));
		}

		primaryVolumeDescriptor.reset();

		bool error = false;

		std::vector<std::unique_ptr<ISO9660::Stat>> statistics(CDIOUtilities::getDirectoryContentsStatistics(*isoFileSystem, directoryPath, &error));

		if(error) {
			return;
		}

		for(const std::unique_ptr<ISO9660::Stat> & statistic : statistics) {
			spdlog::info("{} [LSN {}] {} {}",
				CDIOUtilities::isDirectory(*statistic) ? "d" : "f",
				CDIOUtilities::getFileStartLogicalSectorNumber(*statistic),
				CDIOUtilities::getFileSize(*statistic),
				Utilities::joinPaths(directoryPath, CDIOUtilities::getFileName(*statistic))
			);
		}

		statistics.clear();


		/*
		std::vector<std::string> rootContents(CDIOUtilities::getDirectoryContents(*isoFileSystem, "/gamedata/", &error));

		if(error) {
			return;
		}

		for(const std::string & entryPath : rootContents) {
			spdlog::debug("'{}' - Type: {} - LSN: {} - Size: {}", entryPath, CDIOUtilities::isFile(*isoFileSystem, entryPath) ? "F" : "D", CDIOUtilities::getFileStartLogicalSectorNumber(*isoFileSystem, entryPath), CDIOUtilities::getFileSize(*isoFileSystem, entryPath));
		}

		std::unique_ptr<ISO9660::Stat> tempStatistic(isoFileSystem->stat("/gamedata/nwuser.con", false));
		std::vector<uint8_t> tempData(CDIOUtilities::readFileTemp(*isoFileSystem, CDIOUtilities::getFileStartLogicalSectorNumber(*tempStatistic), CDIOUtilities::getFileSize(*tempStatistic)));

		spdlog::info("tempData.size: {}", tempData.size());
		*/

		if(cueFileName == "Duke - Nuclear Winter (USA).cue") {
			std::string groupFileName("nwinter.grp");

			std::unique_ptr<ByteBuffer> data(CDIOUtilities::readFile(*isoFileSystem, "/gamedata/" + groupFileName));

			if(data == nullptr) {
				return;
			}

			//data->writeTo(fileName);

			std::shared_ptr<Group> group(std::dynamic_pointer_cast<Group>(std::shared_ptr<GameFile>(GameFileFactoryRegistry::getInstance()->readGameFileFrom(*data, "GRP"))));

			if(group == nullptr) {
				spdlog::error("Failed to read group.");
				return;
			}

			group->extractAllFiles(std::string(Utilities::getFileNameNoExtension(groupFileName)));
		}
		else if(cueFileName == "Duke Caribbean - Life's a Beach (USA).cue") {
			std::array<std::string, 3> groupFileNames({ "vaca13.ssi", "vaca15.ssi", "vacapp.ssi" });

			for(const std::string & groupFileName : groupFileNames) {
				std::unique_ptr<ByteBuffer> data(CDIOUtilities::readFile(*isoFileSystem, "/vacation/" + groupFileName));

				if(data == nullptr) {
					return;
				}

				//data->writeTo(fileName);

				std::shared_ptr<Group> group(std::dynamic_pointer_cast<Group>(std::shared_ptr<GameFile>(GameFileFactoryRegistry::getInstance()->readGameFileFrom(*data, "SSI"))));

				if(group == nullptr) {
					spdlog::error("Failed to read group.");
					return;
				}

				group->reverseFileExtensions({ "MNA", "TRA", "NOC", "TAD", "OMD", "PRG", "PAM", "DIM", "COV", "VAW" });
				group->renameFile("GAME.CON", "VACATION.CON");
				group->renameFile("DEFS.CON", "VACATION.DEF");
				group->renameFile("USER.CON", "VACATION.USR");

				std::shared_ptr<GroupFile> gameConGroupFile(group->getFileWithName("VACATION.CON"));
				ByteBuffer & gameConGroupFileData = gameConGroupFile->getData();
				gameConGroupFileData.replaceInstanceOfStringFromStart("USER.CON", "VACATION.USR", true, 1, 0);
				gameConGroupFileData.replaceInstanceOfStringFromStart("DEFS.CON", "VACATION.DEF", true, 1, 0);

				// TODO: except GRP extension:

				group->extractAllFiles(std::string(Utilities::getFileNameNoExtension(groupFileName)), true);

				std::shared_ptr<GroupFile> vacationGroupFile(group->getFileWithName("VACATION.GRP"));

				if(vacationGroupFile != nullptr) {
					std::shared_ptr<Group> vacationGroup(std::dynamic_pointer_cast<Group>(std::shared_ptr<GameFile>(GameFileFactoryRegistry::getInstance()->readGameFileFrom(vacationGroupFile->getData(), "GRP"))));

					// TODO: except: KGROUP.EXE

					vacationGroup->extractAllFiles(Utilities::joinPaths(Utilities::getFileNameNoExtension(groupFileName), "grp"), true);
				}
			}
		}
		else if(cueFileName == "Duke It Out in D.C. (USA).cue") {
			std::array<std::string, 2> groupFileNames({ "dukedc13.ssi", "dukedcpp.ssi" });

			for(const std::string & groupFileName : groupFileNames) {
				std::unique_ptr<ByteBuffer> data(CDIOUtilities::readFile(*isoFileSystem, "/dukedc/" + groupFileName));

				if(data == nullptr) {
					return;
				}

				std::shared_ptr<Group> group(std::dynamic_pointer_cast<Group>(std::shared_ptr<GameFile>(GameFileFactoryRegistry::getInstance()->readGameFileFrom(*data, "SSI"))));

				if(group == nullptr) {
					spdlog::error("Failed to read group.");
					return;
				}

				group->renameFile("USER.CON", "DUKEDC.USR");

				// TODO: need GAME.CON file!:
				/*
				std::shared_ptr<GroupFile> gameConGroupFile(group->getFileWithName("DUKEDC.CON"));
				ByteBuffer & gameConGroupFileData = gameConGroupFile->getData();
				gameConGroupFileData.replaceInstanceOfStringFromStart("USER.CON", "DUKEDC.USR", true, 1, 0);
				*/

				group->extractAllFiles(std::string(Utilities::getFileNameNoExtension(groupFileName)), true);
			}
		}

		isoFileSystem.reset();
	}

	std::shared_ptr<GroupGRP> regularGroup(std::dynamic_pointer_cast<GroupGRP>(std::shared_ptr<GameFile>(GameFileFactoryRegistry::getInstance()->loadGameFileFrom("REGULAR.GRP"))));
	std::shared_ptr<GroupGRP> plutoniumGroup(std::dynamic_pointer_cast<GroupGRP>(std::shared_ptr<GameFile>(GameFileFactoryRegistry::getInstance()->loadGameFileFrom("PLUTONIUM.GRP"))));
	std::shared_ptr<GroupGRP> atomicGroup(std::dynamic_pointer_cast<GroupGRP>(std::shared_ptr<GameFile>(GameFileFactoryRegistry::getInstance()->loadGameFileFrom("ATOMIC.GRP"))));
	std::shared_ptr<GroupGRP> worldTourGroup(std::dynamic_pointer_cast<GroupGRP>(std::shared_ptr<GameFile>(GameFileFactoryRegistry::getInstance()->loadGameFileFrom("WORLD_TOUR.GRP"))));

	std::vector<std::shared_ptr<GroupGRP>> groups({
		regularGroup,
		plutoniumGroup,
		atomicGroup,
		worldTourGroup
	});

	for(const std::shared_ptr<GroupGRP> & group : groups) {
		if(group == nullptr) {
			spdlog::error("Failed to read group.");
			return;
		}

		group->extractAllFiles(std::string(Utilities::getFileNameNoExtension(group->getFileName())), false);
	}

	spdlog::info("All groups extracted.");

	std::string newLine("\r\n");

	{
		std::shared_ptr<GroupGRP> atomicToPlutoniumGroup(std::make_shared<GroupGRP>(*atomicGroup));

		std::shared_ptr<GroupFile> gameConGroupFile(atomicToPlutoniumGroup->getFileWithName("GAME.CON"));
		std::shared_ptr<GroupFile> userConGroupFile(atomicToPlutoniumGroup->getFileWithName("USER.CON"));
		ByteBuffer & gameConGroupFileData = gameConGroupFile->getData();
		ByteBuffer & userConGroupFileData = userConGroupFile->getData();
		gameConGroupFileData.replaceLineByNumber("    action BLIMPRESPAWNTIME", 476u, nullptr, newLine);
		gameConGroupFileData.replaceLineByNumber("    move 0", 477u, nullptr, newLine);
		gameConGroupFileData.replaceLineByNumber("    ifcount RESPAWNACTORTIME", 488u, nullptr, newLine);
		gameConGroupFileData.insertLineByNumber("      move 0", 492u, newLine);
		gameConGroupFileData.replaceLineByNumber("", 3301u, nullptr, newLine);
		gameConGroupFileData.replaceLineByNumber("      ifmove 1", 3372u, nullptr, newLine);
		gameConGroupFileData.replaceLineByNumber("          move 1", 3393u, nullptr, newLine);
		// TODO: broken, adds line to pos 0:
		//gameConGroupFileData.insertLineByNumber("", 8532u);
		gameConGroupFileData.insertLineByNumber("", 8531u, newLine); // TODO: this is a work-around
		userConGroupFileData.removeLineByNumber(86u);
		userConGroupFileData.removeLineByNumber(33u);
		userConGroupFileData.removeLineByNumber(32u);
		userConGroupFileData.insertLineByNumber("define SWEARFREQUENCY          100", 56u, newLine);
		//gameConGroupFileData.replaceInstanceOfStringFromStart("USER.CON", "VACATION.USR", true, 1, 0);

		atomicToPlutoniumGroup->extractAllFiles("ATOMIC_TO_PLUTONIUM", true);
	}

	// TODO:
	/*
	atomic -> plutonium
	world tour -> atomic
	regular -> atomic
	atomic -> regular
	// bonus:
	plutonium -> atomic
	atomic -> world tour
	plutonium -> atomic -> world tour
	world tour -> atomic -> plutonium
	plutonium -> atomic -> regular
	world tour -> atomic -> regular
	regular -> atomic -> plutonium
	regular -> atomic -> world tour
	*/

	// ------------------------------------------------------------------

	/*
	CdIo_t * cdio = cdio_open_bincue(cueFileName.data());

	if(cdio == nullptr) {
		return;
	}

	track_t first_track = cdio_get_first_track_num(cdio);
	track_t last_track = cdio_get_last_track_num(cdio);

	track_t data_track = 0;

	for (track_t t = first_track; t <= last_track; t++) {
		if (cdio_get_track_format(cdio, t) == TRACK_FORMAT_DATA) {
			data_track = t;
			break;
		}
	}

	if (!data_track) {
		spdlog::error("No data track found!");
		cdio_destroy(cdio);
		return;
	}

	spdlog::info("Using data track: {} with format: {}", data_track, magic_enum::enum_name(cdio_get_track_format(cdio, data_track)));

	lsn_t lsn = cdio_get_track_lsn(cdio, data_track);

	uint8_t buffer[2048];

	if (cdio_read_mode1_sector(cdio, buffer, lsn + 16, false) == 0) {
		if (memcmp(buffer + 1, "CD001", 5) == 0) {
			spdlog::info("Valid ISO9660 Primary Volume Descriptor found.");
		} else {
			spdlog::error("No ISO9660 signature at sector 16.");
		}
	}

	if (memcmp(buffer + 1, "CD001", 5) != 0) {
		spdlog::error("No ISO9660 PVD found at sector {}", lsn + 16);
		cdio_destroy(cdio);
		return;
	}

	spdlog::info("ISO9660 Primary Volume Descriptor found at sector {}", lsn + 16);

	cdio_destroy(cdio);

	// iso9660_t * iso = iso9660_open_ext(cueFileName.data(), ISO_EXTENSION_ALL);

	iso9660_t* iso = iso9660_open_fuzzy(cueFileName.data(), ISO_EXTENSION_ALL);

	//iso9660_t* iso = iso9660_open_fuzzy_ext(cueFileName.data(), ISO_EXTENSION_ALL, 16);

	if (!iso) {
		spdlog::error("Failed to open ISO9660 filesystem.");
		//cdio_destroy(cdio);
		iso9660_close(iso);
		return;
	}

	spdlog::info("iso open");

	CdioList_t *entries = iso9660_ifs_readdir(iso, "/");
	if (!entries) {
		spdlog::error("Failed to read root directory!");
		iso9660_close(iso);
		//cdio_destroy(cdio);
		iso9660_close(iso);
		return;
	}

	spdlog::info("ok");

	iso9660_close(iso);
	*/

	return;

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

	if(!settings->unsupportedSymlinksAcknowledged && !Utilities::areSymlinksSupported()) {
		int result = wxMessageBox("Symbolic links are not supported unless this application is launched as administrator. Everything will still work, but this means that mod files will need to be automatically copied to the game directory unnecessarily instead of creating links to them. If you want to avoid this and have faster mod launch times and less disk usage, consider re-launching as administrator!", "Symbolic Links Not Supported", wxOK | wxICON_INFORMATION, nullptr);

		if(result == wxOK) {
			settings->unsupportedSymlinksAcknowledged = true;
		}
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
