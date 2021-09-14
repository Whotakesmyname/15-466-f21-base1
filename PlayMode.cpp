#include "PlayMode.hpp"

//for the GL_ERRORS() macro:
#include "gl_errors.hpp"

//for glm::value_ptr() :
#include <glm/gtc/type_ptr.hpp>

#include <random>
#include <fstream>

#include "Load.hpp"
#include "data_path.hpp"


std::array<PPU466::Tile, 256Ui64> tile_table;
Load<std::array<PPU466::Tile, 256Ui64>> tile_table_ptr(LoadTag::LoadTagEarly, []() {
	std::ifstream infile;
	infile.open(data_path("../assets/tile_table.bin"), std::ios::binary);
	infile.read(reinterpret_cast<char *>(tile_table.data()), sizeof(tile_table));
	return &tile_table;
});

std::array< PPU466::Palette, 8 > palette_table;
Load<std::array< PPU466::Palette, 8 >> palette_table_ptr(LoadTag::LoadTagEarly, []() {
	std::ifstream infile;
	infile.open(data_path("../assets/palette_table.bin"), std::ios::binary);
	infile.read(reinterpret_cast<char *>(palette_table.data()), sizeof(palette_table));
	return &palette_table;
});

std::array< uint16_t, PPU466::BackgroundWidth * PPU466::BackgroundHeight > background;
Load<decltype(background)> background_ptr(LoadTag::LoadTagEarly, []() {
	std::ifstream infile;
	infile.open(data_path("../assets/background.bin"), std::ios::binary);
	infile.read(reinterpret_cast<char *>(background.data()), sizeof(background));
	return &background;
});

PlayMode::PlayMode() {
	//TODO:
	// you *must* use an asset pipeline of some sort to generate tiles.
	// don't hardcode them like this!
	// or, at least, if you do hardcode them like this,
	//  make yourself a script that spits out the code that you paste in here
	//   and check that script into your repository.

	//Also, *don't* use these tiles in your game:

	ppu.palette_table = palette_table;
	ppu.tile_table = tile_table;
	ppu.background = background;

	// { //use tiles 0-16 as some weird dot pattern thing:
	// 	std::array< uint8_t, 8*8 > distance;
	// 	for (uint32_t y = 0; y < 8; ++y) {
	// 		for (uint32_t x = 0; x < 8; ++x) {
	// 			float d = glm::length(glm::vec2((x + 0.5f) - 4.0f, (y + 0.5f) - 4.0f));
	// 			d /= glm::length(glm::vec2(4.0f, 4.0f));
	// 			distance[x+8*y] = std::max(0,std::min(255,int32_t( 255.0f * d )));
	// 		}
	// 	}
	// 	for (uint32_t index = 0; index < 16; ++index) {
	// 		PPU466::Tile tile;
	// 		uint8_t t = (255 * index) / 16;
	// 		for (uint32_t y = 0; y < 8; ++y) {
	// 			uint8_t bit0 = 0;
	// 			uint8_t bit1 = 0;
	// 			for (uint32_t x = 0; x < 8; ++x) {
	// 				uint8_t d = distance[x+8*y];
	// 				if (d > t) {
	// 					bit0 |= (1 << x);
	// 				} else {
	// 					bit1 |= (1 << x);
	// 				}
	// 			}
	// 			tile.bit0[y] = bit0;
	// 			tile.bit1[y] = bit1;
	// 		}
	// 		ppu.tile_table[index] = tile;
	// 	}
	// }

	//use sprite 32 as a "player":
	// ppu.tile_table[32].bit0 = {
	// 	0b01111110,
	// 	0b11111111,
	// 	0b11111111,
	// 	0b11111111,
	// 	0b11111111,
	// 	0b11111111,
	// 	0b11111111,
	// 	0b01111110,
	// };
	// ppu.tile_table[32].bit1 = {
	// 	0b00000000,
	// 	0b00000000,
	// 	0b00011000,
	// 	0b00100100,
	// 	0b00000000,
	// 	0b00100100,
	// 	0b00000000,
	// 	0b00000000,
	// };

	// //makes the outside of tiles 0-16 solid:
	// ppu.palette_table[0] = {
	// 	glm::u8vec4(0x00, 0x00, 0x00, 0x00),
	// 	glm::u8vec4(0x00, 0x00, 0x00, 0xff),
	// 	glm::u8vec4(0x00, 0x00, 0x00, 0x00),
	// 	glm::u8vec4(0x00, 0x00, 0x00, 0xff),
	// };

	// //makes the center of tiles 0-16 solid:
	// ppu.palette_table[1] = {
	// 	glm::u8vec4(0x00, 0x00, 0x00, 0x00),
	// 	glm::u8vec4(0x00, 0x00, 0x00, 0x00),
	// 	glm::u8vec4(0x00, 0x00, 0x00, 0xff),
	// 	glm::u8vec4(0x00, 0x00, 0x00, 0xff),
	// };

	// //used for the player:
	// ppu.palette_table[7] = {
	// 	glm::u8vec4(0x00, 0x00, 0x00, 0x00),
	// 	glm::u8vec4(0xff, 0xff, 0x00, 0xff),
	// 	glm::u8vec4(0x00, 0x00, 0x00, 0xff),
	// 	glm::u8vec4(0x00, 0x00, 0x00, 0xff),
	// };

	// //used for the misc other sprites:
	// ppu.palette_table[6] = {
	// 	glm::u8vec4(0x00, 0x00, 0x00, 0x00),
	// 	glm::u8vec4(0x88, 0x88, 0xff, 0xff),
	// 	glm::u8vec4(0x00, 0x00, 0x00, 0xff),
	// 	glm::u8vec4(0x00, 0x00, 0x00, 0x00),
	// };

}

