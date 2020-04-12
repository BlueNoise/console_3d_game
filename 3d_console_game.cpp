// 3d_console_game.cpp
//

/* 
Screen coordinates frame of reference:
-------------> x+ (width)
|
|
|
|
y+
(height)

*/
#include <iostream>
#include <string>
#include <chrono>

#include <Windows.h>

std::wstring CreateGameMap()
{
	std::wstring game_map;
	/*
		Map coordinates frame of reference :
		------------->y + (height)
		|
		|
		|
		|
		x +
		(width)
	*/
	game_map += L"################";
	game_map += L"#..............#";
	game_map += L"#..............#";
	game_map += L"#..............#";
	game_map += L"#..............#";
	game_map += L"#..............#";
	game_map += L"#..............#";
	game_map += L"#..............#";
	game_map += L"#..............#";
	game_map += L"#..............#";
	game_map += L"#..............#";
	game_map += L"#..............#";
	game_map += L"#..............#";
	game_map += L"#..........#...#";
	game_map += L"#..............#";
	game_map += L"################";

	return game_map;
}

int main()
{
	// screen info
	constexpr std::size_t screen_width = 120;
	constexpr std::size_t screen_height = 40;

	// player info
	float player_x = 13.0f;
	float player_y = 6.0f;
	float player_angle = 3.1416f/2.f;
	const float walking_speed = 1.f;
	const float rotation_speed = .5f;
	const float depth = 16.f;
	constexpr float player_fov = 3.1416f / 4.0f;

	wchar_t *screen_buffer = new wchar_t[screen_height * screen_width];
	HANDLE handle_console = CreateConsoleScreenBuffer(GENERIC_READ | GENERIC_WRITE, 0, NULL, CONSOLE_TEXTMODE_BUFFER, NULL);
	SetConsoleActiveScreenBuffer(handle_console);

	// game map info
	constexpr std::size_t map_width = 16; // along X axis
	constexpr std::size_t map_height = 16; // along Y axis
	const std::wstring game_map = CreateGameMap();	

	auto tp1 = std::chrono::high_resolution_clock::now();
	auto tp2 = std::chrono::high_resolution_clock::now();
	auto elapsed_time = std::chrono::duration_cast<std::chrono::seconds>(tp2 - tp1);
	float delta_time = 0; // in seconds

	// Game loop
	while (true)
	{
		std::chrono::duration<float> elapsed_time = tp2 - tp1;
		delta_time = elapsed_time.count();
		tp1 = tp2;
		
		if (GetAsyncKeyState((unsigned short)'A') & 0x8000)
			player_angle += rotation_speed * delta_time;

		if (GetAsyncKeyState((unsigned short)'D') & 0x8000)
			player_angle -= rotation_speed * delta_time;

		if (GetAsyncKeyState((unsigned short)'W') & 0x8000)
		{
			player_x += walking_speed * delta_time * cos(player_angle);
			player_y += walking_speed * delta_time * sin(player_angle);
		}
		if (GetAsyncKeyState((unsigned short)'S') & 0x8000)
		{
			player_x -= walking_speed * delta_time * cos(player_angle);
			player_y -= walking_speed * delta_time * sin(player_angle);
		}

		// ray casting logic --  traverse each column of the screen
		for (std::size_t x = 0; x < screen_width; x++) 
		{
			// divide the ray angles with small increments using the screen width resolution
			float ray_angle = (player_angle - player_fov / 2) + ((float)x / (float)screen_width) * player_fov;
			float distance_to_wall = 0;
			bool wall_hit = false;
			
			const float ray_length_increment = 0.1f;
			float ray_length = 0;
			float ray_x = 0.f;
			float ray_y = 0.f;

			std::size_t i = 0;
			while (!wall_hit && ray_length < depth)
			{
				const std::size_t map_x_idx = int(player_x + ray_length * cos(ray_angle)); // in X direction
				const std::size_t map_y_idx = int(player_y + ray_length * sin(ray_angle)); // in Y direction
				
				ray_length += ray_length_increment;

				// check if the ray lands outside the map, and if yes, assume a wall hit
				if( map_x_idx < 0 || map_x_idx > map_width || map_y_idx < 0 || map_y_idx > map_height)
				{
					wall_hit = true;
					ray_length = depth; // assume maximum distance to wall when that happens
				}
				else 
				{
					if (game_map[map_y_idx + map_x_idx * map_height] == '#')
					{
						wall_hit = true;
					}
				}
			}

			// Key part here!!! Calculate the distance to ceiling and floor in screen coordinates
			const std::size_t ceiling_screen_y_idx = (float)(screen_height / 2.0) - screen_height / (float)ray_length;
			const std::size_t floor_screen_y_idx = screen_height - ceiling_screen_y_idx;

			// apply gradual shading depending on distance to wall
			short shade = ' ';
			if (ray_length < depth / 4.f) shade = 0x2588;  // closest (boldest)
			else if (ray_length < depth / 3.f) shade = 0x2593;
			else if (ray_length < depth / 2.f) shade = 0x2592; 
			else if (ray_length < depth ) shade = 0x2591; // farthest (light coloring)
			else shade = ' ';

			// go through each pixels of that column (x) and determine the coloring based on distance
			for (std::size_t y = 0; y < screen_height; y++) 
			{
				if (y < ceiling_screen_y_idx) // it is the ceiling
					screen_buffer[y*screen_width + screen_width - x - 1] = L' ';
				else if (y > ceiling_screen_y_idx && y < floor_screen_y_idx) // its in a wall
					screen_buffer[y*screen_width + screen_width - x - 1] = shade;
				else
					screen_buffer[y*screen_width + screen_width - x - 1] = L' ';
			}
		}
		DWORD bytes_written = 0;
		const _COORD coordinates = { 0,0 };
		WriteConsoleOutputCharacter(handle_console,screen_buffer, screen_width*screen_height, coordinates, &bytes_written);
		tp2 = std::chrono::high_resolution_clock::now();
		
	}

	return 0;
}