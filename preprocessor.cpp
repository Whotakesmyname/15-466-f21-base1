#include <cstdio>
#include <fstream>
#include <vector>
#include <string>

#include <glm/glm.hpp>

#include "load_save_png.hpp"
#include "PPU466.hpp"
#include "data_path.hpp"


int main() {
    std::printf("Starting preprocessing...\n");
    // Palette processing
    std::array< PPU466::Palette, 8 > palette_table;

    PPU466::Palette planet_palette = {
        glm::u8vec4(255, 255, 255, 0),  // transparent
        glm::u8vec4(169, 161, 163, 0xff),  // light grey
        glm::u8vec4(90, 92, 101, 0xff),  // dark grey
        glm::u8vec4(18, 23, 69, 0xff),  // blue
    };
    palette_table[2] = planet_palette;

    PPU466::Palette star_palette = {
        glm::u8vec4(255, 255, 255, 0),
        glm::u8vec4(255, 255, 255, 255),  // white
        glm::u8vec4(255, 0x72, 0, 255),  // orange
        glm::u8vec4(255, 0xfe, 0, 255),  // yellow
    };
    palette_table[3] = star_palette;

    PPU466::Palette spaceship_palette = {
        glm::u8vec4(255, 255, 255, 0),
        glm::u8vec4(0x34, 0x9a, 0xff, 0xff),  // blue
        glm::u8vec4(0xc2, 0xc2, 0xc2, 0xff),  // light grey
        glm::u8vec4(255, 255, 255, 0),
    };
    palette_table[1] = spaceship_palette;

    // Tile table processing
    std::array<PPU466::Tile, 16 * 16> data = {0};
    glm::uvec2 size;
    std::vector<glm::u8vec4> img;
    char namebuf[50];

    // spaceship
    {
        PPU466::Tile tile = {0};
        load_png(data_path("../assets/spaceship.png"), &size, &img, OriginLocation::LowerLeftOrigin);
        for (int i = 0; i < 8; ++i) {
            for (int j = 0; j < 8; ++j) {
                int index = 8 * i + j;
                switch(img[index].r) {
                    case 0x34:
                        tile.bit0[i] |= (uint8_t)1 << j;
                        tile.bit1[i] &= ~((uint8_t)1 << j);
                        break;
                    case 0xc2:
                        tile.bit0[i] &= ~((uint8_t)1 << j);
                        tile.bit1[i] |= (uint8_t)1 << j;
                        break;
                    default:
                        tile.bit0[i] |= (uint8_t)1 << j;
                        tile.bit1[i] |= (uint8_t)1 << j;
                }
            }
        }
        data[1] = tile;
    }

    // planet
    for (int i = 0; i < 144; ++i) {
        PPU466::Tile tile = {0};
        int x = i % 12, y = 12 - (i / 12);
        sprintf_s(namebuf, "../assets/earth/earth_%02d.png", i + 1);
        load_png(data_path(namebuf), &size, &img, OriginLocation::LowerLeftOrigin);
        for (int j = 0; j < 8; ++j) {  // row
            for (int k = 0; k < 8; ++k) {  // col
                int index = 8 * j + k;
                switch (img[index].r) {
                    case 255:  // transparent
                        tile.bit0[j] &= ~((uint8_t)1 << k);
                        tile.bit1[j] &= ~((uint8_t)1 << k);
                        break;
                    case 169:
                        tile.bit0[j] |= (uint8_t)1 << k;
                        tile.bit1[j] &= ~((uint8_t)1 << k);
                        break;
                    case 90:
                        tile.bit0[j] &= ~((uint8_t)1 << k);
                        tile.bit1[j] |= (uint8_t)1 << k;
                        break;
                    case 18:
                        tile.bit1[j] |= (uint8_t)1 << k;
                        tile.bit1[j] |= (uint8_t)1 << k;
                        break;
                    default:
                        assert(false && "Unreachable");
                }
            }
        }
        data[2+12*y+x] = tile;  // planet tiles start from 2
    }

    // star1
    {
        PPU466::Tile tile = {0};
        load_png(data_path("../assets/star1.png"), &size, &img, OriginLocation::LowerLeftOrigin);
        for (int i = 0; i < 8; ++i) {
            for (int j = 0; j < 8; ++j) {
                int index = 8 * i + j;
                switch(img[index].a) {
                    case 0:
                        tile.bit0[i] &= ~((uint8_t)1 << j);
                        tile.bit1[i] &= ~((uint8_t)1 << j);
                        break;
                    default:
                        tile.bit0[i] |= (uint8_t)1 << j;
                        tile.bit1[i] &= ~((uint8_t)1 << j);
                }
            }
        }
        data[200] = tile;
    }
    // star3
    {
        PPU466::Tile tile = {0};
        load_png(data_path("../assets/star3.png"), &size, &img, OriginLocation::LowerLeftOrigin);
        for (int i = 0; i < 8; ++i) {
            for (int j = 0; j < 8; ++j) {
                int index = 8 * i + j;
                switch(img[index].a) {
                    case 0:
                        tile.bit0[i] &= ~((uint8_t)1 << j);
                        tile.bit1[i] &= ~((uint8_t)1 << j);
                        break;
                    default:
                        tile.bit0[i] &= ~((uint8_t)1 << j);
                        tile.bit1[i] |= (uint8_t)1 << j;
                }
            }
        }
        data[201] = tile;
    }

    // Background 
    std::array< uint16_t, PPU466::BackgroundWidth * PPU466::BackgroundHeight > background = {0};
    uint16_t planet_i = 2;
    for (int i = 0; i < PPU466::BackgroundHeight; ++i) {
        for (int j = 0; j < PPU466::BackgroundWidth; ++j) {
            int calibred_y = i - PPU466::BackgroundHeight/4;
            int calibred_x = j - PPU466::BackgroundWidth/4;
            if (calibred_x >= -6 && calibred_x < 6 && calibred_y >= -6 && calibred_y < 6) {
                background[i*PPU466::BackgroundWidth+j] = ((uint16_t)2 << 8) | ((uint16_t)(planet_i++));
            } else if (((i % 3) == 0) && ((j % 3) == 2)) {
                background[i*PPU466::BackgroundWidth+j] = ((uint16_t)3 << 8) | ((uint16_t)(200));
            } else if (((i % 3) == 1) && ((j % 3) == 1)) {
                background[i*PPU466::BackgroundWidth+j] = ((uint16_t)3 << 8) | ((uint16_t)(201));
            }
        }
    }
    std::printf("used planet tile: %d\n", planet_i);

    // write to file
    std::ofstream(data_path("../assets/tile_table.bin"), std::ios::binary).write(reinterpret_cast<char*>(&data), sizeof(data));
    std::ofstream(data_path("../assets/palette_table.bin"), std::ios::binary).write(reinterpret_cast<char*>(&palette_table), sizeof(palette_table));
    std::ofstream(data_path("../assets/background.bin"), std::ios::binary).write(reinterpret_cast<char*>(&background), sizeof(background));

    std::printf("Finished!\n");

    return 0;
}