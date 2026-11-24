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
 * @brief Scaffolds the world save directory structure and generates a Perlin-noise biome map file.
 * @param w The horizontal width of the new world in tiles.
 * @param h The vertical height of the new world in tiles.
 * @param world_file_path Root directory path where the world folders and files will be created.
 * @param seed Seed value for reproducible terrain generation (0 falls back to a random hardware seed).
 */
void world::create_world(int w, int h, const char *world_file_path, unsigned int seed) {
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

        // Update instance dimensions and allocate flattened map buffer
        width = w;
        height = h;
        worldvector.resize(width * height);

        // 3. Build permutation tables for Elevation and Temperature Perlin noise
        unsigned int baseSeed = (seed != 0) ? seed : rd();
        std::vector<int> pElev(256), pTemp(256);
        std::iota(pElev.begin(), pElev.end(), 0);
        std::iota(pTemp.begin(), pTemp.end(), 0);

        std::mt19937 genElev(baseSeed);
        std::mt19937 genTemp(baseSeed + 1337u); // Offset seed so temperature differs from elevation
        std::shuffle(pElev.begin(), pElev.end(), genElev);
        std::shuffle(pTemp.begin(), pTemp.end(), genTemp);

        // Duplicate tables to avoid index overflow during gradient lookups
        pElev.insert(pElev.end(), pElev.begin(), pElev.end());
        pTemp.insert(pTemp.end(), pTemp.begin(), pTemp.end());

        // Local lambda computing multi-octave 2D Perlin noise normalized to [0.0, 1.0]
        auto samplePerlin = [](const std::vector<int> &p, float x, float y, int octaves) -> float {
            auto fade = [](float t) { return t * t * t * (t * (t * 6.0f - 15.0f) + 10.0f); };
            auto lerp = [](float t, float a, float b) { return a + t * (b - a); };
            auto grad = [](int hash, float gx, float gy) {
                switch (hash & 7) {
                    case 0:  return  gx + gy;
                    case 1:  return -gx + gy;
                    case 2:  return  gx - gy;
                    case 3:  return -gx - gy;
                    case 4:  return  gx;
                    case 5:  return -gx;
                    case 6:  return  gy;
                    default: return -gy;
                }
            };

            float total = 0.0f;
            float frequency = 1.0f;
            float amplitude = 1.0f;
            float maxValue = 0.0f;

            for (int o = 0; o < octaves; o++) {
                float sx = x * frequency;
                float sy = y * frequency;

                int X = static_cast<int>(std::floor(sx)) & 255;
                int Y = static_cast<int>(std::floor(sy)) & 255;
                sx -= std::floor(sx);
                sy -= std::floor(sy);

                float u = fade(sx);
                float v = fade(sy);

                int A  = p[X] + Y;
                int AA = p[A];
                int AB = p[A + 1];
                int B  = p[X + 1] + Y;
                int BA = p[B];
                int BB = p[B + 1];

                float n = lerp(v,
                    lerp(u, grad(p[AA], sx, sy),        grad(p[BA], sx - 1.0f, sy)),
                    lerp(u, grad(p[AB], sx, sy - 1.0f), grad(p[BB], sx - 1.0f, sy - 1.0f))
                );

                total += n * amplitude;
                maxValue += amplitude;
                amplitude *= 0.5f;
                frequency *= 2.0f;
            }

            float normalized = (total / maxValue) * 0.5f + 0.5f;
            return std::clamp(normalized, 0.0f, 1.0f);
        };

        // 4. Write the binary map header (Width followed by Height)
        outFile.write(reinterpret_cast<const char *>(&w), sizeof(w));
        outFile.write(reinterpret_cast<const char *>(&h), sizeof(h));

        const float elevationScale = 0.02f;   // Controls continent/ocean size
        const float temperatureScale = 0.008f; // Controls climate zone size
        const float seaLevel = 0.35f;          // Elevation threshold separating ocean from land
        const float lapseRate = 0.90f;         // How strongly high elevation cools down temperature

        // 5. Generate tiles in row-major order (y outer, x inner) matching get_tileID(x, y)
        for (int y = 0; y < h; y++) {
            for (int x = 0; x < w; x++) {
                float elevation = samplePerlin(pElev, x * elevationScale, y * elevationScale, 5);
                float baseTemp  = samplePerlin(pTemp, x * temperatureScale, y * temperatureScale, 4);

                // Higher elevation reduces temperature; lowlands stay warm
                float altitude = std::max(0.0f, elevation - seaLevel);
                float temperature = std::clamp(baseTemp - (altitude * lapseRate), 0.0f, 1.0f);

                // Placeholder value-to-tileID mapping (ready to replace with your final function)
                uint8_t tileID = 0;
                if (elevation < seaLevel) {
                    tileID = 0; // Ocean / Water
                } else if (temperature < 0.20f) {
                    tileID = 1; // Ice / Snow peak biome (lowest temp / highest altitude)
                } else if (temperature < 0.40f) {
                    tileID = 2; // Tundra / Cold highland biome
                } else if (temperature < 0.65f) {
                    tileID = 3; // Temperate Grassland
                } else if (temperature < 0.80f) {
                    tileID = 4; // Savanna / Warm lowland plains
                } else {
                    tileID = 5; // Desert biome (highest temp & low elevation)
                }

                worldvector[y * width + x] = tileID;
                outFile.write(reinterpret_cast<const char *>(&tileID), sizeof(tileID));
            }
        }

        outFile.close();
        std::cout << "Perlin world (Seed: " << baseSeed << ") successfully generated at: " << world_file_path << std::endl;

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