PlayMode::~PlayMode() {
}

bool PlayMode::handle_event(SDL_Event const &evt, glm::uvec2 const &window_size) {

	if (evt.type == SDL_KEYDOWN) {
		if (evt.key.keysym.sym == SDLK_LEFT) {
			left.downs += 1;
			left.pressed = true;
			return true;
		} else if (evt.key.keysym.sym == SDLK_RIGHT) {
			right.downs += 1;
			right.pressed = true;
			return true;
		} else if (evt.key.keysym.sym == SDLK_UP) {
			up.downs += 1;
			up.pressed = true;
			return true;
		} else if (evt.key.keysym.sym == SDLK_DOWN) {
			down.downs += 1;
			down.pressed = true;
			return true;
		}
	} else if (evt.type == SDL_KEYUP) {
		if (evt.key.keysym.sym == SDLK_LEFT) {
			left.pressed = false;
			return true;
		} else if (evt.key.keysym.sym == SDLK_RIGHT) {
			right.pressed = false;
			return true;
		} else if (evt.key.keysym.sym == SDLK_UP) {
			up.pressed = false;
			return true;
		} else if (evt.key.keysym.sym == SDLK_DOWN) {
			down.pressed = false;
			return true;
		}
	}

	return false;
}

void PlayMode::update(float elapsed) {

	if (left.pressed) speed.x -= accl_rate * elapsed;
	if (right.pressed) speed.x += accl_rate * elapsed;
	if (down.pressed) speed.y -= accl_rate * elapsed;
	if (up.pressed) speed.y += accl_rate * elapsed;

	const auto planet_pos = glm::vec2(PPU466::ScreenWidth/2.f, PPU466::ScreenHeight/2.f);

	auto dis_diff = planet_pos - player_at;
	speed += gravity / dis_diff * elapsed;

	// position update
	player_at += speed * elapsed;
	if (player_at.x > PPU466::ScreenWidth) {
		player_at.x -= PPU466::ScreenWidth;
	}
	if (player_at.x < 0) {
		player_at.x += PPU466::ScreenWidth;
	}
	if (player_at.y > PPU466::ScreenHeight) {
		player_at.y -= PPU466::ScreenHeight;
	}
	if (player_at.y < 0) {
		player_at.y += PPU466::ScreenHeight;
	}
	
	if (glm::length(dis_diff) < 48) {
		// crash
		player_at = glm::vec2(0.f, 0.f);
		speed = glm::vec2(0.f, 0.f);
	}

	gravity = (rand() / RAND_MAX) * 10 + 25.f;

	//reset button press counters:
	left.downs = 0;
	right.downs = 0;
	up.downs = 0;
	down.downs = 0;
}

void PlayMode::draw(glm::uvec2 const &drawable_size) {
	//--- set ppu state based on game state ---


	//player sprite:
	ppu.sprites[0].x = int32_t(player_at.x);
	ppu.sprites[0].y = int32_t(player_at.y);
	ppu.sprites[0].index = 1;
	ppu.sprites[0].attributes = 1;

	//some other misc sprites:
	for (uint32_t i = 1; i < 63; ++i) {
		ppu.sprites[i].index = 0;
		ppu.sprites[i].attributes = 6;
		ppu.sprites[i].attributes |= 0x80; //'behind' bit
	}

	//--- actually draw ---
	ppu.draw(drawable_size);
}
