#pragma once
#include <Windows.h>
#include <filesystem>
#include <iostream>
#include <string>
#include <unordered_map>

namespace asa::settings
{
	bool Load();
	bool OpenFile(
		bool verbose, std::filesystem::path path, std::ifstream& fileOut);

	namespace actionMappings
	{
		struct ActionMapping;
		inline std::unordered_map<std::string, ActionMapping*> inputMap{};

		struct ActionMapping
		{
			ActionMapping(std::string name) : name(name)
			{
				inputMap[name] = this;
			};

			std::string name;
			bool shift{ false }, ctrl{ false }, alt{ false }, cmd{ false };
			std::string key;
		};
		std::ostream& operator<<(std::ostream& os, const ActionMapping& m);

		const auto inputsRelPath = std::filesystem::path(
			R"(ShooterGame\Saved\Config\Windows\Input.ini)");

		inline ActionMapping accessInventory("AccessInventory");
		inline ActionMapping crouch("Crouch");
		inline ActionMapping prone("Prone");
		inline ActionMapping orbitCam("OrbitCam");
		inline ActionMapping pauseMenu("PauseMenu");
		inline ActionMapping ping("Ping");
		inline ActionMapping poop("Poop");
		inline ActionMapping reload("Reload");
		inline ActionMapping showTribeManager("ShowTribeManager");
		inline ActionMapping toggleMap("ToggleMap");
		inline ActionMapping transferItem("TransferItem");
		inline ActionMapping use("Use");
		inline ActionMapping useItem1("UseItem1");
		inline ActionMapping useItem2("UseItem2");
		inline ActionMapping useItem3("UseItem3");
		inline ActionMapping useItem4("UseItem4");
		inline ActionMapping useItem5("UseItem5");
		inline ActionMapping useItem6("UseItem6");
		inline ActionMapping useItem7("UseItem7");
		inline ActionMapping useItem8("UseItem8");
		inline ActionMapping useItem9("UseItem9");
		inline ActionMapping useItem10("UseItem10");
		inline ActionMapping showMyInventory("ShowMyInventory");
		inline ActionMapping dropItem("DropItem");
		inline ActionMapping fire("Fire");
		inline ActionMapping targeting("Targeting");
		inline ActionMapping showExtendedInfo("ShowExtendedInfo");
		inline ActionMapping toggleTooltip("ToggleTooltip");

		bool LoadActionMappings(bool verbose = true);
	}

	namespace gameUserSettings
	{
		const auto userSettingsRelPath = std::filesystem::path(
			R"(ShooterGame\Saved\Config\Windows\GameUserSettings.ini)");

		bool LoadGameUserSettings(bool verbose = true);

		inline float UIScaling{ 0 };
		inline float UIQuickbarScaling{ 0 };
		inline float CameraShakeScale{ 0 };
		inline float FOVMultiplier{ 0 };

		inline bool bFirstPersonRiding{ false };
		inline bool bThirdPersonPlayer{ false };
		inline bool bShowStatusNotificationMessages{ false };

	}
}
