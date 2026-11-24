/**
 * @file main.cpp
 * @brief Entry point and core game loop for the 2D tile-based sandbox game.
 *
 * This file initializes the Raylib window and global game subsystems (`renderer`, `world`, and `player`),
 * handles initial world creation/loading via CLI prompts, and runs the 60 FPS update/render loop.
 * Core loop responsibilities include camera-centered tile culling, WASD player movement, Ctrl+S
 * world/building serialization, and a toggleable build mode (`B` key) for placing interactive chests.
 */

#include "../building/chest.h"
#include "../player/player.h"
#include "../renderer/renderer.h"
#include "../world/world.h"
#include "raylib.h"
#include <algorithm>
#include <cstdint>
#include <iostream>
#include <set>
#include <string>

// Global subsystem instances and runtime state
renderer RENDERER;                      ///< Global texture manager and tile rendering pipeline
world WORLD;                            ///< Active world map grid and building/container registry
player PLAYER_ENTITY;                   ///< Active player entity tracking position, speed, and inventory

Vector2 worldMousePos;                  ///< Cached world-space mouse coordinates
bool isBuildMode = false;               ///< Toggles structure placement preview and left-click building
std::string currentWorldFolder = "";    ///< Root directory path of the currently active world save

/**
 * @func placement1
 * @brief Handles grid-snapping preview and chest placement when Build Mode is active.
 * @param mouseScreen Current cursor position in screen-space pixels (0..800, 0..480).
 */
void placement1(Vector2 mouseScreen) {
  // 1. Calculate the top-left corner of the camera viewport in world pixels (centered on player)
  float topLeftWorldX = PLAYER_ENTITY.playerX - GetScreenWidth()/2.0f;
  float topLeftWorldY = PLAYER_ENTITY.playerY - GetScreenHeight()/2.0f;

  // 2. Translate screen-space cursor coordinates into world-space coordinates
  float mouseWorldX = topLeftWorldX + mouseScreen.x;
  float mouseWorldY = topLeftWorldY + mouseScreen.y;

  // 3. Convert world pixels into 32x32 grid tile indices
  int gridX = (int)(mouseWorldX / 32.0f);
  int gridY = (int)(mouseWorldY / 32.0f);

  // Adjust floor division for negative world coordinates so snapping stays accurate
  if (mouseWorldX < 0)
    gridX--;
  if (mouseWorldY < 0)
    gridY--;

  // 4. Convert snapped grid coordinates back into screen-space pixels for rendering the ghost preview
  float screenX = (gridX * 32.0f) - topLeftWorldX;
  float screenY = (gridY * 32.0f) - topLeftWorldY;

  // 5. Verify target tile is within valid world boundaries before drawing or placing
  if (gridX >= 0 && gridX < WORLD.width && gridY >= 0 && gridY < WORLD.height) {
    // Draw a semi-transparent (40% opacity) iron chest sprite as a placement preview
    DrawTexture(RENDERER.textures[10], screenX, screenY, Fade(WHITE, 0.4f));

    // Place a new chest on left-click if the target tile does not already contain a chest (Tile ID 11)
    if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON) &&
        std::set<uint8_t>{11}.count(WORLD.get_tileID(gridX, gridY)) == 0) {

      uint8_t newTileID = 11;
      WORLD.set_tileID(gridX, gridY, newTileID); // Mark tile as occupied by a chest

  if ( IsMouseButtonPressed(MOUSE_LEFT_BUTTON) && std::set<uint8_t>{11}.count(WORLD.get_tileID(gridX , gridY)) == 0 ) {
    uint8_t newTileID = 11;
    WORLD.set_tileID(gridX, gridY, newTileID);
    struct chest newChest(gridX, gridY, 5);
    newChest.chestID = 1;
    newChest.slots[0] = ((ChestItem){0, 1});
    newChest.slots[1] = ((ChestItem){1, 5});
    WORLD.chests.push_back(newChest);
    for (const auto &chest : WORLD.chests) {
      chest.printInventory();
      // Instantiate a 5-slot basic chest with starter debug items and register it in the world
      struct chest newChest(gridX, gridY, 5);
      newChest.chestID = 1;
      newChest.slots[0] = ((ChestItem){0, 1});
      newChest.slots[1] = ((ChestItem){1, 5});
      WORLD.chests.push_back(newChest);

      // Debug output: print all active chest inventories to the console
      for (const auto &chest : WORLD.chests) {
        chest.printInventory();
      }
    }
  }
}
}
}

