/**
 * @file renderer.cpp
 * @brief Implements the renderer class methods for GPU texture loading, tile rendering, and cleanup.
 *
 * This file provides the Raylib-backed rendering pipeline for the game. It sequentially loads
 * all texture assets defined in `assetPaths` into GPU memory, renders individual tiles on screen
 * using their corresponding `tileID` index, and releases all allocated VRAM when the game exits.
 */

#include "renderer.h"
#include "raylib.h"
#include <cstdint>
#include <iostream>
#include <string>
#include <vector>

Texture2D tex; ///< Temporary Texture2D handle used while loading assets from disk into VRAM

/**
 * @func renderer::load_assets
 * @brief Iterates through `assetPaths`, loads each image from disk as a Raylib Texture2D, and stores it.
 */
void renderer::load_assets() {
  for (const auto& path : assetPaths) {
    tex = LoadTexture(path.c_str()); // Load image file into GPU memory (VRAM)
    textures.push_back(tex);         // Store handle at index matching its tileID
  }
}

/**
 * @func renderer::draw_tile
 * @brief Draws a loaded tile texture at the specified screen coordinates without color tinting.
 * @param x The horizontal screen X-coordinate in pixels where the top-left of the texture will be drawn.
 * @param y The vertical screen Y-coordinate in pixels where the top-left of the texture will be drawn.
 * @param tileID The index into the `textures` vector identifying which sprite to render.
 */
void renderer::draw_tile(int x, int y, uint8_t tileID) {
  // Render texture with WHITE tint (preserves original sprite colors)
  DrawTexture(textures[tileID], x, y, WHITE);
}

/**
 * @func renderer::unload_assets
 * @brief Unloads all active Texture2D objects from GPU memory to prevent VRAM leaks.
 */
void renderer::unload_assets() {
  for (const auto& tex : textures) {
    UnloadTexture(tex); // Free texture data from GPU memory
  }
}