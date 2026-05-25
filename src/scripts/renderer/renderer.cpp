#include <iostream>
#include "raylib.h"
#include <cstdint>
#include <string>
#include <vector>
#include "renderer.h"


Texture2D tex;

void renderer::load_assets(){
  for(const auto& path : assetPaths) {
    tex = LoadTexture(path.c_str());
    textures.push_back(tex);
  }
}

void renderer::draw_tile(int x , int y , uint8_t tileID){
  DrawTexture(textures[tileID] , x , y , WHITE);
}

void renderer::unload_assets(){
  for(const auto& tex : textures){
    UnloadTexture(tex);
  }
}
