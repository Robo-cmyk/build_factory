/**
 * @file world.cpp
 * @brief Implements the world class methods for procedural map generation, binary serialization,
 *        coordinate-to-tile lookups, and persistent storage of world buildings/chests.
 *
 * This implementation handles directory scaffolding (`/map`, `/building`, `/player`) and binary 
 * file I/O for saving and loading game states. Tile grids are flattened into a 1D vector using 
 * row-major order (`y * width + x`), while buildings like chests are serialized with dynamic 
 * slot layouts based on their container type ID.
 */

#include "world.h"
#include "../building/chest.h"
#include <cstdint>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <random>
#include <vector>

// Global/file-scope RNG components for procedural tile generation
uint8_t zero = 0;                          ///< Temporary buffer holding generated tile IDs during creation
int zeroint = 0;                           ///< Unused placeholder integer
std::random_device rd;                     ///< Hardware entropy source for seeding the RNG
std::mt19937 gen(rd());                    ///< Mersenne Twister pseudo-random number generator
std::uniform_int_distribution<> dis(0, 9); ///< Uniform distribution generating tile IDs from 0 to 9

/**
 * @func world::create_world
 * @brief Scaffolds the world save directory structure and generates a randomized binary map file.
 * @param w The horizontal width of the new world in tiles.
 * @param h The vertical height of the new world in tiles.
 * @param world_file_path Root directory path where the world folders and files will be created.
 */
void world::create_world(int w, int h, const char *world_file_path) {
    try {
        // 1. Convert the root path to a string and build required subdirectories
        std::string rootStr(world_file_path);
        std::string mapDir = rootStr + "/map";
        std::string buildingDir = rootStr + "/building";
        std::string playerDir = rootStr + "/player";

        std::filesystem::create_directories(mapDir);
        std::filesystem::create_directories(buildingDir);
        std::filesystem::create_directories(playerDir);

        // 2. Open the binary map file inside the /map subfolder
        std::string mapFilePath = mapDir + "/world.dat";
        std::ofstream outFile(mapFilePath, std::ios::binary);

        if (!outFile.is_open()) {
            std::cerr << "Error: Could not create world.dat at " << mapFilePath << std::endl;
            return;
        }

        // Update instance dimensions immediately
        width = w;
        height = h;

        // 3. Write the binary map header (Width followed by Height)
        outFile.write(reinterpret_cast<const char *>(&w), sizeof(w));
        outFile.write(reinterpret_cast<const char *>(&h), sizeof(h));

        // 4. Generate and write randomized tile IDs sequentially (w * h bytes total)
        for (int i = 0; i < w; i++) {
            for (int j = 0; j < h; j++) {
                zero = dis(gen); // Generate a random tile ID between 0 and 9
                outFile.write(reinterpret_cast<const char *>(&zero), 1); 
            }
        }

        outFile.close();
        std::cout << "World structure successfully generated at: " << world_file_path << std::endl;

    } catch (const std::filesystem::filesystem_error& e) {
        std::cerr << "Filesystem setup failed: " << e.what() << std::endl;
    }
}

/**
 * @func world::load_world
 * @brief Reads map dimensions and tile data from `world.dat` into memory.
 * @param world_file_path Root directory path of the saved world.
 */
void world::load_world(const char *world_file_path) {
    // Construct path to the binary map file
    std::string mapFilePath = std::string(world_file_path) + "/map/world.dat";
    std::ifstream inFile(mapFilePath, std::ios::binary);

    if (!inFile.is_open()) {
        std::cerr << "Error: Map file missing at " << mapFilePath << std::endl;
        return;
    }

    // Read the width and height header from the start of the file
    inFile.read(reinterpret_cast<char *>(&width), sizeof(width));
    inFile.read(reinterpret_cast<char *>(&height), sizeof(height));

    // Resize the flattened 1D vector to fit all 2D grid tiles
    worldvector.resize(width * height);

    // Bulk-read the entire tile grid directly into the vector's underlying memory
    inFile.read(reinterpret_cast<char *>(worldvector.data()),
                worldvector.size() * sizeof(uint8_t));

    inFile.close();
    std::cout << "Successfully loaded map layout! Size: " << width << "x" << height << std::endl;
}

/**
 * @func world::get_tileID
 * @brief Fetches the tile ID at a given (x, y) grid coordinate.
 * @param x The X-coordinate (column) of the tile.
 * @param y The Y-coordinate (row) of the tile.
 * @return uint8_t The tile ID stored at the flattened index `x + y * width`.
 */
uint8_t world::get_tileID(int x, int y) { 
    // Convert 2D coordinates to 1D row-major index (Note: assumes coordinates are within bounds)
    return worldvector[x + y * width]; 
}

/**
 * @func world::set_tileID
 * @brief Safely updates the tile ID at the specified (x, y) coordinate after bounds checking.
 * @param x The X-coordinate (column) of the tile to update.
 * @param y The Y-coordinate (row) of the tile to update.
 * @param tileID The new tile ID value to assign.
 */
void world::set_tileID(int x, int y, uint8_t tileID) {
    // Ensure coordinates fall within valid grid dimensions
    if (x >= 0 && x < width && y >= 0 && y < height) {
        int index = y * width + x; // Convert 2D coordinate to 1D flattened index
        if (index >= 0 && index < (int)worldvector.size()) {
            worldvector[index] = tileID;
        }
    }
}

