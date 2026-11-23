#pragma once

#include <string>

/**
 * @file Building.h
 * @brief Defines the base Building class for placeable world structures.
 *
 * This header establishes the foundational base class for all interactive and static
 * structures placed on the game grid. It tracks grid positioning, orientation, and
 * structure type identification, while providing a virtual destructor to ensure safe
 * polymorphic cleanup when inherited by specialized buildings (such as chests or machines).
 */

/**
 * @class Building
 * @brief Base class representing a generic structure placed at a specific grid coordinate in the world.
 */
class Building {
public:
  int x;              ///< Grid X-coordinate (column) where the building is located
  int y;              ///< Grid Y-coordinate (row) where the building is located
  int direction;      ///< Facing orientation of the building (e.g., 0 = North, 1 = East, 2 = South, 3 = West)
  std::string type;   ///< String identifier categorizing the building type (e.g., "chest", "furnace")

  /**
   * @func Building::~Building
   * @brief Virtual default destructor ensuring derived building objects are cleanly destroyed via base pointers.
   */
  virtual ~Building() = default;
};