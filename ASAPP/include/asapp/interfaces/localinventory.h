#pragma once
#include "baseinventory.h"
#include "playerinfo.h"

namespace asa::interfaces
{
	class LocalInventory : public BaseInventory
	{
	public:
		LocalInventory() : BaseInventory(false){};

		enum Tab
		{
			INVENTORY,
			COSMETICS,
			CRAFTING
		};

		PlayerInfo info{};

		InvTabButton inventoryTab{ 177, 118, 183, 47 };
		InvTabButton cosmeticsTab{ 358, 120, 184, 46 };
		InvTabButton craftingTab{ 542, 120, 183, 45 };

		void open();
		void switch_to(Tab tab);
		void toggle_tooltips();

		void equip(items::Item* item, PlayerInfo::Slot slot);
	};

	inline LocalInventory* gLocalinventory = new LocalInventory();
}