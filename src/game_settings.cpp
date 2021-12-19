#include "game_settings.h"

namespace fs = std::filesystem;
namespace loot {
	const std::set<std::string> GameSettings::oldDefaultBranches(
		{ "master", "v0.7", "v0.8", "v0.10", "v0.13", "v0.14", "v0.15" });
	const std::string GameSettings::currentDefaultBranch("v0.17");

	GameSettings::GameSettings() :
		type_(GameType::tes4), minimumHeaderVersion_(0.0f) {}

	GameSettings::GameSettings(const GameType gameCode, const std::string& folder) :
		type_(gameCode), repositoryBranch_(currentDefaultBranch) {
		if (Type() == GameType::tes3) {
			name_ = "TES III: Morrowind";
			registryKeys_ = { "Software\\Bethesda Softworks\\Morrowind\\Installed Path",
							 "Software\\Microsoft\\Windows\\CurrentVersion\\Uninstall\\"
							 "Steam App 22320\\InstallLocation",
							 "Software\\GOG.com\\Games\\1440163901\\path",
							 "Software\\Microsoft\\Windows\\CurrentVersion\\Uninstall\\"
							 "1440163901_is1\\InstallLocation",
							 "Software\\GOG.com\\Games\\1435828767\\path",
							 "Software\\Microsoft\\Windows\\CurrentVersion\\Uninstall\\"
							 "1435828767_is1\\InstallLocation" };
			pluginsFolderName_ = "Data Files";
			lootFolderName_ = "Morrowind";
			masterFile_ = "Morrowind.esm";
			minimumHeaderVersion_ = 1.2f;
			repositoryURL_ = "https://github.com/loot/morrowind.git";
		} else if (Type() == GameType::tes4) {
			name_ = "TES IV: Oblivion";
			registryKeys_ = { "Software\\Bethesda Softworks\\Oblivion\\Installed Path",
							 "Software\\Microsoft\\Windows\\CurrentVersion\\Uninstall\\"
							 "Steam App 22330\\InstallLocation",
							 "Software\\Microsoft\\Windows\\CurrentVersion\\Uninstall\\"
							 "Steam App 900883\\InstallLocation",
							 "Software\\GOG.com\\Games\\1242989820\\path",
							 "Software\\Microsoft\\Windows\\CurrentVersion\\Uninstall\\"
							 "1242989820_is1\\InstallLocation",
							 "Software\\GOG.com\\Games\\1458058109\\path",
							 "Software\\Microsoft\\Windows\\CurrentVersion\\Uninstall\\"
							 "1458058109_is1\\InstallLocation" };
			pluginsFolderName_ = "Data";
			lootFolderName_ = "Oblivion";
			masterFile_ = "Oblivion.esm";
			minimumHeaderVersion_ = 0.8f;
			repositoryURL_ = "https://github.com/loot/oblivion.git";
		} else if (Type() == GameType::tes5) {
			name_ = "TES V: Skyrim";
			registryKeys_ = { "Software\\Bethesda Softworks\\Skyrim\\Installed Path",
							 "Software\\Microsoft\\Windows\\CurrentVersion\\Uninstall\\"
							 "Steam App 72850\\InstallLocation" };
			pluginsFolderName_ = "Data";
			lootFolderName_ = "Skyrim";
			masterFile_ = "Skyrim.esm";
			minimumHeaderVersion_ = 0.94f;
			repositoryURL_ = "https://github.com/loot/skyrim.git";
		} else if (Type() == GameType::tes5se) {
			name_ = "TES V: Skyrim Special Edition";
			registryKeys_ = {
				"Software\\Bethesda Softworks\\Skyrim Special Edition\\Installed Path",
				"Software\\Microsoft\\Windows\\CurrentVersion\\Uninstall\\Steam App "
				"489830\\InstallLocation" };
			pluginsFolderName_ = "Data";
			lootFolderName_ = "Skyrim Special Edition";
			masterFile_ = "Skyrim.esm";
			minimumHeaderVersion_ = 1.7f;
			repositoryURL_ = "https://github.com/loot/skyrimse.git";
		} else if (Type() == GameType::tes5vr) {
			name_ = "TES V: Skyrim VR";
			registryKeys_ = { "Software\\Bethesda Softworks\\Skyrim VR\\Installed Path",
							 "Software\\Microsoft\\Windows\\CurrentVersion\\Uninstall\\"
							 "Steam App 611670\\InstallLocation" };
			pluginsFolderName_ = "Data";
			lootFolderName_ = "Skyrim VR";
			masterFile_ = "Skyrim.esm";
			minimumHeaderVersion_ = 1.7f;
			repositoryURL_ = "https://github.com/loot/skyrimvr.git";
		} else if (Type() == GameType::fo3) {
			name_ = "Fallout 3";
			registryKeys_ = { "Software\\Bethesda Softworks\\Fallout3\\Installed Path",
							 "Software\\Microsoft\\Windows\\CurrentVersion\\Uninstall\\"
							 "Steam App 22300\\InstallLocation",
							 "Software\\Microsoft\\Windows\\CurrentVersion\\Uninstall\\"
							 "Steam App 22370\\InstallLocation",
							 "Software\\GOG.com\\Games\\1454315831\\path",
							 "Software\\Microsoft\\Windows\\CurrentVersion\\Uninstall\\"
							 "1454315831_is1\\InstallLocation",
							 "Software\\GOG.com\\Games\\1248282609\\path",
							 "Software\\Microsoft\\Windows\\CurrentVersion\\Uninstall\\"
							 "1248282609_is1\\InstallLocation" };
			pluginsFolderName_ = "Data";
			lootFolderName_ = "Fallout3";
			masterFile_ = "Fallout3.esm";
			minimumHeaderVersion_ = 0.94f;
			repositoryURL_ = "https://github.com/loot/fallout3.git";
		} else if (Type() == GameType::fonv) {
			name_ = "Fallout: New Vegas";
			registryKeys_ = { "Software\\Bethesda Softworks\\FalloutNV\\Installed Path",
							 "Software\\Microsoft\\Windows\\CurrentVersion\\Uninstall\\"
							 "Steam App 22380\\InstallLocation",
							 "Software\\Microsoft\\Windows\\CurrentVersion\\Uninstall\\"
							 "Steam App 22490\\InstallLocation",
							 "Software\\GOG.com\\Games\\1312824873\\path",
							 "Software\\Microsoft\\Windows\\CurrentVersion\\Uninstall\\"
							 "1312824873_is1\\InstallLocation",
							 "Software\\GOG.com\\Games\\1454587428\\path",
							 "Software\\Microsoft\\Windows\\CurrentVersion\\Uninstall\\"
							 "1454587428_is1\\InstallLocation" };
			pluginsFolderName_ = "Data";
			lootFolderName_ = "FalloutNV";
			masterFile_ = "FalloutNV.esm";
			minimumHeaderVersion_ = 1.32f;
			repositoryURL_ = "https://github.com/loot/falloutnv.git";
		} else if (Type() == GameType::fo4) {
			name_ = "Fallout 4";
			registryKeys_ = { "Software\\Bethesda Softworks\\Fallout4\\Installed Path",
							 "Software\\Microsoft\\Windows\\CurrentVersion\\Uninstall\\"
							 "Steam App 377160\\InstallLocation" };
			pluginsFolderName_ = "Data";
			lootFolderName_ = "Fallout4";
			masterFile_ = "Fallout4.esm";
			minimumHeaderVersion_ = 0.95f;
			repositoryURL_ = "https://github.com/loot/fallout4.git";
		} else if (Type() == GameType::fo4vr) {
			name_ = "Fallout 4 VR";
			registryKeys_ = {
				"Software\\Bethesda Softworks\\Fallout 4 VR\\Installed Path",
				"Software\\Microsoft\\Windows\\CurrentVersion\\Uninstall\\Steam App "
				"611660\\InstallLocation" };
			pluginsFolderName_ = "Data";
			lootFolderName_ = "Fallout4VR";
			masterFile_ = "Fallout4.esm";
			minimumHeaderVersion_ = 0.95f;
			repositoryURL_ = "https://github.com/loot/fallout4vr.git";
		}

		if (!folder.empty())
			lootFolderName_ = folder;
	}

