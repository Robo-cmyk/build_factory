/**
 * @file player.cpp
 * @brief Implements the player class methods for JSON-based inventory serialization and deserialization.
 *
 * This file uses the `nlohmann::json` library to persist the player's inventory state to disk at
 * `<world_root>/player/inventory.json`. If no save file exists during loading, it initializes
 * the player with a default starter kit (logs and an axe) and immediately creates the save file.
 */

#include "player.h"
#include "../nlohmanjson/json.hpp"
#include <fstream>
#include <string>

using json = nlohmann::json; ///< Type alias for convenience when working with nlohmann JSON objects

/**
 * @func player::save_inventory
 * @brief Serializes the player's current inventory items into a formatted JSON file.
 * @param filename Root directory path of the world save folder (writes to `/player/inventory.json`).
 */
void player::save_inventory(const char *filename) {
  json j;
  j["inventory"] = json::array(); // Initialize root JSON array to hold item entries

  // Convert each InventoryItem struct into a JSON object
  for (const auto &item : inv.items) {
    j["inventory"].push_back({{"item_id", item.itemID},
                              {"name", item.name},
                              {"count", item.count},
                              {"slot", item.invslot}});
  }

  // Open output stream and write pretty-printed JSON (4-space indentation)
  std::ofstream file(std::string(filename) + "/player/inventory.json");
  if (file.is_open()) {
    file << j.dump(4);
  }
}

/**
 * @func player::load_inventory
 * @brief Loads the player's inventory from `inventory.json`, or creates a default starter inventory if missing.
 * @param filename Root directory path of the world save folder (reads from `/player/inventory.json`).
 */
void player::load_inventory(const char *filename) {
  std::ifstream file(std::string(filename) + "/player/inventory.json");

  if (!file.is_open()) {
    // Fallback: Populate default starter items (10 logs in slot 0, 1 axe in slot 1) and persist to disk
    inv.items = {{0, "log", 10, 0}, {1, "axe", 1, 1}};
    save_inventory(filename);
  } else {
    // Parse existing JSON file and rebuild the in-memory inventory vector
    json j;
    file >> j;
    inv.items.clear(); // Clear any existing items before populating from disk

    for (const auto &element : j["inventory"]) {
      InventoryItem item;
      item.itemID = element["item_id"];
      item.name = element["name"];
      item.count = element["count"];
      item.invslot = element["slot"];
      inv.items.push_back(item);
    }
  }
}