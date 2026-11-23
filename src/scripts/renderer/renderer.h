#pragma once

#include "raylib.h"
#include <cstdint>
#include <iostream>
#include <string>
#include <vector>

/**
 * @file renderer.h
 * @brief Defines the renderer class responsible for loading, managing, and drawing 2D game textures.
 *
 * The renderer class wraps Raylib's Texture2D lifecycle management. It maintains a registry of
 * file paths for environment tiles and structures, loads them into GPU memory on startup,
 * maps tile IDs to their corresponding textures during the render loop, and cleanly unloads
 * them upon shutdown.
 */

class renderer {
public:
  /**
   * @func draw_tile
   * @brief Renders a specific tile texture at the given screen or grid coordinates.
   * @param x The horizontal X position where the tile should be drawn.
   * @param y The vertical Y position where the tile should be drawn.
   * @param tileID The index/identifier mapping to the corresponding texture in the textures vector.
   */
  void draw_tile(int x, int y, uint8_t tileID);

  /**
   * @func load_assets
   * @brief Iterates through assetPaths and loads all sprite textures into GPU memory.
   */
  void load_assets();

  /**
   * @func unload_assets
   * @brief Frees all loaded Raylib Texture2D resources from GPU memory to prevent memory leaks.
   */
  void unload_assets();

public:
  /**
   * @brief Ordered list of file paths for game textures; vector indices correspond directly to tile IDs.
   */
  std::vector<std::string> assetPaths = {
      "src/assets/nature/grass/grass_1.png",  ///< Tile ID 0: Grass variant 1
      "src/assets/nature/grass/grass_2.png",  ///< Tile ID 1: Grass variant 2
      "src/assets/nature/grass/grass_3.png",  ///< Tile ID 2: Grass variant 3
      "src/assets/nature/grass/grass_4.png",  ///< Tile ID 3: Grass variant 4
      "src/assets/nature/grass/grass_5.png",  ///< Tile ID 4: Grass variant 5
      "src/assets/nature/grass/grass_6.png",  ///< Tile ID 5: Grass variant 6
      "src/assets/nature/grass/grass_7.png",  ///< Tile ID 6: Grass variant 7
      "src/assets/nature/grass/grass_8.png",  ///< Tile ID 7: Grass variant 8
      "src/assets/nature/grass/grass_9.png",  ///< Tile ID 8: Grass variant 9
      "src/assets/nature/grass/grass_10.png", ///< Tile ID 9: Grass variant 10
      "src/assets/chests/chest_iron1.png"     ///< Tile ID 10: Iron chest sprite
  };

  std::vector<Texture2D> textures; ///< Loaded Raylib GPU textures indexed by their corresponding tileID
};