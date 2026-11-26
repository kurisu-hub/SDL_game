#pragma once
#include "tower_manager.h"
#include <SDL_ttf.h>

// 左键选塔查看升级，右键选塔查看退款。面板以格子定位目标，切关时关闭。
class TowerActionPanel
{
public:
	enum class Action { Upgrade, Withdraw };

	void close()
	{
		SDL_DestroyTexture(label);
		label = nullptr;
	}

	void show(SDL_Renderer* renderer, SDL_Point idx, SDL_Point center, Action action)
	{
		close();
		auto* towers = TowerManager::instance();
		const Tower* tower = towers->find_tower(idx);
		if (!tower) return;
		selected = idx;
		selected_center = center;
		current = action;
		std::string text;
		if (action == Action::Withdraw)
			text = u8"撤回 · 返还 " + std::to_string((int)towers->refund_at(idx)) + u8" 金币";
		else
		{
			const int level = tower->get_level();
			const double price = tower->upgrade_price();
			if (price < 0) text = u8"Lv.10 已满级";
			else if (CoinManager::instance()->get_current_coin_num() < price)
				text = u8"金币不足 · 升级需要 " + std::to_string((int)price) + u8" 金币";
			else text = u8"升级 Lv." + std::to_string(level) + " > " + std::to_string(level + 1)
				+ " · " + std::to_string((int)price) + u8" 金币";
		}
		auto* font = ResourcesManager::instance()->get_font_pool().at(ResID::Font_Main);
		SDL_Surface* surface = TTF_RenderUTF8_Blended(font, text.c_str(), {255, 246, 220, 255});
		if (!surface) return;
		label = SDL_CreateTextureFromSurface(renderer, surface);
		box.w = surface->w + 24;
		box.h = surface->h + 12;
		SDL_FreeSurface(surface);
		int width, height;
		SDL_GetRendererOutputSize(renderer, &width, &height);
		box.x = std::max(0, std::min(center.x - box.w / 2, width - box.w));
		box.y = center.y - SIZE_TILE / 2 - box.h - 3;
		if (box.y < 0) box.y = std::min(center.y + SIZE_TILE / 2 + 3, height - box.h);
	}

	// 面板打开时消耗左键：按钮内执行一次，按钮外仅取消，避免误开建塔轮盘。
	bool handle_left_click(SDL_Point mouse)
	{
		if (!label) return false;
		if (SDL_PointInRect(&mouse, &box))
		{
			auto* towers = TowerManager::instance();
			if (current == Action::Withdraw) towers->withdraw_at(selected);
			else towers->upgrade_at(selected);
		}
		close();
		return true;
	}

	void render(SDL_Renderer* renderer) const
	{
		if (!label) return;
		SDL_Rect tile = {selected_center.x - SIZE_TILE / 2, selected_center.y - SIZE_TILE / 2, SIZE_TILE, SIZE_TILE};
		SDL_SetRenderDrawColor(renderer, 242, 194, 98, 255);
		SDL_RenderDrawRect(renderer, &tile);
		SDL_SetRenderDrawColor(renderer, 44, 39, 47, 255);
		SDL_RenderFillRect(renderer, &box);
		SDL_SetRenderDrawColor(renderer, 242, 194, 98, 255);
		SDL_RenderDrawRect(renderer, &box);
		SDL_Rect dst = {box.x + 12, box.y + 6, box.w - 24, box.h - 12};
		SDL_RenderCopy(renderer, label, nullptr, &dst);
	}

private:
	Action current = Action::Upgrade;
	SDL_Point selected{}, selected_center{};
	SDL_Rect box{};
	SDL_Texture* label = nullptr;
};
