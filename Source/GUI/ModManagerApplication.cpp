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

#include <google/vcdecoder.h>
#include <google/vcencoder.h>

/*
class OpenVCDiffByteBufferOutputStream final {
public:
	OpenVCDiffByteBufferOutputStream(std::shared_ptr<ByteBuffer> data)
		: m_data(data) {
		if(m_data == nullptr) {
			m_data = std::make_shared<ByteBuffer>();
		}
	}

	std::shared_ptr<ByteBuffer> getData() const {
		return m_data;
	}

	OpenVCDiffByteBufferOutputStream& append(const char * data, size_t size) {
		m_data->writeBytes(reinterpret_cast<const uint8_t *>(data), size);

		return *this;
	}

	void clear() {
		m_data->clear();
	}

	void push_back(char value) {
		m_data->writeByte(static_cast<uint8_t>(value));
	}

	void reserve(size_t numberOfBytes) {
		m_data->reserve(numberOfBytes);
	}

	size_t size() const {
		return m_data->getSize();
	}

private:
	std::shared_ptr<ByteBuffer> m_data;
};
*/

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

	//std::unique_ptr<ByteBuffer> dataA(ByteBuffer::readFrom("1.txt"));
	//std::unique_ptr<ByteBuffer> dataB(ByteBuffer::readFrom("2.txt"));

	//std::unique_ptr<ByteBuffer> dataA(ByteBuffer::readFrom("WORLD_TOUR.GRP"));
	//std::unique_ptr<ByteBuffer> dataB(ByteBuffer::readFrom("ATOMIC.GRP"));

	/*
	open_vcdiff::HashedDictionary dictionary(reinterpret_cast<const char *>(dataA->getRawData()), dataA->getSize(), false);

	if(!dictionary.Init()) {
		spdlog::error("Failed to initialize hashed dictionary.");
		return;
	}

	std::shared_ptr<ByteBuffer> diff(std::make_shared<ByteBuffer>());
	OpenVCDiffByteBufferOutputStream deltaInterface(diff);
	open_vcdiff::VCDiffStreamingEncoder encoder(&dictionary, open_vcdiff::VCD_FORMAT_INTERLEAVED, true); // VCD_FORMAT_INTERLEAVED / VCD_FORMAT_JSON

	if(!encoder.StartEncoding(&deltaInterface)) {
		spdlog::error("Failed to start encoding.");
		return;
	}

	if(!encoder.EncodeChunk(reinterpret_cast<const char *>(dataB->getRawData()), dataB->getSize(), &deltaInterface)) {
		spdlog::error("Failed to encode chunk.");
		return;
	}

	if(!encoder.FinishEncoding(&deltaInterface)) {
		spdlog::error("Failed to finish encoding.");
		return;
	}

	std::shared_ptr<ByteBuffer> output(std::make_shared<ByteBuffer>());
	OpenVCDiffByteBufferOutputStream outputInterface(output);
	open_vcdiff::VCDiffStreamingDecoder decoder;

	decoder.StartDecoding(reinterpret_cast<const char *>(dataA->getRawData()), dataA->getSize());

	if(!decoder.DecodeChunk(reinterpret_cast<const char *>(diff->getRawData()), diff->getSize(), &outputInterface)) {
		spdlog::error("Failed to decode chunk.");
		return;
	}

	if(!decoder.FinishDecoding()) {
		spdlog::error("Failed to finish decoding.");
		return;
	}
	*/

	//std::unique_ptr<ByteBuffer> diff(dataA->diff(*dataB));
	//std::unique_ptr<ByteBuffer> output(dataA->patch(*diff));

	//spdlog::info("DATA A SIZE: {}, SHA1: {}", dataA->getSize(), dataA->getSHA1());
	//spdlog::info("DATA B SIZE: {}, SHA1: {}", dataB->getSize(), dataB->getSHA1());
	//spdlog::info("DELTA  SIZE: {}, SHA1: {}", diff->getSize(), diff->getSHA1());
	//spdlog::info("OUTPUT SIZE: {}, SHA1: {}", output->getSize(), output->getSHA1());

