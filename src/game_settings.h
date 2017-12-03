#ifndef LOOT_GUI_STATE_GAME_SETTINGS
#define LOOT_GUI_STATE_GAME_SETTINGS

#include <set>
#include <string>

#include <boost/filesystem.hpp>

#include "loot/enum/game_type.h"

namespace loot {
	class GameSettings {
	public:
		GameSettings();
		explicit GameSettings(const GameType gameType, const std::string& lootFolder = "");

		bool IsRepoBranchOldDefault() const;

		bool operator == (const GameSettings& rhs) const;  //Compares names and folder names.

		GameType Type() const;
		std::string Name() const;  //Returns the game's name, eg. "TES IV: Oblivion".
		std::string FolderName() const;
		std::string Master() const;
		std::string RegistryKey() const;
		std::string RepoURL() const;
		std::string RepoBranch() const;
		boost::filesystem::path GamePath() const;

		GameSettings& SetName(const std::string& name);
		GameSettings& SetMaster(const std::string& masterFile);
		GameSettings& SetRegistryKey(const std::string& registry);
		GameSettings& SetRepoURL(const std::string& repositoryURL);
		GameSettings& SetRepoBranch(const std::string& repositoryBranch);
		GameSettings& SetGamePath(const boost::filesystem::path& path);

	private:
		static const std::set<std::string> oldDefaultBranches;

		GameType type_;
		std::string name_;
		std::string masterFile_;

		std::string registryKey_;

		std::string lootFolderName_;
		std::string repositoryURL_;
		std::string repositoryBranch_;

		boost::filesystem::path gamePath_;  //Path to the game's folder.
	};
}

#endif