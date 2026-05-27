#pragma once
#include <cstdint>
#include <iostream>
#include <vector>

struct ChestItem {
  int ItemID;
  int count = 0;
};

struct chest {
  int x, y;
  uint8_t chestID = 0;

  std::vector<ChestItem> slots;
  chest(int gridX, int gridY, int slotcount) {
    x = gridX;
    y = gridY;
    slots.resize(slotcount);
    for(auto& slot : slots){
      slot = (ChestItem) { -1 , 0};
    }
  }

  void printInventory() const {
    std::cout << "\n ---CHEST at (" << x << "," << y << ") ---" << std::endl;
    for (size_t i = 0; i < slots.size(); i++) {
      if (slots[i].ItemID == -1) {
        std::cout << "Slot " << i << ": [Empty]" << std::endl;
      } else {
        std::cout << "Slot" << i << ": [ItemId :" << slots[i].ItemID << " | Count"
                  << slots[i].count<< " ]" << std::endl;
      }
    }
    std::cout << "-----------------------------------------\n" << std::endl;
  }
};