	bool GameSettings::IsRepoBranchOldDefault() const {
		return oldDefaultBranches.count(repositoryBranch_) == 1;
	}

	bool GameSettings::operator==(const GameSettings& rhs) const {
		return name_ == rhs.Name() || lootFolderName_ == rhs.FolderName();
	}

	GameType GameSettings::Type() const {
		return type_;
	}

	std::string GameSettings::Name() const {
		return name_;
	}

	std::string GameSettings::FolderName() const {
		return lootFolderName_;
	}

	std::string GameSettings::Master() const {
		return masterFile_;
	}

	float GameSettings::MinimumHeaderVersion() const {
		return minimumHeaderVersion_;
	}

	std::vector<std::string> GameSettings::RegistryKeys() const {
		return registryKeys_;
	}

	std::string GameSettings::RepoURL() const {
		return repositoryURL_;
	}

	std::string GameSettings::RepoBranch() const {
		return repositoryBranch_;
	}

	fs::path GameSettings::GamePath() const {
		return gamePath_;
	}

	fs::path GameSettings::GameLocalPath() const {
		return gameLocalPath_;
	}

	fs::path GameSettings::DataPath() const {
		return gamePath_ / pluginsFolderName_;
	}

	GameSettings& GameSettings::SetName(const std::string& name) {
		name_ = name;
		return *this;
	}