/*
	std::string rawDelta;
	open_vcdiff::VCDiffEncoder encoder(reinterpret_cast<const char *>(dataA->getRawData()), dataA->getSize());
	encoder.SetFormatFlags(open_vcdiff::VCD_FORMAT_INTERLEAVED);
	encoder.Encode(reinterpret_cast<const char *>(dataB->getRawData()), dataB->getSize(), &rawDelta);

	std::string target;
	open_vcdiff::VCDiffDecoder decoder;
	decoder.Decode(reinterpret_cast<const char *>(dataA->getRawData()), dataA->getSize(), rawDelta, &target);
*/

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

	std::shared_ptr<GroupGRP> sourceGroup(atomicGroup);
	std::shared_ptr<GroupGRP> targetGroup(plutoniumGroup);

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

	// Atomic -> Plutonium
	{
		std::shared_ptr<GroupGRP> modifiedGroup(std::make_shared<GroupGRP>(*atomicGroup));

		std::shared_ptr<GroupFile> gameConGroupFile(modifiedGroup->getFileWithName("GAME.CON"));
		std::shared_ptr<GroupFile> userConGroupFile(modifiedGroup->getFileWithName("USER.CON"));
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

		std::vector<std::pair<std::string, std::string>> fileDiffs({
			std::make_pair("DEMO1.DMO", "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"),
			std::make_pair("DEMO2.DMO", "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"),
			std::make_pair("DEMO3.DMO", "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"),
			std::make_pair("DUKE3D.BIN", "1sPEUwABnyAAgWWfIAAAgV4Ac4JSAAE8RE5VTktORU4gTk5OVU5LTkVOTU4gTjNORE4gTi1OIE5BQlRCT0JNQklCQ0IgQkVCREJJQlRCSUJPQk5CE0aCekOORloCIFM5jkNjZj0CIDM5Y0NlPQNxIFM5Y2NlPQRxIH4zNWRDaTkBMSBwIHBGcEFwWHA6cCBwKHQ5dDd0MnQpdCB0MnQ3dDh0LXQ0dDZ0N3QwdCBwIHAgcCBTb4EaATIgdCB0U3B1cHBwcHBvcHJwdHA6cCBwKHQ5dDd0MnQpdCB0MnQ3dDh0LXQ1dDZ0NXQ1dGOGNIEh")
		});

		for(const std::pair<std::string, std::string> & fileDiff : fileDiffs) {
			std::shared_ptr<GroupFile> currentSourceGroupFile(modifiedGroup->getFileWithName(fileDiff.first));
			ByteBuffer & currentSourceGroupFileData = currentSourceGroupFile->getData();
			std::shared_ptr<GroupFile> currentTargetGroupFile(targetGroup->getFileWithName(fileDiff.first));
			ByteBuffer & currentTargetGroupFileData = currentTargetGroupFile->getData();

			std::unique_ptr<ByteBuffer> currentCalculatedSourceGroupFileDiff(currentSourceGroupFileData.diff(currentTargetGroupFileData));

			// spdlog::info("{}: {}", fileDiff.first, currentCalculatedSourceGroupFileDiff->toBase64());

			std::unique_ptr<ByteBuffer> currentSourceGroupFileDiff(ByteBuffer::fromBase64(fileDiff.second));
			currentSourceGroupFile->setData(currentSourceGroupFileData.patch(*currentSourceGroupFileDiff));
		}

		modifiedGroup->swapFilePositionsByName("DEMO1.DMO", "DEMO2.DMO");

		modifiedGroup->extractAllFiles("ATOMIC_TO_PLUTONIUM", true);

		spdlog::info("[PLUTONIUM]");

		for(size_t i = 0; i < atomicGroup->numberOfFiles(); i++) {
			spdlog::info("{} {}", i, atomicGroup->getFile(i)->getFileName());
		}

		spdlog::info("[ATOMIC]");

		for(size_t i = 0; i < plutoniumGroup->numberOfFiles(); i++) {
			spdlog::info("{} {}", i, plutoniumGroup->getFile(i)->getFileName());
		}
	}

	// World Tour -> Atomic

	//??:
	//regular -> atomic
	//atomic -> regular

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
