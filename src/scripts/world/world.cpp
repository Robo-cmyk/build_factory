#include "world.h"
#include "../building/chest.h"
#include <cstdint>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <ostream>
#include <random>
#include <stdint.h>
#include <vector>

uint8_t zero = 0;
int zeroint = 0;

std::random_device rd;

std::mt19937 gen(rd());

std::uniform_int_distribution<> dis(0, 9);

void world::create_world(int w, int h, const char *world_file_path) {
  std::ofstream outFile(world_file_path, std::ios::binary);
  outFile.write(reinterpret_cast<const char *>(&w), sizeof(w));
  outFile.write(reinterpret_cast<const char *>(&h), sizeof(h));
  for (int i = 0; i < w; i++) {
    for (int j = 0; j < h; j++) {
      zero = dis(gen);
      outFile.write(reinterpret_cast<const char *>(&zero), 1);
    }
  }
}

void world::load_world(const char *world_file_path) {
  std::ifstream inFile(world_file_path, std::ios::binary);

  inFile.read(reinterpret_cast<char *>(&width), sizeof(width));
  inFile.read(reinterpret_cast<char *>(&height), sizeof(height));

  worldvector.resize(width * height);

  inFile.read(reinterpret_cast<char *>(worldvector.data()),
              worldvector.size() * sizeof(uint8_t));
}

uint8_t world::get_tileID(int x, int y) { return worldvector[x + y * width]; }

void world::set_tileID(int x, int y, uint8_t tileID) {
  if (x >= 0 && x < width && y >= 0 && y < height) {
    int index = y * width + x;
    if (index >= 0 && index < (int)worldvector.size()) {
      worldvector[index] = tileID;
    }
  }
}

void world::save_world(const char *filename) {
  std::ofstream worldfile(filename, std::ios::binary);
  worldfile.write(reinterpret_cast<char *>(&width), sizeof(width));
  worldfile.write(reinterpret_cast<char *>(&height), sizeof(height));
  if (worldfile.is_open()) {
    for (const auto &tile : worldvector) {
      worldfile.write(reinterpret_cast<const char *>(&tile), sizeof(tile));
    }
  }
}

void world::load_buildings(const char *folder) {
  try {
    for (const auto &entry : std::filesystem::directory_iterator(folder))
      if (entry.is_regular_file()) {
        if (entry.path().filename() == "chests.dat") {
          std::ifstream buildingfile(entry.path() , std::ios::binary);
          if (buildingfile.is_open()) {

            // LOOP: Keep reading as long as we haven't hit the End-Of-File
            // (EOF)
            while (buildingfile.peek() != EOF) {

              uint8_t chestTypeId = 0;
              int gridX = 0;
              int gridY = 0;

              // 1. Read the universal header data for this individual chest
              buildingfile.read(reinterpret_cast<char *>(&chestTypeId),
                                sizeof(chestTypeId));
              buildingfile.read(reinterpret_cast<char *>(&gridX),
                                sizeof(gridX));
              buildingfile.read(reinterpret_cast<char *>(&gridY),
                                sizeof(gridY));

              // 2. Determine slot size based on the Type ID we just read
              int totalSlots = (chestTypeId == 1) ? 5 : 15;

              // 3. Create a temporary staging chest in memory
              chest tempChest(gridX, gridY, totalSlots);

              // 4. INNER LOOP: Read the items and counts for THIS specific
              // chest
              for (int i = 0; i < totalSlots; i++) {
                int readItemId = 0;
                int readCount = 0;

                buildingfile.read(reinterpret_cast<char *>(&readItemId),
                                  sizeof(readItemId));
                buildingfile.read(reinterpret_cast<char *>(&readCount),
                                  sizeof(readCount));

                tempChest.slots[i].ItemID = readItemId;
                tempChest.slots[i].count = readCount;
              }

              // 5. Push the completed chest to your master vector
              chests.push_back(tempChest);
            }

            // Always clean up and close the file when the loop finishes!
            buildingfile.close();
          }
        }
      }
  } catch (const std::filesystem::filesystem_error &e) {
    std::cerr << "Error" << e.what() << std::endl;
  }
}

void world::save_buildings(const char *folder) {
  try {
    // 1. Ensure the save folder actually exists on the hard drive
    if (!std::filesystem::exists(folder)) {
      std::filesystem::create_directories(folder);
    }

    // ==========================================
    // SECTION 1: SAVE CHESTS
    // ==========================================
    std::string chestPath = std::string(folder) + "/chests.dat";
    std::ofstream chestFile(chestPath, std::ios::binary);

    if (chestFile.is_open()) {
      for (const auto &c : chests) {
        // Determine ChestTypeId: 1 for basic (5 slots), 2 for iron (15 slots)
        uint8_t chestTypeId = (c.slots.size() == 5) ? 1 : 2;

        // Write the chest header data
        chestFile.write(reinterpret_cast<const char *>(&chestTypeId),
                        sizeof(chestTypeId));
        chestFile.write(reinterpret_cast<const char *>(&c.x), sizeof(c.x));
        chestFile.write(reinterpret_cast<const char *>(&c.y), sizeof(c.y));
        // Write all the slots inside this chest
        for (size_t i = 0; i < c.slots.size(); i++) {
          chestFile.write(reinterpret_cast<const char *>(&c.slots[i].ItemID),
                          sizeof(c.slots[i].ItemID));
          chestFile.write(reinterpret_cast<const char *>(&c.slots[i].count),
                          sizeof(c.slots[i].count));
        }
      }
      chestFile.close();
      std::cout << "Successfully saved chests to " << chestPath << std::endl;
    } else {
      std::cerr << "Failed to open " << chestPath << " for writing!"
                << std::endl;
    }

  } catch (const std::filesystem::filesystem_error &e) {
    std::cerr << "Filesystem Error during save: " << e.what() << std::endl;
  }
}