/**
 * @func world::save_world
 * @brief Serializes the current world dimensions and tile vector to `world.dat`.
 * @param filename Root directory path of the world save folder.
 */
void world::save_world(const char *filename) {
    std::ofstream worldfile(std::string(filename) + "/map/world.dat", std::ios::binary);
    if (worldfile.is_open()) {
        // Write map dimension headers
        worldfile.write(reinterpret_cast<char *>(&width), sizeof(width));
        worldfile.write(reinterpret_cast<char *>(&height), sizeof(height));
        
        // Write every tile ID sequentially to disk
        for (const auto &tile : worldvector) {
            worldfile.write(reinterpret_cast<const char *>(&tile), sizeof(tile));
        }
        worldfile.close();
    }
}

/**
 * @func world::load_buildings
 * @brief Scans the `/building` directory and deserializes container data (such as `chests.dat`).
 * @param folder Root directory path of the saved world.
 */
void world::load_buildings(const char *folder) {
    try {
        // Iterate through all files inside the /building subdirectory
        for (const auto &entry : std::filesystem::directory_iterator(std::string(folder) + "/building")) {
            if (entry.is_regular_file()) {
                if (entry.path().filename() == "chests.dat") {
                    std::ifstream buildingfile(entry.path(), std::ios::binary);
                    if (buildingfile.is_open()) {

                        // Read sequential chest records until End-Of-File (EOF) is reached
                        while (buildingfile.peek() != EOF) {
                            uint8_t chestTypeId = 0;
                            int gridX = 0;
                            int gridY = 0;

                            // 1. Read the chest header (Type ID and grid coordinates)
                            buildingfile.read(reinterpret_cast<char *>(&chestTypeId), sizeof(chestTypeId));
                            buildingfile.read(reinterpret_cast<char *>(&gridX), sizeof(gridX));
                            buildingfile.read(reinterpret_cast<char *>(&gridY), sizeof(gridY));

                            // 2. Determine inventory capacity based on Type ID (1 = basic 5-slot, else 15-slot)
                            int totalSlots = (chestTypeId == 1) ? 5 : 15;

                            // 3. Instantiate a temporary chest object at the loaded coordinates
                            chest tempChest(gridX, gridY, totalSlots);

                            // 4. Populate each inventory slot with its saved ItemID and stack count
                            for (int i = 0; i < totalSlots; i++) {
                                int readItemId = 0;
                                int readCount = 0;

                                buildingfile.read(reinterpret_cast<char *>(&readItemId), sizeof(readItemId));
                                buildingfile.read(reinterpret_cast<char *>(&readCount), sizeof(readCount));

                                tempChest.slots[i].ItemID = readItemId;
                                tempChest.slots[i].count = readCount;
                            }

                            // 5. Append the reconstructed chest to the world's active chest list
                            chests.push_back(tempChest);
                        }

                        buildingfile.close();
                    }
                }
            }
        }
    } catch (const std::filesystem::filesystem_error &e) {
        std::cerr << "Error: " << e.what() << std::endl;
    }
}

/**
 * @func world::save_buildings
 * @brief Serializes all active chests and their slot contents to `/building/chests.dat`.
 * @param folder Root directory path of the saved world.
 */
void world::save_buildings(const char *folder) {
    try {
        // 1. Ensure the /building subdirectory exists before writing
        std::string buildingDir = std::string(folder) + "/building";
        if (!std::filesystem::exists(buildingDir)) {
            std::filesystem::create_directories(buildingDir);
        }

        // ==========================================
        // SECTION 1: SAVE CHESTS
        // ==========================================
        std::string chestPath = buildingDir + "/chests.dat";
        std::ofstream chestFile(chestPath, std::ios::binary);

        if (chestFile.is_open()) {
            for (const auto &c : chests) {
                // Encode chest capacity as a Type ID: 1 for basic (5 slots), 2 for iron (15 slots)
                uint8_t chestTypeId = (c.slots.size() == 5) ? 1 : 2;

                // Write chest metadata header
                chestFile.write(reinterpret_cast<const char *>(&chestTypeId), sizeof(chestTypeId));
                chestFile.write(reinterpret_cast<const char *>(&c.x), sizeof(c.x));
                chestFile.write(reinterpret_cast<const char *>(&c.y), sizeof(c.y));

                // Serialize every slot's item ID and item count sequentially
                for (size_t i = 0; i < c.slots.size(); i++) {
                    chestFile.write(reinterpret_cast<const char *>(&c.slots[i].ItemID), sizeof(c.slots[i].ItemID));
                    chestFile.write(reinterpret_cast<const char *>(&c.slots[i].count), sizeof(c.slots[i].count));
                }
            }
            chestFile.close();
            std::cout << "Successfully saved chests to " << chestPath << std::endl;
        } else {
            std::cerr << "Failed to open " << chestPath << " for writing!" << std::endl;
        }

    } catch (const std::filesystem::filesystem_error &e) {
        std::cerr << "Filesystem Error during save: " << e.what() << std::endl;
    }
}