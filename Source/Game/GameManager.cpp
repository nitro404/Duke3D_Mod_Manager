#include "GameManager.h"

#include "GameDownload.h"
#include "GameDownloadCollection.h"
#include "GameDownloadFile.h"
#include "GameDownloadVersion.h"
#include "GameLocator.h"
#include "Game/GameVersion.h"
#include "Game/NoCDCracker.h"
#include "Game/Configuration/GameConfiguration.h"
#include "Game/File/Group/GRP/GroupGRP.h"
#include "Manager/SettingsManager.h"

#include <Archive/ArchiveFactoryRegistry.h>
#include <Bitbucket/BitbucketService.h>
#include <GitHub/GitHubService.h>
#include <Network/HTTPService.h>
#include <Utilities/FileUtilities.h>
#include <Utilities/RapidJSONUtilities.h>
#include <Utilities/StringUtilities.h>
#include <Utilities/TidyHTMLUtilities.h>
#include <Utilities/TimeUtilities.h>
#include <Utilities/TinyXML2Utilities.h>

#include <magic_enum.hpp>
#include <rapidjson/document.h>
#include <spdlog/spdlog.h>
#include <tinyxml2.h>

#include <array>
#include <filesystem>
#include <optional>
#include <sstream>
#include <vector>

using namespace std::chrono_literals;

static const std::string LAMEDUKE_FALLBACK_DOWNLOAD_URL("https://archive.org/download/lameduke/lameduke.zip"); // https://archive.org/details/lameduke
static const std::string BETA_VERSION_FALLBACK_DOWNLOAD_URL("http://hendricks266.duke4.net/d3dbeta/d3d_beta.7z"); // http://hendricks266.duke4.net/d3dbeta.php
static const std::string REGULAR_VERSION_FALLBACK_DOWNLOAD_URL("https://archive.org/download/DUKE3D_DOS/DUKE3D.zip"); // https://archive.org/details/DUKE3D_DOS
static const std::string ATOMIC_EDITION_FALLBACK_DOWNLOAD_URL("https://archive.org/download/duke-3d-atomic3datomictweak/Duke3d_ATOMIC.rar"); // https://archive.org/details/duke-3d-atomic3datomictweak

static const std::string LAMEDUKE_DOWNLOAD_SHA1("41ae6fe00aa57aef38d5744cb18ca9e25bc73bbb");
static const std::string BETA_VERSION_FALLBACK_DOWNLOAD_SHA1("c24ea12ab57955b96b93db6eeeb4231d4c137a6d");
static const std::string REGULAR_VERSION_FALLBACK_DOWNLOAD_SHA1("9bc975193ccc4738a31fe2dc6958f0b34135b9ae");
static const std::string ATOMIC_EDITION_FALLBACK_DOWNLOAD_SHA1("ac2df3fb75f84584cadc82db00ef46ab21ef6a95");

static const std::string JFDUKE32_DOWNLOAD_PAGE_URL("http://www.jonof.id.au/jfduke3d");
static const std::string EDUKE32_DOWNLOAD_PAGE_URL("https://dukeworld.com/eduke32/synthesis/latest");
static const std::string NETDUKE32_DOWNLOAD_API_URL("https://voidpoint.io/api/graphql");
static const std::string RED_NUKEM_DOWNLOAD_PAGE_URL("https://lerppu.net/wannabethesis");

static const std::string WORLD_TOUR_GAME_LONG_NAME("Duke Nukem 3D: 20th Anniversary World Tour");
static const std::string WORLD_TOUR_SUBDIRECTORY_NAME("WorldTour");

struct GameFileInformation {
	std::string fileName;
	std::vector<std::string> hashes;
	bool required = true;
};

static const std::array<GameFileInformation, 15> BETA_VERSION_GAME_FILE_INFO_LIST = {
	GameFileInformation{ "COMMIT.DAT",   { "ee7ea8d9578e7ef80dafe5c8629ef3d579d92603" } },
	GameFileInformation{ "COMMIT.EXE",   { "7caa1d3665bfee9e5997942f1353bbd95854408d" } },
	GameFileInformation{ "DOS4GW.EXE",   { "a897c7eb79145a308c4e60b43413bb2daa360506" } },
	GameFileInformation{ "DUKE.RTS",     { "a9356036aea01583c85b71410f066285afe3af2b" } },
	GameFileInformation{ "DUKE3D.EXE",   { GameVersion::BETA_VERSION_GAME_EXECUTABLE_SHA1 } },
	GameFileInformation{ "DUKE3D.GRP",   { GroupGRP::DUKE_NUKEM_3D_BETA_VERSION_GROUP_SHA1_FILE_HASH } },
	GameFileInformation{ "MODEM.PCK",    { "4314bc9bd2c953fe3cd3a51258e483056560ddd6" } },
	GameFileInformation{ "README.TXT",   { "b73b8ac76be08ba0079c0219c7797981ce18357b" } },
	GameFileInformation{ "SETMAIN.EXE",  { "f2892c2d765bd4abca2bd41f0e7a1142c3214c81" } },
	GameFileInformation{ "SETUP.EXE",    { "7110b4b66e5823527750dcda2a8df5939a978c9b" } },
	GameFileInformation{ "ULTRAMID.INI", { "874de9ded41bd7df054e0d5f56afc4ebadcbd3ab" } },
	GameFileInformation{ "UNIVBE.DRV",   { "b2b8f3da67385cf2b486430c846f49007646bc14" } },
	GameFileInformation{ "UNIVBE.EXE",   { "9cbf105f55eb6d442b6e0e6ebf1f97c821f2927e" } },
	GameFileInformation{ "UVCONFIG.EXE", { "50bdb9f8f9885daa32cb08b666c886acb32ad0ec" } },
	GameFileInformation{ "VBETEST.EXE",  { "7aeed202c608a98057b8a605ca82c2e902670754" } }
};

static const std::array<GameFileInformation, 12> REGULAR_VERSION_GAME_FILE_INFO_LIST = {
	GameFileInformation{ "COMMIT.EXE",   { "fbb3d9ae08cd451b2a4cfc75b92de6dedc82d4b2" } },
	GameFileInformation{ "DEMO1.DMO",    { "c55a1a4635738da4e91b49475604608a04c03fcd" } },
	GameFileInformation{ "DEMO2.DMO",    { "c33e21c845584b8703264e032dd7808871b04286" } },
	GameFileInformation{ "DN3DHELP.EXE", { "eceefd4f9fa868ced33fcdd68673f1fb9cd8bfc5" } },
	GameFileInformation{ "DUKE.RTS",     { "738c7f5fd0c8b57ee2e87ae7a97bf8e21a821d07" } },
	GameFileInformation{ "DUKE3D.EXE",   { GameVersion::REGULAR_VERSION_GAME_EXECUTABLE_SHA1 } },
	GameFileInformation{ "DUKE3D.GRP",   { GroupGRP::DUKE_NUKEM_3D_REGULAR_VERSION_GROUP_SHA1_FILE_HASH } },
	GameFileInformation{ "MODEM.PCK",    { "c0353741d28ded860d708f0915a27028fb47b9f3" } },
	GameFileInformation{ "SETMAIN.EXE",  { "e43d96ea4429ab6dd5aab0509bd8705223ba5b21" } },
	GameFileInformation{ "SETUP.EXE",    { "7110b4b66e5823527750dcda2a8df5939a978c9b" } },
	GameFileInformation{ "TENGAME.INI",  { "2b6e157bede128d48788ebb80f29f1e635dd20dd" }, false },
	GameFileInformation{ "ULTRAMID.INI", { "54a404652aecfddf73aea0c11326f9f95fdd1e25" } }
};

static const std::array<GameFileInformation, 11> PLUTONIUM_PAK_GAME_FILE_INFO_LIST = {
	GameFileInformation{ "COMMIT.EXE",   { "8832f0fc61f7286b14cac0cd1ff0a56e179c5d6f" } },
	GameFileInformation{ "DN3DHELP.EXE", { "c25dce13ef39d1aa107612dc20ce9d05e8ac8d3f" } },
	GameFileInformation{ "DUKE.RTS",     { "738c7f5fd0c8b57ee2e87ae7a97bf8e21a821d07" } },
	GameFileInformation{ "DUKE3D.EXE",   { GameVersion::PLUTONIUM_PAK_GAME_EXECTUABLE_UNCRACKED_SHA1, GameVersion::PLUTONIUM_PAK_GAME_EXECTUABLE_CRACKED_SHA1 } },
	GameFileInformation{ "DUKE3D.GRP",   { GroupGRP::DUKE_NUKEM_3D_PLUTONIUM_PAK_GROUP_SHA1_FILE_HASH } },
	GameFileInformation{ "LICENSE.DOC",  { "ce1a1bb1afbd714bb96ec0c0d8e0b23a94f14c0b" } },
	GameFileInformation{ "MODEM.PCK",    { "e860d0b4bef48f19cf36cb8406b2b1230ea7ef6a" } },
	GameFileInformation{ "SETMAIN.EXE",  { "40bd08600df2cd6328e69889b5325b72a123614e" } },
	GameFileInformation{ "SETUP.EXE",    { "861db7aa6dfc868b6a0b333b4cb091e276a18832" } },
	GameFileInformation{ "TENGAME.INI",  { "01a0f0e66fb05f5b2de72a8dd3ad19cdfa4ae323" }, false },
	GameFileInformation{ "ULTRAMID.INI", { "54a404652aecfddf73aea0c11326f9f95fdd1e25" } }
};

static const std::array<GameFileInformation, 11> ATOMIC_EDITION_GAME_FILE_INFO_LIST = {
	GameFileInformation{ "COMMIT.EXE",   { "8832f0fc61f7286b14cac0cd1ff0a56e179c5d6f" } },
	GameFileInformation{ "DN3DHELP.EXE", { "eeb5666d6e1b705e3684f8ed84ab5ac50b30a690" } },
	GameFileInformation{ "DUKE.RTS",     { "738c7f5fd0c8b57ee2e87ae7a97bf8e21a821d07" } },
	GameFileInformation{ "DUKE3D.EXE",   { GameVersion::ATOMIC_EDITION_GAME_EXECTUABLE_UNCRACKED_SHA1, GameVersion::ATOMIC_EDITION_GAME_EXECTUABLE_CRACKED_SHA1 } },
	GameFileInformation{ "DUKE3D.GRP",   { GroupGRP::DUKE_NUKEM_3D_ATOMIC_EDITION_GROUP_SHA1_FILE_HASH, GroupGRP::DUKE_NUKEM_3D_WORLD_TOUR_GROUP_SHA1_FILE_HASH } },
	GameFileInformation{ "LICENSE.DOC",  { "ce1a1bb1afbd714bb96ec0c0d8e0b23a94f14c0b" } },
	GameFileInformation{ "MODEM.PCK",    { "7d88d2ae3a0fc21fcaaeb9cc5c1e72399c0fd0cb" } },
	GameFileInformation{ "SETMAIN.EXE",  { "40bd08600df2cd6328e69889b5325b72a123614e" } },
	GameFileInformation{ "SETUP.EXE",    { "861db7aa6dfc868b6a0b333b4cb091e276a18832" } },
	GameFileInformation{ "TENGAME.INI",  { "01a0f0e66fb05f5b2de72a8dd3ad19cdfa4ae323" }, false },
	GameFileInformation{ "ULTRAMID.INI", { "54a404652aecfddf73aea0c11326f9f95fdd1e25" } }
};

static const std::array<GameFileInformation, 103> LAMEDUKE_GAME_FILE_INFO_LIST = {
	GameFileInformation{ "BIGLITE.VOC",  { "a1aff591e958e82a91a3606ab76f7deab25b0e2b" } },
	GameFileInformation{ "BODYBLOP.VOC", { "e2e8288cdfa91d1c78a7574df81d4515717b4bd5" } },
	GameFileInformation{ "BROWNEYE.MID", { "a99fdeb0997d3b4c99c96538fa19929949c56cfa" } },
	GameFileInformation{ "CLIPIN.VOC",   { "f9fb586bbd3560e495a4d538bde38e1fcf9aca30" } },
	GameFileInformation{ "CLIPOUT.VOC",  { "a76d608e8733437b266927ba92f2b4b23b13a191" } },
	GameFileInformation{ "COPTER.VOC",   { "9f204c3b1e6638b728a1b6962e57aed969b70d39" } },
	GameFileInformation{ "D3D.EXE",      { "484f2fa2010aee608c7805f13c80093674343d0c" } },
	GameFileInformation{ "DEFS.CON",     { "4c00b2028b4d8e9d9f37701a42c88065b48c1a20" } },
	GameFileInformation{ "DOOR1.VOC",    { "58a9782876bc4609efc0e7d252bdf4763c36c641" } },
	GameFileInformation{ "DOOR2.VOC",    { "e29fd0cb34cc67a09af2c6183714523484719acd" } },
	GameFileInformation{ "DOS4GW.EXE",   { "b6c2bed388c6ea1b3c02b941c9541500598f2de0" } },
	GameFileInformation{ "DRIPPING.VOC", { "59c701bff7e47b6edf15c7499047743ce52dc7ce" } },
	GameFileInformation{ "E2M1.MID",     { "ff490de338a4ed78363fa4ac71596f2fc6a42cac" } },
	GameFileInformation{ "ELEV1.VOC",    { "cede502c5ad7ebd183cd9ee03585227c9aa3c0fe" } },
	GameFileInformation{ "ELEV2.VOC",    { "728fcbd79606eaad9d7db3594947fbe56468973b" } },
	GameFileInformation{ "ELEV3.VOC",    { "e8d2b05a50c5b1e9369f25b6fdd7988b373f1ce3" } },
	GameFileInformation{ "EXPL1.VOC",    { "4c4958c1e7167ae19c39d7419ae747516ff052c5" } },
	GameFileInformation{ "FASTWAY.MID",  { "ff490de338a4ed78363fa4ac71596f2fc6a42cac" } },
	GameFileInformation{ "FILE_ID.DIZ",  { "7114f45db77c10e8f2a79b848ac9520fabe647bd" } },
	GameFileInformation{ "FTA_HF.VOC",   { "d4753c68aadce2528ed987b47e20cb8744e858b0" } },
	GameFileInformation{ "FTA_HM.VOC",   { "045e363fa675b4b2f3c18a5c17c21e6f6fd3c056" } },
	GameFileInformation{ "FTA_R.VOC",    { "01483b1794e8444b324752c94f9205bb48e7e5ae" } },
	GameFileInformation{ "GAME.CON",     { "03b9ea6acb41f8f6282328edd18c1c329621ed0b" } },
	GameFileInformation{ "GETWEAPN.VOC", { "adc82b90b41d3c76a5395e1dd2528fe13928de22" } },
	GameFileInformation{ "GLASS.VOC",    { "422c44cf999c7e0fbd0b3ebfa6bfbcf97e29c162" } },
	GameFileInformation{ "GO.BAT",       { "cfdeebe2104e4e6c9d701a3d6badd785a5ed4ba6" } },
	GameFileInformation{ "GUN1.VOC",     { "e56446e2f01e24d716a176614ab0de83b9079997" } },
	GameFileInformation{ "JETPAKI.VOC",  { "a7398407ccf310f5df5a5287cad964be160df792" } },
	GameFileInformation{ "JETPAKOF.VOC", { "303c75c499b7598a72e934f22bc9d2928917ff81" } },
	GameFileInformation{ "JETPAKON.VOC", { "9ca60a5da8640bea6178982fd18240f7a5a8fe1d" } },
	GameFileInformation{ "JUMP.VOC",     { "5bdece71b1c654555b72fb18cb1d58516ce07042" } },
	GameFileInformation{ "KNUCKLE.VOC",  { "51ff67d7d13a2b0d638d3d1345657e976bb58e6a" } },
	GameFileInformation{ "L1.MAP",       { "d2da8e361760e5c056998ccd8f0fba1b6d63f0b2" } },
	GameFileInformation{ "L2.MAP",       { "821c6378dd13df306744efc0d585e487931b8636" } },
	GameFileInformation{ "L3.MAP",       { "5b0d21ae42b44f55159a7b8fd29466f9da69de61" } },
	GameFileInformation{ "L4.MAP",       { "5a7c8bd8c2d513f8ce95e0e55127803649c9a450" } },
	GameFileInformation{ "L5.MAP",       { "cd8842674926b234ec6082bddc89d07fe6f0aea3" } },
	GameFileInformation{ "L6.MAP",       { "5c2f7df3d2fa10cb7c4464dd990976ff36b81ddd" } },
	GameFileInformation{ "L7.MAP",       { "41faf4d019d0db03288f56aa3ff216e13a308eb8" } },
	GameFileInformation{ "L8.MAP",       { "1beb444d9a7c876620db39e37f1ada5a911141f7" } },
	GameFileInformation{ "L9.MAP",       { "030ef2305a46edfd0b81dac46bfab0cb31018199" } },
	GameFileInformation{ "LAMEDUKE.TXT", { "9a4a56604ec9e74e3956dae7614048724debe02b" } },
	GameFileInformation{ "LAND.VOC",     { "876807adeb5c96f91bb0a0e82566df8118b9c091" } },
	GameFileInformation{ "LITEFLCK.VOC", { "34762b9ec26f44ea5f5b3bd9e6a349d93983a69b" } },
	GameFileInformation{ "LOOKUP.DAT",   { "eecc70f7d4d9f883956ab17b82c4005f3a4b1f93" } },
	GameFileInformation{ "M1.MAP",       { "06addf61eb7b2c981b05e8a44640ab164a7e64c2" } },
	GameFileInformation{ "M2.MAP",       { "c3e169e5780a57fada97dc96eec3e527979f0c90" } },
	GameFileInformation{ "M3.MAP",       { "8c5399b0a89a6e783a41a7e6c19e212bc966ad35" } },
	GameFileInformation{ "M4.MAP",       { "bcaf01023a9f54fb02378d1e1df969d4761f3cf1" } },
	GameFileInformation{ "M5.MAP",       { "16f7daf2604efe6b24a2f460c5e88e6094c43da7" } },
	GameFileInformation{ "M6.MAP",       { "ee6692c6c442912d05340f8ada4dae9f24f79982" } },
	GameFileInformation{ "M7.MAP",       { "046f6d6f3b453507c4a2d454e9bddc5a510aa5c9" } },
	GameFileInformation{ "M8.MAP",       { "b280d8bf297d2b2627e9d3ea61931159b9455ef2" } },
	GameFileInformation{ "MISTACHE.MID", { "6e4bbbd8c19d8080158cfa4ef47c112e8bee58c9" } },
	GameFileInformation{ "MUSIC.CON",    { "e97f4492cee60e5307afe0e5e6b20ec400d7d566" } },
	GameFileInformation{ "MYDEMO_0.DMO", { "4e80185f5ad2ec3fd4cc1b599f37e70980f35246" } },
	GameFileInformation{ "MYDEMO_1.DMO", { "ded58c20db6acc912ab28b59f1a19c104c3930e8" } },
	GameFileInformation{ "MYDEMO_2.DMO", { "482bf0f6aa9949e9f3a5c686a96a0ba5e9a34031" } },
	GameFileInformation{ "MYDEMO_3.DMO", { "0f52f5369cf660ba9deb200d0edc2e17ec88dd3b" } },
	GameFileInformation{ "MYDEMO_4.DMO", { "8668b4d3e2ade968f2bbed194239612e3c2f7d40" } },
	GameFileInformation{ "MYDEMO_5.DMO", { "609b5f061733fbec6d1d7b5797a8ba620e728744" } },
	GameFileInformation{ "MYDEMO_6.DMO", { "081535ff024f94227cb1c80165aaad9cd14e173a" } },
	GameFileInformation{ "MYDEMO_7.DMO", { "6d0779c44a34a2bac8a093149621d317748717b0" } },
	GameFileInformation{ "N1.MAP",       { "255ce58aa33c0d00c196f9350be3d50c7ae611da" } },
	GameFileInformation{ "N2.MAP",       { "c163fb086227c9cc116e4daf3d7c2c47b43751b6" } },
	GameFileInformation{ "N3.MAP",       { "a9300bf1f49b14a53b8be96200fef4c8597fa8dd" } },
	GameFileInformation{ "N4.MAP",       { "e0232443c01146fd23c19cb79f6cbf9cf670eec5" } },
	GameFileInformation{ "N5.MAP",       { "6a640df1994b3adf9ae0b655a8d0b4d7594102e1" } },
	GameFileInformation{ "N6.MAP",       { "316a92ea2dc2753cf925b711a733ccb7f6377ee3" } },
	GameFileInformation{ "N7.MAP",       { "12f382db7c0b1efbd7245250b6d51a876d5d646e" } },
	GameFileInformation{ "N8.MAP",       { "c55b076e1e1610236f836d5465d9836162d1d913" } },
	GameFileInformation{ "O1.MAP",       { "5490c2f988366388adff0e2ed81ae240e12ec3dc" } },
	GameFileInformation{ "O2.MAP",       { "801e65fae0f75dce40b6202b32cf387be952c9af" } },
	GameFileInformation{ "O3.MAP",       { "1972e51e9ac58de33215aba3dc2a6cc0f58fe48f" } },
	GameFileInformation{ "O4.MAP",       { "644d6b37e2e3726108a0883ed7ddfb1775ee7067" } },
	GameFileInformation{ "ONBOARD.VOC",  { "1e875d9a1e5e9d68c00c4b0a1a9be8458b85cb17" } },
	GameFileInformation{ "PALETTE.DAT",  { "7fb7cc8093d713ec8a4aeb57751784587d22e67b" } },
	GameFileInformation{ "REACTOR.VOC",  { "2302ac5090e13cc2a66cb1ecfae6ae3c1d7f788d" } },
	GameFileInformation{ "README.TXT",   { "9a4a56604ec9e74e3956dae7614048724debe02b" } },
	GameFileInformation{ "REBOUND.VOC",  { "dfb030929e48e1bd6a079da47056ff41176a5569" } },
	GameFileInformation{ "RPG.VOC",      { "19c7282cb39f2a825ef1c2e9b5e4a8b30fe093b1" } },
	GameFileInformation{ "RUNNING.VOC",  { "02d9abb1c23dbdd66cfbe4ff52a3809305fe9795" } },
	GameFileInformation{ "RUNNINGW.VOC", { "50f4364e7006594e6ac959c369e788c302945b94" } },
	GameFileInformation{ "SCUBA.VOC",    { "0f2f2652de48916efb452e5f4fd87431957555ae" } },
	GameFileInformation{ "SETUP.DAT",    { "b522e2269110e1de33957b8c540ebc3598b023c4" } },
	GameFileInformation{ "SETUP.EXE",    { "cd7e2c5ba6415a51b78eb69c148c45cefc6ae458" } },
	GameFileInformation{ "SHOOTING.VOC", { "8c1c5b8bdc30bb5c68a4f04e62dcec7812ce7bc9" } },
	GameFileInformation{ "SQUISH1.VOC",  { "94eb876bafb053d5028c96bad6b21733daf5ece2" } },
	GameFileInformation{ "SUBWAY.VOC",   { "428012f639bae88872101208118c72c666d192cf" } },
	GameFileInformation{ "SWITCH.VOC",   { "3453de0ab881a9e25e2208800ff5718230ce1f49" } },
	GameFileInformation{ "TABLES.DAT",   { "e46b638e982e4f6df796db9573e6e6de235841fd" } },
	GameFileInformation{ "TILES000.ART", { "1c32e055878823e625fc3e33360a887e24ffe681" } },
	GameFileInformation{ "TILES001.ART", { "494dbbdc84a20f9456db1a2c1ea6094d5d1b6eae" } },
	GameFileInformation{ "TILES002.ART", { "3f061f2460116120b698a3631f064daea5e6d74f" } },
	GameFileInformation{ "TILES003.ART", { "4ca9e7c6e0233d767e21f187cd2d3df7a9f8db5b" } },
	GameFileInformation{ "TILES004.ART", { "fd8566dbbcf168f00f5f9cdffac0a06b137ae437" } },
	GameFileInformation{ "TILES005.ART", { "261633119a26564037a3bf9986a634d2ebdc6591" } },
	GameFileInformation{ "TILES006.ART", { "54443e4520ba253fb3d34e153affca1a79e3922a" } },
	GameFileInformation{ "TILES007.ART", { "a79ce810fe2549d7457d8013ef3a4d2b1fc6bf8b" } },
	GameFileInformation{ "USERDEFS.TMP", { "80b636059adeaa2eb420cf49e7d5d1e19b7599af" } },
	GameFileInformation{ "WAR1.MAP",     { "368cb78ef802e85e4249d197b844d78c85697aee" } },
	GameFileInformation{ "WAR2.MAP",     { "b480f7e50acd26e93dccd3ada7257172eed14887" } },
	GameFileInformation{ "WATERFAL.VOC", { "db78e7f4390b724c34796e061b8a3232f085d3c1" } }
};

