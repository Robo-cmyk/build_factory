#include "../building/chest.h"
#include "../player/player.h"
#include "../renderer/renderer.h"
#include "../world/world.h"
#include "raylib.h"
#include <cstdint>
#include <random>
#include <string>
#include <set>

renderer RENDERER;
world WORLD;
player PLAYER_ENTITY;

Vector2 worldMousePos;

bool isBuildMode = false;

void placement1(Vector2 mouseScreen) {
  // 1. Match your draw_game camera exactly
  float topLeftWorldX = PLAYER_ENTITY.playerX - 400.0f;
  float topLeftWorldY = PLAYER_ENTITY.playerY - 240.0f;

  // 2. Get true world position
  float mouseWorldX = topLeftWorldX + mouseScreen.x;
  float mouseWorldY = topLeftWorldY + mouseScreen.y;

  // 3. Convert to grid space
  int gridX = (int)(mouseWorldX / 32.0f);
  int gridY = (int)(mouseWorldY / 32.0f);

  // Handle negative coordinates safely
  if (mouseWorldX < 0)
    gridX--;
  if (mouseWorldY < 0)
    gridY--;

  // 4. Convert back to screen position using your draw_game math
  float screenX = (gridX * 32.0f) - topLeftWorldX;
  float screenY = (gridY * 32.0f) - topLeftWorldY;

  // --- VISUAL DEBUGGER LOOP ---

  // 5. Bounds check against your WORLD dimensions
  if (gridX >= 0 && gridX < WORLD.width && gridY >= 0 && gridY < WORLD.height) {

    // TEST B: Draw the actual snapping green box
    DrawRectangle((int)screenX, (int)screenY, 32, 32, (Color){0, 0, 250, 150});
  }

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
    }
  }
}

void draw_game(float world_x, float world_y) {
  // 1. Calculate the raw screen-space top-left corner in world pixels.
  // This shifts our window view so the player is centered at (400, 240)
  float topLeftWorldX = world_x - 400.0f;
  float topLeftWorldY = world_y - 240.0f;

  // 2. Find the exact integer tile row/col where the screen view starts
  int startTileX = (int)(topLeftWorldX / 32.0f);
  int startTileY = (int)(topLeftWorldY / 32.0f);

  // Handle negative positions safely so the tiles don't glitch at the map
  // origin
  // Handle negative positions safely so the tiles don't glitch at the map
  // origin
  startTileX--;
  if (topLeftWorldY < 0)
    startTileY--;

  // 3. Loop through a fixed grid relative to your screen space.
  // 27 columns across (800 / 32 = 25 tiles + 2 extra for edge padding)
  // 17 rows down (480 / 32 = 15 tiles + 2 extra for edge padding)
  for (int col = 0; col < 27; col++) {
    for (int row = 0; row < 17; row++) {

      // Map the screen loop to the correct absolute tile index in your vector
      int targetTileX = startTileX + col;
      int targetTileY = startTileY + row;

      // 4. FIX Calculate the screen position directly from the absolute world
      // position This bypasses the actualX/actualY snap point entirely.
      float drawX = (targetTileX * 32.0f) - topLeftWorldX;
      float drawY = (targetTileY * 32.0f) - topLeftWorldY;

      // Simple bounds check for your 100x100 world vector
      if (targetTileX >= 0 && targetTileX < WORLD.width && targetTileY >= 0 &&
          targetTileY < WORLD.height) {

        int tileID = WORLD.get_tileID(targetTileX, targetTileY);

        // Draw using standard integer conversion at the very last step
        if (tileID != 11) {
          RENDERER.draw_tile((int)drawX, (int)drawY, tileID);
        } else {
          DrawRectangle((int)drawX, (int)drawY, 32, 32,
                        (Color){0, 0, 255, 100});
        }
      }
    }
  }
}

int main() {
  InitWindow(800, 480, "Game");
  SetTargetFPS(60);
  //  WORLD.create_world(100, 100, "../src/assets/maps/world.dat");
  RENDERER.load_assets();
  //  WORLD.create_world(50 , 50 ,"../src/assets/maps/world.dat");
  //  WORLD.create_world(50, 50, "../src/assets/maps/world.dat");
  std::string worldname = "";
  while (worldname != "1" && worldname != "2") {
    std::cout << "Choose (1) create_world (2) load_world -> ";

    std::cin >> worldname;
    if (worldname == "1") {
      std::cout << "Enter world name -> ";
      std::cin >> worldname;
      worldname = "../src/assets/maps/" + worldname;
      int w, h;
      std::cout << "Enter width should be between 100 to 10000 -> ";
      std::cin >> w;
      std::cout << "Enter height should be between 100 to 10000 -> ";
      std::cin >> h;
      if ((w >= 100 && w <= 10000) && (h >= 100 && h <= 10000)) {
        WORLD.create_world(w, h, (worldname).c_str());
      } else {
        WORLD.create_world(100, 100, (worldname).c_str());
      }
      WORLD.load_world(worldname.c_str());
      WORLD.load_buildings(worldname.c_str());
      PLAYER_ENTITY.load_inventory(worldname.c_str());
      PLAYER_ENTITY.playerX = WORLD.width * 16;
      PLAYER_ENTITY.playerY = WORLD.height * 16;
      PLAYER_ENTITY.speed = 10.0f;
      break;
    } else if (worldname == "2") {
      std::cout << "Enter world name -> ";
      std::cin >> worldname;
      worldname = "../src/assets/maps/" + worldname;
      WORLD.load_world((worldname).c_str());

      PLAYER_ENTITY.load_inventory(worldname.c_str());
      PLAYER_ENTITY.playerX = WORLD.width * 16;
      PLAYER_ENTITY.playerY = WORLD.height * 16;
      PLAYER_ENTITY.speed = 10.0f;
      break;
    }
  }
  while (!WindowShouldClose()) {
    ClearBackground(BLACK);
    BeginDrawing();

    // Keyboard handling/////////////////////////////////////////////////

    float speed = PLAYER_ENTITY.speed;

    // 1. Clean Save Check (Completely separate!)
    if ((IsKeyDown(KEY_LEFT_CONTROL) || IsKeyDown(KEY_RIGHT_CONTROL)) &&
        IsKeyPressed(KEY_S)) {
      WORLD.save_world(worldname.c_str());
      WORLD.save_buildings(worldname.c_str());
      TraceLog(LOG_INFO, "World saved successfully.");
    }

    // 2. Pure, Unrestricted Movement (No broken math conditions blocking you)
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

    // 3. Build mode toggle
    if (IsKeyPressed(KEY_B)) {
      isBuildMode = !isBuildMode;
    } ////////////////////////////////////////////////////////////////////
    /////////////////////////////////////////////////////////////////////

    // Draw tiles on screen/////////////////
    draw_game(PLAYER_ENTITY.playerX, PLAYER_ENTITY.playerY);

    //////////////////////////////////////

    // Build mode
    if (isBuildMode) {
      placement1(GetMousePosition());
    }


    EndDrawing();
  }

  RENDERER.unload_assets();

  CloseWindow();
  return 0;
}
