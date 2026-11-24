#pragma once

#include <cstdint>
#include <vector>
#include "../building/chest.h"

/**
 * @file world.h
 * @brief Defines the world class responsible for managing the game map, tile data, and world structures.
 *
 * The world class handles creating, loading, and saving 2D grid-based maps stored as a flattened
 * 1D vector of tile IDs. It also manages the persistence and storage of interactive buildings
 * and containers (such as chests) placed within the world.
 */

class world {
public:
  /**
   * @func load_world
   * @brief Loads an existing world map from a specified file into memory.
   * @param filepath Path to the file containing the saved world data.
   */
  void load_world(const char *filepath);

  /**
   * @func create_world
   * @brief Generates a new world with the given dimensions and saves it to a file.
   * @param width The horizontal width of the world in tiles.
   * @param height The vertical height of the world in tiles.
   * @param filepath Path where the newly created world file will be saved.
   */
  void create_world(int width, int height, const char *filepath, unsigned int seed);

  /**
   * @func save_world
   * @brief Serializes and writes the current world state to disk.
   * @param filepath Destination path where the world data should be saved.
   */
  void save_world(const char *filepath);

  /**
   * @func get_tileID
   * @brief Retrieves the tile identifier at the specified 2D coordinates.
   * @param x The X-coordinate (column) of the tile.
   * @param y The Y-coordinate (row) of the tile.
   * @return uint8_t The ID representing the tile type at (x, y).
   */
  uint8_t get_tileID(int x, int y);

  /**
   * @func load_buildings
   * @brief Loads building and container data (e.g., chests) from a file.
   * @param filepath Path to the file containing serialized building data.
   */
  void load_buildings(const char *filepath);

  /**
   * @func save_buildings
   * @brief Saves all current building and container states to a file.
   * @param filepath Destination path where the building data should be written.
   */
  void save_buildings(const char *filepath);

  /**
   * @func set_tileID
   * @brief Updates the tile identifier at the specified 2D coordinates.
   * @param x The X-coordinate (column) of the tile to modify.
   * @param y The Y-coordinate (row) of the tile to modify.
   * @param tileID The new tile ID to assign to the coordinates.
   */
  void set_tileID(int x, int y, uint8_t tileID);

  int width;                         ///< Total width of the world grid (in tiles)
  int height;                        ///< Total height of the world grid (in tiles)
  std::vector<uint8_t> worldvector;  ///< Flattened 1D array storing 2D tile IDs (typically indexed via y * width + x)
  std::vector<chest> chests;         ///< Collection of all active chests/containers located within the world
};