static std::string getGroupFilePathWithSubdirectory(const std::string & subdirectoryName) {
	if(subdirectoryName.empty()) {
		spdlog::error("Group file subdirectory name cannot be empty.");
		return {};
	}

	SettingsManager * settings = SettingsManager::getInstance();

	if(settings->downloadsDirectoryPath.empty()) {
		spdlog::error("Missing downloads directory path setting.");
		return {};
	}

	if(settings->groupDownloadsDirectoryName.empty()) {
		spdlog::error("Missing game downloads directory name setting.");
		return {};
	}

	return Utilities::joinPaths(settings->downloadsDirectoryPath, settings->groupDownloadsDirectoryName, subdirectoryName, GroupGRP::DUKE_NUKEM_3D_GROUP_FILE_NAME);
}

static std::string getWorldTourGroupFilePath() {
	return getGroupFilePathWithSubdirectory(WORLD_TOUR_SUBDIRECTORY_NAME);
}

GameManager::GameManager()
	: m_initialized(false)
	, m_localMode(false)
	, m_gameVersions(std::make_shared<GameVersionCollection>()) { }

GameManager::~GameManager() { }

bool GameManager::isInitialized() const {
	return m_initialized;
}

bool GameManager::initialize() {
	if(m_initialized) {
		return true;
	}

	SettingsManager * settings = SettingsManager::getInstance();

	bool gameVersionsLoaded = m_gameVersions->loadFrom(settings->gameVersionsListFilePath);

	if(!gameVersionsLoaded || m_gameVersions->numberOfGameVersions() == 0) {
		if(!gameVersionsLoaded) {
			spdlog::warn("Missing or invalid game versions configuration file '{}', using default values.", settings->gameVersionsListFilePath);
		}
		else if(m_gameVersions->numberOfGameVersions() == 0) {
			spdlog::warn("Empty game versions configuration file '{}', using default values.", settings->gameVersionsListFilePath);
		}

		// use default game version configurations
		m_gameVersions->setDefaultGameVersions();
	}

	m_gameVersions->addMissingDefaultGameVersions();

	if(m_localMode && !loadOrUpdateGameDownloadList()) {
		return false;
	}

	m_initialized = true;

	return true;
}

std::shared_ptr<GameVersionCollection> GameManager::getGameVersions() const {
	return m_gameVersions;
}

bool GameManager::isUsingLocalMode() const {
	return m_localMode;
}

void GameManager::setLocalMode(bool localMode) {
	if(m_initialized) {
		spdlog::error("Cannot change local mode after initialization.");
		return;
	}

	m_localMode = localMode;
}

std::string GameManager::getGameDownloadsListFilePath() const {
	SettingsManager * settings = SettingsManager::getInstance();

	if(m_localMode) {
		if(settings->gameDownloadsListFilePath.empty()) {
			spdlog::error("Missing local game downloads file path setting.");
			return {};
		}

		return settings->gameDownloadsListFilePath;
	}

	if(settings->downloadsDirectoryPath.empty()) {
		spdlog::error("Missing downloads directory path setting.");
		return {};
	}

	if(settings->gameDownloadsDirectoryName.empty()) {
		spdlog::error("Missing game downloads directory name setting.");
		return {};
	}

	if(settings->remoteGamesListFileName.empty()) {
		spdlog::error("Missing remote games list file name setting.");
		return {};
	}

	return Utilities::joinPaths(settings->downloadsDirectoryPath, settings->gameDownloadsDirectoryName, settings->remoteGamesListFileName);
}

bool GameManager::shouldUpdateGameDownloadList() const {
	if(m_localMode) {
		return false;
	}

	SettingsManager * settings = SettingsManager::getInstance();

	if(!settings->downloadThrottlingEnabled || !settings->gameDownloadListLastDownloadedTimestamp.has_value()) {
		return true;
	}

	std::string gameDownloadsListFilePath(getGameDownloadsListFilePath());

	if(!gameDownloadsListFilePath.empty()) {
		spdlog::error("Failed to determine game downloads file list path. Are your settings configured correctly?");
		return false;
	}

	if(!std::filesystem::is_regular_file(std::filesystem::path(gameDownloadsListFilePath))) {
		return true;
	}

	return std::chrono::system_clock::now() - settings->gameDownloadListLastDownloadedTimestamp.value() > settings->gameDownloadListUpdateFrequency;
}

bool GameManager::loadOrUpdateGameDownloadList(bool forceUpdate) const {
	std::string gameDownloadsListFilePath(getGameDownloadsListFilePath());

	if(gameDownloadsListFilePath.empty()) {
		spdlog::error("Failed to determine game downloads file list path. Are your settings configured correctly?");
		return false;
	}

	if(m_localMode) {
		std::unique_ptr<GameDownloadCollection> gameDownloads(std::make_unique<GameDownloadCollection>());

		spdlog::info("Loading local game downloads list file...");

		if(gameDownloads->loadFrom(gameDownloadsListFilePath) && GameDownloadCollection::isValid(gameDownloads.get())) {
			m_gameDownloads = std::move(gameDownloads);
			return true;
		}

		spdlog::error("Failed to load local game downloads list file.");

		return false;
	}

	if(!forceUpdate && std::filesystem::is_regular_file(std::filesystem::path(gameDownloadsListFilePath))) {
		std::unique_ptr<GameDownloadCollection> gameDownloads(std::make_unique<GameDownloadCollection>());

		if(gameDownloads->loadFrom(gameDownloadsListFilePath) && GameDownloadCollection::isValid(gameDownloads.get())) {
			m_gameDownloads = std::move(gameDownloads);

			if(!shouldUpdateGameDownloadList()) {
				return true;
			}
		}
		else {
			spdlog::error("Failed to load game download collection from JSON file.");
		}
	}

	return updateGameDownloadList(forceUpdate);
}

bool GameManager::updateGameDownloadList(bool force) const {
	if(m_localMode) {
		return false;
	}

	HTTPService * httpService = HTTPService::getInstance();

	if(!httpService->isInitialized()) {
		return false;
	}

	SettingsManager * settings = SettingsManager::getInstance();

	if(settings->remoteDownloadsDirectoryName.empty()) {
		spdlog::error("Missing remote downloads directory name setting.");
		return {};
	}

	if(settings->remoteGameDownloadsDirectoryName.empty()) {
		spdlog::error("Missing remote game downloads directory name setting.");
		return {};
	}

	if(settings->remoteGamesListFileName.empty()) {
		spdlog::error("Missing remote games list file name setting.");
		return {};
	}

	std::string gameListRemoteFilePath(Utilities::joinPaths(settings->remoteDownloadsDirectoryName, settings->remoteGameDownloadsDirectoryName, settings->remoteGamesListFileName));
	std::string gameListURL(Utilities::joinPaths(httpService->getBaseURL(), gameListRemoteFilePath));

	spdlog::info("Downloading Duke Nukem 3D game download list from: '{}'...", gameListURL);

	std::shared_ptr<HTTPRequest> request(httpService->createRequest(HTTPRequest::Method::Get, gameListURL));

	std::map<std::string, std::string>::const_iterator fileETagIterator = settings->fileETags.find(settings->remoteGamesListFileName);

	if(!force && fileETagIterator != settings->fileETags.end() && !fileETagIterator->second.empty()) {
		request->setIfNoneMatchETag(fileETagIterator->second);
	}

	std::shared_ptr<HTTPResponse> response(httpService->sendRequestAndWait(request));

	if(response->isFailure()) {
		spdlog::error("Failed to download Duke Nukem 3D game download list with error: {}", response->getErrorMessage());
		return false;
	}

	if(response->getStatusCode() == magic_enum::enum_integer(HTTPStatusCode::NotModified)) {
		spdlog::info("Duke Nukem 3D game download list is already up to date!");

		settings->gameDownloadListLastDownloadedTimestamp = std::chrono::system_clock::now();

		return true;
	}
	else if(response->isFailureStatusCode()) {
		std::string statusCodeName(HTTPUtilities::getStatusCodeName(response->getStatusCode()));
		spdlog::error("Failed to download Duke Nukem 3D game download list ({}{})!", response->getStatusCode(), statusCodeName.empty() ? "" : " " + statusCodeName);
		return false;
	}

	spdlog::info("Duke Nukem 3D game download list downloaded successfully after {} ms.", response->getRequestDuration().value().count());

	std::unique_ptr<rapidjson::Document> gameDownloadCollectionDocument(response->getBodyAsJSON());

	if(gameDownloadCollectionDocument == nullptr) {
		spdlog::error("Failed to parse game download collection JSON data.");
		return false;
	}

	std::unique_ptr<GameDownloadCollection> gameDownloads(GameDownloadCollection::parseFrom(*gameDownloadCollectionDocument));

	if(!GameDownloadCollection::isValid(gameDownloads.get())) {
		spdlog::error("Failed to parse game download collection from JSON data.");
		return false;
	}

	std::string gameDownloadsListFilePath(getGameDownloadsListFilePath());

	if(gameDownloadsListFilePath.empty()) {
		spdlog::error("Failed to determine game downloads file list path. Are your settings configured correctly?");
		return false;
	}

	if(!response->getBody()->writeTo(gameDownloadsListFilePath, true)) {
		spdlog::error("Failed to write game download collection JSON data to file: '{}'.", gameDownloadsListFilePath);
		return false;
	}

	m_gameDownloads = std::move(gameDownloads);

	settings->fileETags.emplace(settings->remoteGamesListFileName, response->getETag());
	settings->gameDownloadListLastDownloadedTimestamp = std::chrono::system_clock::now();
	settings->save();

	return true;
}

bool GameManager::isGameDownloadable(const std::string & gameVersionID) {
	return Utilities::areStringsEqualIgnoreCase(gameVersionID, GameVersion::LAMEDUKE.getID()) ||
		   Utilities::areStringsEqualIgnoreCase(gameVersionID, GameVersion::ORIGINAL_BETA_VERSION.getID()) ||
		   Utilities::areStringsEqualIgnoreCase(gameVersionID, GameVersion::ORIGINAL_REGULAR_VERSION.getID()) ||
		   Utilities::areStringsEqualIgnoreCase(gameVersionID, GameVersion::ORIGINAL_PLUTONIUM_PAK.getID()) ||
		   Utilities::areStringsEqualIgnoreCase(gameVersionID, GameVersion::ORIGINAL_ATOMIC_EDITION.getID()) ||
		   Utilities::areStringsEqualIgnoreCase(gameVersionID, GameVersion::JFDUKE3D.getID()) ||
		   Utilities::areStringsEqualIgnoreCase(gameVersionID, GameVersion::EDUKE32.getID()) ||
		   Utilities::areStringsEqualIgnoreCase(gameVersionID, GameVersion::NETDUKE32.getID()) ||
		   Utilities::areStringsEqualIgnoreCase(gameVersionID, GameVersion::RAZE.getID()) ||
		   Utilities::areStringsEqualIgnoreCase(gameVersionID, GameVersion::RED_NUKEM.getID()) ||
		   Utilities::areStringsEqualIgnoreCase(gameVersionID, GameVersion::BELGIAN_CHOCOLATE_DUKE3D.getID()) ||
		   Utilities::areStringsEqualIgnoreCase(gameVersionID, GameVersion::DUKE3DW.getID()) ||
		   Utilities::areStringsEqualIgnoreCase(gameVersionID, GameVersion::PKDUKE3D.getID()) ||
		   Utilities::areStringsEqualIgnoreCase(gameVersionID, GameVersion::XDUKE.getID()) ||
		   Utilities::areStringsEqualIgnoreCase(gameVersionID, GameVersion::RDUKE.getID()) ||
		   Utilities::areStringsEqualIgnoreCase(gameVersionID, GameVersion::DUKE3D_W32.getID());
}

std::string GameManager::getGameDownloadURL(const std::string & gameVersionID) {
	if(!loadOrUpdateGameDownloadList()) {
		return {};
	}

	if(Utilities::areStringsEqualIgnoreCase(gameVersionID, GameVersion::LAMEDUKE.getID()) ||
	   Utilities::areStringsEqualIgnoreCase(gameVersionID, GameVersion::ORIGINAL_BETA_VERSION.getID()) ||
	   Utilities::areStringsEqualIgnoreCase(gameVersionID, GameVersion::ORIGINAL_REGULAR_VERSION.getID()) ||
	   Utilities::areStringsEqualIgnoreCase(gameVersionID, GameVersion::ORIGINAL_PLUTONIUM_PAK.getID()) ||
	   Utilities::areStringsEqualIgnoreCase(gameVersionID, GameVersion::ORIGINAL_ATOMIC_EDITION.getID())) {
		std::shared_ptr<GameDownloadFile> gameDownloadFile(m_gameDownloads->getLatestGameDownloadFile(gameVersionID, GameDownloadFile::Type::Game, GameVersion::OperatingSystem::DOS));

		if(gameDownloadFile == nullptr) {
			spdlog::error("'{}' game download entry not found in game download collection.", gameVersionID);
			return {};
		}

		return Utilities::joinPaths(getRemoteGameDownloadsBaseURL(), gameDownloadFile->getFileName());
	}

	DeviceInformationBridge * deviceInformationBridge = DeviceInformationBridge::getInstance();

	std::optional<DeviceInformationBridge::OperatingSystemType> optionalOperatingSystemType(deviceInformationBridge->getOperatingSystemType());

	if(!optionalOperatingSystemType.has_value()) {
		spdlog::error("Failed to determine operating system type.");
		return {};
	}

	std::optional<DeviceInformationBridge::OperatingSystemArchitectureType> optionalOperatingSystemArchitectureType(deviceInformationBridge->getOperatingSystemArchitectureType());

	if(!optionalOperatingSystemArchitectureType.has_value()) {
		spdlog::error("Failed to determine operating system architecture type.");
		return {};
	}

	if(Utilities::areStringsEqualIgnoreCase(gameVersionID, GameVersion::JFDUKE3D.getID())) {
		return getJFDuke3DDownloadURL(optionalOperatingSystemType.value(), optionalOperatingSystemArchitectureType.value());
	}
	else if(Utilities::areStringsEqualIgnoreCase(gameVersionID, GameVersion::EDUKE32.getID())) {
		return getEDuke32DownloadURL(optionalOperatingSystemType.value(), optionalOperatingSystemArchitectureType.value());
	}
	else if(Utilities::areStringsEqualIgnoreCase(gameVersionID, GameVersion::NETDUKE32.getID())) {
		return getNetDuke32DownloadURL(optionalOperatingSystemType.value(), optionalOperatingSystemArchitectureType.value());
	}
	else if(Utilities::areStringsEqualIgnoreCase(gameVersionID, GameVersion::RAZE.getID())) {
		return getRazeDownloadURL(optionalOperatingSystemType.value(), optionalOperatingSystemArchitectureType.value());
	}
	else if(Utilities::areStringsEqualIgnoreCase(gameVersionID, GameVersion::RED_NUKEM.getID())) {
		return getRedNukemDownloadURL(optionalOperatingSystemType.value(), optionalOperatingSystemArchitectureType.value());
	}
	else if(Utilities::areStringsEqualIgnoreCase(gameVersionID, GameVersion::BELGIAN_CHOCOLATE_DUKE3D.getID())) {
		return getBelgianChocolateDuke3DDownloadURL(optionalOperatingSystemType.value(), optionalOperatingSystemArchitectureType.value());
	}
	else if(Utilities::areStringsEqualIgnoreCase(gameVersionID, GameVersion::DUKE3DW.getID())) {
		return getDuke3dwDownloadURL(optionalOperatingSystemType.value(), optionalOperatingSystemArchitectureType.value());
	}
	else if(Utilities::areStringsEqualIgnoreCase(gameVersionID, GameVersion::PKDUKE3D.getID())) {
		return getPKDuke3DDownloadURL(optionalOperatingSystemType.value(), optionalOperatingSystemArchitectureType.value());
	}
	else if(Utilities::areStringsEqualIgnoreCase(gameVersionID, GameVersion::XDUKE.getID())) {
		return getXDukeDownloadURL(optionalOperatingSystemType.value(), optionalOperatingSystemArchitectureType.value());
	}
	else if(Utilities::areStringsEqualIgnoreCase(gameVersionID, GameVersion::RDUKE.getID())) {
		return getRDukeDownloadURL(optionalOperatingSystemType.value(), optionalOperatingSystemArchitectureType.value());
	}
	else if(Utilities::areStringsEqualIgnoreCase(gameVersionID, GameVersion::DUKE3D_W32.getID())) {
		return getDuke3d_w32DownloadURL(optionalOperatingSystemType.value(), optionalOperatingSystemArchitectureType.value());
	}

	return {};
}

std::string GameManager::getGameDownloadSHA1(const std::string & gameVersionID) {
	if(!m_initialized) {
		spdlog::error("Game manager not initialized!");
		return {};
	}

	if(Utilities::areStringsEqualIgnoreCase(gameVersionID, GameVersion::LAMEDUKE.getID()) ||
	   Utilities::areStringsEqualIgnoreCase(gameVersionID, GameVersion::ORIGINAL_BETA_VERSION.getID()) ||
	   Utilities::areStringsEqualIgnoreCase(gameVersionID, GameVersion::ORIGINAL_REGULAR_VERSION.getID()) ||
	   Utilities::areStringsEqualIgnoreCase(gameVersionID, GameVersion::ORIGINAL_PLUTONIUM_PAK.getID()) ||
	   Utilities::areStringsEqualIgnoreCase(gameVersionID, GameVersion::ORIGINAL_ATOMIC_EDITION.getID())) {
		std::shared_ptr<GameDownloadFile> gameDownloadFile(m_gameDownloads->getLatestGameDownloadFile(gameVersionID, GameDownloadFile::Type::Game, GameVersion::OperatingSystem::DOS));

		if(gameDownloadFile == nullptr) {
			spdlog::error("'{}' game download entry not found in game download collection.", gameVersionID);
			return {};
		}

		return gameDownloadFile->getSHA1();
	}

	return {};
}

