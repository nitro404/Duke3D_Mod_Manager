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


#if 0
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

	// Atomic -> Plutonium (DONE!)
	bool atomicToPlutonium = false;
	if(atomicToPlutonium) {
		std::shared_ptr<GroupGRP> sourceGroup(atomicGroup);
		std::shared_ptr<GroupGRP> targetGroup(plutoniumGroup);

		std::shared_ptr<GroupGRP> modifiedGroup(std::make_shared<GroupGRP>(*sourceGroup));

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
			// note: 16380 characters maximum per string
			std::make_pair("DEMO1.DMO", "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"),
			std::make_pair("DEMO2.DMO", "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"),
			std::make_pair("DEMO3.DMO", "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"),
			std::make_pair("DUKE3D.BIN", "KLUv/SDwLQYAckwtL2Br0wEwhVQQBADJsiqqaFgGmStqbd+8Qp6Lub5BlqwfXa3207O1a+KdGHGMkrYDBBlEI1669Milb8Ve9mLhfUPOQHaB6x98euSQP57w6dB9g0Of7l6wXoV9WMla6zWC4yiehBRlGBNAYTE6aAwAhb3oIj/URHwaB/1AWJqHkbEEUrOSV1ryyion6Xx5JSUraUmJWh9qrTRSK0XVOlEGddBqwWkNGYsWkbXmDZY1b5wGOTY6AgQAQlgNF4qQu3LNyhM=")
		});

		for(const std::pair<std::string, std::string> & fileDiff : fileDiffs) {
			std::shared_ptr<GroupFile> currentSourceGroupFile(modifiedGroup->getFileWithName(fileDiff.first));
			ByteBuffer & currentSourceGroupFileData = currentSourceGroupFile->getData();
			std::shared_ptr<GroupFile> currentTargetGroupFile(targetGroup->getFileWithName(fileDiff.first));
			ByteBuffer & currentTargetGroupFileData = currentTargetGroupFile->getData();

			std::unique_ptr<ByteBuffer> currentCompressedCalculatedSourceGroupFileDiff(currentSourceGroupFileData.diff(currentTargetGroupFileData)->compressed(ByteBuffer::CompressionMethod::ZStandard));

			//spdlog::info("{}: {}", fileDiff.first, currentCompressedCalculatedSourceGroupFileDiff->toBase64());

			std::unique_ptr<ByteBuffer> currentSourceGroupFileDiff(ByteBuffer::fromBase64(fileDiff.second)->decompressed(ByteBuffer::CompressionMethod::ZStandard));
			currentSourceGroupFile->setData(currentSourceGroupFileData.patch(*currentSourceGroupFileDiff));
		}

		modifiedGroup->swapFilePositionsByName("DEMO1.DMO", "DEMO2.DMO");

		modifiedGroup->extractAllFiles("ATOMIC_TO_PLUTONIUM", true);

		modifiedGroup->saveTo("ATOMIC_TO_PLUTONIUM.GRP");

		/*
		spdlog::info("[PLUTONIUM]");

		for(size_t i = 0; i < sourceGroup->numberOfFiles(); i++) {
			spdlog::info("{} {}", i, sourceGroup->getFile(i)->getFileName());
		}

		spdlog::info("[ATOMIC]");

		for(size_t i = 0; i < targetGroup->numberOfFiles(); i++) {
			spdlog::info("{} {}", i, targetGroup->getFile(i)->getFileName());
		}
		*/
	}

	// World Tour -> Atomic (WIP)
	bool worldTourToAtomic = true;
	if(worldTourToAtomic) {
		static const std::vector<std::string> ATOMIC_GROUP_FILE_ORDER({
			"LOGO.ANM",
			"CINEOV2.ANM",
			"CINEOV3.ANM",
			"RADLOGO.ANM",
			"DUKETEAM.ANM",
			"VOL41A.ANM",
			"VOL42A.ANM",
			"VOL43A.ANM",
			"VOL4E1.ANM",
			"VOL4E2.ANM",
			"VOL4E3.ANM",
			"DEFS.CON",
			"GAME.CON",
			"USER.CON",
			"D3DTIMBR.TMB",
			"DUKE3D.BIN",
			"LOOKUP.DAT",
			"PALETTE.DAT",
			"TABLES.DAT",
			"FLYBY.VOC",
			"SECRET.VOC",
			"BLANK.VOC",
			"ROAM06.VOC",
			"ROAM58.VOC",
			"PREDRG.VOC",
			"GBLASR01.VOC",
			"PREDPN.VOC",
			"PREDDY.VOC",
			"CHOKN12.VOC",
			"PREDRM.VOC",
			"LIZSPIT.VOC",
			"PIGRM.VOC",
			"ROAM29.VOC",
			"ROAM67.VOC",
			"PIGRG.VOC",
			"PIGPN.VOC",
			"PIGDY.VOC",
			"PIGWRN.VOC",
			"OCTARM.VOC",
			"OCTARG.VOC",
			"OCTAAT1.VOC",
			"OCTAAT2.VOC",
			"OCTAPN.VOC",
			"OCTADY.VOC",
			"BOS1RM.VOC",
			"BOS1RG.VOC",
			"BOS1PN.VOC",
			"BOS1DY.VOC",
			"KICKHIT.VOC",
			"RICOCHET.VOC",
			"BULITHIT.VOC",
			"PISTOL.VOC",
			"CLIPOUT.VOC",
			"CLIPIN.VOC",
			"CHAINGUN.VOC",
			"SHOTGNCK.VOC",
			"RPGFIRE.VOC",
			"BOMBEXPL.VOC",
			"PBOMBBNC.VOC",
			"WPNSEL21.VOC",
			"SHRINK.VOC",
			"LSRBMBPT.VOC",
			"LSRBMBWN.VOC",
			"SHRINKER.VOC",
			"VENTBUST.VOC",
			"GLASS.VOC",
			"GLASHEVY.VOC",
			"SHORTED.VOC",
			"SPLASH.VOC",
			"ALERT.VOC",
			"REACTOR.VOC",
			"SUBWAY.VOC",
			"GEARGRND.VOC",
			"GASP.VOC",
			"GASPS07.VOC",
			"PISSING.VOC",
			"KNUCKLE.VOC",
			"DRINK18.VOC",
			"EXERT.VOC",
			"HARTBEAT.VOC",
			"PAIN13.VOC",
			"PAIN28.VOC",
			"PAIN39.VOC",
			"PAIN87.VOC",
			"WETFEET.VOC",
			"LAND02.VOC",
			"DUCTWLK.VOC",
			"PAIN54.VOC",
			"PAIN75.VOC",
			"PAIN93.VOC",
			"PAIN68.VOC",
			"DAMN03.VOC",
			"DAMNIT04.VOC",
			"COMEON02.VOC",
			"WAITIN03.VOC",
			"COOL01.VOC",
			"AHMUCH03.VOC",
			"DANCE01.VOC",
			"LETSRK03.VOC",
			"READY2A.VOC",
			"RIPEM08.VOC",
			"ROCKIN02.VOC",
			"AHH04.VOC",
			"GULP01.VOC",
			"PAY02.VOC",
			"AMESS06.VOC",
			"BITCHN04.VOC",
			"DOOMED16.VOC",
			"HOLYCW01.VOC",
			"HOLYSH02.VOC",
			"IMGOOD12.VOC",
			"ONLYON03.VOC",
			"PIECE02.VOC",
			"RIDES09.VOC",
			"2RIDE06.VOC",
			"THSUK13A.VOC",
			"WANSOM4A.VOC",
			"MYSELF3A.VOC",
			"NEEDED03.VOC",
			"SHAKE2A.VOC",
			"DUKNUK14.VOC",
			"GETSOM1A.VOC",
			"GOTHRT01.VOC",
			"GROOVY02.VOC",
			"WHRSIT05.VOC",
			"BOOBY04.VOC",
			"DIESOB03.VOC",
			"DSCREM04.VOC",
			"LOOKIN01.VOC",
			"PISSIN01.VOC",
			"GETITM19.VOC",
			"SCUBA.VOC",
			"JETPAKON.VOC",
			"JETPAKI.VOC",
			"JETPAKOF.VOC",
			"GOGGLE12.VOC",
			"THUD.VOC",
			"SQUISH.VOC",
			"TELEPORT.VOC",
			"GBELEV01.VOC",
			"GBELEV02.VOC",
			"SWITCH.VOC",
			"FLUSH.VOC",
			"QUAKE.VOC",
			"MONITOR.VOC",
			"POOLBAL1.VOC",
			"ONBOARD.VOC",
			"BUBBLAMB.VOC",
			"MACHAMB.VOC",
			"WIND54.VOC",
			"STEAMHIS.VOC",
			"BARMUSIC.VOC",
			"WARAMB13.VOC",
			"WARAMB21.VOC",
			"WARAMB23.VOC",
			"WARAMB29.VOC",
			"COMPAMB.VOC",
			"SLIDOOR.VOC",
			"OPENDOOR.VOC",
			"EDOOR10.VOC",
			"EDOOR11.VOC",
			"FSCRM10.VOC",
			"H2OGRGL2.VOC",
			"GRIND.VOC",
			"ENGHUM.VOC",
			"LAVA06.VOC",
			"PHONBUSY.VOC",
			"ROAM22.VOC",
			"AMB81B.VOC",
			"ROAM98B.VOC",
			"H2ORUSH2.VOC",
			"PROJRUN.VOC",
			"FIRE09.VOC",
			"!PRISON.VOC",
			"!PIG.VOC",
			"!BOSS.VOC",
			"MICE3.VOC",
			"DRIP3.VOC",
			"ITEM15.VOC",
			"BONUS.VOC",
			"CATFIRE.VOC",
			"KILLME.VOC",
			"SHOTGUN7.VOC",
			"DMDEATH.VOC",
			"HAPPEN01.VOC",
			"DSCREM15.VOC",
			"DSCREM16.VOC",
			"DSCREM17.VOC",
			"DSCREM18.VOC",
			"DSCREM38.VOC",
			"RIP01.VOC",
			"NOBODY01.VOC",
			"CHEW05.VOC",
			"LETGOD01.VOC",
			"HAIL01.VOC",
			"BLOWIT01.VOC",
			"EATSHT01.VOC",
			"FACE01.VOC",
			"INHELL01.VOC",
			"SUKIT01.VOC",
			"PISSES01.VOC",
			"MUSTDIE.VOC",
			"DEFEATED.VOC",
			"MONOLITH.VOC",
			"HYDRO50.VOC",
			"ASWTCH23.VOC",
			"HYDRO22.VOC",
			"HYDRO24.VOC",
			"HYDRO27.VOC",
			"HYDRO34.VOC",
			"HYDRO40.VOC",
			"HYDRO43.VOC",
			"VAULT04.VOC",
			"2BWILD.VOC",
			"FREEZE.VOC",
			"B2ATK02.VOC",
			"B2ATK01.VOC",
			"B2DIE03.VOC",
			"B2PAIN03.VOC",
			"B2REC03.VOC",
			"B3ROAM01.VOC",
			"B3DIE03G.VOC",
			"B3REC03G.VOC",
			"B3PAIN04.VOC",
			"CTRLRM25.VOC",
			"HLIDLE03.VOC",
			"LIZSHIT3.VOC",
			"ADOOR1.VOC",
			"ADOOR2.VOC",
			"FORCE01.VOC",
			"JONES04.VOC",
			"QUAKE06.VOC",
			"TERMIN01.VOC",
			"BORN01.VOC",
			"CON03.VOC",
			"SNAKRM.VOC",
			"SNAKRG.VOC",
			"SNAKATA.VOC",
			"SNAKATB.VOC",
			"SNAKPN.VOC",
			"SNAKDY.VOC",
			"COMMRM.VOC",
			"COMMRG.VOC",
			"COMMAT.VOC",
			"COMMPN.VOC",
			"COMMDY.VOC",
			"COMMSP.VOC",
			"SLIMAT.VOC",
			"SLHTCH01.VOC",
			"SLIDIE03.VOC",
			"SLIREC06.VOC",
			"SLIROA02.VOC",
			"KTITX.VOC",
			"WHIPYU01.VOC",
			"GUNHIT2.VOC",
			"HEADRIP3.VOC",
			"BUCKLE.VOC",
			"JETP2.VOC",
			"ZIPPER2.VOC",
			"NEWS.VOC",
			"WHISTLE.VOC",
			"CHEER.VOC",
			"GMEOVR05.VOC",
			"GRABBAG.VOC",
			"NAME01.VOC",
			"R&R01.VOC",
			"LANI08.VOC",
			"LANI05.VOC",
			"LANIDUK2.VOC",
			"TYPING.VOC",
			"CLANG1.VOC",
			"INTRO4H1.VOC",
			"INTRO4H2.VOC",
			"INTROA.VOC",
			"INTROB.VOC",
			"INTROC.VOC",
			"SQUISH1A.VOC",
			"SBR1C.VOC",
			"BANG1.VOC",
			"SKIDCR1.VOC",
			"MUZAK028.VOC",
			"MUZAKDIE.VOC",
			"ENLARGE.VOC",
			"DOM03.VOC",
			"DOM09.VOC",
			"DOM11.VOC",
			"DOM12.VOC",
			"DOLPHIN.VOC",
			"DEEPFRY1.VOC",
			"DOGWHINE.VOC",
			"DOGYELP.VOC",
			"SWEET03.VOC",
			"SWEET04.VOC",
			"SWEET05.VOC",
			"SWEET16.VOC",
			"TRUMBLE.VOC",
			"TCLAP2A.VOC",
			"ALARM1A.VOC",
			"RAIN3A.VOC",
			"JEEP2A.VOC",
			"CDOOR1B.VOC",
			"WAVE1A.VOC",
			"EXSHOT3B.VOC",
			"SHORN1.VOC",
			"SBELL1A.VOC",
			"GRUN.VOC",
			"BRUN.VOC",
			"GSCORE.VOC",
			"BSCORE.VOC",
			"TANK3A.VOC",
			"BLROAM2A.VOC",
			"BLREC4B.VOC",
			"BLRIP1A.VOC",
			"BLRIP1B.VOC",
			"BLSPIT1A.VOC",
			"BLPAIN1B.VOC",
			"BLDIE3A.VOC",
			"BQROAM2A.VOC",
			"BQREC2A.VOC",
			"BQPAIN2A.VOC",
			"BQSHOCK3.VOC",
			"BQEGG1A.VOC",
			"BQDIE1A.VOC",
			"ABORT01.VOC",
			"KICK01-I.VOC",
			"AISLE402.VOC",
			"ANNOY03.VOC",
			"BOOKEM03.VOC",
			"CRY01.VOC",
			"DETRUCT2.VOC",
			"EAT08.VOC",
			"ESCAPE01.VOC",
			"MAKEDAY1.VOC",
			"SOHELP02.VOC",
			"VACATN01.VOC",
			"YIPPIE01.VOC",
			"YOHOHO01.VOC",
			"YOHOHO09.VOC",
			"INDPNC01.VOC",
			"MDEVL01.VOC",
			"MEAT04-N.VOC",
			"PARTY03.VOC",
			"POSTAL01.VOC",
			"SMACK02.VOC",
			"VOCAL02.VOC",
			"BEBACK01.VOC",
			"GETCRAP1.VOC",
			"GUYSUK01.VOC",
			"SLACKER1.VOC",
			"GOAWAY.VOC",
			"GRABBAG.MID",
			"STALKER.MID",
			"DETHTOLL.MID",
			"STREETS.MID",
			"WATRWLD1.MID",
			"SNAKE1.MID",
			"THECALL.MID",
			"STORM.MID",
			"FATCMDR.MID",
			"GUTWRNCH.MID",
			"NAMES.MID",
			"GLOOMY.MID",
			"FUTURMIL.MID",
			"INTENTS.MID",
			"INVADER.MID",
			"SUBWAY.MID",
			"AHGEEZ.MID",
			"233C.MID",
			"ALFREDH.MID",
			"ALIENZ.MID",
			"GOTHAM.MID",
			"INHIDING.MID",
			"LORDOFLA.MID",
			"PIZZED.MID",
			"ROBOCREP.MID",
			"SPOOK.MID",
			"STALAG.MID",
			"URBAN.MID",
			"WHOMP.MID",
			"XPLASMA.MID",
			"BRIEFING.MID",
			"MISSIMP.MID",
			"WAREHAUS.MID",
			"BAKEDGDS.MID",
			"PREPD.MID",
			"CF.MID",
			"LEMCHILL.MID",
			"LAYERS.MID",
			"FLOGHORN.MID",
			"POB.MID",
			"DEPART.MID",
			"RESTRICT.MID",
			"TILES000.ART",
			"TILES001.ART",
			"TILES002.ART",
			"TILES003.ART",
			"TILES004.ART",
			"TILES005.ART",
			"TILES006.ART",
			"TILES007.ART",
			"TILES008.ART",
			"TILES009.ART",
			"TILES010.ART",
			"TILES011.ART",
			"TILES012.ART",
			"TILES013.ART",
			"TILES014.ART",
			"TILES015.ART",
			"TILES016.ART",
			"TILES017.ART",
			"TILES018.ART",
			"TILES019.ART",
			"E1L3.MAP",
			"E1L4.MAP",
			"E1L5.MAP",
			"E1L6.MAP",
			"E1L7.MAP",
			"E2L1.MAP",
			"E2L2.MAP",
			"E2L3.MAP",
			"E2L4.MAP",
			"E2L5.MAP",
			"E2L6.MAP",
			"E2L7.MAP",
			"E2L8.MAP",
			"E2L9.MAP",
			"E2L10.MAP",
			"E2L11.MAP",
			"E3L1.MAP",
			"E3L2.MAP",
			"E3L3.MAP",
			"E3L4.MAP",
			"E3L5.MAP",
			"E3L6.MAP",
			"E3L7.MAP",
			"E3L8.MAP",
			"E3L9.MAP",
			"E3L10.MAP",
			"E1L1.MAP",
			"E1L2.MAP",
			"E1L8.MAP",
			"E3L11.MAP",
			"E4L1.MAP",
			"E4L2.MAP",
			"E4L3.MAP",
			"E4L4.MAP",
			"E4L5.MAP",
			"E4L6.MAP",
			"E4L7.MAP",
			"E4L8.MAP",
			"E4L9.MAP",
			"E4L10.MAP",
			"E4L11.MAP",
			"DEMO2.DMO",
			"DEMO1.DMO",
			"DEMO3.DMO"
		});

		std::shared_ptr<GroupGRP> sourceGroup(worldTourGroup);
		std::shared_ptr<GroupGRP> targetGroup(atomicGroup);

		std::shared_ptr<GroupGRP> modifiedGroup(std::make_shared<GroupGRP>(*sourceGroup));

		std::vector<std::pair<std::string, std::string>> fileDiffs({
			std::make_pair("TILES000.ART", "1sPEUwAB69k6AIxv69k6AACMZwBziHMADoAAAvmAAAD8AAAAAIATS4kAAoBDL0wCgFNXNAKAIyMEAgAjKIFAY1OBIAKAgyMKARkAAAAAAAAAAAAAAAAAAAAAAAAAAIAAAACAUzuBEAEZgAAAAAAAAACAAAABgAAAAIAAAACAAAAAAGMnFJM8AAGBPYAAAACAAAAAgAAAAAAAAACAAAAAgAAAAIAAAAAAAAAAAAAAAIAAAAAAAAAAAAAAAIAAAACAAAAAgAAAAIAAAACAAAAAAAAAAACCAAAFAAAAAAABAQAAAACAAAAAgAAAAIAAAACAAAAAgAAAAIAAAAAAAAAAgAAAAIAAAACAAAECgAABBIAAAQqAAAABgAAAAIAAAQAAAAYCgAACB4AABvyAAAAAAAAAAIAAAACAAAAAgAAAAIAAAgAAAAAAgDMntedQQySCFwKAU4LmJ4VwAYFDGRgZGBkXGBYL/xAbGhkaGRgYGBkYGBcWC/8QGxoYGhiNjYwYGBgYFgz/EBsZGhkYjYyNGBkYFxYL/w0bGRkaGYyMjBgYFxgWDA0KGxqNjYyNjI2MjIsXFgwRChsZjYyMjYyMjIuLGBYMDQobGoyNjIyMjIuMixgVC/8NGxkZGBiMi4wYGBgXFgz/EBsZGBgZjIyLGBcYFxUL/w8bGRkZGIyLixgYFxgWDP8PGxkYGBgYGBgYFxgXFQv/EBsYGBgYGBcXQ4ECgudqAYJ9AwMDAgMCAgIC//////8DBQQEBAQEBAQEBAQEBAUEBAQEAwICAv////8FBwcGDAsLCwwFBgYGBgUFBQQEBAMCAv///wUGCAsKDQ4PDw4JCQkIiomIBwYFBAQCAgMD/wYIDBIPEwsJDBILCwsLiouJCQcGBQQDAgYFpgcIDA4OFgsODxgKCgoLiouKCQgHBgUDAgoIBQYHCw8NFRAKDBMIiomKjI6NiomIBgUDAg0LBQYICg8PFxQLEBMKi4qMjoyNi4mIBgUDAg8MBQYICg8OFxQLEBMMjIyLiouKiYiIBgYEAgsJBAcLCg4NFxEMEhQNEBAPjIuLCwoHBgYDAgQE/wgQDBERFQ4QFRQQExMUjY2MDQsIBwUDAv///wcYERQTGBISFRgWGBgajo6PDwwKCAUDAv////8PFBgUGBgWGBgXFhUUEg8ODQwJBgQCAv////8JDhETExMTFBUVFBMREA4NDAsIBgQCAv//////CQsLDQ0ODQwNDQwMDAwKU4U2g38GFBITExNjIoU7CBMTExMTEREzISkIFBISExITEkMgKAGFCRMUFBITExITEhEREBARERAQEA4MCv//GRkWEhMQHBoYFRUUFBUTFBQTEhMSExISEhESEhIRERARERAODAr//xoUEv//ExwaGBaLi4uLixQUFBITiYqKihIRERISERAQNYgRDgwK/wYEAwL///8dGhgWFRUVFBUTExMUEoqJiYoSERISERE1iIkREA4MCv8GBAMC////HRoYFIuLi4qLFBMUFBOKiomKExITEhGJiBGJEBEODAr/BgQDAv///x0aGBWLFYsVExQTExMSioqKiRETExIRETWIiRAQDgwK/wYEAwL///8cGhgUixWLixMUioqJiouKiomJiYmJEBEREDWJEQ4MCv8GBAMC////HBoYFTeLNxSKFYuKi4mJiYqJiomJiRIREREQERAODAr/BgQDAv///xwaGBUVFBQVExSLioqLiomKiYqJiYkRiYmJiIkQDgwK/wYEAwL///8cGhgWN4o3FDcTioqLi4uKiYmJiYmIEBERERAQEQ4MCv8GBAMC////HBoYFosUixSLFBQTExSJiYmJExISERGIiYmIiBAODAr/BgQDAv///x4aGBWLFIsUihMTExMSioqJihEREhERiREQEIgQDgwK/wYEAwL///8eGhgVixWLFIsUFBMTE4qJiYkSEhISEokRERCJEA4MCv8GBAMC////HhoYFDcUN4s3ExQTFBOKiYmKExMSEhIRiYmJEBEODAr//xwXEv//Ex4aGBWLFBQUExMTFBQUFBQTExMRERISEREQEBEQDgwK//8ZGhUSExAcGhgVixUUFBUUExMUExITEhMTEhERERIREBAREA4MCv//ExgaFRERHhoYFYuLi4uLExMUEhIUEhMTExJTIIUpCBQTExISExNjIScIFBISEhMSETMiKAYTExITEkPo0TYn"),
			std::make_pair("TILES001.ART", "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"),
			// TODO: diff is too big:
			//std::make_pair("TILES009.ART", ""),
			//std::make_pair("TILES012.ART", "")
		});

// 01:03:59.621 [25320] I: TILES009.ART: 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goKCgYGBgWCBgYGBgYGCgVVfXFIkJSUlJSgsLF9BQkJCQV9fViotKy0rLzAcZzAwJjMogQIBHWBgYKKjZGVnaGprbGtoZGcDYGBgYGBgYGBgYGBgQyhFAYEgoYGhgoWEg4OBoaGhoWBgYGBgYGBgYGBgYGBgpGhoBWBqcGpnYGBgYGBggYKBgYGBgYGBgYGBgYGBWF9AXV1eXl4GJSgsLFVWX1skJCRWViovLS4tMDAfHx8fFCcsK4GBgYGBgYGBgYGBgYGBgWJpbri5bGNlZYGBYGBgYGJiYmSmZWZnZ2VlZGOiomBgYKOjomBqBGBgYGBgYGBgYGBgYFMjgUgBgSXwKSeEg4GhoYKEhYGhoaGhoWBgYGBgYGBgYGBgYGBgYAhvc21rc29qaGViYmBgYIKCgYGBgYGBgYGBgYGBgV1BQUFBQUFfWiUqKy4rCV9bJCwsr60sLC0uLS8vfCalJSQkLSuBgYGBgYGBgYGBgYFgYGBocXI5dmVlZGCBgWBgYGBgYGBgYGBgYGBipGSmZWdnZ2VlIyNgBgNgYGBgYGBgYGBgYGBjKYFIAYEfYKGhgoSBoaGhoWBgYGBgYGBgYGBgYGBgYGCua3NwdXVwbm1uamqtaGemJCSigYGBgYGBgYGBgYEFWlpZWltaWVYkKS4sLi8HVCcsMS0sLCsuKy0vLSYmKSgpKysrYGBggYGBgYGBYGBgYGAGbXN1dWhlZ4GBYGBgYGBgYGBgYGBgYGBgYGBgYGBgYGBipGUEZQRgYGBgYGBgYGBgYGBgMymBSAGBH2ChoaGBoaGhYGBgYGBggYKDgWBgYGBgYGBgBWhnZ2hyamdtb3FwcXFsbmxqaCSigWCBgYGBgYGBgiIio6OBI2MkJCgsLK8KX1+nsworKS4sLCovLy4rKi0qKysrK2BgYGBgYGBgYGBgYGBisW9xdW1oZGhlo2KiYGBgYGBgYGBgYGCBgYGBYGBgYGBgYGBgYGBgYGBgYGBgYGBgYGBgYEMrgUgBgR2hoaGhYGBgYGBggYaEh4aEYIFgYGBgYGBgYGJia3JpZWdmbW5vcHBwcm5sameigYGBgYGBgYGBgoEjJlqpJSYmKC4sQV9fQV9fX18FLi0rLi4uLi8uLisrKysrYGBgYGBgYGBgYGBgYARua25wb2poamtqamlnZ2cDZKNiomBggYGBgYGBgWBgYGBg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Z2BgYGBgYGBgBGpqb3Fxc3BxcK0GA2JncGyiYGBgpGVlZGBgrWdnaGpqbHBxc3NvbmpqaANggoKBgoGBgYGBgYGBYoIkIyMFVqdWpyQoMDAzNdWzQUFBQkIKMzEuMDEzMiouxTEtLTAzLYEhAvBDMy4BZGBgYGBgYGBgYAdoamxsbG5vcHNwb3JzbmZiYGBgYGBgYGBgYmRkZmVl8GVnaGlrb3Nta2dggoGCgoGBgYGBgYGBgYIkIyMlJgdeXisuMzDKNcrLqVZAX0FBCjErMzI0MisvxzBTZIEXAVtgYGBgYGBgYGBgY2dm8GZoaWpra2tsa2RkomBgYGBgYGBgpGhoZGBipANlZWVl8GRmZWZlYIGCg4GBgYGBgYGBgWKCIyQjJQZfQV8oCTIzM8o0yss1skNBXwcqY22BPwFmYGBgYGBgYGKtB6RgYGIDZ2Vm8GRkY6JkA2BgYGCkamhlo2hwbGhgYGBgYGBiowOlZWVlo2CigYKCgYGBgYGBgYGBgSMlJVxfXEBfpjMxMjQ1NDNBX0FBX10CKTE1MzQzLjAvLzAwMzGBUwGBE6GBgYGBgYKCgoGCgoQkJidnaWtrbG5ubm5ub29xb22Q8CgkJCOCgoImb2xqaGZko2JgYGBgYGBgA29xbm9vCwhlYGBgA2dlZKRgYGBgYAZvcG9wc3BrZ6OjYmBgYGBgYGBgYGJgYIKBgoKBgYGBgYGBgW01:03:59.660 [25320] I: TILES012.ART: 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

		for(const std::pair<std::string, std::string> & fileDiff : fileDiffs) {
			std::shared_ptr<GroupFile> currentSourceGroupFile(modifiedGroup->getFileWithName(fileDiff.first));
			ByteBuffer & currentSourceGroupFileData = currentSourceGroupFile->getData();
			std::shared_ptr<GroupFile> currentTargetGroupFile(targetGroup->getFileWithName(fileDiff.first));
			ByteBuffer & currentTargetGroupFileData = currentTargetGroupFile->getData();

			std::unique_ptr<ByteBuffer> currentCalculatedSourceGroupFileDiff(currentSourceGroupFileData.diff(currentTargetGroupFileData));

			//spdlog::info("{}: {}", fileDiff.first, currentCalculatedSourceGroupFileDiff->toBase64());

			std::unique_ptr<ByteBuffer> currentSourceGroupFileDiff(ByteBuffer::fromBase64(fileDiff.second));
			currentSourceGroupFile->setData(currentSourceGroupFileData.patch(*currentSourceGroupFileDiff));
		}

		modifiedGroup->reorderFilesByName(ATOMIC_GROUP_FILE_ORDER);

		modifiedGroup->extractAllFiles("WORLD_TOUR_TO_ATOMIC", true);

		modifiedGroup->saveTo("WORLD_TOUR_TO_ATOMIC.GRP");

		spdlog::info("[WORLD TOUR]");

		for(size_t i = 0; i < sourceGroup->numberOfFiles(); i++) {
			spdlog::info("{} {}", i, sourceGroup->getFile(i)->getFileName());
		}

		spdlog::info("[ATOMIC]");

		for(size_t i = 0; i < targetGroup->numberOfFiles(); i++) {
			spdlog::info("{} {}", i, targetGroup->getFile(i)->getFileName());
		}

		spdlog::info("[MODIFIED]");

		for(size_t i = 0; i < modifiedGroup->numberOfFiles(); i++) {
			spdlog::info("{} {}", i, modifiedGroup->getFile(i)->getFileName());
		}
	}

	static const std::vector<std::string> WORLD_TOUR_FILE_ORDER({
		"CINEOV2.ANM",
		"CINEOV3.ANM",
		"DUKETEAM.ANM",
		"LOGO.ANM",
		"RADLOGO.ANM",
		"VOL41A.ANM",
		"VOL42A.ANM",
		"VOL43A.ANM",
		"VOL4E1.ANM",
		"VOL4E2.ANM",
		"VOL4E3.ANM",
		"TILES000.ART",
		"TILES001.ART",
		"TILES002.ART",
		"TILES003.ART",
		"TILES004.ART",
		"TILES005.ART",
		"TILES006.ART",
		"TILES007.ART",
		"TILES008.ART",
		"TILES009.ART",
		"TILES010.ART",
		"TILES011.ART",
		"TILES012.ART",
		"TILES013.ART",
		"TILES014.ART",
		"TILES015.ART",
		"TILES016.ART",
		"TILES017.ART",
		"TILES018.ART",
		"TILES019.ART",
		"DEFS.CON",
		"GAME.CON",
		"USER.CON",
		"LOOKUP.DAT",
		"PALETTE.DAT",
		"TABLES.DAT",
		"DEMO1.DMO",
		"DEMO2.DMO",
		"DEMO3.DMO",
		"E1L1.MAP",
		"E1L2.MAP",
		"E1L3.MAP",
		"E1L4.MAP",
		"E1L5.MAP",
		"E1L6.MAP",
		"E1L7.MAP",
		"E1L8.MAP",
		"E2L1.MAP",
		"E2L10.MAP",
		"E2L11.MAP",
		"E2L2.MAP",
		"E2L3.MAP",
		"E2L4.MAP",
		"E2L5.MAP",
		"E2L6.MAP",
		"E2L7.MAP",
		"E2L8.MAP",
		"E2L9.MAP",
		"E3L1.MAP",
		"E3L10.MAP",
		"E3L11.MAP",
		"E3L2.MAP",
		"E3L3.MAP",
		"E3L4.MAP",
		"E3L5.MAP",
		"E3L6.MAP",
		"E3L7.MAP",
		"E3L8.MAP",
		"E3L9.MAP",
		"E4L1.MAP",
		"E4L10.MAP",
		"E4L11.MAP",
		"E4L2.MAP",
		"E4L3.MAP",
		"E4L4.MAP",
		"E4L5.MAP",
		"E4L6.MAP",
		"E4L7.MAP",
		"E4L8.MAP",
		"E4L9.MAP",
		"D3DTIMBR.TMB",
		"DUKE3D.BIN",
		"233C.MID",
		"AHGEEZ.MID",
		"ALFREDH.MID",
		"ALIENZ.MID",
		"BAKEDGDS.MID",
		"BRIEFING.MID",
		"CF.MID",
		"DEPART.MID",
		"DETHTOLL.MID",
		"FATCMDR.MID",
		"FLOGHORN.MID",
		"FUTURMIL.MID",
		"GLOOMY.MID",
		"GOTHAM.MID",
		"GRABBAG.MID",
		"GUTWRNCH.MID",
		"INHIDING.MID",
		"INTENTS.MID",
		"INVADER.MID",
		"LAYERS.MID",
		"LEMCHILL.MID",
		"LORDOFLA.MID",
		"MISSIMP.MID",
		"NAMES.MID",
		"PIZZED.MID",
		"POB.MID",
		"PREPD.MID",
		"RESTRICT.MID",
		"ROBOCREP.MID",
		"SNAKE1.MID",
		"SPOOK.MID",
		"STALAG.MID",
		"STALKER.MID",
		"STORM.MID",
		"STREETS.MID",
		"SUBWAY.MID",
		"THECALL.MID",
		"URBAN.MID",
		"WAREHAUS.MID",
		"WATRWLD1.MID",
		"WHOMP.MID",
		"XPLASMA.MID",
		"!BOSS.VOC",
		"!PIG.VOC",
		"!PRISON.VOC",
		"2BWILD.VOC",
		"2RIDE06.VOC",
		"ABORT01.VOC",
		"ADOOR1.VOC",
		"ADOOR2.VOC",
		"AHH04.VOC",
		"AHMUCH03.VOC",
		"AISLE402.VOC",
		"ALARM1A.VOC",
		"ALERT.VOC",
		"AMB81B.VOC",
		"AMESS06.VOC",
		"ANNOY03.VOC",
		"ASWTCH23.VOC",
		"B2ATK01.VOC",
		"B2ATK02.VOC",
		"B2DIE03.VOC",
		"B2PAIN03.VOC",
		"B2REC03.VOC",
		"B3DIE03G.VOC",
		"B3PAIN04.VOC",
		"B3REC03G.VOC",
		"B3ROAM01.VOC",
		"BANG1.VOC",
		"BARMUSIC.VOC",
		"BEBACK01.VOC",
		"BITCHN04.VOC",
		"BLANK.VOC",
		"BLDIE3A.VOC",
		"BLOWIT01.VOC",
		"BLPAIN1B.VOC",
		"BLREC4B.VOC",
		"BLRIP1A.VOC",
		"BLRIP1B.VOC",
		"BLROAM2A.VOC",
		"BLSPIT1A.VOC",
		"BOMBEXPL.VOC",
		"BONUS.VOC",
		"BOOBY04.VOC",
		"BOOKEM03.VOC",
		"BORN01.VOC",
		"BOS1DY.VOC",
		"BOS1PN.VOC",
		"BOS1RG.VOC",
		"BOS1RM.VOC",
		"BQDIE1A.VOC",
		"BQEGG1A.VOC",
		"BQPAIN2A.VOC",
		"BQREC2A.VOC",
		"BQROAM2A.VOC",
		"BQSHOCK3.VOC",
		"BRUN.VOC",
		"BSCORE.VOC",
		"BUBBLAMB.VOC",
		"BUCKLE.VOC",
		"BULITHIT.VOC",
		"CATFIRE.VOC",
		"CDOOR1B.VOC",
		"CHAINGUN.VOC",
		"CHEER.VOC",
		"CHEW05.VOC",
		"CHOKN12.VOC",
		"CLANG1.VOC",
		"CLIPIN.VOC",
		"CLIPOUT.VOC",
		"COMEON02.VOC",
		"COMMAT.VOC",
		"COMMDY.VOC",
		"COMMPN.VOC",
		"COMMRG.VOC",
		"COMMRM.VOC",
		"COMMSP.VOC",
		"COMPAMB.VOC",
		"CON03.VOC",
		"COOL01.VOC",
		"CRY01.VOC",
		"CTRLRM25.VOC",
		"DAMN03.VOC",
		"DAMNIT04.VOC",
		"DANCE01.VOC",
		"DEEPFRY1.VOC",
		"DEFEATED.VOC",
		"DETRUCT2.VOC",
		"DIESOB03.VOC",
		"DMDEATH.VOC",
		"DOGWHINE.VOC",
		"DOGYELP.VOC",
		"DOLPHIN.VOC",
		"DOM03.VOC",
		"DOM09.VOC",
		"DOM11.VOC",
		"DOM12.VOC",
		"DOOMED16.VOC",
		"DRINK18.VOC",
		"DRIP3.VOC",
		"DSCREM04.VOC",
		"DSCREM15.VOC",
		"DSCREM16.VOC",
		"DSCREM17.VOC",
		"DSCREM18.VOC",
		"DSCREM38.VOC",
		"DUCTWLK.VOC",
		"DUKNUK14.VOC",
		"EAT08.VOC",
		"EATSHT01.VOC",
		"EDOOR10.VOC",
		"EDOOR11.VOC",
		"ENGHUM.VOC",
		"ENLARGE.VOC",
		"ESCAPE01.VOC",
		"EXERT.VOC",
		"EXSHOT3B.VOC",
		"FACE01.VOC",
		"FIRE09.VOC",
		"FLUSH.VOC",
		"FLYBY.VOC",
		"FORCE01.VOC",
		"FREEZE.VOC",
		"FSCRM10.VOC",
		"GASP.VOC",
		"GASPS07.VOC",
		"GBELEV01.VOC",
		"GBELEV02.VOC",
		"GBLASR01.VOC",
		"GEARGRND.VOC",
		"GETCRAP1.VOC",
		"GETITM19.VOC",
		"GETSOM1A.VOC",
		"GLASHEVY.VOC",
		"GLASS.VOC",
		"GMEOVR05.VOC",
		"GOAWAY.VOC",
		"GOGGLE12.VOC",
		"GOTHRT01.VOC",
		"GRABBAG.VOC",
		"GRIND.VOC",
		"GROOVY02.VOC",
		"GRUN.VOC",
		"GSCORE.VOC",
		"GULP01.VOC",
		"GUNHIT2.VOC",
		"GUYSUK01.VOC",
		"H2OGRGL2.VOC",
		"H2ORUSH2.VOC",
		"HAIL01.VOC",
		"HAPPEN01.VOC",
		"HARTBEAT.VOC",
		"HEADRIP3.VOC",
		"HLIDLE03.VOC",
		"HOLYCW01.VOC",
		"HOLYSH02.VOC",
		"HYDRO22.VOC",
		"HYDRO24.VOC",
		"HYDRO27.VOC",
		"HYDRO34.VOC",
		"HYDRO40.VOC",
		"HYDRO43.VOC",
		"HYDRO50.VOC",
		"IMGOOD12.VOC",
		"INDPNC01.VOC",
		"INHELL01.VOC",
		"INTRO4H1.VOC",
		"INTRO4H2.VOC",
		"INTROA.VOC",
		"INTROB.VOC",
		"INTROC.VOC",
		"ITEM15.VOC",
		"JEEP2A.VOC",
		"JETP2.VOC",
		"JETPAKI.VOC",
		"JETPAKOF.VOC",
		"JETPAKON.VOC",
		"JONES04.VOC",
		"KICK01-I.VOC",
		"KICKHIT.VOC",
		"KILLME.VOC",
		"KNUCKLE.VOC",
		"KTITX.VOC",
		"LAND02.VOC",
		"LANI05.VOC",
		"LANI08.VOC",
		"LANIDUK2.VOC",
		"LAVA06.VOC",
		"LETGOD01.VOC",
		"LETSRK03.VOC",
		"LIZSHIT3.VOC",
		"LIZSPIT.VOC",
		"LOOKIN01.VOC",
		"LSRBMBPT.VOC",
		"LSRBMBWN.VOC",
		"MACHAMB.VOC",
		"MAKEDAY1.VOC",
		"MDEVL01.VOC",
		"MEAT04-N.VOC",
		"MICE3.VOC",
		"MONITOR.VOC",
		"MONOLITH.VOC",
		"MUSTDIE.VOC",
		"MUZAK028.VOC",
		"MUZAKDIE.VOC",
		"MYSELF3A.VOC",
		"NAME01.VOC",
		"NEEDED03.VOC",
		"NEWS.VOC",
		"NOBODY01.VOC",
		"OCTAAT1.VOC",
		"OCTAAT2.VOC",
		"OCTADY.VOC",
		"OCTAPN.VOC",
		"OCTARG.VOC",
		"OCTARM.VOC",
		"ONBOARD.VOC",
		"ONLYON03.VOC",
		"OPENDOOR.VOC",
		"PAIN13.VOC",
		"PAIN28.VOC",
		"PAIN39.VOC",
		"PAIN54.VOC",
		"PAIN68.VOC",
		"PAIN75.VOC",
		"PAIN87.VOC",
		"PAIN93.VOC",
		"PARTY03.VOC",
		"PAY02.VOC",
		"PBOMBBNC.VOC",
		"PHONBUSY.VOC",
		"PIECE02.VOC",
		"PIGDY.VOC",
		"PIGPN.VOC",
		"PIGRG.VOC",
		"PIGRM.VOC",
		"PIGWRN.VOC",
		"PISSES01.VOC",
		"PISSIN01.VOC",
		"PISSING.VOC",
		"PISTOL.VOC",
		"POOLBAL1.VOC",
		"POSTAL01.VOC",
		"PREDDY.VOC",
		"PREDPN.VOC",
		"PREDRG.VOC",
		"PREDRM.VOC",
		"PROJRUN.VOC",
		"QUAKE.VOC",
		"QUAKE06.VOC",
		"R&R01.VOC",
		"RAIN3A.VOC",
		"REACTOR.VOC",
		"READY2A.VOC",
		"RICOCHET.VOC",
		"RIDES09.VOC",
		"RIP01.VOC",
		"RIPEM08.VOC",
		"ROAM06.VOC",
		"ROAM22.VOC",
		"ROAM29.VOC",
		"ROAM58.VOC",
		"ROAM67.VOC",
		"ROAM98B.VOC",
		"ROCKIN02.VOC",
		"RPGFIRE.VOC",
		"SBELL1A.VOC",
		"SBR1C.VOC",
		"SCUBA.VOC",
		"SECRET.VOC",
		"SHAKE2A.VOC",
		"SHORN1.VOC",
		"SHORTED.VOC",
		"SHOTGNCK.VOC",
		"SHOTGUN7.VOC",
		"SHRINK.VOC",
		"SHRINKER.VOC",
		"SKIDCR1.VOC",
		"SLACKER1.VOC",
		"SLHTCH01.VOC",
		"SLIDIE03.VOC",
		"SLIDOOR.VOC",
		"SLIMAT.VOC",
		"SLIREC06.VOC",
		"SLIROA02.VOC",
		"SMACK02.VOC",
		"SNAKATA.VOC",
		"SNAKATB.VOC",
		"SNAKDY.VOC",
		"SNAKPN.VOC",
		"SNAKRG.VOC",
		"SNAKRM.VOC",
		"SOHELP02.VOC",
		"SPLASH.VOC",
		"SQUISH.VOC",
		"SQUISH1A.VOC",
		"STEAMHIS.VOC",
		"SUBWAY.VOC",
		"SUKIT01.VOC",
		"SWEET03.VOC",
		"SWEET04.VOC",
		"SWEET05.VOC",
		"SWEET16.VOC",
		"SWITCH.VOC",
		"TANK3A.VOC",
		"TCLAP2A.VOC",
		"TELEPORT.VOC",
		"TERMIN01.VOC",
		"THSUK13A.VOC",
		"THUD.VOC",
		"TRUMBLE.VOC",
		"TYPING.VOC",
		"VACATN01.VOC",
		"VAULT04.VOC",
		"VENTBUST.VOC",
		"VOCAL02.VOC",
		"WAITIN03.VOC",
		"WANSOM4A.VOC",
		"WARAMB13.VOC",
		"WARAMB21.VOC",
		"WARAMB23.VOC",
		"WARAMB29.VOC",
		"WAVE1A.VOC",
		"WETFEET.VOC",
		"WHIPYU01.VOC",
		"WHISTLE.VOC",
		"WHRSIT05.VOC",
		"WIND54.VOC",
		"WPNSEL21.VOC",
		"YIPPIE01.VOC",
		"YOHOHO01.VOC",
		"YOHOHO09.VOC",
		"ZIPPER2.VOC"
	});

	//??:
	//regular -> atomic
	//atomic -> regular
	// bonus:
	//plutonium -> atomic
	//atomic -> world tour

	// TODO:
	/*
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
#endif

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
