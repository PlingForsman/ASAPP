#include "asapp/game/resources.h"
#include "asapp/config.h"
#include <iostream>

#define LOAD_RESOURCE(dir, name)                                               \
	name = asa::resources::LoadResource(dir / (std::string(#name) + ".png"));

static bool IsValidAssetsDir(std::filesystem::path path)
{
	if (path.empty()) {
		std::cout << "[!] resources::Init requires resources::assetDir to be "
					 "specified!"
				  << std::endl;
		return false;
	}

	if (!std::filesystem::exists(path)) {
		std::cout << std::format(
						 "[!] Assets directory '{}' not found!", path.string())
				  << std::endl;
		return false;
	}
	return true;
}

cv::Mat asa::resources::LoadResource(std::filesystem::path path)
{
	auto mat = cv::imread(path.string());
	std::cout << "\t[-] Resource loaded @ " << path << std::endl;
	return mat;
}

bool asa::resources::Init()
{
	if (!IsValidAssetsDir(config::assetsDir)) {
		return false;
	}

	std::cout << "[+] Initializing resources..." << std::endl;
	if (!InitInterfaces() || !InitItems() || !InitText()) {
		return false;
	}

	return true;
}

bool asa::resources::interfaces::InitInterfaces()
{
	auto dir = config::assetsDir / "interfaces";
	if (!IsValidAssetsDir(dir)) {
		return false;
	}

	LOAD_RESOURCE(dir, cb_arrowdown);
	LOAD_RESOURCE(dir, regions);
	return true;
}

bool asa::resources::items::InitItems()
{
	auto dir = config::assetsDir / "items";
	if (!IsValidAssetsDir(dir)) {
		return false;
	}

	LOAD_RESOURCE(dir, metal);
	LOAD_RESOURCE(dir, metal_ingot);
	LOAD_RESOURCE(dir, paste);
	LOAD_RESOURCE(dir, crystal);
	LOAD_RESOURCE(dir, fiber);
	LOAD_RESOURCE(dir, flint);
	LOAD_RESOURCE(dir, gunpowder);
	LOAD_RESOURCE(dir, sparkpowder);
	LOAD_RESOURCE(dir, metal);
	LOAD_RESOURCE(dir, obsidian);
	LOAD_RESOURCE(dir, polymer);
	LOAD_RESOURCE(dir, stone);
	LOAD_RESOURCE(dir, thatch);
	LOAD_RESOURCE(dir, wood);
	LOAD_RESOURCE(dir, gasmask);

	return true;
}

bool asa::resources::text::InitText()
{
	auto dir = config::assetsDir / "text";
	if (!IsValidAssetsDir(dir)) {
		return false;
	}
	LOAD_RESOURCE(dir, added);
	LOAD_RESOURCE(dir, removed);
	LOAD_RESOURCE(dir, day);
	LOAD_RESOURCE(dir, lootcrate);

	return true;
}