std::string GameManager::getGroupDownloadURL(const std::string & gameVersionID) {
	if(!m_initialized) {
		spdlog::error("Game manager not initialized!");
		return {};
	}

	std::shared_ptr<GameVersion> groupGameVersion(getGroupGameVersion(gameVersionID));

	std::shared_ptr<GameDownloadFile> groupDownloadFile(m_gameDownloads->getLatestGameDownloadFile(groupGameVersion->getID(), GameDownloadFile::Type::Group, GameVersion::OperatingSystem::DOS));

	if(groupDownloadFile == nullptr) {
		spdlog::error("'{}' group download entry not found in game download collection.", gameVersionID);
		return {};
	}

	return Utilities::joinPaths(getRemoteGameDownloadsBaseURL(), groupDownloadFile->getFileName());
}

std::string GameManager::getFallbackGameDownloadURL(const std::string & gameVersionID) const {
	if(Utilities::areStringsEqualIgnoreCase(gameVersionID, GameVersion::LAMEDUKE.getID())) {
		return LAMEDUKE_FALLBACK_DOWNLOAD_URL;
	}
	else if(Utilities::areStringsEqualIgnoreCase(gameVersionID, GameVersion::ORIGINAL_BETA_VERSION.getID())) {
		return BETA_VERSION_FALLBACK_DOWNLOAD_URL;
	}
	else if(Utilities::areStringsEqualIgnoreCase(gameVersionID, GameVersion::ORIGINAL_REGULAR_VERSION.getID())) {
		return REGULAR_VERSION_FALLBACK_DOWNLOAD_URL;
	}
	else if(Utilities::areStringsEqualIgnoreCase(gameVersionID, GameVersion::ORIGINAL_PLUTONIUM_PAK.getID())) {
		spdlog::warn("No fallback download URL for Plutonium Pak.");
		return {};
	}
	else if(Utilities::areStringsEqualIgnoreCase(gameVersionID, GameVersion::ORIGINAL_ATOMIC_EDITION.getID())) {
		return ATOMIC_EDITION_FALLBACK_DOWNLOAD_URL;
	}

	if(!loadOrUpdateGameDownloadList()) {
		return {};
	}

	DeviceInformationBridge * deviceInformationBridge = DeviceInformationBridge::getInstance();

	std::optional<DeviceInformationBridge::OperatingSystemType> optionalOperatingSystemType(deviceInformationBridge->getOperatingSystemType());

	if(!optionalOperatingSystemType.has_value()) {
		spdlog::error("Failed to determine operating system type.");
		return {};
	}

	std::optional<DeviceInformationBridge::OperatingSystemArchitectureType> optionalOperatingSystemArchitectureType(deviceInformationBridge->getOperatingSystemArchitectureType());

	if(!optionalOperatingSystemArchitectureType.has_value()) {
		spdlog::error("Failed to determine operating system architecture type.");
		return {};
	}

	std::shared_ptr<GameDownloadFile> gameDownloadFile(m_gameDownloads->getLatestGameDownloadFile(gameVersionID, GameDownloadFile::Type::Game, optionalOperatingSystemType.value(), optionalOperatingSystemArchitectureType.value()));

	if(gameDownloadFile == nullptr) {
		switch(optionalOperatingSystemArchitectureType.value()) {
			case DeviceInformationBridge::OperatingSystemArchitectureType::x86: {
				spdlog::error("Could not find '{}' game file download information for '{}' ({}).", gameVersionID, magic_enum::enum_name(optionalOperatingSystemType.value()), magic_enum::enum_name(optionalOperatingSystemArchitectureType.value()));
				return {};
			}
			case DeviceInformationBridge::OperatingSystemArchitectureType::x64: {
				gameDownloadFile = m_gameDownloads->getLatestGameDownloadFile(gameVersionID, GameDownloadFile::Type::Game, optionalOperatingSystemType.value(), DeviceInformationBridge::OperatingSystemArchitectureType::x86);

				if(gameDownloadFile == nullptr) {
					spdlog::error("Could not find '{}' game file download information for '{}' ({} or {}).", gameVersionID, magic_enum::enum_name(optionalOperatingSystemType.value()), magic_enum::enum_name(DeviceInformationBridge::OperatingSystemArchitectureType::x64), magic_enum::enum_name(DeviceInformationBridge::OperatingSystemArchitectureType::x86));
					return {};
				}

				break;
			}
		}
	}

	return Utilities::joinPaths(getRemoteGameDownloadsBaseURL(), gameDownloadFile->getFileName());
}

std::string GameManager::getFallbackGameDownloadSHA1(const std::string & gameVersionID) const {
	if(Utilities::areStringsEqualIgnoreCase(gameVersionID, GameVersion::LAMEDUKE.getID())) {
		return LAMEDUKE_DOWNLOAD_SHA1;
	}
	else if(Utilities::areStringsEqualIgnoreCase(gameVersionID, GameVersion::ORIGINAL_BETA_VERSION.getID())) {
		return BETA_VERSION_FALLBACK_DOWNLOAD_SHA1;
	}
	else if(Utilities::areStringsEqualIgnoreCase(gameVersionID, GameVersion::ORIGINAL_REGULAR_VERSION.getID())) {
		return REGULAR_VERSION_FALLBACK_DOWNLOAD_SHA1;
	}
	else if(Utilities::areStringsEqualIgnoreCase(gameVersionID, GameVersion::ORIGINAL_PLUTONIUM_PAK.getID())) {
		spdlog::warn("No fallback download for Plutonium Pak.");
		return {};
	}
	else if(Utilities::areStringsEqualIgnoreCase(gameVersionID, GameVersion::ORIGINAL_ATOMIC_EDITION.getID())) {
		return ATOMIC_EDITION_FALLBACK_DOWNLOAD_SHA1;
	}

	if(!loadOrUpdateGameDownloadList()) {
		return {};
	}

	DeviceInformationBridge * deviceInformationBridge = DeviceInformationBridge::getInstance();

	std::optional<DeviceInformationBridge::OperatingSystemType> optionalOperatingSystemType(deviceInformationBridge->getOperatingSystemType());

	if(!optionalOperatingSystemType.has_value()) {
		spdlog::error("Failed to determine operating system type.");
		return {};
	}

	std::optional<DeviceInformationBridge::OperatingSystemArchitectureType> optionalOperatingSystemArchitectureType(deviceInformationBridge->getOperatingSystemArchitectureType());

	if(!optionalOperatingSystemArchitectureType.has_value()) {
		spdlog::error("Failed to determine operating system architecture type.");
		return {};
	}

	std::shared_ptr<GameDownloadFile> gameDownloadFile(m_gameDownloads->getLatestGameDownloadFile(gameVersionID, GameDownloadFile::Type::Game, optionalOperatingSystemType.value(), optionalOperatingSystemArchitectureType.value()));

	if(gameDownloadFile == nullptr) {
		switch(optionalOperatingSystemArchitectureType.value()) {
			case DeviceInformationBridge::OperatingSystemArchitectureType::x86: {
				spdlog::error("Could not find '{}' game file download information for '{}' ({}).", gameVersionID, magic_enum::enum_name(optionalOperatingSystemType.value()), magic_enum::enum_name(optionalOperatingSystemArchitectureType.value()));
				return {};
			}
			case DeviceInformationBridge::OperatingSystemArchitectureType::x64: {
				gameDownloadFile = m_gameDownloads->getLatestGameDownloadFile(gameVersionID, GameDownloadFile::Type::Game, optionalOperatingSystemType.value(), DeviceInformationBridge::OperatingSystemArchitectureType::x86);

				if(gameDownloadFile == nullptr) {
					spdlog::error("Could not find '{}' game file download information for '{}' ({} or {}).", gameVersionID, magic_enum::enum_name(optionalOperatingSystemType.value()), magic_enum::enum_name(DeviceInformationBridge::OperatingSystemArchitectureType::x64), magic_enum::enum_name(DeviceInformationBridge::OperatingSystemArchitectureType::x86));
					return {};
				}

				break;
			}
		}
	}

	return gameDownloadFile->getSHA1();
}

std::string GameManager::getRemoteGameDownloadsBaseURL() const {
	if(!m_initialized) {
		spdlog::error("Game manager not initialized!");
		return {};
	}

	SettingsManager * settings = SettingsManager::getInstance();

	if(settings->apiBaseURL.empty()) {
		spdlog::error("Missing API base URL setting.");
		return {};
	}

	if(settings->remoteDownloadsDirectoryName.empty()) {
		spdlog::error("Missing remote downloads directory name setting.");
		return {};
	}

	if(settings->remoteGameDownloadsDirectoryName.empty()) {
		spdlog::error("Missing remote game downloads directory name setting.");
		return {};
	}

	return Utilities::joinPaths(settings->apiBaseURL, settings->remoteDownloadsDirectoryName, settings->remoteGameDownloadsDirectoryName);
}

std::string GameManager::getJFDuke3DDownloadURL(DeviceInformationBridge::OperatingSystemType operatingSystemType, DeviceInformationBridge::OperatingSystemArchitectureType operatingSystemArchitectureType) const {
	static const std::string WINDOWS_IDENTIFIER("win");
	static const std::string WINDOWS_X86_ARCHITECTURE_IDENTIFIER("x86");

	if(!m_initialized) {
		spdlog::error("Game manager not initialized!");
		return {};
	}

	HTTPService * httpService = HTTPService::getInstance();

	if(!httpService->isInitialized()) {
		return {};
	}

	std::shared_ptr<GameVersion> jfDuke3DGameVersion(m_gameVersions->getGameVersionWithID(GameVersion::JFDUKE3D.getID()));
	const GameVersion * jfDuke3DGameVersionRaw = jfDuke3DGameVersion != nullptr ? jfDuke3DGameVersion.get() : &GameVersion::JFDUKE3D;

	if(!jfDuke3DGameVersionRaw->hasSupportedOperatingSystemType(operatingSystemType)) {
		return {};
	}

	size_t schemeSuffixIndex = JFDUKE32_DOWNLOAD_PAGE_URL.find("://");

	if(schemeSuffixIndex == std::string::npos) {
		spdlog::error("Malformed JFDuke3D download page URL, missing scheme.");
		return {};
	}

	size_t firstPathSeparatorIndex = JFDUKE32_DOWNLOAD_PAGE_URL.find_first_of("/", schemeSuffixIndex + 3);

	std::string_view downloadPageBaseURL(JFDUKE32_DOWNLOAD_PAGE_URL.data(), JFDUKE32_DOWNLOAD_PAGE_URL.length());

	if(firstPathSeparatorIndex != std::string::npos) {
		downloadPageBaseURL = std::string_view(JFDUKE32_DOWNLOAD_PAGE_URL.data(), firstPathSeparatorIndex);
	}

	std::shared_ptr<HTTPRequest> downloadPageRequest(httpService->createRequest(HTTPRequest::Method::Get, JFDUKE32_DOWNLOAD_PAGE_URL));
	downloadPageRequest->setConnectionTimeout(5s);
	downloadPageRequest->setNetworkTimeout(10s);

	std::future<std::shared_ptr<HTTPResponse>> futureResponse(httpService->sendRequest(downloadPageRequest));

	if(!futureResponse.valid()) {
		spdlog::error("Failed to create JFDuke3D download page HTTP request!");
		return {};
	}

	futureResponse.wait();

	std::shared_ptr<HTTPResponse> response(futureResponse.get());

	if(response->isFailure()) {
		spdlog::error("Failed to retrieve JFDuke3D download page with error: {}", response->getErrorMessage());
		return {};
	}

	if(response->isFailureStatusCode()) {
		std::string statusCodeName(HTTPUtilities::getStatusCodeName(response->getStatusCode()));
		spdlog::error("Failed to get JFDuke3D download page ({}{})!", response->getStatusCode(), statusCodeName.empty() ? "" : " " + statusCodeName);
		return {};
	}

	std::string pageHTML(Utilities::tidyHTML(response->getBodyAsString()));

	if(pageHTML.empty()) {
		spdlog::error("Failed to tidy JFDuke3D download page HTML.");
		return {};
	}

	response.reset();

	tinyxml2::XMLDocument document;

	if(document.Parse(pageHTML.c_str(), pageHTML.length()) != tinyxml2::XML_SUCCESS) {
		spdlog::error("Failed to parse JDFuke3D download page XHTML with error: '{}'.", document.ErrorStr());
		return {};
	}

	std::vector<const tinyxml2::XMLElement *> downloadElements(Utilities::findXMLElementsWithAttributeValue(document.RootElement(), "class", "download"));

	if(downloadElements.empty()) {
		spdlog::error("JFDuke3D download page parsing failed, missing download element.");
		return {};
	}

	if(downloadElements.size() != 1) {
		spdlog::warn("More than one download element found on JFDuke3D download page.");
	}

	const tinyxml2::XMLElement * downloadElement = downloadElements[0];

	std::vector<const tinyxml2::XMLElement *> downloadLinkElements(Utilities::findXMLElementsWithName(downloadElement, "a"));

	const char * classAttributeRawValue = nullptr;
	const char * linkAttributeRawValue = nullptr;
	std::string_view classAttributeValue;
	std::string_view linkAttributeValue;
	std::string downloadPath;
	std::string windowsX86DownloadPath;
	std::string windowsX64DownloadPath;

	for(std::vector<const tinyxml2::XMLElement *>::const_iterator i = downloadLinkElements.cbegin(); i != downloadLinkElements.cend(); ++i) {
		classAttributeRawValue = (*i)->Attribute("class");

		if(classAttributeRawValue == nullptr) {
			continue;
		}

		linkAttributeRawValue = (*i)->Attribute("href");

		if(linkAttributeRawValue == nullptr) {
			continue;
		}

		classAttributeValue = classAttributeRawValue;
		linkAttributeValue = linkAttributeRawValue;

		if(operatingSystemType == DeviceInformationBridge::OperatingSystemType::MacOS) {
			if(classAttributeValue.find("mac") != std::string::npos) {
				downloadPath = linkAttributeValue;
			}
		}
		else if(operatingSystemType == DeviceInformationBridge::OperatingSystemType::Windows) {
			if(classAttributeValue.find("win") != std::string::npos) {
				if(classAttributeValue.find("x86") == std::string::npos) {
					windowsX86DownloadPath = linkAttributeValue;
				}
				else {
					windowsX64DownloadPath = linkAttributeValue;
				}
			}
		}
	}

	if(operatingSystemType == DeviceInformationBridge::OperatingSystemType::Windows) {
		if(operatingSystemArchitectureType == DeviceInformationBridge::OperatingSystemArchitectureType::x64) {
			if(!windowsX64DownloadPath.empty()) {
				downloadPath = windowsX64DownloadPath;
			}
			else {
				downloadPath = windowsX86DownloadPath;
			}
		}
		else if(operatingSystemArchitectureType == DeviceInformationBridge::OperatingSystemArchitectureType::x86) {
			downloadPath = windowsX86DownloadPath;
		}
	}

	if(downloadPath.empty()) {
		spdlog::error("Failed to determine JFDuke3D download URL from download page XHTML.");
		return {};
	}

	if(downloadPath[0] != '/') {
		return Utilities::joinPaths(JFDUKE32_DOWNLOAD_PAGE_URL, downloadPath);
	}
	else {
		return Utilities::joinPaths(downloadPageBaseURL, downloadPath);
	}
}

struct NetDuke32Release {
	std::string name;
	std::string tagName;
	std::string version;
	std::string htmlDescription;
	std::chrono::time_point<std::chrono::system_clock> createdTimestamp;
	std::chrono::time_point<std::chrono::system_clock> releasedTimestamp;
	std::string downloadURL;
	std::string authorUserName;
	std::string commitTitle;
	std::string commitHash;
};

