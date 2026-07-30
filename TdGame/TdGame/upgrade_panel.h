#ifndef _UPGRADE_PANEL_H_
#define _UPGRADE_PANEL_H_

#include"panel.h"
#include"resources_manager.h"
#include"tower_manager.h"
#include"coin_manager.h"

class UpgradePanel:public Panel
{
public:
	UpgradePanel()
	{
		const ResourcesManager::TexturePool& texture_pool
			= ResourcesManager::instance()->get_texture_pool();

		tex_idle = texture_pool.find(ResID::Tex_UIUpgradeIdle)->second;
		tex_hovered_top = texture_pool.find(ResID::Tex_UIUpgradeHoveredTop)->second;
		tex_hovered_right = texture_pool.find(ResID::Tex_UIUpgradeHoveredRight)->second;
		tex_hovered_left = texture_pool.find(ResID::Tex_UIUpgradeHoveredLeft)->second;

		

	}

	~UpgradePanel() = default;
	
	void on_update(SDL_Renderer*renderer)
	{
		static TowerManager* instance = TowerManager::instance();
		
		val_top = (int)instance->get_upgrade_const(TowerType::Axeman);
		val_left = (int)instance->get_upgrade_const(TowerType::Archer);
		val_right = (int)instance->get_upgrade_const(TowerType::Gunner);
		
		Panel::on_update(renderer);
	}


protected:
	virtual void on_click_top_area()override
	{
		CoinManager* instance = CoinManager::instance();

		if (val_top <= instance->get_current_coin_num())
		{
			TowerManager::instance()->get_upgrade_const(TowerType::Axeman);
			instance->decrease_coin(val_top);
		}
	}

	virtual void on_click_left_area()override
	{
		CoinManager* instance = CoinManager::instance();

		if (val_top <= instance->get_current_coin_num())
		{
			TowerManager::instance()->get_upgrade_const(TowerType::Archer);
			instance->decrease_coin(val_top);
		}
	}
	virtual void on_click_right_area()override
	{
		CoinManager* instance = CoinManager::instance();

		if (val_top <= instance->get_current_coin_num())
		{
			TowerManager::instance()->get_upgrade_const(TowerType::Gunner);
			instance->decrease_coin(val_top);
		}
	}
private:

};

#endif // !_UPGRADE_PANEL_H_
