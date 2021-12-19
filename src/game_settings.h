#ifndef LOOT_GUI_STATE_GAME_GAME_SETTINGS
#define LOOT_GUI_STATE_GAME_GAME_SETTINGS

#include <filesystem>
#include <optional>
#include <set>
#include <string>

#include "loot/enum/game_type.h"

namespace loot {
	constexpr inline std::string_view NEHRIM_STEAM_REGISTRY_KEY =
		"Software\\Microsoft\\Windows\\CurrentVersion\\Uninstall\\Steam App "
		"1014940\\InstallLocation";

	class GameSettings {
	public:
		GameSettings();
		explicit GameSettings(const GameType gameType,
			const std::string& lootFolder = "");

		bool IsRepoBranchOldDefault() const;

		bool operator==(
			const GameSettings& rhs) const;  // Compares names and folder names.

		GameType Type() const;
		std::string Name() const;  //Returns the game's name, eg. "TES IV: Oblivion".
		std::string FolderName() const;
		std::string Master() const;
		float MinimumHeaderVersion() const;
		std::vector<std::string> RegistryKeys() const;
		std::string RepoURL() const;
		std::string RepoBranch() const;
		std::filesystem::path GamePath() const;
		std::filesystem::path GameLocalPath() const;
		std::filesystem::path DataPath() const;

		GameSettings& SetName(const std::string& name);
		GameSettings& SetMaster(const std::string& masterFile);
		GameSettings& SetMinimumHeaderVersion(float minimumHeaderVersion);
		GameSettings& SetRegistryKeys(const std::vector<std::string>& registry);
		GameSettings& SetRepoURL(const std::string& repositoryURL);
		GameSettings& SetRepoBranch(const std::string& repositoryBranch);
		GameSettings& SetGamePath(const std::filesystem::path& path);
    	GameSettings& SetGameLocalPath(const std::filesystem::path& GameLocalPath);
		GameSettings& SetGameLocalFolder(const std::string& folderName);

		void MigrateSettings();

	private:
		static const std::set<std::string> oldDefaultBranches;
		static const std::string currentDefaultBranch;

		GameType type_;
		std::string name_;
		std::string masterFile_;
		float minimumHeaderVersion_;

		std::vector<std::string> registryKeys_;

		std::string pluginsFolderName_;
		std::string lootFolderName_;

		std::string repositoryURL_;
		std::string repositoryBranch_;

		std::filesystem::path gamePath_;  //Path to the game's folder.
    	std::filesystem::path gameLocalPath_;
	};
}

#endif