static std::vector<NetDuke32Release> getNetDuke32Releases(size_t maxNumberOfReleases = 10) {
	static const std::string NETDUKE32_BASE_DOWNLOAD_URL("https://voidpoint.io");

	if(maxNumberOfReleases == 0) {
		return {};
	}

	HTTPService * httpService = HTTPService::getInstance();

	if(!httpService->isInitialized()) {
		return {};
	}

	std::shared_ptr<HTTPRequest> downloadPageRequest(httpService->createRequest(HTTPRequest::Method::Post, NETDUKE32_DOWNLOAD_API_URL));
	downloadPageRequest->setConnectionTimeout(5s);
	downloadPageRequest->setNetworkTimeout(10s);

	rapidjson::Document releaseListGraphQLQueryDocument(rapidjson::kArrayType);
	rapidjson::MemoryPoolAllocator<rapidjson::CrtAllocator> & allocator = releaseListGraphQLQueryDocument.GetAllocator();

	rapidjson::Value queryValue(rapidjson::kObjectType);
	queryValue.Reserve(3, allocator);

	queryValue.AddMember(rapidjson::StringRef("operationName"), rapidjson::StringRef("allReleases"), allocator);

	rapidjson::Value queryVariablesValue(rapidjson::kObjectType);

	queryVariablesValue.AddMember(rapidjson::StringRef("fullPath"), rapidjson::StringRef("StrikerTheHedgefox/eduke32-csrefactor"), allocator);
	queryVariablesValue.AddMember(rapidjson::StringRef("first"), rapidjson::Value(maxNumberOfReleases), allocator);
	queryVariablesValue.AddMember(rapidjson::StringRef("sort"), rapidjson::Value("RELEASED_AT_DESC"), allocator);

	queryValue.AddMember(rapidjson::StringRef("variables"), queryVariablesValue, allocator);

	queryValue.AddMember(rapidjson::StringRef("query"), rapidjson::StringRef("query allReleases($fullPath: ID!, $first: Int, $last: Int, $before: String, $after: String, $sort: ReleaseSort) { project(fullPath: $fullPath) { id releases(first: $first last: $last before: $before after: $after sort: $sort) { nodes { ...Release __typename } pageInfo { startCursor hasPreviousPage hasNextPage endCursor __typename } __typename } __typename } } fragment Release on Release { id name tagName tagPath descriptionHtml releasedAt createdAt upcomingRelease historicalRelease assets { count sources { nodes { format url __typename } __typename } links { nodes { id name url directAssetUrl linkType external __typename } __typename } __typename } evidences { nodes { id filepath collectedAt sha __typename } __typename } links { editUrl selfUrl openedIssuesUrl closedIssuesUrl openedMergeRequestsUrl mergedMergeRequestsUrl closedMergeRequestsUrl __typename } commit { id sha webUrl title __typename } author { id webUrl avatarUrl username __typename } milestones { nodes { id title description webPath stats { totalIssuesCount closedIssuesCount __typename } __typename } __typename } __typename }"), allocator);

	releaseListGraphQLQueryDocument.PushBack(queryValue, allocator);

	downloadPageRequest->setBody(releaseListGraphQLQueryDocument);

	std::shared_ptr<HTTPResponse> response(httpService->sendRequestAndWait(downloadPageRequest));

	if(response->isFailure()) {
		spdlog::error("Failed to retrieve NetDuke32 release list with error: {}", response->getErrorMessage());
		return {};
	}
	else if(response->isFailureStatusCode()) {
		std::string statusCodeName(HTTPUtilities::getStatusCodeName(response->getStatusCode()));
		spdlog::error("Failed to get NetDuke32 release list ({}{})!", response->getStatusCode(), statusCodeName.empty() ? "" : " " + statusCodeName);
		return {};
	}

	std::unique_ptr<rapidjson::Document> releaseListDocument(response->getBodyAsJSON());

	response.reset();

	if(releaseListDocument == nullptr) {
		spdlog::error("Failed to parse NetDuke32 release list JSON data.");
		return {};
	}

	if(!releaseListDocument->IsArray()) {
		spdlog::error("Invalid NetDuke32 release list root type '{}', expected 'array'.", Utilities::typeToString(releaseListDocument->GetType()));
		return {};
	}

	if(releaseListDocument->Empty()) {
		spdlog::error("Empty NetDuke32 release list root value, expected non-empty 'array'.");
		return {};
	}

	const rapidjson::Value & queryResultValue = (*releaseListDocument)[0];

	if(!queryResultValue.IsObject()) {
		spdlog::error("Invalid NetDuke32 release list query result type '{}', expected 'object'.", Utilities::typeToString(queryResultValue.GetType()));
		return {};
	}

	if(!queryResultValue.HasMember("data")) {
		spdlog::error("NetDuke32 release list query result is missing 'data' property.");
		return {};
	}

	const rapidjson::Value & dataValue = queryResultValue["data"];

	if(!dataValue.IsObject()) {
		spdlog::error("Invalid NetDuke32 release list query result 'data' type '{}', expected 'object'.", Utilities::typeToString(dataValue.GetType()));
		return {};
	}

	if(!dataValue.HasMember("project")) {
		spdlog::error("NetDuke32 release list query result 'data' is missing 'project' property.");
		return {};
	}

	const rapidjson::Value & projectValue = dataValue["project"];

	if(!projectValue.IsObject()) {
		spdlog::error("Invalid NetDuke32 release list query result 'data' 'project' type '{}', expected 'object'.", Utilities::typeToString(projectValue.GetType()));
		return {};
	}

	if(!projectValue.HasMember("releases")) {
		spdlog::error("NetDuke32 release list query result 'data' 'project' is missing 'releases' property.");
		return {};
	}

	const rapidjson::Value & releasesValue = projectValue["releases"];

	if(!releasesValue.IsObject()) {
		spdlog::error("Invalid NetDuke32 release list query result 'data' 'project' 'releases' type '{}', expected 'object'.", Utilities::typeToString(releasesValue.GetType()));
		return {};
	}

	if(!releasesValue.HasMember("nodes")) {
		spdlog::error("NetDuke32 release list query result 'data' 'project' 'releases' is missing 'nodes' property.");
		return {};
	}

	const rapidjson::Value & nodesValue = releasesValue["nodes"];

	if(!nodesValue.IsArray()) {
		spdlog::error("Invalid NetDuke32 release list query result 'data' 'project' 'releases' 'nodes' type '{}', expected 'array'.", Utilities::typeToString(nodesValue.GetType()));
		return {};
	}

	if(nodesValue.Empty()) {
		spdlog::error("Empty NetDuke32 release list query result 'data' 'project' 'releases' 'nodes' value, expected non-empty 'array'.");
		return {};
	}

	std::vector<NetDuke32Release> netDuke32Releases;
	netDuke32Releases.reserve(nodesValue.Size());

	for(rapidjson::Value::ConstValueIterator i = nodesValue.Begin(); i != nodesValue.End(); ++i) {
		const rapidjson::Value & releaseNodeValue = *i;
		NetDuke32Release netDuke32Release;

		if(!releaseNodeValue.IsObject()) {
			spdlog::error("Invalid NetDuke32 release list query result 'data' 'project' 'releases' 'nodes' entry #{} type '{}', expected 'object'.", i - nodesValue.Begin(), Utilities::typeToString(nodesValue.GetType()));
			continue;
		}

		// parse release name property
		if(!releaseNodeValue.HasMember("name")) {
			spdlog::error("NetDuke32 release list query result 'data' 'project' 'releases' 'nodes' entry #{} is missing 'name' property.", i - nodesValue.Begin());
			continue;
		}

		const rapidjson::Value & nameValue = releaseNodeValue["name"];

		if(!nameValue.IsString()) {
			spdlog::error("Invalid NetDuke32 release list query result 'data' 'project' 'releases' 'nodes' entry #{} 'name' property type '{}', expected 'string'.", i - nodesValue.Begin(), Utilities::typeToString(nameValue.GetType()));
			continue;
		}

		netDuke32Release.name = nameValue.GetString();

		// parse release tag name property
		if(!releaseNodeValue.HasMember("tagName")) {
			spdlog::error("NetDuke32 release list query result 'data' 'project' 'releases' 'nodes' entry #{} is missing 'tagName' property.", i - nodesValue.Begin());
			continue;
		}

		const rapidjson::Value & tagNameValue = releaseNodeValue["tagName"];

		if(!tagNameValue.IsString()) {
			spdlog::error("Invalid NetDuke32 release list query result 'data' 'project' 'releases' 'nodes' entry #{} 'tagName' property type '{}', expected 'string'.", i - nodesValue.Begin(), Utilities::typeToString(tagNameValue.GetType()));
			continue;
		}

		netDuke32Release.tagName = tagNameValue.GetString();

		// parse release HTML description property
		if(!releaseNodeValue.HasMember("descriptionHtml")) {
			spdlog::error("NetDuke32 release list query result 'data' 'project' 'releases' 'nodes' entry #{} is missing 'descriptionHtml' property.", i - nodesValue.Begin());
			continue;
		}

		const rapidjson::Value & htmlDescriptionValue = releaseNodeValue["descriptionHtml"];

		if(!htmlDescriptionValue.IsString()) {
			spdlog::error("Invalid NetDuke32 release list query result 'data' 'project' 'releases' 'nodes' entry #{} 'descriptionHtml' property type '{}', expected 'string'.", i - nodesValue.Begin(), Utilities::typeToString(htmlDescriptionValue.GetType()));
			continue;
		}

		netDuke32Release.htmlDescription = htmlDescriptionValue.GetString();

		// parse release created timestamp
		if(!releaseNodeValue.HasMember("createdAt")) {
			spdlog::error("NetDuke32 release list query result 'data' 'project' 'releases' 'nodes' entry #{} is missing 'createdAt' property.", i - nodesValue.Begin());
			continue;
		}

		const rapidjson::Value & createdAtValue = releaseNodeValue["createdAt"];

		if(!createdAtValue.IsString()) {
			spdlog::error("Invalid NetDuke32 release list query result 'data' 'project' 'releases' 'nodes' entry #{} 'createdAt' property type '{}', expected 'string'.", i - nodesValue.Begin(), Utilities::typeToString(createdAtValue.GetType()));
			continue;
		}

		std::optional<std::chrono::time_point<std::chrono::system_clock>> optionalCreatedTimestamp(Utilities::parseTimePointFromString(createdAtValue.GetString(), Utilities::TimeFormat::ISO8601));

		if(!optionalCreatedTimestamp.has_value()) {
			spdlog::error("Invalid NetDuke32 release list query result 'data' 'project' 'releases' 'nodes' entry #{} 'createdAt' ISO8601 timestamp string value: '{}'.", i - nodesValue.Begin(), createdAtValue.GetString());
			continue;
		}

		netDuke32Release.createdTimestamp = optionalCreatedTimestamp.value();

		// parse release released timestamp
		if(!releaseNodeValue.HasMember("releasedAt")) {
			spdlog::error("NetDuke32 release list query result 'data' 'project' 'releases' 'nodes' entry #{} is missing 'releasedAt' property.", i - nodesValue.Begin());
			continue;
		}

		const rapidjson::Value & releasedAtValue = releaseNodeValue["releasedAt"];

		if(!releasedAtValue.IsString()) {
			spdlog::error("Invalid NetDuke32 release list query result 'data' 'project' 'releases' 'nodes' entry #{} 'releasedAt' property type '{}', expected 'string'.", i - nodesValue.Begin(), Utilities::typeToString(releasedAtValue.GetType()));
			continue;
		}

		std::optional<std::chrono::time_point<std::chrono::system_clock>> optionalReleasedTimestamp(Utilities::parseTimePointFromString(releasedAtValue.GetString(), Utilities::TimeFormat::ISO8601));

		if(!optionalReleasedTimestamp.has_value()) {
			spdlog::error("Invalid NetDuke32 release list query result 'data' 'project' 'releases' 'nodes' entry #{} 'releasedAt' ISO8601 timestamp string value: '{}'.", i - nodesValue.Begin(), releasedAtValue.GetString());
			continue;
		}

		netDuke32Release.releasedTimestamp = optionalReleasedTimestamp.value();

		// get release author information
		if(!releaseNodeValue.HasMember("author")) {
			spdlog::error("NetDuke32 release list query result 'data' 'project' 'releases' 'nodes' entry #{} is missing 'author' property.", i - nodesValue.Begin());
			continue;
		}

		const rapidjson::Value & authorValue = releaseNodeValue["author"];

		if(!authorValue.IsObject()) {
			spdlog::error("Invalid NetDuke32 release list query result 'data' 'project' 'releases' 'nodes' entry #{} 'author' property type '{}', expected 'object'.", i - nodesValue.Begin(), Utilities::typeToString(authorValue.GetType()));
			continue;
		}

		// parse author user name
		if(!authorValue.HasMember("username")) {
			spdlog::error("NetDuke32 release list query result 'data' 'project' 'releases' 'nodes' entry #{} 'author' is missing 'username' property.", i - nodesValue.Begin());
			continue;
		}

		const rapidjson::Value & authorUserNameValue = authorValue["username"];

		if(!authorUserNameValue.IsString()) {
			spdlog::error("Invalid NetDuke32 release list query result 'data' 'project' 'releases' 'nodes' entry #{} 'author' 'username' property type '{}', expected 'string'.", i - nodesValue.Begin(), Utilities::typeToString(authorUserNameValue.GetType()));
			continue;
		}

		netDuke32Release.authorUserName = authorUserNameValue.GetString();

		// get release commit information
		if(!releaseNodeValue.HasMember("commit")) {
			spdlog::error("NetDuke32 release list query result 'data' 'project' 'releases' 'nodes' entry #{} is missing 'commit' property.", i - nodesValue.Begin());
			continue;
		}

		const rapidjson::Value & commitValue = releaseNodeValue["commit"];

		if(!commitValue.IsObject()) {
			spdlog::error("Invalid NetDuke32 release list query result 'data' 'project' 'releases' 'nodes' entry #{} 'commit' property type '{}', expected 'object'.", i - nodesValue.Begin(), Utilities::typeToString(commitValue.GetType()));
			continue;
		}

		// parse commit title
		if(!commitValue.HasMember("title")) {
			spdlog::error("NetDuke32 release list query result 'data' 'project' 'releases' 'nodes' entry #{} 'commit' is missing 'title' property.", i - nodesValue.Begin());
			continue;
		}

		const rapidjson::Value & commitTitleValue = commitValue["title"];

		if(!commitTitleValue.IsString()) {
			spdlog::error("Invalid NetDuke32 release list query result 'data' 'project' 'releases' 'nodes' entry #{} 'commit' 'title' property type '{}', expected 'string'.", i - nodesValue.Begin(), Utilities::typeToString(commitTitleValue.GetType()));
			continue;
		}

		netDuke32Release.commitTitle = commitTitleValue.GetString();

		// parse commit hash
		if(!commitValue.HasMember("sha")) {
			spdlog::error("NetDuke32 release list query result 'data' 'project' 'releases' 'nodes' entry #{} 'commit' is missing 'sha' property.", i - nodesValue.Begin());
			continue;
		}

		const rapidjson::Value & commitShaValue = commitValue["sha"];

		if(!commitShaValue.IsString()) {
			spdlog::error("Invalid NetDuke32 release list query result 'data' 'project' 'releases' 'nodes' entry #{} 'commit' 'sha' property type '{}', expected 'string'.", i - nodesValue.Begin(), Utilities::typeToString(commitShaValue.GetType()));
			continue;
		}

		netDuke32Release.commitHash = commitShaValue.GetString();

		// parse release version from name
		size_t versionStartIndex = netDuke32Release.name.find_first_of(" ");

		if(versionStartIndex != std::string::npos && netDuke32Release.name.length() >= versionStartIndex + 1) {
			versionStartIndex++;

			if(netDuke32Release.name.length() >= versionStartIndex + 1 && std::tolower(netDuke32Release.name[versionStartIndex]) == 'v') {
				versionStartIndex++;
			}

			netDuke32Release.version = netDuke32Release.name.substr(versionStartIndex);
		}
		else {
			netDuke32Release.version = nameValue.GetString();
		}

		// parse HTML description into XML data to extract download URL
		std::string xhtmlDescription(Utilities::tidyHTML(netDuke32Release.htmlDescription));

		if(xhtmlDescription.empty()) {
			spdlog::error("Failed to tidy NetDuke32 'data' 'project' 'releases' 'nodes' entry #{} HTML description into XHTML.", i - nodesValue.Begin());
			continue;
		}

		tinyxml2::XMLDocument descriptionDocument;

		if(descriptionDocument.Parse(xhtmlDescription.c_str(), xhtmlDescription.length()) != tinyxml2::XML_SUCCESS) {
			spdlog::error("Failed to parse NetDuke32 'data' 'project' 'releases' 'nodes' entry #{} XHTML description with error: '{}'.", i - nodesValue.Begin(), descriptionDocument.ErrorStr());
			continue;
		}

		const tinyxml2::XMLElement * descriptionBodyElement = Utilities::findFirstXMLElementWithName(descriptionDocument.RootElement(), "body");

		if(descriptionBodyElement == nullptr) {
			spdlog::error("NetDuke32 'data' 'project' 'releases' 'nodes' entry #{} XHTML description has no body element.", i - nodesValue.Begin());
			continue;
		}

		const tinyxml2::XMLElement * currentElement = descriptionBodyElement->FirstChildElement();

		if(descriptionBodyElement == nullptr) {
			spdlog::error("NetDuke32 'data' 'project' 'releases' 'nodes' entry #{} XHTML description body element has no children.", i - nodesValue.Begin());
			continue;
		}

		do {
			if(Utilities::findFirstXMLElementContainingText(currentElement, "Download:", true) == nullptr) {
				continue;
			}

			const tinyxml2::XMLElement * downloadLinkElement = Utilities::findFirstXMLElementWithName(currentElement, "a");

			if(downloadLinkElement == nullptr) {
				continue;
			}

			const char * downloadLinkHref = downloadLinkElement->Attribute("href");

			if(Utilities::stringLength(downloadLinkHref) == 0) {
				spdlog::error("NetDuke32 'data' 'project' 'releases' 'nodes' entry #{} XHTML description download link is missing or has an empty 'href' attribute value.", i - nodesValue.Begin());
				continue;
			}

			netDuke32Release.downloadURL = Utilities::joinPaths(NETDUKE32_BASE_DOWNLOAD_URL, downloadLinkHref);

			break;
		} while((currentElement = currentElement->NextSiblingElement()) != nullptr);

		if(netDuke32Release.downloadURL.empty()) {
			spdlog::error("NetDuke32 'data' 'project' 'releases' 'nodes' entry #{} XHTML description is malformed or has no download link.", i - nodesValue.Begin());
			continue;
		}

		netDuke32Releases.push_back(netDuke32Release);
	}

	return netDuke32Releases;
}

std::string GameManager::getNetDuke32DownloadURL(DeviceInformationBridge::OperatingSystemType operatingSystemType, DeviceInformationBridge::OperatingSystemArchitectureType operatingSystemArchitectureType) const {
	if(!m_initialized) {
		spdlog::error("Game manager not initialized!");
		return {};
	}

	std::shared_ptr<GameVersion> netDuke32GameVersion(m_gameVersions->getGameVersionWithID(GameVersion::NETDUKE32.getID()));
	const GameVersion * netDuke32GameVersionRaw = netDuke32GameVersion != nullptr ? netDuke32GameVersion.get() : &GameVersion::NETDUKE32;

	if(!netDuke32GameVersionRaw->hasSupportedOperatingSystemType(operatingSystemType)) {
		return {};
	}

	std::vector<NetDuke32Release> netDuke32Releases(getNetDuke32Releases(1));

	if(netDuke32Releases.empty()) {
		return {};
	}

	return netDuke32Releases.front().downloadURL;
}

std::string GameManager::getEDuke32DownloadURL(DeviceInformationBridge::OperatingSystemType operatingSystemType, DeviceInformationBridge::OperatingSystemArchitectureType operatingSystemArchitectureType) const {
	static const std::string WINDOWS_X86_ARCHITECTURE_IDENTIFIER("win32");
	static const std::string WINDOWS_X64_ARCHITECTURE_IDENTIFIER("win64");

	if(!m_initialized) {
		spdlog::error("Game manager not initialized!");
		return {};
	}

	HTTPService * httpService = HTTPService::getInstance();

	if(!httpService->isInitialized()) {
		return {};
	}

	std::shared_ptr<GameVersion> eDuke32GameVersion(m_gameVersions->getGameVersionWithID(GameVersion::EDUKE32.getID()));
	const GameVersion * eDuke32GameVersionRaw = eDuke32GameVersion != nullptr ? eDuke32GameVersion.get() : &GameVersion::EDUKE32;

	if(!eDuke32GameVersionRaw->hasSupportedOperatingSystemType(operatingSystemType)) {
		return {};
	}

	std::shared_ptr<HTTPRequest> downloadPageRequest(httpService->createRequest(HTTPRequest::Method::Get, EDUKE32_DOWNLOAD_PAGE_URL));
	downloadPageRequest->setConnectionTimeout(5s);
	downloadPageRequest->setNetworkTimeout(10s);

	std::future<std::shared_ptr<HTTPResponse>> futureResponse(httpService->sendRequest(downloadPageRequest));

	if(!futureResponse.valid()) {
		spdlog::error("Failed to create eDuke32 download page HTTP request!");
		return {};
	}

	futureResponse.wait();

	std::shared_ptr<HTTPResponse> response(futureResponse.get());

	if(response->isFailure()) {
		spdlog::error("Failed to retrieve eDuke32 download page with error: {}", response->getErrorMessage());
		return {};
	}

	if(response->isFailureStatusCode()) {
		std::string statusCodeName(HTTPUtilities::getStatusCodeName(response->getStatusCode()));
		spdlog::error("Failed to get eDuke32 download page ({}{})!", response->getStatusCode(), statusCodeName.empty() ? "" : " " + statusCodeName);
		return {};
	}

	std::string pageHTML(Utilities::tidyHTML(response->getBodyAsString()));

	if(pageHTML.empty()) {
		spdlog::error("Failed to tidy eDuke32 download page HTML.");
		return {};
	}

	response.reset();

	tinyxml2::XMLDocument document;

	if(document.Parse(pageHTML.c_str(), pageHTML.length()) != tinyxml2::XML_SUCCESS) {
		spdlog::error("Failed to parse eDuke32 download page XHTML with error: '{}'.", document.ErrorStr());
		return {};
	}

	const tinyxml2::XMLElement * listingElement = Utilities::findXMLElementWithID(document.RootElement(), "listing");

	if(listingElement == nullptr) {
		spdlog::error("eDuke32 download page parsing failed, could not find listing element.");
		return {};
	}

	std::string_view currentDownloadFileName;
	std::string windowsX86DownloadFileName;
	std::string windowsX64DownloadFileName;
	const tinyxml2::XMLElement * listingEntryLinkElement = nullptr;
	const tinyxml2::XMLElement * listingEntryElement = listingElement->FirstChildElement();

	while(true) {
		if(listingEntryElement == nullptr) {
			break;
		}

		listingEntryLinkElement = Utilities::findFirstXMLElementWithName(listingEntryElement, "a");

		if(listingEntryLinkElement != nullptr) {
			currentDownloadFileName = listingEntryLinkElement->Attribute("href");

			if(currentDownloadFileName.find("eduke32") == 0 && currentDownloadFileName.find("debug") == std::string::npos) {
				if(currentDownloadFileName.find(WINDOWS_X86_ARCHITECTURE_IDENTIFIER) != std::string::npos) {
					windowsX86DownloadFileName = currentDownloadFileName;
				}
				else if(currentDownloadFileName.find(WINDOWS_X64_ARCHITECTURE_IDENTIFIER) != std::string::npos) {
					windowsX64DownloadFileName = currentDownloadFileName;
				}
			}
		}

		listingEntryElement = listingEntryElement->NextSiblingElement();
	}

	std::string finalDownloadFileName;

	if(operatingSystemArchitectureType == DeviceInformationBridge::OperatingSystemArchitectureType::x64) {
		if(!windowsX64DownloadFileName.empty()) {
			finalDownloadFileName = windowsX64DownloadFileName;
		}
		else {
			finalDownloadFileName = windowsX86DownloadFileName;
		}
	}
	else if(operatingSystemArchitectureType == DeviceInformationBridge::OperatingSystemArchitectureType::x86) {
		finalDownloadFileName = windowsX86DownloadFileName;
	}

	if(finalDownloadFileName.empty()) {
		spdlog::error("Failed to determine eDuke32 download URL from download page XHTML.");

		return {};
	}

	return Utilities::joinPaths(EDUKE32_DOWNLOAD_PAGE_URL, finalDownloadFileName);
}

std::string GameManager::getRazeDownloadURL(DeviceInformationBridge::OperatingSystemType operatingSystemType, DeviceInformationBridge::OperatingSystemArchitectureType operatingSystemArchitectureType) const {
	static const std::string ARM64_ARCHITECTURE_IDENTIFIER("arm64");
	static const std::string LINUX_IDENTIFIER("linux");
	static const std::string MACOS_IDENTIFIER("mac");
	static const std::string PDB_IDENTIFIER("pdb");

	if(!m_initialized) {
		spdlog::error("Game manager not initialized!");
		return {};
	}

	std::shared_ptr<GameVersion> razeGameVersion(m_gameVersions->getGameVersionWithID(GameVersion::RAZE.getID()));
	const GameVersion * razeGameVersionRaw = razeGameVersion != nullptr ? razeGameVersion.get() : &GameVersion::RAZE;

	if(!razeGameVersionRaw->hasSupportedOperatingSystemType(operatingSystemType)) {
		return {};
	}

	GitHubService * gitHubService = GitHubService::getInstance();

	std::unique_ptr<GitHubRelease> latestRelease(gitHubService->getLatestRelease(GameVersion::RAZE.getSourceCodeURL()));

	if(latestRelease == nullptr) {
		return {};
	}

	std::shared_ptr<GitHubReleaseAsset> currentReleaseAsset;
	std::shared_ptr<GitHubReleaseAsset> latestReleaseAsset;
	std::string lowerCaseAssetFileName;

	for(size_t i = 0; i < latestRelease->numberOfAssets(); i++) {
		currentReleaseAsset = latestRelease->getAsset(i);
		lowerCaseAssetFileName = Utilities::toLowerCase(currentReleaseAsset->getFileName());

		if(lowerCaseAssetFileName.find(PDB_IDENTIFIER) != std::string::npos ||
		   lowerCaseAssetFileName.find(ARM64_ARCHITECTURE_IDENTIFIER) != std::string::npos) {
			continue;
		}

		if(latestReleaseAsset != nullptr) {
			spdlog::warn("Found multiple '{}' asset downloads, GitHub release may be misconfigured.", GameVersion::RAZE.getLongName());
			continue;
		}

		if(lowerCaseAssetFileName.find(LINUX_IDENTIFIER) != std::string::npos) {
			if(operatingSystemType != DeviceInformationBridge::OperatingSystemType::Linux) {
				continue;
			}

			latestReleaseAsset = currentReleaseAsset;
		}
		else if(lowerCaseAssetFileName.find(MACOS_IDENTIFIER) != std::string::npos) {
			if(operatingSystemType != DeviceInformationBridge::OperatingSystemType::MacOS) {
				continue;
			}

			latestReleaseAsset = currentReleaseAsset;
		}

		if(operatingSystemType == DeviceInformationBridge::OperatingSystemType::Windows) {
			latestReleaseAsset = currentReleaseAsset;
		}
	}

	if(latestReleaseAsset == nullptr) {
		spdlog::error("Could not find '{}' GitHub release asset with matching download file name for '{}'.", GameVersion::RAZE.getLongName(), magic_enum::enum_name(operatingSystemType));
		return {};
	}

	return latestReleaseAsset->getDownloadURL();
}

