#pragma once
#include <cstdint>
#include <vector>
#include <string>

struct InventoryItem {
  uint8_t itemID;
  std::string name;
  int count;
  uint8_t invslot;
};

struct Inventory {
  std::vector<InventoryItem> items;
};

class player {
  public:
    int playerX;
    int playerY;
    float speed = 1.0f;

    Inventory inv;

    void save_inventory(const char*);
    void load_inventory(const char*);

  
};