/**
 * @func draw_game
 * @brief Renders the visible portion of the world tilemap centered around the given world coordinates.
 * @param world_x Camera center X-coordinate in world pixels.
 * @param world_y Camera center Y-coordinate in world pixels.
 */
void draw_game(float world_x, float world_y) {
  // 1. Compute top-left camera boundary in world pixels (half of 800x480 resolution)
  float topLeftWorldX = world_x - GetScreenWidth()/2.0f;
  float topLeftWorldY = world_y - GetScreenHeight()/2.0f;

  // 2. Determine the starting grid indices visible at the top-left corner of the viewport
  int startTileX = (int)(topLeftWorldX / 32.0f);
  int startTileY = (int)(topLeftWorldY / 32.0f);

  // Pad starting indices by -1 to prevent pop-in along the left and top screen edges
  startTileX--;
  if (topLeftWorldY < 0)
    startTileY--;

  // 3. Frustum-culled render loop: only iterate over tiles visible within the 800x480 screen (27x17 grid)
  for (int col = 0; col < GetScreenWidth()/2 + 2; col++) {
    for (int row = 0; row < GetScreenHeight()/2 + 2; row++) {

      int targetTileX = startTileX + col;
      int targetTileY = startTileY + row;

      // 4. Project world tile coordinates into screen-space pixel coordinates
      float drawX = (targetTileX * 32.0f) - topLeftWorldX;
      float drawY = (targetTileY * 32.0f) - topLeftWorldY;

      // Ensure tile lies within valid map boundaries before querying or drawing
      if (targetTileX >= 0 && targetTileX < WORLD.width && targetTileY >= 0 &&
          targetTileY < WORLD.height) {

        int tileID = WORLD.get_tileID(targetTileX, targetTileY);

        if (tileID != 11) {
          // Standard terrain tile (IDs 0–9)
          RENDERER.draw_tile((int)drawX, (int)drawY, tileID);
        } else {
          // Tile ID 11 maps to index 10 in RENDERER.textures (iron chest sprite)
          DrawTexture(RENDERER.textures[10], drawX, drawY, WHITE);
        }
      }
    }
  }
}

/**
 * @func main
 * @brief Initializes the game window, handles world setup/loading, and executes the main game loop.
 * @return int Exit status code (0 on clean shutdown).
 */