std::string GameManager::getBelgianChocolateDuke3DDownloadURL(DeviceInformationBridge::OperatingSystemType operatingSystemType, DeviceInformationBridge::OperatingSystemArchitectureType operatingSystemArchitectureType) const {
	static const std::string WINDOWS_X86_ARCHITECTURE_IDENTIFIER("win32");
	static const std::string WINDOWS_X64_ARCHITECTURE_IDENTIFIER("win64");

	if(!m_initialized) {
		spdlog::error("Game manager not initialized!");
		return {};
	}

	// Note: While Belgian Chocolate Duke Nukem 3D does compile on Linux and MacOS, no binary files are currently available on GitHub
	if(operatingSystemType != DeviceInformationBridge::OperatingSystemType::Windows) {
		return {};
	}

	GitHubService * gitHubService = GitHubService::getInstance();

	std::unique_ptr<GitHubRelease> latestRelease(gitHubService->getLatestRelease(GameVersion::BELGIAN_CHOCOLATE_DUKE3D.getSourceCodeURL()));

	if(latestRelease == nullptr) {
		return {};
	}

	std::string_view architectureIdentifier;

	switch(operatingSystemArchitectureType) {
		case DeviceInformationBridge::OperatingSystemArchitectureType::x86: {
			architectureIdentifier = WINDOWS_X86_ARCHITECTURE_IDENTIFIER;
			break;
		}
		case DeviceInformationBridge::OperatingSystemArchitectureType::x64: {
			architectureIdentifier = WINDOWS_X64_ARCHITECTURE_IDENTIFIER;
			break;
		}
	}

	std::shared_ptr<GitHubReleaseAsset> currentReleaseAsset;
	std::shared_ptr<GitHubReleaseAsset> latestReleaseAsset;

	for(size_t i = 0; i < latestRelease->numberOfAssets(); i++) {
		currentReleaseAsset = latestRelease->getAsset(i);

		if(Utilities::toLowerCase(currentReleaseAsset->getFileName()).find(architectureIdentifier) != std::string::npos) {
			latestReleaseAsset = currentReleaseAsset;
			break;
		}
	}

	if(latestReleaseAsset == nullptr) {
		switch(operatingSystemArchitectureType) {
			case DeviceInformationBridge::OperatingSystemArchitectureType::x86: {
				spdlog::error("Could not find '{}' GitHub release asset with matching download file name for '{}' ({}).",  GameVersion::BELGIAN_CHOCOLATE_DUKE3D.getLongName(), magic_enum::enum_name(operatingSystemType), magic_enum::enum_name(operatingSystemArchitectureType));
				return {};
			}
			case DeviceInformationBridge::OperatingSystemArchitectureType::x64: {
				for(size_t i = 0; i < latestRelease->numberOfAssets(); i++) {
					currentReleaseAsset = latestRelease->getAsset(i);

					if(Utilities::toLowerCase(currentReleaseAsset->getFileName()).find(WINDOWS_X86_ARCHITECTURE_IDENTIFIER) != std::string::npos) {
						latestReleaseAsset = currentReleaseAsset;
						break;
					}
				}

				if(latestReleaseAsset == nullptr) {
					spdlog::error("Could not find '{}' GitHub release asset with matching download file name for '{}' ({} or {}).", GameVersion::BELGIAN_CHOCOLATE_DUKE3D.getLongName(), magic_enum::enum_name(operatingSystemType), magic_enum::enum_name(DeviceInformationBridge::OperatingSystemArchitectureType::x64), magic_enum::enum_name(DeviceInformationBridge::OperatingSystemArchitectureType::x86));
					return {};
				}

				break;
			}
		}
	}

	return latestReleaseAsset->getDownloadURL();
}

std::string GameManager::getRedNukemDownloadURL(DeviceInformationBridge::OperatingSystemType operatingSystemType, DeviceInformationBridge::OperatingSystemArchitectureType operatingSystemArchitectureType) const {
	const std::string WINDOWS_X86_ARCHITECTURE_IDENTIFIER("win32");
	const std::string WINDOWS_X64_ARCHITECTURE_IDENTIFIER("win64");

	if(!m_initialized) {
		spdlog::error("Game manager not initialized!");
		return {};
	}

	HTTPService * httpService = HTTPService::getInstance();

	if(!httpService->isInitialized()) {
		return {};
	}

	std::shared_ptr<GameVersion> redNukemGameVersion(m_gameVersions->getGameVersionWithID(GameVersion::RED_NUKEM.getID()));
	const GameVersion * redNukemGameVersionRaw = redNukemGameVersion != nullptr ? redNukemGameVersion.get() : &GameVersion::RED_NUKEM;

	if(!redNukemGameVersionRaw->hasSupportedOperatingSystemType(operatingSystemType)) {
		return {};
	}

	std::shared_ptr<HTTPRequest> downloadPageRequest(httpService->createRequest(HTTPRequest::Method::Get, RED_NUKEM_DOWNLOAD_PAGE_URL));
	downloadPageRequest->setConnectionTimeout(5s);
	downloadPageRequest->setNetworkTimeout(10s);

	std::future<std::shared_ptr<HTTPResponse>> futureResponse(httpService->sendRequest(downloadPageRequest));

	if(!futureResponse.valid()) {
		spdlog::error("Failed to create RedNukem download page HTTP request!");
		return {};
	}

	futureResponse.wait();

	std::shared_ptr<HTTPResponse> response(futureResponse.get());

	if(response->isFailure()) {
		spdlog::error("Failed to retrieve RedNukem download page with error: {}", response->getErrorMessage());
		return {};
	}

	if(response->isFailureStatusCode()) {
		std::string statusCodeName(HTTPUtilities::getStatusCodeName(response->getStatusCode()));
		spdlog::error("Failed to get RedNukem download page ({}{})!", response->getStatusCode(), statusCodeName.empty() ? "" : " " + statusCodeName);
		return {};
	}

	std::string pageHTML(Utilities::tidyHTML(response->getBodyAsString()));

	if(pageHTML.empty()) {
		spdlog::error("Failed to tidy RedNukem download page HTML.");
		return {};
	}

	response.reset();

	tinyxml2::XMLDocument document;

	if(document.Parse(pageHTML.c_str(), pageHTML.length()) != tinyxml2::XML_SUCCESS) {
		spdlog::error("Failed to parse RedNukem download page XHTML with error: '{}'.", document.ErrorStr());
		return {};
	}

	const tinyxml2::XMLElement * bodyElement = Utilities::findFirstXMLElementWithName(document.RootElement(), "body");

	if(bodyElement == nullptr) {
		spdlog::error("RedNukem download page XHTML is missing 'body' element.");
		return {};
	}

	std::string version;
	size_t separatorIndex = std::string::npos;
	const tinyxml2::XMLElement * downloadContainerElement = nullptr;
	std::vector<const tinyxml2::XMLElement *> downloadLinks;
	const char * downloadLinkRaw = nullptr;
	std::string_view currentDownloadLink;
	const tinyxml2::XMLElement * currentBodyChildElement = bodyElement->FirstChildElement();
	std::string windowsX86DownloadLink;
	std::string windowsX64DownloadLink;

	while(true) {
		if(currentBodyChildElement == nullptr) {
			break;
		}

		if(Utilities::areStringsEqual(currentBodyChildElement->Name(), "button")) {
			std::string_view buttonText(currentBodyChildElement->GetText());

			separatorIndex = buttonText.find_first_of("-");

			if(separatorIndex != std::string::npos) {
				version = Utilities::trimString(std::string_view(buttonText.data(), separatorIndex));
			}

			if(!version.empty()) {
				downloadContainerElement = currentBodyChildElement->NextSiblingElement();

				if(downloadContainerElement != nullptr) {
					downloadLinks = Utilities::findXMLElementsWithName(downloadContainerElement, "a");

					if(!downloadLinks.empty()) {
						for(std::vector<const tinyxml2::XMLElement *>::const_iterator i = downloadLinks.cbegin(); i != downloadLinks.cend(); ++i) {
							downloadLinkRaw = (*i)->Attribute("href");

							if(downloadLinkRaw != nullptr) {
								currentDownloadLink = downloadLinkRaw;

								if(currentDownloadLink.find("rednukem") == std::string::npos ||
								   currentDownloadLink.find("debug") != std::string::npos) {
									continue;
								}

								if(currentDownloadLink.find(WINDOWS_X86_ARCHITECTURE_IDENTIFIER) != std::string::npos) {
									windowsX86DownloadLink = currentDownloadLink;
								}
								else if(currentDownloadLink.find(WINDOWS_X64_ARCHITECTURE_IDENTIFIER) != std::string::npos) {
									windowsX64DownloadLink = currentDownloadLink;
								}
							}
							else {
								spdlog::warn("RedNukem download link is missing 'href' attribute.");
							}
						}
					}
					else {
						spdlog::warn("RedNukem download container element has no download links.");
					}
				}
				else {
					spdlog::warn("RedNukem download button has no corresponding download container element.");
				}
			}
			else {
				spdlog::warn("Failed to parse version from RedNukem download button.");
			}
		}

		currentBodyChildElement = currentBodyChildElement->NextSiblingElement();
	}

	std::string finalDownloadLink;

	if(operatingSystemArchitectureType == DeviceInformationBridge::OperatingSystemArchitectureType::x64) {
		if(!windowsX64DownloadLink.empty()) {
			finalDownloadLink = windowsX64DownloadLink;
		}
		else {
			finalDownloadLink = windowsX86DownloadLink;
		}
	}
	else if(operatingSystemArchitectureType == DeviceInformationBridge::OperatingSystemArchitectureType::x86) {
		finalDownloadLink = windowsX86DownloadLink;
	}

	if(finalDownloadLink.empty()) {
		spdlog::error("Failed to determine RedNukem download URL from download page XHTML.");

		return {};
	}

	return finalDownloadLink;
}

struct Duke3dwInfo {
	std::string version;
	std::string downloadURL;
};

static Duke3dwInfo getLatestDuke3dwInfo() {
	static const std::string DUKE3DW_DOWNLOAD_PAGE_URL("http://www.proasm.com/duke/Duke3dw.html");
	static const std::string VERSION_IDENTIFIER("Version");
	static const std::string DOWNLOAD_IDENTIFIER("Download");
	static const std::string DUKE3DW_IDENTIFIER("Duke3dw");

	HTTPService * httpService = HTTPService::getInstance();

	if(!httpService->isInitialized()) {
		return {};
	}

	std::shared_ptr<HTTPRequest> downloadPageRequest(httpService->createRequest(HTTPRequest::Method::Get, DUKE3DW_DOWNLOAD_PAGE_URL));
	downloadPageRequest->setConnectionTimeout(5s);
	downloadPageRequest->setNetworkTimeout(10s);

	std::shared_ptr<HTTPResponse> response(httpService->sendRequestAndWait(downloadPageRequest));

	if(response->isFailure()) {
		spdlog::error("Failed to retrieve Duke3dw download page with error: {}", response->getErrorMessage());
		return {};
	}
	else if(response->isFailureStatusCode()) {
		std::string statusCodeName(HTTPUtilities::getStatusCodeName(response->getStatusCode()));
		spdlog::error("Failed to get Duke3dw download page ({}{})!", response->getStatusCode(), statusCodeName.empty() ? "" : " " + statusCodeName);
		return {};
	}

	std::string pageHTML(Utilities::tidyHTML(response->getBodyAsString()));

	response.reset();

	if(pageHTML.empty()) {
		spdlog::error("Failed to tidy Duke3dw download page HTML.");
		return {};
	}

	Duke3dwInfo info;
	tinyxml2::XMLDocument document;

	if(document.Parse(pageHTML.c_str(), pageHTML.length()) != tinyxml2::XML_SUCCESS) {
		spdlog::error("Failed to parse Duke3dw download page XHTML with error: '{}'.", document.ErrorStr());
		return {};
	}

	std::vector<const tinyxml2::XMLElement *> linkElements(Utilities::findXMLElementsWithName(document.RootElement(), "a"));

	if(linkElements.empty()) {
		spdlog::error("No download link elements found on Duke3dw download page.", document.ErrorStr());
		return {};
	}

	for(const tinyxml2::XMLElement * linkElement : linkElements) {
		const tinyxml2::XMLElement * downloadTextElement = Utilities::findFirstXMLElementContainingText(linkElement, DOWNLOAD_IDENTIFIER, false);

		if(downloadTextElement == nullptr ||
		   !Utilities::doesXMLElementContainText(downloadTextElement, DUKE3DW_IDENTIFIER, false)) {
			continue;
		}

		const char * downloadLinkHrefRaw = linkElement->Attribute("href");

		if(Utilities::stringLength(downloadLinkHrefRaw) == 0) {
			continue;
		}

		std::string_view downloadLinkHref(downloadLinkHrefRaw);
		size_t downloadDirectorySeparatorIndex = DUKE3DW_DOWNLOAD_PAGE_URL.find_last_of("/");
		size_t downloadLinkHrefStartIndex = 0;

		if(downloadLinkHref.find("../") == 0) {
			downloadDirectorySeparatorIndex = DUKE3DW_DOWNLOAD_PAGE_URL.find_last_of("/", downloadDirectorySeparatorIndex - 1);
			downloadLinkHrefStartIndex += 3;
		}

		info.downloadURL = Utilities::joinPaths(std::string_view(DUKE3DW_DOWNLOAD_PAGE_URL.c_str(), downloadDirectorySeparatorIndex), std::string_view(downloadLinkHref.data() + downloadLinkHrefStartIndex, downloadLinkHref.length() - downloadLinkHrefStartIndex));

		break;
	}

	if(info.downloadURL.empty()) {
		spdlog::error("Failed to locate Duke3dw download URL in download page XHTML.");
		return {};
	}

	const tinyxml2::XMLElement * versionElement = Utilities::findFirstXMLElementContainingText(document.RootElement(), VERSION_IDENTIFIER);

	if(versionElement != nullptr) {
		const char * versionRaw = versionElement->GetText();

		if(Utilities::stringLength(versionRaw) != 0) {
			std::string_view version(versionRaw);
			size_t versionSeparatorIndex = version.find(VERSION_IDENTIFIER);

			if(versionSeparatorIndex != std::string::npos) {
				versionSeparatorIndex = version.find_first_of(" ", versionSeparatorIndex + VERSION_IDENTIFIER.length());

				if(versionSeparatorIndex != std::string::npos) {
					info.version = version.substr(versionSeparatorIndex + 1);
				}
			}
		}
	}

	return info;
}

std::string GameManager::getDuke3dwDownloadURL(DeviceInformationBridge::OperatingSystemType operatingSystemType, DeviceInformationBridge::OperatingSystemArchitectureType operatingSystemArchitectureType) const {
	if(!m_initialized) {
		spdlog::error("Game manager not initialized!");
		return {};
	}

	std::shared_ptr<GameVersion> duke3dwGameVersion(m_gameVersions->getGameVersionWithID(GameVersion::DUKE3DW.getID()));
	const GameVersion * duke3dwGameVersionRaw = duke3dwGameVersion != nullptr ? duke3dwGameVersion.get() : &GameVersion::DUKE3DW;

	if(!duke3dwGameVersionRaw->hasSupportedOperatingSystemType(operatingSystemType)) {
		return {};
	}

	return getLatestDuke3dwInfo().downloadURL;
}

std::string GameManager::getPKDuke3DDownloadURL(DeviceInformationBridge::OperatingSystemType operatingSystemType, DeviceInformationBridge::OperatingSystemArchitectureType operatingSystemArchitectureType) const {
	if(!m_initialized) {
		spdlog::error("Game manager not initialized!");
		return {};
	}

	std::shared_ptr<GameVersion> duke3dwGameVersion(m_gameVersions->getGameVersionWithID(GameVersion::DUKE3DW.getID()));
	const GameVersion * duke3dwGameVersionRaw = duke3dwGameVersion != nullptr ? duke3dwGameVersion.get() : &GameVersion::DUKE3DW;

	if(!duke3dwGameVersionRaw->hasSupportedOperatingSystemType(operatingSystemType)) {
		return {};
	}

	BitbucketService * bitbucketService = BitbucketService::getInstance();

	std::shared_ptr<BitbucketDownload> latestDownload(bitbucketService->getLatestDownload(GameVersion::PKDUKE3D.getSourceCodeURL()));

	if(!BitbucketDownload::isValid(latestDownload.get())) {
		return {};
	}

	return latestDownload->getDownloadURL();
}

struct XDukeDownload {
	std::string version;
	std::string downloadURL;
};

static std::vector<XDukeDownload> getXDukeDownloads() {
	static const std::string XDUKE_DOWNLOAD_PAGE_URL("http://vision.gel.ulaval.ca/~klein/duke3d");
	static const std::string XDUKE_DOWNLOAD_IDENTIFIER("duke3d_");

	HTTPService * httpService = HTTPService::getInstance();

	if(!httpService->isInitialized()) {
		return {};
	}

	std::shared_ptr<HTTPRequest> downloadPageRequest(httpService->createRequest(HTTPRequest::Method::Get, XDUKE_DOWNLOAD_PAGE_URL));
	downloadPageRequest->setConnectionTimeout(5s);
	downloadPageRequest->setNetworkTimeout(10s);

	std::shared_ptr<HTTPResponse> response(httpService->sendRequestAndWait(downloadPageRequest));

	if(response->isFailure()) {
		spdlog::error("Failed to retrieve xDuke download page with error: {}", response->getErrorMessage());
		return {};
	}
	else if(response->isFailureStatusCode()) {
		std::string statusCodeName(HTTPUtilities::getStatusCodeName(response->getStatusCode()));
		spdlog::error("Failed to get xDuke download page ({}{})!", response->getStatusCode(), statusCodeName.empty() ? "" : " " + statusCodeName);
		return {};
	}

	std::string pageHTML(Utilities::tidyHTML(response->getBodyAsString()));

	response.reset();

	if(pageHTML.empty()) {
		spdlog::error("Failed to tidy xDuke download page HTML.");
		return {};
	}

	Duke3dwInfo info;
	tinyxml2::XMLDocument document;

	if(document.Parse(pageHTML.c_str(), pageHTML.length()) != tinyxml2::XML_SUCCESS) {
		spdlog::error("Failed to parse xDuke download page XHTML with error: '{}'.", document.ErrorStr());
		return {};
	}

	std::vector<const tinyxml2::XMLElement *> linkElements(Utilities::findXMLElementsWithName(document.RootElement(), "a"));

	if(linkElements.empty()) {
		spdlog::error("No download link elements found on xDuke download page.", document.ErrorStr());
		return {};
	}

	std::vector<XDukeDownload> downloads;

	for(const tinyxml2::XMLElement * linkElement : linkElements) {
		const char * downloadLinkHrefRaw = linkElement->Attribute("href");

		if(Utilities::stringLength(downloadLinkHrefRaw) == 0) {
			continue;
		}

		std::string_view downloadLinkHref(downloadLinkHrefRaw);

		if(!Utilities::hasFileExtension(downloadLinkHref, "zip")) {
			continue;
		}

		std::string lowerCaseDownloadLinkHref(Utilities::toLowerCase(downloadLinkHref));

		if(lowerCaseDownloadLinkHref.find(XDUKE_DOWNLOAD_IDENTIFIER) != 0) {
			continue;
		}

		size_t versionStartIndex = downloadLinkHref.find_first_of("0123456789", XDUKE_DOWNLOAD_IDENTIFIER.length());
		size_t versionEndIndex = downloadLinkHref.find_last_of("0123456789");

		downloads.push_back(XDukeDownload{
			std::string(downloadLinkHref.data() + versionStartIndex, versionEndIndex - versionStartIndex + 1),
			Utilities::joinPaths(XDUKE_DOWNLOAD_PAGE_URL, downloadLinkHref)
		});
	}

	std::stable_sort(downloads.begin(), downloads.end(), [](const XDukeDownload & downloadA, const XDukeDownload & downloadB) {
		return Utilities::compareVersions(downloadA.version, downloadB.version).value_or(0) > 0;
	});

	return downloads;
}