	GameSettings& GameSettings::SetMaster(const std::string& masterFile) {
		masterFile_ = masterFile;
		return *this;
	}

	GameSettings& GameSettings::SetMinimumHeaderVersion(float minimumHeaderVersion) {
		minimumHeaderVersion_ = minimumHeaderVersion;
		return *this;
	}

	GameSettings& GameSettings::SetRegistryKeys(
		const std::vector<std::string>& registry) {
		registryKeys_ = registry;
		return *this;
	}

	GameSettings& GameSettings::SetRepoURL(const std::string& repositoryURL) {
		repositoryURL_ = repositoryURL;
		return *this;
	}

	GameSettings& GameSettings::SetRepoBranch(const std::string& repositoryBranch) {
		repositoryBranch_ = repositoryBranch;
		return *this;
	}

	GameSettings& GameSettings::SetGamePath(const fs::path& path) {
		gamePath_ = path;
		return *this;
	}

	GameSettings& GameSettings::SetGameLocalPath(const fs::path& path) {
		gameLocalPath_ = path;
		return *this;
	}

	GameSettings& GameSettings::SetGameLocalFolder(const std::string& folderName) {
		TCHAR path[MAX_PATH];

		HRESULT res = ::SHGetFolderPath(nullptr, CSIDL_LOCAL_APPDATA, nullptr, SHGFP_TYPE_CURRENT, path);
		fs::path appData;
		if (res == S_OK) {
			appData = fs::path(path);
		}
		else {
			appData = fs::path("");
		}
		gameLocalPath_ = appData / fs::path(folderName);
		return *this;
	}

	void GameSettings::MigrateSettings() {
		// If the masterlist repository is set to an official repository and the
		// masterlist branch is an old default branch, update the masterlist
		// branch to the current default.
		if (boost::starts_with(repositoryURL_, "https://github.com/loot/") &&
			IsRepoBranchOldDefault()) {
			SetRepoBranch(currentDefaultBranch);
		}

		// If the game is Skyrim VR or Fallout 4 VR and the masterlist repository
		// is the official Skyrim SE or Fallout 4 repository (respectively),
		// switch to the newer VR-specific repositories.
		if (Type() == GameType::tes5vr &&
			RepoURL() == GameSettings(GameType::tes5se).RepoURL()) {
			SetRepoURL(GameSettings(GameType::tes5vr).RepoURL());
		}

		if (Type() == GameType::fo4vr &&
			RepoURL() == GameSettings(GameType::fo4).RepoURL()) {
			SetRepoURL(GameSettings(GameType::fo4vr).RepoURL());
		}
	}
}
