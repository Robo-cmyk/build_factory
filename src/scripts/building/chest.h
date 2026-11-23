#pragma once

#include <cstdint>
#include <iostream>
#include <vector>

/**
 * @file chest.h
 * @brief Defines the chest structure and its inventory slot representation for world storage containers.
 *
 * This header models placeable storage containers within the game world. It defines `ChestItem`
 * to track individual item stacks (using `-1` as a sentinel ID for empty slots) and the `chest`
 * struct to manage world grid positioning, container tier identification, slot initialization,
 * and console debug printing of inventory contents.
 */

/**
 * @struct ChestItem
 * @brief Represents a single item stack stored inside a chest slot.
 */
struct ChestItem {
  int ItemID;     ///< Unique identifier for the stored item (-1 indicates an empty slot)
  int count = 0;  ///< Number of items stacked in this slot
};

/**
 * @struct chest
 * @brief Represents a placeable storage container at a specific (x, y) grid coordinate in the world.
 */
struct chest {
  int x;                         ///< Grid X-coordinate (column) where the chest is placed
  int y;                         ///< Grid Y-coordinate (row) where the chest is placed
  uint8_t chestID = 0;           ///< Container type/tier identifier (e.g., 1 for basic, 2 for iron)

  std::vector<ChestItem> slots;  ///< Fixed-size inventory array holding the chest's item stacks

  /**
   * @func chest::chest
   * @brief Constructs a new chest at the specified grid coordinates and initializes all slots to empty.
   * @param gridX The horizontal grid X-coordinate of the chest.
   * @param gridY The vertical grid Y-coordinate of the chest.
   * @param slotcount The total number of inventory slots to allocate for this chest.
   */
  chest(int gridX, int gridY, int slotcount) {
    x = gridX;
    y = gridY;
    slots.resize(slotcount); // Allocate requested inventory capacity

    // Initialize every slot as empty (ItemID = -1, count = 0)
    for (auto& slot : slots) {
      slot = (ChestItem){-1, 0};
    }
  }

  /**
   * @func chest::printInventory
   * @brief Outputs the chest's world coordinates and the contents of each slot to standard output for debugging.
   */
  void printInventory() const {
    std::cout << "\n ---CHEST at (" << x << "," << y << ") ---" << std::endl;

    // Iterate through all slots and display either "[Empty]" or the stored ItemID and count
    for (size_t i = 0; i < slots.size(); i++) {
      if (slots[i].ItemID == -1) {
        std::cout << "Slot " << i << ": [Empty]" << std::endl;
      } else {
        std::cout << "Slot" << i << ": [ItemId :" << slots[i].ItemID << " | Count"
                  << slots[i].count << " ]" << std::endl;
      }
    }

    std::cout << "-----------------------------------------\n" << std::endl;
  }
};