std::string GameManager::getXDukeDownloadURL(DeviceInformationBridge::OperatingSystemType operatingSystemType, DeviceInformationBridge::OperatingSystemArchitectureType operatingSystemArchitectureType) const {
	if(!m_initialized) {
		spdlog::error("Game manager not initialized!");
		return {};
	}

	std::shared_ptr<GameVersion> xDukeGameVersion(m_gameVersions->getGameVersionWithID(GameVersion::XDUKE.getID()));
	const GameVersion * xDukeGameVersionRaw = xDukeGameVersion != nullptr ? xDukeGameVersion.get() : &GameVersion::XDUKE;

	if(!xDukeGameVersionRaw->hasSupportedOperatingSystemType(operatingSystemType)) {
		return {};
	}

	std::vector<XDukeDownload> xDukeDownloads(getXDukeDownloads());

	if(xDukeDownloads.empty()) {
		return {};
	}

	return xDukeDownloads.front().downloadURL;
}

std::string GameManager::getRDukeDownloadURL(DeviceInformationBridge::OperatingSystemType operatingSystemType, DeviceInformationBridge::OperatingSystemArchitectureType operatingSystemArchitectureType) const {
	if(!m_initialized) {
		spdlog::error("Game manager not initialized!");
		return {};
	}

	std::shared_ptr<GameVersion> rDukeGameVersion(m_gameVersions->getGameVersionWithID(GameVersion::RDUKE.getID()));
	const GameVersion * rDukeGameVersionRaw = rDukeGameVersion != nullptr ? rDukeGameVersion.get() : &GameVersion::RDUKE;

	if(!rDukeGameVersionRaw->hasSupportedOperatingSystemType(operatingSystemType)) {
		return {};
	}

	GitHubService * gitHubService = GitHubService::getInstance();

	std::unique_ptr<GitHubRelease> latestRelease(gitHubService->getLatestRelease(GameVersion::RDUKE.getSourceCodeURL()));

	if(latestRelease == nullptr || latestRelease->numberOfAssets() == 0) {
		return {};
	}

	return latestRelease->getAsset(0)->getDownloadURL();
}

struct Duke3d_w32Download {
	std::string version;
	std::string downloadURL;
	bool isPatch = false;
};

static std::vector<Duke3d_w32Download> getDuke3d_w32Downloads() {
	static const std::string DUKE3D_W32_DOWNLOAD_PAGE_URL("http://www.rancidmeat.com/project.php3?id=1");
	static const std::string DUKE3D_W32_DOWNLOAD_IDENTIFIER("duke3d_w32_");
	static const std::string DUKE3D_W32_DOWNLOAD_FILE_EXTENSION("zip");
	static const std::string DUKE3D_W32_DOWNLOAD_DEVELOPER_BUILD_IDENTIFIER("dev");
	static const std::string DUKE3D_W32_DOWNLOAD_SOURCE_CODE_IDENTIFIER("src");
	static const std::string DUKE3D_W32_DOWNLOAD_PATCH_IDENTIFIER("patch");

	HTTPService * httpService = HTTPService::getInstance();

	if(!httpService->isInitialized()) {
		return {};
	}

	std::shared_ptr<HTTPRequest> downloadPageRequest(httpService->createRequest(HTTPRequest::Method::Get, DUKE3D_W32_DOWNLOAD_PAGE_URL));
	downloadPageRequest->setConnectionTimeout(5s);
	downloadPageRequest->setNetworkTimeout(10s);

	std::shared_ptr<HTTPResponse> response(httpService->sendRequestAndWait(downloadPageRequest));

	if(response->isFailure()) {
		spdlog::error("Failed to retrieve Duke3d_w32 download page with error: {}", response->getErrorMessage());
		return {};
	}
	else if(response->isFailureStatusCode()) {
		std::string statusCodeName(HTTPUtilities::getStatusCodeName(response->getStatusCode()));
		spdlog::error("Failed to get Duke3d_w32 download page ({}{})!", response->getStatusCode(), statusCodeName.empty() ? "" : " " + statusCodeName);
		return {};
	}

	std::string pageHTML(Utilities::tidyHTML(response->getBodyAsString()));

	response.reset();

	if(pageHTML.empty()) {
		spdlog::error("Failed to tidy Duke3d_w32 download page HTML.");
		return {};
	}

	Duke3dwInfo info;
	tinyxml2::XMLDocument document;

	if(document.Parse(pageHTML.c_str(), pageHTML.length()) != tinyxml2::XML_SUCCESS) {
		spdlog::error("Failed to parse Duke3d_w32 download page XHTML with error: '{}'.", document.ErrorStr());
		return {};
	}

	std::vector<const tinyxml2::XMLElement *> linkElements(Utilities::findXMLElementsWithName(document.RootElement(), "a"));

	if(linkElements.empty()) {
		spdlog::error("No download link elements found on Duke3d_w32 download page.", document.ErrorStr());
		return {};
	}

	std::string_view downloadURLBasePath(DUKE3D_W32_DOWNLOAD_PAGE_URL.c_str(), DUKE3D_W32_DOWNLOAD_PAGE_URL.find_last_of("/"));
	std::vector<Duke3d_w32Download> downloads;

	for(const tinyxml2::XMLElement * linkElement : linkElements) {
		const char * downloadLinkHrefRaw = linkElement->Attribute("href");

		if(Utilities::stringLength(downloadLinkHrefRaw) == 0) {
			continue;
		}

		std::string_view downloadLinkHref(downloadLinkHrefRaw);

		size_t downloadIdentifierIndex = downloadLinkHref.find(DUKE3D_W32_DOWNLOAD_IDENTIFIER);

		if(downloadIdentifierIndex== std::string::npos ||
		   !Utilities::hasFileExtension(downloadLinkHref, "zip") ||
		   downloadLinkHref.find(DUKE3D_W32_DOWNLOAD_DEVELOPER_BUILD_IDENTIFIER) != std::string::npos ||
		   downloadLinkHref.find(DUKE3D_W32_DOWNLOAD_SOURCE_CODE_IDENTIFIER) != std::string::npos) {
			continue;
		}

		Duke3d_w32Download download;

		download.downloadURL = Utilities::joinPaths(downloadURLBasePath, downloadLinkHref);
		download.isPatch = downloadLinkHref.find(DUKE3D_W32_DOWNLOAD_PATCH_IDENTIFIER) != std::string::npos;

		size_t versionStartIndex = downloadLinkHref.find_first_of("0123456789", downloadIdentifierIndex + DUKE3D_W32_DOWNLOAD_IDENTIFIER.length());
		size_t versionEndIndex = downloadLinkHref.find_last_of("0123456789");

		download.version = std::string_view(downloadLinkHref.data() + versionStartIndex, versionEndIndex - versionStartIndex + 1);

		size_t patchSeparatorIndex = download.version.find_first_of("-");

		if(patchSeparatorIndex != std::string::npos) {
			download.version[patchSeparatorIndex] = '.';
		}

		if(Utilities::areStringsEqual(download.version, "19.1") && download.downloadURL.find("bin") == std::string::npos) {
			continue;
		}

		downloads.push_back(download);
	}

	std::stable_sort(downloads.begin(), downloads.end(), [](const Duke3d_w32Download & downloadA, const Duke3d_w32Download & downloadB) {
		return Utilities::compareVersions(downloadA.version, downloadB.version).value_or(0) > 0;
	});

	size_t numberOfPatchDownloadsToRemove = 0;

	for(std::vector<Duke3d_w32Download>::const_reverse_iterator i = downloads.crbegin(); i != downloads.crend(); ++i) {
		if(i->isPatch) {
			numberOfPatchDownloadsToRemove++;
		}
		else {
			break;
		}
	}

	for(size_t i = 0; i < numberOfPatchDownloadsToRemove; i++) {
		downloads.pop_back();
	}

	return downloads;
}

static std::vector<Duke3d_w32Download> getLatestDuke3d_w32Downloads() {
	std::vector<Duke3d_w32Download> downloads(getDuke3d_w32Downloads());

	std::vector<Duke3d_w32Download>::const_iterator baseDownloadIterator(std::find_if(downloads.cbegin(), downloads.cend(), [](const Duke3d_w32Download & download) {
		return !download.isPatch;
	}));

	if(baseDownloadIterator == downloads.cend()) {
		return {};
	}

	size_t numberOfDownloadsToRemove = downloads.cend() - baseDownloadIterator - 1;

	for(size_t i = 0; i < numberOfDownloadsToRemove; i++) {
		downloads.pop_back();
	}

	return downloads;
}

std::string GameManager::getDuke3d_w32DownloadURL(DeviceInformationBridge::OperatingSystemType operatingSystemType, DeviceInformationBridge::OperatingSystemArchitectureType operatingSystemArchitectureType) const {
	if(!m_initialized) {
		spdlog::error("Game manager not initialized!");
		return {};
	}

	std::shared_ptr<GameVersion> duke3d_w32GameVersion(m_gameVersions->getGameVersionWithID(GameVersion::DUKE3D_W32.getID()));
	const GameVersion * duke3d_w32GameVersionRaw = duke3d_w32GameVersion != nullptr ? duke3d_w32GameVersion.get() : &GameVersion::DUKE3D_W32;

	if(!duke3d_w32GameVersionRaw->hasSupportedOperatingSystemType(operatingSystemType)) {
		return {};
	}

	std::vector<Duke3d_w32Download> downloads(getLatestDuke3d_w32Downloads());

	if(downloads.empty()) {
		return {};
	}

	// Note: Duke3d_win32 releases are sometimes distributed as multiple patches, and must be applied on top of a prior full release. There is not currently support for downloading and extracting
	// multiple archives one on top of another, so we will instead download the latest full release for now.
	std::vector<Duke3d_w32Download>::const_iterator latestFullDownloadIterator(std::find_if(downloads.cbegin(), downloads.cend(), [](const Duke3d_w32Download & download) {
		return !download.isPatch;
	}));

	if(latestFullDownloadIterator == downloads.cend()) {
		return {};
	}

	return latestFullDownloadIterator->downloadURL;
}

