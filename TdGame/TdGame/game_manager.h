#ifndef GAME_MANAGER_H_
#define GAME_MANAGER_H_
#include "manager.h"
#include"config_manager.h"
#include"resources_manager.h"
#include"enemy_manager.h"
#include"wave_manager.h"
#include"bullet_manager.h"
#include"tower_manager.h"
#include"status_bar.h"
#include"panel.h"
#include"place_panel.h"
#include"upgrade_panel.h"
#include"player_manager.h"
#include"banner.h"

#include<SDL.h>
#include<SDL_ttf.h>
#include<SDL_image.h>
#include<SDL_mixer.h>
class GameManager :public Manager<GameManager>
{
	friend class Manager<GameManager>;
public:
	int run(int argc, char** argv)
	{
		

		Mix_FadeInMusic(ResourcesManager::instance()->get_music_pool().find(ResID::Music_BGM)->second, -1, 1500);

		Uint64 last_counter = SDL_GetPerformanceCounter();
		const Uint64 counter_freq = SDL_GetPerformanceFrequency();

		while (!is_quit)
		{
			while (SDL_PollEvent(&event))
				on_input();

			Uint64 current_counter = SDL_GetPerformanceCounter();
			double delta = (double)(current_counter - last_counter) / counter_freq;
			last_counter = current_counter;
			if (delta * 1000 < 1000.0 / 60)
			{
				SDL_Delay((Uint32)(1000.0 / 60 - delta * 1000));
			}

			on_update(delta);

			SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
			SDL_RenderClear(renderer);

			on_renderer();

			SDL_RenderPresent(renderer);

		}
		return 0;
	}
protected:
	GameManager()
	{
		init_assert(!SDL_Init(SDL_INIT_EVERYTHING), "SDL初始化失败");
		init_assert(IMG_Init(IMG_INIT_JPG | IMG_INIT_PNG), "SDLIMG初始化失败");
		init_assert(Mix_Init(MIX_INIT_MP3), "SDL_mixer初始化失败");
		init_assert(!TTF_Init(), "TTF初始化失败");
		Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 2048);
		SDL_SetHint(SDL_HINT_IME_SHOW_UI, "1");

		ConfigManager* config = ConfigManager::instance();

		init_assert(config->load_game_config("config.json"), "加载游戏配置失败");

		window = SDL_CreateWindow(config->basic_template.window_title.c_str(), SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
			config->basic_template.window_width, config->basic_template.window_height, SDL_WINDOW_SHOWN);
		init_assert(window, "初始化失败");
		//第二种方式是直接不同的是，我们是先渲染背景颜色填充，再渲染
		renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC | SDL_RENDERER_TARGETTEXTURE);
		init_assert(renderer, "渲染失败");
		init_assert(ResourcesManager::instance()->load_from_file(renderer), "加载游戏资源失败");
		load_stage();

		status_bar.set_position(15, 15);
		
		place_panel = new PlacePanel();
		banner = new Banner();

	}

	~GameManager()
	{
		tower_actions.close();
		SDL_DestroyRenderer(renderer);
		SDL_DestroyWindow(window);

		TTF_Quit();
		Mix_Quit();
		IMG_Quit();
		SDL_Quit();
	}
private:
	struct Stage
	{
		const char* name;
		const char* map;
		const char* waves;
	};
	static constexpr int stage_count = 3;
	const Stage stages[stage_count] = {
		{ u8"村口弯道", "map.csv", "level.json" },
		{ u8"双路会合", "map2.csv", "level2.json" },
		{ u8"两翼围城", "map3.csv", "level3.json" }
	};
	int stage_index = 0;
	int displayed_wave = 0;
	bool result_shown = false;
	bool is_quit = false;
	SDL_Window* window = nullptr;
	SDL_Renderer* renderer = nullptr;
	SDL_Event event;
	SDL_Texture* tex_tile_map = nullptr;
	
	StatusBar status_bar;

	Panel* place_panel = nullptr;
	TowerActionPanel tower_actions;
	Banner* banner = nullptr;

