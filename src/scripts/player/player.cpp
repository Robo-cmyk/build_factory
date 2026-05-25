#include "player.h"
#include "../nlohmanjson/json.hpp"
#include <fstream>

using json = nlohmann::json;

void player::save_inventory(const char* filename){
  json j;
  j["inventory"] = json::array();

  for(const auto& item : inv.items){
      j["inventory"].push_back({
          {"item_id" , item.itemID},
          {"name" , item.name},
          {"count" , item.count},
          {"slot" , item.invslot}
          });
  }

  std::ofstream file(filename);
  if(file.is_open()) {
    file << j.dump(4);
  }
}

void player::load_inventory(const char* filename){
  std::ifstream file(filename);
  if(!file.is_open()){
    inv.items = {
      {0 , "log" , 10 , 0},
      {1 , "axe" , 1 , 1}
    };
    save_inventory(filename);
  }

  json j;
  file >> j;
  inv.items.clear();
  for(const auto& element : j["inventory"] ){
    InventoryItem item;
    item.itemID = element["item_id"];
    item.name = element["name"];
    item.count = element["count"];
    item.invslot = element["slot"];
    inv.items.push_back(item);
  }
}