bool GameManager::installGame(const GameVersion & gameVersion, const std::string & destinationDirectoryPath, bool useFallback, bool overwrite) {
	if(!m_initialized) {
		spdlog::error("Game manager not initialized!");
		return false;
	}

	HTTPService * httpService = HTTPService::getInstance();

	if(!httpService->isInitialized()) {
		return false;
	}

	if(!std::filesystem::exists(std::filesystem::path(destinationDirectoryPath))) {
		std::error_code errorCode;
		std::filesystem::create_directories(destinationDirectoryPath, errorCode);

		if(errorCode) {
			spdlog::error("Failed to create game installation destination directory structure for path '{}': {}", destinationDirectoryPath, errorCode.message());
			return false;
		}
	}

	if(!std::filesystem::is_directory(std::filesystem::path(destinationDirectoryPath))) {
		spdlog::error("Game installation destination directory path is not a valid directory!");
		return false;
	}

	if(!isGameDownloadable(gameVersion.getID())) {
		spdlog::error("'{}' is not downloadable.", gameVersion.getLongName());
		return false;
	}

	std::string gameDownloadURL;
	std::string expectedGameDownloadSHA1;

	if(useFallback) {
		spdlog::info("Using fallback Duke Nukem 3D game files download URL.");

		gameDownloadURL = getFallbackGameDownloadURL(gameVersion.getID());
		expectedGameDownloadSHA1 = getFallbackGameDownloadSHA1(gameVersion.getID());
	}
	else {
		gameDownloadURL = getGameDownloadURL(gameVersion.getID());
		expectedGameDownloadSHA1 = getGameDownloadSHA1(gameVersion.getID());
	}

	if(gameDownloadURL.empty()) {
		spdlog::error("Failed to determine download URL for '{}'.", gameVersion.getLongName());
		return false;
	}

	spdlog::info("Downloading '{}' game files package from: '{}'...", gameVersion.getLongName(), gameDownloadURL);

	std::shared_ptr<HTTPRequest> request(httpService->createRequest(HTTPRequest::Method::Get, gameDownloadURL));

	std::shared_ptr<HTTPResponse> response(httpService->sendRequestAndWait(request));

	if(response->isFailure()) {
		spdlog::error("Failed to download '{}' game files package with error: {}", gameVersion.getLongName(), response->getErrorMessage());

		if(!useFallback) {
			return installGame(gameVersion, destinationDirectoryPath, true, overwrite);
		}

		return false;
	}
	else if(response->isFailureStatusCode()) {
		std::string statusCodeName(HTTPUtilities::getStatusCodeName(response->getStatusCode()));
		spdlog::error("Failed to download '{}' game files package ({}{})!", gameVersion.getLongName(), response->getStatusCode(), statusCodeName.empty() ? "" : " " + statusCodeName);

		if(!useFallback) {
			return installGame(gameVersion, destinationDirectoryPath, true, overwrite);
		}

		return false;
	}

	std::string expectedArchiveMD5Hash(response->getHeaderValue("Content-MD5"));

	if(!expectedArchiveMD5Hash.empty()) {
		std::string actualArchiveMD5Hash(response->getBodyMD5(ByteBuffer::HashFormat::Base64));

		if(Utilities::areStringsEqual(actualArchiveMD5Hash, expectedArchiveMD5Hash)) {
			spdlog::debug("Validated MD5 hash of '{}' game files package.", gameVersion.getLongName());
		}
		else {
			spdlog::error("Failed to validate MD5 hash of '{}' game files package! Expected base 64 MD5 hash: '{}', actual: '{}'.", gameVersion.getLongName(), expectedArchiveMD5Hash, actualArchiveMD5Hash);

			if(!useFallback) {
				return installGame(gameVersion, destinationDirectoryPath, true, overwrite);
			}

			return false;
		}
	}

	if(!expectedGameDownloadSHA1.empty()) {
		std::string actualGameDownloadSHA1(response->getBodySHA1());

		if(Utilities::areStringsEqual(expectedGameDownloadSHA1, actualGameDownloadSHA1)) {
			spdlog::debug("Validated SHA1 hash of '{}' game files package.", gameVersion.getLongName());
		}
		else {
			spdlog::error("Failed to validate SHA1 hash of '{}' game files package! Expected SHA1 hash: '{}', actual: '{}'.", gameVersion.getLongName(), expectedGameDownloadSHA1, actualGameDownloadSHA1);

			return false;
		}
	}

	spdlog::info("'{}' game files downloaded successfully after {} ms, extracting to '{}'...", gameVersion.getLongName(), response->getRequestDuration().value().count(), destinationDirectoryPath);

	std::unique_ptr<Archive> gameFilesArchive(ArchiveFactoryRegistry::getInstance()->createArchiveFrom(response->transferBody(), std::string(Utilities::getFileExtension(gameDownloadURL))));

	if(gameFilesArchive == nullptr) {
		spdlog::error("Failed to create archive handle from '{}' game files archive package!", gameVersion.getLongName());

		if(!useFallback) {
			return installGame(gameVersion, destinationDirectoryPath, true, overwrite);
		}

		return false;
	}

	std::shared_ptr<GameVersion> groupGameVersion(getGroupGameVersion(gameVersion.getID()));
	std::string groupFilePath(getGroupFilePath(gameVersion.getID()));

	bool isLameDuke = Utilities::areStringsEqualIgnoreCase(gameVersion.getID(), GameVersion::LAMEDUKE.getID());
	bool isBetaVersion = Utilities::areStringsEqualIgnoreCase(gameVersion.getID(), GameVersion::ORIGINAL_BETA_VERSION.getID());
	bool isRegularVersion = Utilities::areStringsEqualIgnoreCase(gameVersion.getID(), GameVersion::ORIGINAL_REGULAR_VERSION.getID());
	bool isPlutoniumPak = Utilities::areStringsEqualIgnoreCase(gameVersion.getID(), GameVersion::ORIGINAL_PLUTONIUM_PAK.getID());
	bool isAtomicEdition = Utilities::areStringsEqualIgnoreCase(gameVersion.getID(), GameVersion::ORIGINAL_ATOMIC_EDITION.getID());
	bool isPlutoniumPakOrAtomicEdition = isPlutoniumPak || isAtomicEdition;
	bool isOriginalGame = isLameDuke || isBetaVersion || isRegularVersion || isPlutoniumPakOrAtomicEdition;
	bool isOriginalGameFallback = useFallback && isOriginalGame;

	std::function<bool(std::shared_ptr<ArchiveEntry>, const GameFileInformation &)> extractGameFileFunction([&gameVersion, &groupGameVersion, &groupFilePath, &gameDownloadURL, &destinationDirectoryPath, overwrite](std::shared_ptr<ArchiveEntry> gameFileEntry, const GameFileInformation & gameFileInfo) {
		if(gameFileEntry == nullptr) {
			// skip missing game files that aren't required
			if(!gameFileInfo.required) {
				return true;
			}

			spdlog::error("'{}' fallback game files package file '{}' is missing required '{}' file entry!", gameVersion.getLongName(), Utilities::getFileName(gameDownloadURL), gameFileInfo.fileName);
			return false;
		}

		std::string fileName(Utilities::getFileName(gameFileEntry->getName()));
		std::unique_ptr<ByteBuffer> gameFileData(gameFileEntry->getData());

		if(gameFileData == nullptr) {
			spdlog::error("Failed to obtain '{}' fallback '{}' game file data from game files package file '{}'.", gameVersion.getLongName(), gameFileInfo.fileName, Utilities::getFileName(gameDownloadURL));
			return false;
		}

		bool gameFileSHA1Verified = false;
		std::string calculatedGameFileSHA1(gameFileData->getSHA1());

		for(const std::string & expectedGameFileSHA1 : gameFileInfo.hashes) {
			if(Utilities::areStringsEqual(calculatedGameFileSHA1, expectedGameFileSHA1)) {
				gameFileSHA1Verified = true;
				break;
			}
		}

		if(!gameFileSHA1Verified) {
			if(gameFileInfo.hashes.size() == 1) {
				spdlog::error("'{}' '{}' game file SHA1 hash verification failed. Calculated '{}', but expected: '{}'.", gameVersion.getLongName(), gameFileInfo.fileName, calculatedGameFileSHA1, gameFileInfo.hashes.front());
			}
			else {
				std::stringstream gameFileHashesStream;

				for(const std::string & gameFileSHA1 : gameFileInfo.hashes) {
					if(gameFileHashesStream.tellp() != 0) {
						gameFileHashesStream << ", ";
					}

					gameFileHashesStream << "'" << gameFileSHA1 << "'";
				}

				spdlog::error("'{}' '{}' game file SHA1 hash verification failed. Calculated '{}', but expected one of: {}.", gameVersion.getLongName(), gameFileInfo.fileName, calculatedGameFileSHA1, gameFileHashesStream.str());
			}

			return false;
		}

		if(Utilities::areStringsEqualIgnoreCase(fileName, GroupGRP::DUKE_NUKEM_3D_GROUP_FILE_NAME)) {
			if(groupFilePath.empty()) {
				spdlog::error("Failed to determine group file path. Are your settings configured correctly?");
				return false;
			}

			if(!std::filesystem::is_regular_file(std::filesystem::path(groupFilePath))) {
				std::error_code errorCode;
				std::filesystem::path groupFileBasePath(Utilities::getBasePath(groupFilePath));

				if(!std::filesystem::is_directory(groupFileBasePath)) {
					std::filesystem::create_directories(groupFileBasePath, errorCode);

					if(errorCode) {
						spdlog::error("Failed to create group file path base directory '{}': {}", groupFileBasePath.string(), errorCode.message());
						return false;
					}

					spdlog::debug("Created group file base directory path: '{}'.", groupFileBasePath.string());
				}

				if(!gameFileData->writeTo(groupFilePath, overwrite)) {
					spdlog::error("Failed to write '{}' group filefrom game files package file '{}' to '{}'.", groupGameVersion->getLongName(), Utilities::getFileName(gameDownloadURL), groupFilePath);
					return false;
				}
			}

			std::string groupFileDestinationPath(Utilities::joinPaths(destinationDirectoryPath, GroupGRP::DUKE_NUKEM_3D_GROUP_FILE_NAME));

			if(Utilities::areSymlinksSupported()) {
				std::error_code errorCode;
				std::filesystem::create_symlink(std::filesystem::path(groupFilePath), groupFileDestinationPath, errorCode);

				if(errorCode) {
					spdlog::error("Failed to create '{}' group file symbolic link target with error: {}", groupGameVersion->getLongName(), errorCode.message());
					return false;
				}
			}
			else {
				std::error_code errorCode;
				std::filesystem::copy_file(groupFilePath, groupFileDestinationPath, errorCode);

				if(errorCode) {
					spdlog::error("Failed to copy '{}' group file from '{}' to '{}' with error: '{}'.", groupGameVersion->getLongName(), groupFilePath, groupFileDestinationPath, errorCode.message());
					return false;
				}
			}

			return true;
		}

		std::string gameFileDestinationPath(Utilities::joinPaths(destinationDirectoryPath, gameFileInfo.fileName));

		if(!gameFileData->writeTo(gameFileDestinationPath, overwrite)) {
			spdlog::error("Failed to write '{}' '{}' game file data from game files package file '{}' to '{}'.", gameVersion.getLongName(), gameFileInfo.fileName, Utilities::getFileName(gameDownloadURL), gameFileDestinationPath);
			return false;
		}

		return true;
	});

	if(isOriginalGameFallback) {
		// only extract required files from fallback downloads for original game files since there are other files we don't want or need in these archives
		if(isLameDuke) {
			for(const GameFileInformation & gameFileInfo : LAMEDUKE_GAME_FILE_INFO_LIST) {
				if(!extractGameFileFunction(std::shared_ptr<ArchiveEntry>(gameFilesArchive->getFirstEntryWithName(gameFileInfo.fileName, true)), gameFileInfo)) {
					if(!useFallback) {
						return installGame(gameVersion, destinationDirectoryPath, true, overwrite);
					}

					return false;
				}
			}
		}
		else if(isBetaVersion) {
			for(const GameFileInformation & gameFileInfo : BETA_VERSION_GAME_FILE_INFO_LIST) {
				if(!extractGameFileFunction(std::shared_ptr<ArchiveEntry>(gameFilesArchive->getFirstEntryWithName(gameFileInfo.fileName, true)), gameFileInfo)) {
					if(!useFallback) {
						return installGame(gameVersion, destinationDirectoryPath, true, overwrite);
					}

					return false;
				}
			}
		}
		else if(isRegularVersion) {
			for(const GameFileInformation & gameFileInfo : REGULAR_VERSION_GAME_FILE_INFO_LIST) {
				if(!extractGameFileFunction(std::shared_ptr<ArchiveEntry>(gameFilesArchive->getFirstEntryWithName(gameFileInfo.fileName, true)), gameFileInfo)) {
					if(!useFallback) {
						return installGame(gameVersion, destinationDirectoryPath, true, overwrite);
					}

					return false;
				}
			}
		}
		else if(isPlutoniumPak) {
			for(const GameFileInformation & gameFileInfo : PLUTONIUM_PAK_GAME_FILE_INFO_LIST) {
				if(!extractGameFileFunction(std::shared_ptr<ArchiveEntry>(gameFilesArchive->getFirstEntryWithName(gameFileInfo.fileName, true)), gameFileInfo)) {
					if(!useFallback) {
						return installGame(gameVersion, destinationDirectoryPath, true, overwrite);
					}

					return false;
				}
			}
		}
		else if(isAtomicEdition) {
			for(const GameFileInformation & gameFileInfo : ATOMIC_EDITION_GAME_FILE_INFO_LIST) {
				if(!extractGameFileFunction(std::shared_ptr<ArchiveEntry>(gameFilesArchive->getFirstEntryWithName(gameFileInfo.fileName, true)), gameFileInfo)) {
					if(!useFallback) {
						return installGame(gameVersion, destinationDirectoryPath, true, overwrite);
					}

					return false;
				}
			}
		}
	}
	else {
		if(Utilities::areStringsEqualIgnoreCase(gameVersion.getID(), GameVersion::DUKE3D_W32.getID())) {
			std::shared_ptr<ArchiveEntry> gameExecutableArchiveEntry(gameFilesArchive->getFirstEntryWithName("duke3d_w32.exe", true));

			if(gameExecutableArchiveEntry == nullptr || !gameExecutableArchiveEntry->isFile()) {
				spdlog::error("Failed to locate '{}' game executable in game files archive package.", gameVersion.getLongName());

				if(!useFallback) {
					return installGame(gameVersion, destinationDirectoryPath, true, overwrite);
				}

				return false;
			}

			std::string gameFilesBasePath(Utilities::getBasePath(gameExecutableArchiveEntry->getPath()));

			if(gameFilesArchive->extractAllEntriesInSubdirectory(destinationDirectoryPath, gameFilesBasePath, true, true, overwrite) == 0) {
				spdlog::error("Failed to extract '{}' game files from subdirectory '{}' in archive package to '{}'.", gameVersion.getLongName(), gameFilesBasePath, destinationDirectoryPath);

				if(!useFallback) {
					return installGame(gameVersion, destinationDirectoryPath, true, overwrite);
				}

				return false;
			}

			static const std::array<std::string, 4> ADDITIONAL_DUKE3D_W32_FILE_NAMES = {
				"duke3d_w32.chm",
				"README.txt",
				"gnu.txt",
				"3drealms_readme.txt"
			};

			std::shared_ptr<ArchiveEntry> additionalFileArchiveEntry;

			for(const std::string & additionalFileName : ADDITIONAL_DUKE3D_W32_FILE_NAMES) {
				additionalFileArchiveEntry = gameFilesArchive->getFirstEntryWithName(additionalFileName);

				if(additionalFileArchiveEntry == nullptr || !additionalFileArchiveEntry->isFile()) {
					continue;
				}

				additionalFileArchiveEntry->writeToFile(Utilities::joinPaths(destinationDirectoryPath, additionalFileArchiveEntry->getName()));
			}
		}
		else {
			if(gameFilesArchive->extractAllEntries(destinationDirectoryPath, true) == 0) {
				spdlog::error("Failed to extract '{}' game files archive package to '{}'.", gameVersion.getLongName(), destinationDirectoryPath);

				if(!useFallback) {
					return installGame(gameVersion, destinationDirectoryPath, true, overwrite);
				}

				return false;
			}
		}
	}

	if(Utilities::areStringsEqualIgnoreCase(gameVersion.getID(), GameVersion::JFDUKE3D.getID())) {
		std::filesystem::directory_entry jfDuke3DSubdirectory;

		for(const std::filesystem::directory_entry & entry : std::filesystem::directory_iterator(std::filesystem::path(destinationDirectoryPath))) {
			if(entry.is_directory()) {
				jfDuke3DSubdirectory = entry;
				break;
			}
		}

		if(jfDuke3DSubdirectory.exists()) {
			std::string jfDuke3DSubdirectoryPath(jfDuke3DSubdirectory.path().string());
			std::error_code errorCode;

			for(const std::filesystem::directory_entry & entry : std::filesystem::directory_iterator(std::filesystem::path(jfDuke3DSubdirectory))) {
				std::string currentEntryPath(entry.path().string());
				std::string newEntryPath(Utilities::joinPaths(destinationDirectoryPath, currentEntryPath.substr(currentEntryPath.find(jfDuke3DSubdirectoryPath) + jfDuke3DSubdirectoryPath.length())));

				spdlog::debug("Moving '{}' file '{}' to parent directory...", gameVersion.getLongName(), Utilities::getFileName(currentEntryPath));

				std::filesystem::rename(std::filesystem::path(currentEntryPath), std::filesystem::path(newEntryPath), errorCode);

				if(errorCode) {
					spdlog::error("Failed to move '{}' to '{}' with error: {}", currentEntryPath, newEntryPath, errorCode.message());
				}
			}

			spdlog::debug("Removing empty '{}' subdirectory: '{}'...", gameVersion.getLongName(), Utilities::getFileName(jfDuke3DSubdirectoryPath));

			std::filesystem::remove(std::filesystem::path(jfDuke3DSubdirectoryPath), errorCode);

			if(errorCode) {
				spdlog::error("Failed to remove '{}' subdirectory '{}' with error: {}", gameVersion.getLongName(), jfDuke3DSubdirectoryPath, errorCode.message());
			}
		}
	}
	else if(isRegularVersion || isBetaVersion || isPlutoniumPakOrAtomicEdition) {
		std::function<bool(const GameFileInformation &)> gameFileSHA1VerificationFunction([&gameVersion, &destinationDirectoryPath, useFallback](const GameFileInformation & gameFileInfo) {
			bool gameFileSHA1Verified = false;
			std::string calculatedGameFileSHA1(Utilities::getFileSHA1Hash(Utilities::joinPaths(destinationDirectoryPath, gameFileInfo.fileName)));

			if(calculatedGameFileSHA1.empty()) {
				// skip missing files that aren't required
				if(!gameFileInfo.required || (!useFallback && Utilities::areStringsEqualIgnoreCase(gameFileInfo.fileName, GroupGRP::DUKE_NUKEM_3D_GROUP_FILE_NAME))) {
					return true;
				}

				spdlog::error("Failed to calculate '{}' '{}' game file SHA1.", gameVersion.getLongName(), gameFileInfo.fileName);

				return false;
			}

			for(const std::string & expectedGameFileSHA1 : gameFileInfo.hashes) {
				if(Utilities::areStringsEqual(calculatedGameFileSHA1, expectedGameFileSHA1)) {
					gameFileSHA1Verified = true;
					break;
				}
			}

			if(!gameFileSHA1Verified) {
				if(gameFileInfo.hashes.size() == 1) {
					spdlog::error("'{}' '{}' game file SHA1 hash verification failed. Calculated '{}', but expected: '{}'.", gameVersion.getLongName(), gameFileInfo.fileName, calculatedGameFileSHA1, gameFileInfo.hashes.front());
				}
				else {
					std::stringstream gameFileHashesStream;

					for(const std::string & gameFileSHA1 : gameFileInfo.hashes) {
						if(gameFileHashesStream.tellp() != 0) {
							gameFileHashesStream << ", ";
						}

						gameFileHashesStream << "'" << gameFileSHA1 << "'";
					}

					spdlog::error("'{}' '{}' game file SHA1 hash verification failed. Calculated '{}', but expected one of: {}.", gameVersion.getLongName(), gameFileInfo.fileName, calculatedGameFileSHA1, gameFileHashesStream.str());
				}

				return false;
			}

			return true;
		});

		if(!useFallback) {
			if(isBetaVersion) {
				for(const GameFileInformation & gameFileInfo : BETA_VERSION_GAME_FILE_INFO_LIST) {
					if(!gameFileSHA1VerificationFunction(gameFileInfo)) {
						if(!useFallback) {
							return installGame(gameVersion, destinationDirectoryPath, true, overwrite);
						}

						return false;
					}
				}
			}
			else if(isRegularVersion) {
				for(const GameFileInformation & gameFileInfo : REGULAR_VERSION_GAME_FILE_INFO_LIST) {
					if(!gameFileSHA1VerificationFunction(gameFileInfo)) {
						if(!useFallback) {
							return installGame(gameVersion, destinationDirectoryPath, true, overwrite);
						}

						return false;
					}
				}
			}
			else if(isPlutoniumPak) {
				for(const GameFileInformation & gameFileInfo : PLUTONIUM_PAK_GAME_FILE_INFO_LIST) {
					if(!gameFileSHA1VerificationFunction(gameFileInfo)) {
						if(!useFallback) {
							return installGame(gameVersion, destinationDirectoryPath, true, overwrite);
						}

						return false;
					}
				}
			}
			else if(isAtomicEdition) {
				for(const GameFileInformation & gameFileInfo : ATOMIC_EDITION_GAME_FILE_INFO_LIST) {
					if(!gameFileSHA1VerificationFunction(gameFileInfo)) {
						if(!useFallback) {
							return installGame(gameVersion, destinationDirectoryPath, true, overwrite);
						}

						return false;
					}
				}
			}
		}

		if(isPlutoniumPakOrAtomicEdition) {
			std::string gameExecutablePath(Utilities::joinPaths(destinationDirectoryPath, gameVersion.getGameExecutableName()));

			spdlog::info("Checking '{}' game executable status...", gameVersion.getLongName(), gameExecutablePath);

			NoCDCracker::GameExecutableStatus gameExecutableStatus = NoCDCracker::getGameExecutableStatus(gameExecutablePath);

			if(None(gameExecutableStatus & NoCDCracker::GameExecutableStatus::Exists)) {
				spdlog::error("'{}' game executable does not exist at path: '{}'!", gameVersion.getLongName(), gameExecutablePath);
			}
			else if(Any(gameExecutableStatus & NoCDCracker::GameExecutableStatus::Invalid)) {
				spdlog::error("Invalid '{}' game executable at path: '{}'!", gameVersion.getLongName(), gameExecutablePath);
			}
			else if(Any(gameExecutableStatus & NoCDCracker::GameExecutableStatus::RegularVersion)) {
				spdlog::error("Found '{}' game executable instead of '{}' at path: '{}'!", GameVersion::ORIGINAL_REGULAR_VERSION.getLongName(), gameVersion.getLongName(), gameExecutablePath);
			}
			else if(Any(gameExecutableStatus & (NoCDCracker::GameExecutableStatus::PlutoniumPak | NoCDCracker::GameExecutableStatus::AtomicEdition))) {
				if(Any(gameExecutableStatus & NoCDCracker::GameExecutableStatus::Cracked)) {
					spdlog::info("'{}' game executable already cracked!", gameVersion.getLongName());
				}
				else if(NoCDCracker::crackGameExecutable(gameExecutablePath)) {
					spdlog::info("'{}' game executable cracked successfully! CD no longer required.", gameVersion.getLongName());
				}
				else {
					spdlog::error("Failed to crack '{}' game executable at path: '{}'.", gameVersion.getLongName(), gameExecutablePath);
				}
			}
		}

		if(isBetaVersion || isRegularVersion || isPlutoniumPakOrAtomicEdition) {
			std::unique_ptr<GameConfiguration> gameConfiguration(GameConfiguration::generateDefaultGameConfiguration(gameVersion.getID()));

			if(gameConfiguration == nullptr) {
				spdlog::warn("Failed to generate default game configuration.");
			}
			else {
				if(!gameConfiguration->updateForDOSBox()) {
					spdlog::warn("Failed to update default game configuration for use with DOSBox.");
				}

				if(!gameConfiguration->updateWithBetterControls()) {
					spdlog::warn("Failed to update default game configuration with better controls.");
				}

				std::string gameConfigurationFilePath(Utilities::joinPaths(destinationDirectoryPath, GameConfiguration::DEFAULT_GAME_CONFIGURATION_FILE_NAME));

				if(gameConfiguration->saveTo(gameConfigurationFilePath)) {
					spdlog::info("Successfully generated and saved default game configuration to file: '{}'.", gameConfigurationFilePath);
				}
				else {
					spdlog::warn("Failed to save default game configuration to file: '{}'.", gameConfigurationFilePath);
				}
			}
		}
	}

	// don't need to install the group file or verify it separately when using a fallback download for either of the original game versions, since it's already installed & verified
	if (!isOriginalGameFallback && gameVersion.hasGroupFileInstallPath()) {
		if(!installGroupFile(gameVersion.getID(), Utilities::joinPaths(destinationDirectoryPath, gameVersion.getGroupFileInstallPath().value()), overwrite)) {
			return false;
		}
	}

	return true;
}

bool GameManager::isGroupFileDownloaded(const std::string & gameVersionID) const {
	std::string groupFilePath(getGroupFilePath(gameVersionID));

	if(groupFilePath.empty()) {
		return false;
	}

	if(std::filesystem::is_regular_file(std::filesystem::path(groupFilePath))) {
		return true;
	}

	std::shared_ptr<GameVersion> gameVersion(m_gameVersions->getGameVersionWithID(gameVersionID));

	if(gameVersion == nullptr || !gameVersion->isWorldTourGroupSupported()) {
		return false;
	}

	std::string worldTourGroupFilePath(getWorldTourGroupFilePath());

	if(worldTourGroupFilePath.empty()) {
		return false;
	}

	return std::filesystem::is_regular_file(std::filesystem::path(worldTourGroupFilePath));
}

std::shared_ptr<GameVersion> GameManager::getGroupGameVersion(const std::string & gameVersionID) const {
	if(Utilities::areStringsEqualIgnoreCase(gameVersionID, GameVersion::ORIGINAL_BETA_VERSION.getID()) ||
	   Utilities::areStringsEqualIgnoreCase(gameVersionID, GameVersion::ORIGINAL_REGULAR_VERSION.getID()) ||
	   Utilities::areStringsEqualIgnoreCase(gameVersionID, GameVersion::ORIGINAL_PLUTONIUM_PAK.getID())) {
		return m_gameVersions->getGameVersionWithID(gameVersionID);
	}

	return m_gameVersions->getGameVersionWithID(GameVersion::ORIGINAL_ATOMIC_EDITION.getID());
}

std::string GameManager::getGroupFilePath(const std::string & gameVersionID) const {
	std::shared_ptr<GameVersion> groupGameVersion(getGroupGameVersion(gameVersionID));

	if(groupGameVersion == nullptr) {
		return {};
	}

	return getGroupFilePathWithSubdirectory(groupGameVersion->getModDirectoryName());
}

std::string GameManager::getFallbackGroupDownloadURL(const std::string & gameVersionID) const {
	std::shared_ptr<GameVersion> groupGameVersion(getGroupGameVersion(gameVersionID));

	if(Utilities::areStringsEqualIgnoreCase(groupGameVersion->getID(), GameVersion::ORIGINAL_BETA_VERSION.getID())) {
		return BETA_VERSION_FALLBACK_DOWNLOAD_URL;
	}
	else if(Utilities::areStringsEqualIgnoreCase(groupGameVersion->getID(), GameVersion::ORIGINAL_REGULAR_VERSION.getID())) {
		return REGULAR_VERSION_FALLBACK_DOWNLOAD_URL;
	}
	else if(Utilities::areStringsEqualIgnoreCase(groupGameVersion->getID(), GameVersion::ORIGINAL_ATOMIC_EDITION.getID())) {
		spdlog::warn("No fallback download URL for Plutonium Pak.");
		return {};
	}
	else if(Utilities::areStringsEqualIgnoreCase(groupGameVersion->getID(), GameVersion::ORIGINAL_ATOMIC_EDITION.getID())) {
		return ATOMIC_EDITION_FALLBACK_DOWNLOAD_URL;
	}

	return {};
}

std::string GameManager::getFallbackGroupDownloadSHA1(const std::string & gameVersionID) const {
	std::shared_ptr<GameVersion> groupGameVersion(getGroupGameVersion(gameVersionID));

	if(Utilities::areStringsEqualIgnoreCase(groupGameVersion->getID(), GameVersion::ORIGINAL_BETA_VERSION.getID())) {
		return BETA_VERSION_FALLBACK_DOWNLOAD_SHA1;
	}
	else if(Utilities::areStringsEqualIgnoreCase(groupGameVersion->getID(), GameVersion::ORIGINAL_REGULAR_VERSION.getID())) {
		return REGULAR_VERSION_FALLBACK_DOWNLOAD_SHA1;
	}
	else if(Utilities::areStringsEqualIgnoreCase(groupGameVersion->getID(), GameVersion::ORIGINAL_ATOMIC_EDITION.getID())) {
		spdlog::warn("No fallback download for Plutonium Pak.");
		return {};
	}
	else if(Utilities::areStringsEqualIgnoreCase(groupGameVersion->getID(), GameVersion::ORIGINAL_ATOMIC_EDITION.getID())) {
		return ATOMIC_EDITION_FALLBACK_DOWNLOAD_SHA1;
	}

	return {};
}

bool GameManager::downloadGroupFile(const std::string & gameVersionID) {
	return downloadGroupFile(gameVersionID, false);
}

