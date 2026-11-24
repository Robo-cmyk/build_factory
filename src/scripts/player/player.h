#pragma once

#include <cstdint>
#include <string>
#include <vector>

/**
 * @file player.h
 * @brief Defines the player class and supporting inventory data structures.
 *
 * This header provides the core representation of the player entity, including world/screen
 * positioning, movement speed, and inventory management. It defines individual item entries
 * (`InventoryItem`), the container holding them (`Inventory`), and methods for serializing
 * and deserializing the player's inventory state to disk.
 */

/**
 * @struct InventoryItem
 * @brief Represents a single stackable item entry within a player's inventory.
 */
struct InventoryItem {
  uint8_t itemID;    ///< Unique numeric identifier for the item type
  std::string name;  ///< Display name of the item
  int count;         ///< Current stack quantity of the item
  uint8_t invslot;   ///< Assigned slot index within the inventory UI/grid
};

/**
 * @struct Inventory
 * @brief Container holding the collection of items currently carried by the player.
 */
struct Inventory {
  std::vector<InventoryItem> items; ///< List of active item stacks in the inventory
};

/**
 * @class player
 * @brief Represents the playable character, tracking spatial coordinates, movement, and inventory persistence.
 */
class player {
public:
  int playerX;          ///< Player's current X-coordinate in the game world
  int playerY;          ///< Player's current Y-coordinate in the game world
  float speed = 1.0f;   ///< Movement speed multiplier (defaults to 1.0)

  Inventory inv;        ///< The player's personal item inventory

  /**
   * @func save_inventory
   * @brief Serializes and writes the player's current inventory contents to a file.
   * @param filepath Path to the save directory or target file where inventory data will be stored.
   */
  void save_inventory(const char* filepath);

  /**
   * @func load_inventory
   * @brief Reads and deserializes saved inventory items from disk into `inv`.
   * @param filepath Path to the save directory or source file containing saved inventory data.
   */
  void load_inventory(const char* filepath);
};