int main() {
  SetConfigFlags(FLAG_WINDOW_RESIZABLE);
  InitWindow(800, 480, "Game");
  SetTargetFPS(60);
  RENDERER.load_assets();

  // CLI startup menu: prompt user to generate a new world or load an existing save
  std::string choice = "";
  while (choice != "1" && choice != "2") {
    std::cout << "Choose (1) create_world (2) load_world -> ";
    std::cin >> choice;

    if (choice == "1") {
      std::string worldname;
      std::cout << "Enter world name -> ";
      std::cin >> worldname;

      // Set unified root folder path for map, building, and player subdirectories
      currentWorldFolder = "src/assets/maps/" + worldname;

      int w, h, seed;
      std::cout << "Enter width should be between 100 to 10000 -> ";
      std::cin >> w;
      std::cout << "Enter height should be between 100 to 10000 -> ";
      std::cin >> h;
      std::cout << "Enter seed -> ";
      std::cin >> seed;

      // Validate dimensions; fallback to 100x100 if out of range
      if ((w >= 100 && w <= 10000) && (h >= 100 && h <= 10000)) {
        WORLD.create_world(w, h, currentWorldFolder.c_str(), seed);
      } else {
        WORLD.create_world(100, 100, currentWorldFolder.c_str(), seed);
      }

      // FIX: Load directly out of the folder you created
      WORLD.load_world(currentWorldFolder.c_str());
      WORLD.load_buildings(currentWorldFolder.c_str());

      // Load newly generated map and building data into memory
      WORLD.load_world(currentWorldFolder.c_str());
      WORLD.load_buildings(currentWorldFolder.c_str());

      // Initialize player inventory and spawn player at the center of the map
      PLAYER_ENTITY.load_inventory("src/assets/maps/inventory.json");
      PLAYER_ENTITY.playerX = WORLD.width * 16;
      PLAYER_ENTITY.playerY = WORLD.height * 16;
      PLAYER_ENTITY.speed = 10.0f;
      break;

    } else if (choice == "2") {
      std::string worldname;
      std::cout << "Enter world name -> ";
      std::cin >> worldname;

      currentWorldFolder = "src/assets/maps/" + worldname;

      // Load existing map and building data from disk
      WORLD.load_world(currentWorldFolder.c_str());
      WORLD.load_buildings(currentWorldFolder.c_str());

      // Load player inventory and center player spawn position
      PLAYER_ENTITY.load_inventory("src/assets/maps/inventory.json");
      PLAYER_ENTITY.playerX = WORLD.width * 16;
      PLAYER_ENTITY.playerY = WORLD.height * 16;
      PLAYER_ENTITY.speed = 10.0f;
      break;
    }
  }

  // Main 60 FPS game loop
  while (!WindowShouldClose()) {
    ClearBackground(BLACK);
    BeginDrawing();

    float speed = PLAYER_ENTITY.speed;

    // 1. Save Shortcut (Ctrl + S): Persist world tiles and buildings to currentWorldFolder
    if ((IsKeyDown(KEY_LEFT_CONTROL) || IsKeyDown(KEY_RIGHT_CONTROL)) &&
        IsKeyPressed(KEY_S)) {
      WORLD.save_world(currentWorldFolder.c_str());
      WORLD.save_buildings(currentWorldFolder.c_str());
      TraceLog(LOG_INFO, "World saved successfully.");
    }

    // 2. Player Movement (WASD)
    if (IsKeyDown(KEY_W)) {
      PLAYER_ENTITY.playerY -= speed;
    }
    if (IsKeyDown(KEY_S)) {
      PLAYER_ENTITY.playerY += speed;
    }
    if (IsKeyDown(KEY_D)) {
      PLAYER_ENTITY.playerX += speed;
    }
    if (IsKeyDown(KEY_A)) {
      PLAYER_ENTITY.playerX -= speed;
    }

    //prevent player from going out of bounds
    PLAYER_ENTITY.playerY = std::clamp(PLAYER_ENTITY.playerY, GetScreenHeight()/2, WORLD.height);
    PLAYER_ENTITY.playerX = std::clamp(PLAYER_ENTITY.playerX, GetScreenWidth()/2, WORLD.width);

    //increase/decrease speed on scrolling mouse wheel
    if(GetMouseWheelMoveV().y != 0.0) {
        PLAYER_ENTITY.speed = std::max(PLAYER_ENTITY.speed + GetMouseWheelMoveV().y, 0.0f);
    }

    // 3. Build mode toggle
    if (IsKeyPressed(KEY_B)) {
      isBuildMode = !isBuildMode;
    }

    // 4. Render visible world tiles around the player
    draw_game(PLAYER_ENTITY.playerX, PLAYER_ENTITY.playerY);

    // 5. Render placement ghost and handle chest building if Build Mode is active
    if (isBuildMode) {
      placement1(GetMousePosition());
    }

    EndDrawing();
  }

  // Cleanup GPU resources and close Raylib context
  RENDERER.unload_assets();
  CloseWindow();
  return 0;
}