bool GameManager::downloadGroupFile(const std::string & gameVersionID, bool useFallback) {
	if(!m_initialized) {
		spdlog::error("Game manager not initialized!");
		return {};
	}

	std::string destinationGroupFilePath(getGroupFilePath(gameVersionID));

	if(destinationGroupFilePath.empty()) {
		spdlog::error("Failed to determine group file path. Are your settings configured correctly?");
		return false;
	}

	if(std::filesystem::is_regular_file(std::filesystem::path(destinationGroupFilePath))) {
		spdlog::info("Duke Nukem 3D '{}' group file already downloaded.", gameVersionID);
		return true;
	}

	std::shared_ptr<GameVersion> gameVersion(m_gameVersions->getGameVersionWithID(gameVersionID));

	if(gameVersion == nullptr) {
		spdlog::error("Cannot download group file for missing or invalid game version with ID: '{}'.", gameVersionID);
		return false;
	}

	std::shared_ptr<GameVersion> groupGameVersion(getGroupGameVersion(gameVersionID));
	bool isBetaVersion = Utilities::areStringsEqualIgnoreCase(groupGameVersion->getID(), GameVersion::ORIGINAL_BETA_VERSION.getID());
	bool isRegularVersion = Utilities::areStringsEqualIgnoreCase(groupGameVersion->getID(), GameVersion::ORIGINAL_REGULAR_VERSION.getID());
	bool isPlutoniumPak = Utilities::areStringsEqualIgnoreCase(groupGameVersion->getID(), GameVersion::ORIGINAL_PLUTONIUM_PAK.getID());
	bool isAtomicEdition = Utilities::areStringsEqualIgnoreCase(groupGameVersion->getID(), GameVersion::ORIGINAL_ATOMIC_EDITION.getID());
	bool isPlutoniumPakOrAtomicEdition = isPlutoniumPak || isAtomicEdition;
	bool isWorldTourGroup = false;

	if(!useFallback && isAtomicEdition) {
		GameLocator * gameLocator = GameLocator::getInstance();

		for(size_t i = 0; i < gameLocator->numberOfGamePaths(); i++) {
			std::string sourceGroupFilePath(Utilities::joinPaths(*gameLocator->getGamePath(i), GroupGRP::DUKE_NUKEM_3D_GROUP_FILE_NAME));

			if(!std::filesystem::is_regular_file(std::filesystem::path(sourceGroupFilePath))) {
				spdlog::warn("Duke Nukem 3D group file is missing at: '{}'.", sourceGroupFilePath);
				continue;
			}

			spdlog::debug("Calculating SHA1 hash of Duke Nukem 3D group file: '{}'...", sourceGroupFilePath);

			std::string groupSHA1(Utilities::getFileSHA1Hash(sourceGroupFilePath));

			if(groupSHA1.empty()) {
				spdlog::error("Failed to calculate SHA1 hash of Duke Nukem 3D group file: '{}'!", sourceGroupFilePath);
			}
			else if(Utilities::areStringsEqual(groupSHA1, GroupGRP::DUKE_NUKEM_3D_ATOMIC_EDITION_GROUP_SHA1_FILE_HASH)) {
				spdlog::debug("Verified '{}' group file SHA1 hash.", GameVersion::ORIGINAL_ATOMIC_EDITION.getLongName());
			}
			else if(Utilities::areStringsEqual(groupSHA1, GroupGRP::DUKE_NUKEM_3D_WORLD_TOUR_GROUP_SHA1_FILE_HASH)) {
				spdlog::debug("Verified '{}' group file SHA1 hash.", WORLD_TOUR_GAME_LONG_NAME);

				isWorldTourGroup = true;

				if(!gameVersion->isWorldTourGroupSupported()) {
					spdlog::debug("'{}' group is not supported by '{}', skipping.", WORLD_TOUR_GAME_LONG_NAME, gameVersion->getLongName());
					continue;
				}

				destinationGroupFilePath = getWorldTourGroupFilePath();
			}
			else if(Utilities::areStringsEqual(groupSHA1, GroupGRP::DUKE_NUKEM_3D_PLUTONIUM_PAK_GROUP_SHA1_FILE_HASH)) {
				spdlog::error("Calculated '{}' SHA1 hash for Duke Nukem 3D group file '{}', when '{}' group file was expected! This may cause unexpected gameplay issues.", GameVersion::ORIGINAL_PLUTONIUM_PAK.getLongName(), sourceGroupFilePath, GameVersion::ORIGINAL_ATOMIC_EDITION.getLongName());
			}
			else if(Utilities::areStringsEqual(groupSHA1, GroupGRP::DUKE_NUKEM_3D_REGULAR_VERSION_GROUP_SHA1_FILE_HASH)) {
				spdlog::error("Calculated '{}' SHA1 hash for Duke Nukem 3D group file '{}', when '{}' group file was expected! This may cause unexpected gameplay issues.", GameVersion::ORIGINAL_REGULAR_VERSION.getLongName(), sourceGroupFilePath, GameVersion::ORIGINAL_ATOMIC_EDITION.getLongName());
			}
			else {
				spdlog::warn("Unexpected SHA1 hash calculated for Duke Nukem 3D group file '{}'! Game data may be modified, and may cause gameplay issues.", sourceGroupFilePath);
			}

			if(std::filesystem::is_regular_file(destinationGroupFilePath)) {
				return true;
			}

			spdlog::debug("Copying Duke Nukem 3D group file from '{}' to: '{}'...", sourceGroupFilePath, destinationGroupFilePath);

			std::error_code errorCode;
			std::filesystem::path groupFileBasePath(Utilities::getBasePath(destinationGroupFilePath));

			if(!std::filesystem::is_directory(groupFileBasePath)) {
				std::filesystem::create_directories(groupFileBasePath, errorCode);

				if(errorCode) {
					spdlog::error("Failed to create group file path base directory '{}': {}", groupFileBasePath.string(), errorCode.message());
					return false;
				}

				spdlog::debug("Created group file base directory path: '{}'.", groupFileBasePath.string());
			}

			std::filesystem::copy_file(std::filesystem::path(sourceGroupFilePath), destinationGroupFilePath, errorCode);

			if(errorCode) {
				spdlog::error("Failed to copy Duke Nukem 3D group file from '{}' to '{}' with error: '{}'.", sourceGroupFilePath, destinationGroupFilePath, errorCode.message());
			}

			return true;
		}
	}

	HTTPService * httpService = HTTPService::getInstance();

	if(!httpService->isInitialized()) {
		spdlog::error("HTTP service is not initialized!");
		return false;
	}

	std::string groupDownloadURL;

	if(useFallback) {
		groupDownloadURL = getFallbackGroupDownloadURL(gameVersionID);

		spdlog::info("Using fallback {} group file download URL.", groupGameVersion->getLongName());
	}
	else {
		groupDownloadURL = getGroupDownloadURL(gameVersionID);
	}

	if(groupDownloadURL.empty()) {
		spdlog::error("Failed to determine group download URL for '{}'.", groupGameVersion->getLongName());
		return false;
	}

	spdlog::info("Downloading '{}' group file from: '{}'...", groupGameVersion->getLongName(), groupDownloadURL);

	std::shared_ptr<HTTPRequest> request(httpService->createRequest(HTTPRequest::Method::Get, groupDownloadURL));

	std::shared_ptr<HTTPResponse> response(httpService->sendRequestAndWait(request));

	if(response->isFailure()) {
		spdlog::error("Failed to download {} group file with error: {}", groupGameVersion->getLongName(), response->getErrorMessage());
		return false;
	}
	else if(response->isFailureStatusCode()) {
		std::string statusCodeName(HTTPUtilities::getStatusCodeName(response->getStatusCode()));
		spdlog::error("Failed to download {} group file ({}{})!", groupGameVersion->getLongName(), response->getStatusCode(), statusCodeName.empty() ? "" : " " + statusCodeName);

		if(!useFallback) {
			return downloadGroupFile(gameVersionID, true);
		}

		return false;
	}

	if(useFallback) {
		std::string responseSHA1(response->getBodySHA1());
		std::string fallbackGroupDownloadSHA1(getFallbackGroupDownloadSHA1(gameVersionID));

		if(!fallbackGroupDownloadSHA1.empty()) {
			if(Utilities::areStringsEqual(fallbackGroupDownloadSHA1, responseSHA1)) {
				spdlog::debug("{} fallback group file archive SHA1 hash validated.", groupGameVersion->getLongName());
			}
			else {
				spdlog::error("Duke Nukem 3D {} fallback group file archive '{}' SHA1 hash validation failed! Expected '{}', but calculated: '{}'.", groupGameVersion->getLongName(), getFallbackGroupDownloadURL(gameVersionID), getFallbackGroupDownloadSHA1(gameVersionID), responseSHA1);
				return false;
			}
		}
	}
	else {
		if(m_gameDownloads == nullptr) {
			loadOrUpdateGameDownloadList();
		}

		if(m_gameDownloads != nullptr) {
			std::shared_ptr<GameDownload> gameDownload(m_gameDownloads->getDownloadWithID(gameVersionID));

			if(gameDownload != nullptr && gameDownload->numberOfVersions() != 0) {
				std::string groupFileDownloadFileName(Utilities::getFileName(groupDownloadURL));

				std::shared_ptr<GameDownloadVersion> gameDownloadVersion(gameDownload->getVersion(0));
				std::shared_ptr<GameDownloadFile> groupFileDownload(gameDownloadVersion->getFileWithName(groupFileDownloadFileName));

				if(groupFileDownload != nullptr) {
					std::string responseSHA1(response->getBodySHA1());

					if(Utilities::areStringsEqual(responseSHA1, groupFileDownload->getSHA1())) {
						spdlog::debug("{} group file archive SHA1 hash validated.", groupGameVersion->getLongName());
					}
					else {
						spdlog::error("{} group file archive '{}' SHA1 hash validation failed! Expected '{}', but calculated: '{}'.", groupGameVersion->getLongName(), Utilities::getFileName(groupDownloadURL), groupFileDownload->getSHA1(), responseSHA1);

						return downloadGroupFile(gameVersionID, true);
					}
				}
			}
		}
	}

	spdlog::info("{} group file downloaded successfully after {} ms, extracting to '{}'...", groupGameVersion->getLongName(), response->getRequestDuration().value().count(), destinationGroupFilePath);

	std::unique_ptr<Archive> groupArchive(ArchiveFactoryRegistry::getInstance()->createArchiveFrom(response->transferBody(), std::string(Utilities::getFileExtension(groupDownloadURL))));

	if(groupArchive == nullptr) {
		spdlog::error("Failed to create archive handle from '{}' group package file!", groupGameVersion->getLongName());

		if(!useFallback) {
			return downloadGroupFile(gameVersionID, true);
		}

		return false;
	}

	std::shared_ptr<ArchiveEntry> groupFileEntry(groupArchive->getFirstEntryWithName(GroupGRP::DUKE_NUKEM_3D_GROUP_FILE_NAME, true));

	if(groupFileEntry == nullptr) {
		spdlog::error("{} group package file is missing group file entry!", groupGameVersion->getLongName());

		if(!useFallback) {
			return downloadGroupFile(gameVersionID, true);
		}

		return false;
	}

	std::unique_ptr<ByteBuffer> groupFileData(groupFileEntry->getData());

	if(groupFileData == nullptr) {
		spdlog::error("Failed to obtain '{}' group file data from package file.", groupGameVersion->getLongName());

		if(!useFallback) {
			return downloadGroupFile(gameVersionID, true);
		}

		return false;
	}

	std::string calculatedGroupSHA1(groupFileData->getSHA1());
	std::string_view expectedGroupSHA1;

	if(isBetaVersion) {
		expectedGroupSHA1 = GroupGRP::DUKE_NUKEM_3D_BETA_VERSION_GROUP_SHA1_FILE_HASH;
	}
	else if(isRegularVersion) {
		expectedGroupSHA1 = GroupGRP::DUKE_NUKEM_3D_REGULAR_VERSION_GROUP_SHA1_FILE_HASH;
	}
	else if(isPlutoniumPak) {
		expectedGroupSHA1 = GroupGRP::DUKE_NUKEM_3D_PLUTONIUM_PAK_GROUP_SHA1_FILE_HASH;
	}
	else {
		expectedGroupSHA1 = GroupGRP::DUKE_NUKEM_3D_ATOMIC_EDITION_GROUP_SHA1_FILE_HASH;
	}

	if(calculatedGroupSHA1.empty()) {
		spdlog::error("Failed to calculate SHA1 hash of downloaded {} group file!", groupGameVersion->getLongName());
	}
	else if(Utilities::areStringsEqual(calculatedGroupSHA1, expectedGroupSHA1)) {
		spdlog::debug("Verified downloaded '{}' group file SHA1 hash.", groupGameVersion->getLongName());
	}
	else {
		spdlog::error("Extracted '{}' group file SHA1 hash verification failed! Expected '{}', but calculated '{}'.", groupGameVersion->getLongName(), expectedGroupSHA1, calculatedGroupSHA1);
		return false;
	}

	if(!groupFileData->writeTo(destinationGroupFilePath, true)) {
		spdlog::error("Failed to write '{}' group file data from package file to '{}'.", groupGameVersion->getLongName(), destinationGroupFilePath);
		return false;
	}

	return true;
}

bool GameManager::installGroupFile(const std::string & gameVersionID, const std::string & directoryPath, bool overwrite) {
	if(!m_initialized) {
		spdlog::error("Game manager not initialized!");
		return {};
	}

	bool isWorldTourGroup = false;
	std::shared_ptr<GameVersion> groupGameVersion(getGroupGameVersion(gameVersionID));
	std::shared_ptr<GameVersion> gameVersion(m_gameVersions->getGameVersionWithID(gameVersionID));

	if(gameVersion == nullptr) {
		spdlog::error("Failed to install group file to missing or invalid game version with ID: '{}'.", gameVersionID);
		return false;
	}

	std::filesystem::path destinationGroupFilePath(Utilities::joinPaths(directoryPath, GroupGRP::DUKE_NUKEM_3D_GROUP_FILE_NAME));

	if(!isGroupFileDownloaded(gameVersionID) && !downloadGroupFile(gameVersionID)) {
		spdlog::error("Failed to install '{}' group file to destination: '{}'.", groupGameVersion->getLongName(), destinationGroupFilePath.string());
		return false;
	}

	std::string groupFilePath(getGroupFilePath(gameVersionID));

	if(groupFilePath.empty()) {
		spdlog::error("Failed to determine group file path. Are your settings configured correctly?");
		return false;
	}

	if(!std::filesystem::is_regular_file(std::filesystem::path(groupFilePath))) {
		if(!gameVersion->isWorldTourGroupSupported()) {
			spdlog::error("Failed to install group file to '{}', since it does not support the '{}' group file, and the '{}' group file is missing.", gameVersion->getLongName(), WORLD_TOUR_GAME_LONG_NAME, groupGameVersion->getLongName());
			return false;
		}

		groupFilePath = getWorldTourGroupFilePath();
		isWorldTourGroup = true;

		if(!std::filesystem::is_regular_file(std::filesystem::path(groupFilePath))) {
			spdlog::error("Failed to install group file to '{}', since both the '{}' and '{}' group files are missing.", gameVersion->getLongName(), WORLD_TOUR_GAME_LONG_NAME, groupGameVersion->getLongName());
			return false;
		}
	}

	if(std::filesystem::is_regular_file(destinationGroupFilePath) || std::filesystem::is_symlink(destinationGroupFilePath)) {
		if(!overwrite) {
			spdlog::error("'{}' group already exists at destination '{}', specify overwrite to replace.", groupGameVersion->getLongName(), destinationGroupFilePath.string());
			return false;
		}

		std::error_code errorCode;
		std::filesystem::remove(destinationGroupFilePath, errorCode);

		if(errorCode) {
			spdlog::error("Failed to remove existing '{}' group from destination '{}' with error: {}", groupGameVersion->getLongName(), destinationGroupFilePath.string(), errorCode.message());
			return false;
		}
	}

	if(!Utilities::areSymlinksSupported()) {
		spdlog::info("Copying '{}' group file from '{}' to '{}'.", isWorldTourGroup ? WORLD_TOUR_GAME_LONG_NAME : groupGameVersion->getLongName(), groupFilePath, destinationGroupFilePath.string());

		std::error_code errorCode;
		std::filesystem::copy_file(std::filesystem::path(groupFilePath), destinationGroupFilePath, errorCode);

		if(errorCode) {
			spdlog::error("Failed to copy '{}' group file from '{}' to '{}' with error: '{}'.", isWorldTourGroup ? WORLD_TOUR_GAME_LONG_NAME : groupGameVersion->getLongName(), groupFilePath, destinationGroupFilePath.string(), errorCode.message());
			return false;
		}

		return true;
	}

	spdlog::info("Creating '{}' group file symlink '{}' to target '{}' at destination '{}'.", isWorldTourGroup ? WORLD_TOUR_GAME_LONG_NAME : groupGameVersion->getLongName(), GroupGRP::DUKE_NUKEM_3D_GROUP_FILE_NAME, groupFilePath, destinationGroupFilePath.string());

	std::error_code errorCode;
	std::filesystem::create_symlink(std::filesystem::path(groupFilePath), destinationGroupFilePath, errorCode);

	if(errorCode) {
		spdlog::error("Failed to create '{}' group file symlink '{}' to target '{}' at destination '{}' with error: {}", isWorldTourGroup ? WORLD_TOUR_GAME_LONG_NAME : groupGameVersion->getLongName(), GroupGRP::DUKE_NUKEM_3D_GROUP_FILE_NAME, groupFilePath, destinationGroupFilePath.string(), errorCode.message());
		return false;
	}

	return true;
}

void GameManager::updateGroupFileSymlinks() {
	if(!m_initialized) {
		spdlog::error("Game manager not initialized!");
		return;
	}

	if(!Utilities::areSymlinksSupported()) {
		spdlog::debug("Cannot update game version group file symbolic link targets, symbolic links are not supported.");
		return;
	}

	spdlog::debug("Updating game installation group file symbolic link targets.");

	for(const std::shared_ptr<GameVersion> & gameVersion : m_gameVersions->getGameVersions()) {
		if(!gameVersion->hasGroupFile()) {
			continue;
		}

		if(!gameVersion->isConfigured()) {
			spdlog::trace("Skipping unconfigured '{}' game version group file symbolic link target update.", gameVersion->getLongName());
			continue;
		}

		std::string groupFilePath(getGroupFilePath(gameVersion->getID()));

		if(groupFilePath.empty()) {
			spdlog::error("Failed to determine '{}' group file path. Are your settings configured correctly?", gameVersion->getLongName());
			continue;
		}

		bool isWorldTourGroup = false;
		std::shared_ptr<GameVersion> groupGameVersion(getGroupGameVersion(gameVersion->getID()));

		if(!std::filesystem::is_regular_file(std::filesystem::path(groupFilePath))) {
			if(!gameVersion->isWorldTourGroupSupported()) {
				spdlog::error("'{}' group file does not exist at path: '{}' and '{}' group is not supported, skipping '{}' game installation symbolic link target update.", groupGameVersion->getLongName(), groupFilePath, WORLD_TOUR_GAME_LONG_NAME, gameVersion->getLongName());
				continue;
			}

			spdlog::trace("'{}' group file does not exist at path: '{}', checking for '{}' group file.", groupGameVersion->getLongName(), groupFilePath, WORLD_TOUR_GAME_LONG_NAME);

			groupFilePath = getWorldTourGroupFilePath();
			isWorldTourGroup = true;

			if(!std::filesystem::is_regular_file(std::filesystem::path(groupFilePath))) {
				spdlog::error("'{}' group file does not exist at path: '{}', skipping '{}' game installation symbolic link target update.", WORLD_TOUR_GAME_LONG_NAME, groupFilePath, gameVersion->getLongName());
				continue;
			}
		}

		spdlog::trace("Analyzing '{}' game installation group file symblic link target.", gameVersion->getLongName());

		std::filesystem::path gameGroupFilePath(Utilities::joinPaths(gameVersion->getGamePath(), GroupGRP::DUKE_NUKEM_3D_GROUP_FILE_NAME));

		if(std::filesystem::is_regular_file(gameGroupFilePath) && !std::filesystem::is_symlink(gameGroupFilePath)) {
			spdlog::trace("'{}' has local group file, no symbolic link target update required.", gameVersion->getLongName());
			continue;
		}

		if(std::filesystem::is_symlink(gameGroupFilePath)) {
			std::error_code errorCode;
			std::filesystem::path currentGroupFileSymlinkTargetFilePath(std::filesystem::read_symlink(gameGroupFilePath, errorCode));

			if(errorCode) {
				spdlog::warn("Failed to read '{}' game version group file symbolic link target with error: {}", gameVersion->getLongName(), errorCode.message());
				continue;
			}

			if(std::filesystem::exists(gameGroupFilePath)) {
				bool groupFileSymbolicLinkTargetMatches = std::filesystem::equivalent(std::filesystem::path(groupFilePath), currentGroupFileSymlinkTargetFilePath, errorCode);

				if(!errorCode) {
					if(groupFileSymbolicLinkTargetMatches) {
						spdlog::trace("'{}' game version group file symbolic link target already already set to '{}' group.", gameVersion->getLongName(), isWorldTourGroup ? WORLD_TOUR_GAME_LONG_NAME : groupGameVersion->getLongName());
						continue;
					}

					if(gameVersion->isWorldTourGroupSupported() && Utilities::areStringsEqualIgnoreCase(groupGameVersion->getID(), GameVersion::ORIGINAL_ATOMIC_EDITION.getID())) {
						bool worldTourGroupFileSymbolicLinkTargetMatches = std::filesystem::equivalent(std::filesystem::path(getWorldTourGroupFilePath()), currentGroupFileSymlinkTargetFilePath, errorCode);

						if(!errorCode) {
							if(worldTourGroupFileSymbolicLinkTargetMatches) {
								spdlog::trace("'{}' game version group file symbolic link target alternatively already already set to '{}' group.", gameVersion->getLongName(), WORLD_TOUR_GAME_LONG_NAME);
								continue;
							}
						}
						else {
							spdlog::warn("Failed to compare '{}' game version group file symbolic link target to group file path with error: {}", gameVersion->getLongName(), errorCode.message());
						}
					}
				}
				else {
					spdlog::warn("Failed to compare '{}' game version group file symbolic link target to group file path with error: {}", gameVersion->getLongName(), errorCode.message());
				}
			}
			else {
				spdlog::trace("'{}' game installation group file symbolic link target '{}' does not exist.", gameVersion->getLongName(), currentGroupFileSymlinkTargetFilePath.string());
			}

			spdlog::info("Updating '{}' game version group file symbolic link target from '{}' to '{}'.", gameVersion->getLongName(), currentGroupFileSymlinkTargetFilePath.string(), groupFilePath);

			std::filesystem::remove(gameGroupFilePath, errorCode);

			if(errorCode) {
				spdlog::warn("Failed to remove existing '{}' game version group file symbolic link with error: {}", gameVersion->getLongName(), errorCode.message());
			}
		}
		else {
			if(gameVersion->hasGroupFileInstallPath()) {
				spdlog::warn("'{}' game version is missing Duke Nukem 3D group file.", gameVersion->getLongName());
			}
			else {
				continue;
			}

			spdlog::info("Creating '{}' game version group file symbolic link with target: '{}'.", gameVersion->getLongName(), groupFilePath);
		}

		std::error_code errorCode;
		std::filesystem::create_symlink(std::filesystem::path(groupFilePath), gameGroupFilePath, errorCode);

		if(errorCode) {
			spdlog::error("Failed to create '{}' group file symbolic link target with error: {}", gameVersion->getLongName(), errorCode.message());
			continue;
		}
	}
}
