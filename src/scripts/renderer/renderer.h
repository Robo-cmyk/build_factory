
#pragma once
#include "raylib.h"
#include <cstdint>
#include <iostream>
#include <string>
#include <vector>

class renderer {
public:
  void draw_tile(int, int, uint8_t);
  void load_assets();
  void unload_assets();

public:
  std::vector<std::string> assetPaths = {
      "src/assets/nature/grass/grass_1.png",
      "src/assets/nature/grass/grass_2.png",
      "src/assets/nature/grass/grass_3.png",
      "src/assets/nature/grass/grass_4.png",
      "src/assets/nature/grass/grass_5.png",
      "src/assets/nature/grass/grass_6.png",
      "src/assets/nature/grass/grass_7.png",
      "src/assets/nature/grass/grass_8.png",
      "src/assets/nature/grass/grass_9.png",
      "src/assets/nature/grass/grass_10.png",
      "src/assets/chests/chest_iron1.png"
  }; // <--- FIX 1: Added closing brace and semicolon for the assetPaths vector initialization

  std::vector<Texture2D> textures;
}; // <--- FIX 2: Added closing brace and semicolon for the class renderer block