private:
	// 替换地图和波次后重建底图；关卡名与当前波次显示在窗口标题中。
	void load_stage()
	{
		const Stage& stage = stages[stage_index];
		auto* config = ConfigManager::instance();
		init_assert(config->map.load(stage.map), stage.map);
		init_assert(config->load_level_config(stage.waves), stage.waves);
		SDL_DestroyTexture(tex_tile_map);
		init_assert(generate_tile_map_texture(), "加载图片贴图失败");
		displayed_wave = 0;
		update_stage_title();
	}

	void update_stage_title()
	{
		int wave = WaveManager::instance()->wave_number();
		if (wave == displayed_wave) return;
		displayed_wave = wave;
		auto* config = ConfigManager::instance();
		std::string title = config->basic_template.window_title + u8"  第" + std::to_string(stage_index + 1)
			+ u8"/3关 · " + stages[stage_index].name + u8"  第" + std::to_string(wave)
			+ "/" + std::to_string(config->wave_list.size()) + u8"波  |  左键点塔升级，右键点塔撤回";
		SDL_SetWindowTitle(window, title.c_str());
	}

	// 胜利横幅结束后换图。先回收持有旧路线的实体，再重置本关资源与输入状态。
	void advance_stage()
	{
		WaveManager::release();
		BulletManager::release();
		TowerManager::release();
		EnemyManager::release();
		CoinManager::release();
		HomeManager::release();
		PlayerManager::release();
		delete place_panel;
		tower_actions.close();
		place_panel = new PlacePanel();
		auto* config = ConfigManager::instance();
		config->is_game_over = false;
		config->is_game_win = false;
		result_shown = false;
		++stage_index;
		load_stage();
		status_bar.on_update(renderer);
	}

	void init_assert(bool flag, const char* err_msg)
	{
		if (flag)return;
		SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, "游戏初始化失败", err_msg, window);
		exit(-1);
	}
	void on_input()
	{	

		static SDL_Point pos_center;
		static SDL_Point idx_tile_selected;
		static ConfigManager* instance = ConfigManager::instance();
		
		switch (event.type)
		{
		case SDL_QUIT:
			is_quit = true;
			break;
		case SDL_MOUSEBUTTONDOWN:
			if (instance->is_game_over)
				break;
			if (event.button.button != SDL_BUTTON_LEFT && event.button.button != SDL_BUTTON_RIGHT) break;
			place_panel->hide();
			if (event.button.button == SDL_BUTTON_LEFT &&
				tower_actions.handle_left_click({event.button.x, event.button.y})) return;
			tower_actions.close();
			if (get_cursor_idx_tile(idx_tile_selected, event.button.x, event.button.y))
			{
				get_selected_tile_center_pos(pos_center, idx_tile_selected);
				if (TowerManager::instance()->find_tower(idx_tile_selected))
				{
					auto action = event.button.button == SDL_BUTTON_RIGHT ?
						TowerActionPanel::Action::Withdraw : TowerActionPanel::Action::Upgrade;
					tower_actions.show(renderer, idx_tile_selected, pos_center, action);
				}
				else if (event.button.button == SDL_BUTTON_LEFT && !check_home(idx_tile_selected) && can_place_tower(idx_tile_selected))
				{
					place_panel->set_idx_tile(idx_tile_selected);
					place_panel->set_center_pos(pos_center);
					place_panel->show();
				}
			}
			break;
		default:
			break;
		}
		if (!instance->is_game_over)
		{
			place_panel->on_input(event);
			PlayerManager::instance()->on_input(event);
		}

	}
	void on_update(double delta)
	{

		static ConfigManager* instance = ConfigManager::instance();
		if (!instance->is_game_over)
		{	
			status_bar.on_update(renderer);
			place_panel->on_update(renderer);
			WaveManager::instance()->on_update(delta);
			EnemyManager::instance()->on_update(delta);
			BulletManager::instance()->on_update(delta);

			TowerManager::instance()->on_update(delta);
			PlayerManager::instance()->on_update(delta);
			CoinManager::instance()->on_update(delta);

			if (HomeManager::instance()->get_current_hp() <= 0)
			{
				instance->is_game_win = false;
				instance->is_game_over = true;
			}
			if (!instance->is_game_over)
			{
				update_stage_title();
				return;
			}
		}
		if (!result_shown)
		{
			static const ResourcesManager::SoundPool& sound_pool =
				ResourcesManager::instance()->get_sound_pool();
			result_shown = true;
			bool continuing = instance->is_game_win && stage_index + 1 < stage_count;
			if (!continuing) Mix_FadeOutMusic(1500);
			SDL_SetWindowTitle(window, continuing ? u8"守住了！即将进入下一关" :
				(instance->is_game_win ? u8"三关全部通关！" : u8"基地失守"));
			Mix_PlayChannel(-1, sound_pool.find(instance->is_game_win ? ResID::Sound_Win : ResID::Sound_Loss)->second, 0);
			banner->show();

		}
		

		
		banner->on_update(delta);
		if (banner->check_end_display())
		{
			if (instance->is_game_win && stage_index + 1 < stage_count)
				advance_stage();
			else
				is_quit = true;
		}
		
	}
	void on_renderer()
	{
		static ConfigManager* instance = ConfigManager::instance();
		static SDL_Rect& rect_dst = instance->rect_tile_map;
		SDL_RenderCopy(renderer, tex_tile_map, nullptr, &rect_dst);

		EnemyManager::instance()->on_renderer(renderer);

		TowerManager::instance()->on_render(renderer);
		BulletManager::instance()->on_render(renderer);
		CoinManager::instance()->on_render(renderer);
		PlayerManager::instance()->on_render(renderer);
		if (!instance->is_game_over)
		{
			place_panel->on_render(renderer);
			status_bar.on_render(renderer);
			tower_actions.render(renderer);
			
			return;
		}

		int width_screen, height_screen;
		SDL_GetWindowSizeInPixels(window, &width_screen, &height_screen);
		banner->set_position({ (double)width_screen / 2,(double)height_screen / 2 });
		banner->on_render(renderer);
	}
	bool generate_tile_map_texture()
	{
		//第二种方式是一个精灵
		const Map& map = ConfigManager::instance()->map;
		const TileMap& tile_map = map.get_tile_map();
		SDL_Rect& rect_tile_map = ConfigManager::instance()->rect_tile_map;
		SDL_Texture* tex_tile_set = ResourcesManager::instance()->get_texture_pool().find(ResID::Tex_Tileset)->second;

		int width_tex_tile_set, height_tex_tile_set;
		//可以切换不同的方式
		SDL_QueryTexture(tex_tile_set, nullptr, nullptr, &width_tex_tile_set, &height_tex_tile_set);
		int num_tile_single_line = (int)std::ceil((double)width_tex_tile_set / SIZE_TILE);

		int width_tex_tile_map, height_tex_tile_map;
		width_tex_tile_map = (int)map.get_width() * SIZE_TILE;
		height_tex_tile_map = (int)map.get_height() * SIZE_TILE;
		tex_tile_map = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_ARGB8888,
			SDL_TEXTUREACCESS_TARGET, width_tex_tile_map, height_tex_tile_map);
		if (!tex_tile_map)return false;

		ConfigManager* config = ConfigManager::instance();
		rect_tile_map.x = (config->basic_template.window_width - width_tex_tile_map) / 2;
		rect_tile_map.y = (config->basic_template.window_height - height_tex_tile_map) / 2;
		rect_tile_map.w = width_tex_tile_map;
		rect_tile_map.h = height_tex_tile_map;

		SDL_SetTextureBlendMode(tex_tile_map, SDL_BLENDMODE_BLEND);
		SDL_SetRenderTarget(renderer, tex_tile_map);

		for (int y = 0;y < map.get_height();y++)
		{
			for (int x = 0;x < map.get_width();x++)
			{
				SDL_Rect rect_src;
				const Tile& tile = tile_map[y][x];

				const SDL_Rect rect_dst =
				{
					x * SIZE_TILE,y * SIZE_TILE,
					SIZE_TILE,SIZE_TILE
				};

				rect_src =
				{
					(tile.terrain % num_tile_single_line) * SIZE_TILE,
					(tile.terrain / num_tile_single_line) * SIZE_TILE,
					SIZE_TILE,SIZE_TILE
				};
				SDL_RenderCopy(renderer, tex_tile_set, &rect_src, &rect_dst);

				if (tile.decoration >= 0)
				{
					rect_src =
					{
						(tile.decoration % num_tile_single_line) * SIZE_TILE,
						(tile.decoration / num_tile_single_line) * SIZE_TILE,
						SIZE_TILE,SIZE_TILE
					};
					SDL_RenderCopy(renderer, tex_tile_set, &rect_src, &rect_dst);
				}

			}
		}
		const SDL_Point& idx_home = map.get_idx_home();
		const SDL_Rect rect_dst =
		{
			idx_home.x * SIZE_TILE,idx_home.y * SIZE_TILE,
			SIZE_TILE,SIZE_TILE
		};
		SDL_RenderCopy(renderer, ResourcesManager::instance()->get_texture_pool().find(ResID::Tex_Home)->second, nullptr, &rect_dst);
		SDL_SetRenderTarget(renderer, nullptr);
		return true;
	}

	bool get_cursor_idx_tile(SDL_Point& idx_tile_selected, int screen_x, int screen_y)
	{
		static const Map& map = ConfigManager::instance()->map;
		static const SDL_Rect& rect_tile_map = ConfigManager::instance()->rect_tile_map;
		
		if (screen_x < rect_tile_map.x||screen_x>rect_tile_map.x+rect_tile_map.w||
			screen_y<rect_tile_map.y||screen_y>rect_tile_map.x+rect_tile_map.h)
		{
			return false;
		}
		//防止到达最大的边界导致数组越界
		idx_tile_selected.x = std::min((screen_x - rect_tile_map.x) / SIZE_TILE,(int)map.get_width()-1);
		idx_tile_selected.y = std::min((screen_y - rect_tile_map.y) / SIZE_TILE, (int)map.get_height() - 1);
		 

		
		
		return true;
	}
	//是否能放置防御塔
	bool can_place_tower(const SDL_Point& idx_tile_selected)const
	{
		static const Map& map = ConfigManager::instance()->map;
		const Tile& tile = map.get_tile_map()[idx_tile_selected.y][idx_tile_selected.x];
		
		return (tile.decoration < 0 && tile.direction == Tile::Direction::None && !tile.has_tower);
		
	}

	void get_selected_tile_center_pos(SDL_Point& pos, const SDL_Point& idx_tile_selected)const
	{
		static const SDL_Rect& rect_tile_map = ConfigManager::instance()->rect_tile_map;

		pos.x = rect_tile_map.x + idx_tile_selected.x * SIZE_TILE + SIZE_TILE / 2;
		pos.y = rect_tile_map.y + idx_tile_selected.y * SIZE_TILE + SIZE_TILE / 2;
	}
	//检测是否为基地
	bool check_home(const SDL_Point&idx_tile_selected)
	{
		static const Map& map = ConfigManager::instance()->map;
		static const SDL_Point& idx_home = map.get_idx_home();
		
		return (idx_home.x == idx_tile_selected.x && idx_home.y == idx_tile_selected.y);

	}
};

#endif // !GAME_MANAGER_H_
#pragma once
