#pragma once
#include "../includes.hpp"

class c_menu : public singleton<c_menu> {
public:
	void ColorPicker(const char * name, float * color, bool alpha);
	void draw( bool is_open );
	void menu_setup(ImGuiStyle &style);

	float dpi_scale = 1.f;

	ImFont* skeet_icon;
	ImFont* skeet_menu;
	ImFont* weapon_font;
	ImFont* keybind_font;


	ImFont* iconfont;
	ImFont* themefont;
	ImFont* info;
	LPDIRECT3DTEXTURE9 logggo;
	LPDIRECT3DTEXTURE9 RGBline;


	ImFont* info_little;
	ImFont* two;
	ImFont* three;
	ImFont* tabsf;
	ImFont* ee;
	int current_profile = -1;


	//IDirect3DTexture9 *background_t = nullptr;

	LPDIRECT3DTEXTURE9 bg = nullptr;

	float public_alpha;
	IDirect3DDevice9* device;
	float color_buffer[4] = { 1.f, 1.f, 1.f, 1.f };
private:
	struct {
		
	} styles;

	ImGuiStyle* style = &ImGui::GetStyle();

	float preview_alpha = 1.f;

	bool players_section;

	// we need to use 'int child' to seperate content in 2 childs
	void Legit();
	void Aimbot();
	void Antiaim();
	void Visuals();
	void Misc();
	void Skins();
	void Skin_Changer();
	void Configurations();
	void Luas();
	void Chams();

	std::string preview = ("None");
};
