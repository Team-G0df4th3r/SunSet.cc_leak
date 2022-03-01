#include <ShlObj_core.h>
#include <unordered_map>
#include "menu.h"
#include "../ImGui/code_editor.h"
#include "../constchars.h"
#include "../cheats/misc/logs.h"
#include "../ImGui/checkmark.h"
#include "../ImGui/MenuControls.h"
#include "../nSkinz/SkinChanger.h"
#include "../BASS/API.h"
#include "../BASS/bass.h"
//#include <ImGui/imgui.cpp>

#define _SILENCE_EXPERIMENTAL_FILESYSTEM_DEPRECATION_WARNING

#define ALPHA (ImGuiColorEditFlags_AlphaPreview | ImGuiColorEditFlags_NoTooltip | ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_NoLabel | ImGuiColorEditFlags_AlphaBar| ImGuiColorEditFlags_InputRGB | ImGuiColorEditFlags_Float)
#define NOALPHA (ImGuiColorEditFlags_NoTooltip | ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_NoLabel | ImGuiColorEditFlags_NoAlpha | ImGuiColorEditFlags_InputRGB | ImGuiColorEditFlags_Float)

std::vector <std::string> files;
std::vector <std::string> scripts;
std::string editing_script;


ImFont* iconfont = nullptr;
ImFont* themefont = nullptr;
ImFont* info = nullptr;

ImFont* info_little = nullptr;
ImFont* two = nullptr;
ImFont* three = nullptr;
ImFont* tabsf = nullptr;
ImFont* ee = nullptr;
ImVec2 pos;
ImDrawList* draw;

int CurrentTAB = 0;
static int animateid[2048];

auto selected_script = 0;
auto loaded_editing_script = false;

static auto menu_setupped = false;
static auto should_update = true;

IDirect3DTexture9* all_skins[36];

std::string get_wep(int id, int custom_index = -1, bool knife = true)
{
	if (custom_index > -1)
	{
		if (knife)
		{
			switch (custom_index)
			{
			case 0: return ("weapon_knife");
			case 1: return ("weapon_bayonet");
			case 2: return ("weapon_knife_css");
			case 3: return ("weapon_knife_skeleton");
			case 4: return ("weapon_knife_outdoor");
			case 5: return ("weapon_knife_cord");
			case 6: return ("weapon_knife_canis");
			case 7: return ("weapon_knife_flip");
			case 8: return ("weapon_knife_gut");
			case 9: return ("weapon_knife_karambit");
			case 10: return ("weapon_knife_m9_bayonet");
			case 11: return ("weapon_knife_tactical");
			case 12: return ("weapon_knife_falchion");
			case 13: return ("weapon_knife_survival_bowie");
			case 14: return ("weapon_knife_butterfly");
			case 15: return ("weapon_knife_push");
			case 16: return ("weapon_knife_ursus");
			case 17: return ("weapon_knife_gypsy_jackknife");
			case 18: return ("weapon_knife_stiletto");
			case 19: return ("weapon_knife_widowmaker");
			}
		}
		else
		{
			switch (custom_index)
			{
			case 0: return ("ct_gloves"); //-V1037
			case 1: return ("studded_bloodhound_gloves");
			case 2: return ("t_gloves");
			case 3: return ("ct_gloves");
			case 4: return ("sporty_gloves");
			case 5: return ("slick_gloves");
			case 6: return ("leather_handwraps");
			case 7: return ("motorcycle_gloves");
			case 8: return ("specialist_gloves");
			case 9: return ("studded_hydra_gloves");
			}
		}
	}
	else
	{
		switch (id)
		{
		case 0: return ("knife");
		case 1: return ("gloves");
		case 2: return ("weapon_ak47");
		case 3: return ("weapon_aug");
		case 4: return ("weapon_awp");
		case 5: return ("weapon_cz75a");
		case 6: return ("weapon_deagle");
		case 7: return ("weapon_elite");
		case 8: return ("weapon_famas");
		case 9: return ("weapon_fiveseven");
		case 10: return ("weapon_g3sg1");
		case 11: return ("weapon_galilar");
		case 12: return ("weapon_glock");
		case 13: return ("weapon_m249");
		case 14: return ("weapon_m4a1_silencer");
		case 15: return ("weapon_m4a1");
		case 16: return ("weapon_mac10");
		case 17: return ("weapon_mag7");
		case 18: return ("weapon_mp5sd");
		case 19: return ("weapon_mp7");
		case 20: return ("weapon_mp9");
		case 21: return ("weapon_negev");
		case 22: return ("weapon_nova");
		case 23: return ("weapon_hkp2000");
		case 24: return ("weapon_p250");
		case 25: return ("weapon_p90");
		case 26: return ("weapon_bizon");
		case 27: return ("weapon_revolver");
		case 28: return ("weapon_sawedoff");
		case 29: return ("weapon_scar20");
		case 30: return ("weapon_ssg08");
		case 31: return ("weapon_sg556");
		case 32: return ("weapon_tec9");
		case 33: return ("weapon_ump45");
		case 34: return ("weapon_usp_silencer");
		case 35: return ("weapon_xm1014");
		default: return ("unknown");
		}
	}
}

IDirect3DTexture9* get_skin_preview(const char* weapon_name, const std::string& skin_name, IDirect3DDevice9* device)
{
	IDirect3DTexture9* skin_image = nullptr;
	std::string vpk_path;

	if (strcmp(weapon_name, ("unknown")) && strcmp(weapon_name, ("knife")) && strcmp(weapon_name, ("gloves"))) //-V526
	{
		if (skin_name.empty() || skin_name == ("default"))
			vpk_path = ("resource/flash/econ/weapons/base_weapons/") + std::string(weapon_name) + (".png");
		else
			vpk_path = ("resource/flash/econ/default_generated/") + std::string(weapon_name) + ("_") + std::string(skin_name) + ("_light_large.png");
	}
	else
	{
		if (!strcmp(weapon_name, ("knife")))
			vpk_path = ("resource/flash/econ/weapons/base_weapons/weapon_knife.png");
		else if (!strcmp(weapon_name, ("gloves")))
			vpk_path = ("resource/flash/econ/weapons/base_weapons/ct_gloves.png");
		else if (!strcmp(weapon_name, ("unknown")))
			vpk_path = ("resource/flash/econ/weapons/base_weapons/weapon_snowball.png");

	}
	const auto handle = m_basefilesys()->Open(vpk_path.c_str(), ("r"), ("GAME"));
	if (handle)
	{
		int file_len = m_basefilesys()->Size(handle);
		char* image = new char[file_len]; //-V121

		m_basefilesys()->Read(image, file_len, handle);
		m_basefilesys()->Close(handle);

		D3DXCreateTextureFromFileInMemory(device, image, file_len, &skin_image);
		delete[] image;
	}

	if (!skin_image)
	{
		std::string vpk_path;

		if (strstr(weapon_name, ("bloodhound")) != NULL || strstr(weapon_name, ("hydra")) != NULL)
			vpk_path = ("resource/flash/econ/weapons/base_weapons/ct_gloves.png");
		else
			vpk_path = ("resource/flash/econ/weapons/base_weapons/") + std::string(weapon_name) + (".png");

		const auto handle = m_basefilesys()->Open(vpk_path.c_str(), ("r"), ("GAME"));

		if (handle)
		{
			int file_len = m_basefilesys()->Size(handle);
			char* image = new char[file_len]; //-V121

			m_basefilesys()->Read(image, file_len, handle);
			m_basefilesys()->Close(handle);

			D3DXCreateTextureFromFileInMemory(device, image, file_len, &skin_image);
			delete[] image;
		}
	}

	return skin_image;
}

void c_menu::menu_setup(ImGuiStyle& style) //-V688
{

}

std::string get_config_dir()
{
	std::string folder;
	static TCHAR path[MAX_PATH];

	if (SUCCEEDED(SHGetFolderPath(NULL, 0x001a, NULL, NULL, path)))
		folder = std::string(path) + ("\\SunSet.CC\\Configs\\");

	CreateDirectory(folder.c_str(), NULL);
	return folder;
}

void load_config()
{
	if (cfg_manager->files.empty())
		return;

	cfg_manager->load(cfg_manager->files.at(g_cfg.selected_config), false);
	c_lua::get().unload_all_scripts();

	for (auto& script : g_cfg.scripts.scripts)
		c_lua::get().load_script(c_lua::get().get_script_id(script));

	scripts = c_lua::get().scripts;

	if (selected_script >= scripts.size())
		selected_script = scripts.size() - 1; //-V103

	for (auto& current : scripts)
	{
		if (current.size() >= 5 && current.at(current.size() - 1) == 'c')
			current.erase(current.size() - 5, 5);
		else if (current.size() >= 4)
			current.erase(current.size() - 4, 4);
	}

	for (auto i = 0; i < g_cfg.skins.skinChanger.size(); ++i)
		all_skins[i] = nullptr;

	g_cfg.scripts.scripts.clear();

	cfg_manager->load(cfg_manager->files.at(g_cfg.selected_config), true);
	cfg_manager->config_files();

	eventlogs::get().add(("Loaded ") + files.at(g_cfg.selected_config) + (" config"), false);
}

void save_config()
{
	if (cfg_manager->files.empty())
		return;

	g_cfg.scripts.scripts.clear();

	for (auto i = 0; i < c_lua::get().scripts.size(); ++i)
	{
		auto script = c_lua::get().scripts.at(i);

		if (c_lua::get().loaded.at(i))
			g_cfg.scripts.scripts.emplace_back(script);
	}

	cfg_manager->save(cfg_manager->files.at(g_cfg.selected_config));
	cfg_manager->config_files();

	eventlogs::get().add(("Saved ") + files.at(g_cfg.selected_config) + (" config"), false);
}

void remove_config()
{
	if (cfg_manager->files.empty())
		return;

	eventlogs::get().add(("Removed ") + files.at(g_cfg.selected_config) + (" config"), false);

	cfg_manager->remove(cfg_manager->files.at(g_cfg.selected_config));
	cfg_manager->config_files();

	files = cfg_manager->files;

	if (g_cfg.selected_config >= files.size())
		g_cfg.selected_config = files.size() - 1; //-V103

	for (auto& current : files)
		if (current.size() > 2)
			current.erase(current.size() - 3, 3);
}

void add_config()
{
	auto empty = true;

	for (auto current : g_cfg.new_config_name)
	{
		if (current != ' ')
		{
			empty = false;
			break;
		}
	}

	if (empty)
		g_cfg.new_config_name = ("config");

	eventlogs::get().add(("Added ") + g_cfg.new_config_name + (" config"), false);

	if (g_cfg.new_config_name.find((".ss")) == std::string::npos)
		g_cfg.new_config_name += (".ss");

	cfg_manager->save(g_cfg.new_config_name);
	cfg_manager->config_files();

	g_cfg.selected_config = cfg_manager->files.size() - 1; //-V103
	files = cfg_manager->files;

	for (auto& current : files)
		if (current.size() > 2)
			current.erase(current.size() - 3, 3);
}

__forceinline void padding(float x, float y)
{

}

void child_title(const char* label)
{

}

namespace config_listbox
{
	static auto vector_getter = [](void* vec, int idx, const char** out_text)
	{
		auto& vector = *static_cast<std::vector<std::string>*>(vec);
		if (idx < 0 || idx >= static_cast<int>(vector.size())) { return false; }
		*out_text = vector.at(idx).c_str();
		return true;
	};

	bool ListBox(const char* label, int* currIndex, std::vector<std::string>& values, int height_in_items = -1)
	{
		ImGui::NewLine(); ImGui::SameLine(42.f); ImGui::PushItemWidth(158.f);
		if (values.empty()) { return false; }
		return ImGui::ListBox(label, currIndex, vector_getter, static_cast<void*>(&values), values.size(), height_in_items);
	}

	static bool ListBox(const char* label, int* current_item, std::function<const char* (int)> lambda, int items_count, int height_in_items)
	{
		return ImGui::ListBox(label, current_item, [](void* data, int idx, const char** out_text)
			{
				*out_text = (*reinterpret_cast<std::function<const char* (int)>*>(data))(idx);
				return true;
			}, &lambda, items_count, height_in_items);
	}
}

void draw_combo(const char* name, int& variable, const char* labels[], int count)
{
	padding(-3, -6);
	ImGui::Text(name);
	padding(0, -5);
	ImGui::Combo(std::string(("##COMBO") + std::string(name)).c_str(), &variable, labels, count);
}

void draw_combo(const char* name, int& variable, bool (*items_getter)(void*, int, const char**), void* data, int count)
{
	ImGui::SetCursorPosY(ImGui::GetCursorPosY() - 6);
	ImGui::Text(name);
	ImGui::SetCursorPosY(ImGui::GetCursorPosY() - 5);
	ImGui::Combo(std::string(("##COMBO__") + std::string(name)).c_str(), &variable, items_getter, data, count);
}

bool LabelClick(const char* label, bool* v, const char* unique_id)
{
	ImGuiWindow* window = ImGui::GetCurrentWindow();
	if (window->SkipItems)
		return false;

	// The concatoff/on thingies were for my weapon config system so if we're going to make that, we still need this aids.
	char Buf[64];
	_snprintf(Buf, 62, ("%s"), label);

	char getid[128];
	sprintf_s(getid, 128, ("%s%s"), label, unique_id);


	ImGuiContext& g = *GImGui;
	const ImGuiStyle& style = g.Style;
	const ImGuiID id = window->GetID(getid);
	const ImVec2 label_size = ImGui::CalcTextSize(label, NULL, true);

	const ImRect check_bb(window->DC.CursorPos, ImVec2(label_size.y + style.FramePadding.y * 2 + window->DC.CursorPos.x, window->DC.CursorPos.y + label_size.y + style.FramePadding.y * 2));
	ImGui::ItemSize(check_bb, style.FramePadding.y);

	ImRect total_bb = check_bb;

	if (label_size.x > 0)
	{
		ImGui::SameLine(0, style.ItemInnerSpacing.x);
		const ImRect text_bb(ImVec2(window->DC.CursorPos.x, window->DC.CursorPos.y + style.FramePadding.y), ImVec2(window->DC.CursorPos.x + label_size.x, window->DC.CursorPos.y + style.FramePadding.y + label_size.y));

		ImGui::ItemSize(ImVec2(text_bb.GetWidth(), check_bb.GetHeight()), style.FramePadding.y);
		total_bb = ImRect(ImMin(check_bb.Min, text_bb.Min), ImMax(check_bb.Max, text_bb.Max));
	}

	if (!ImGui::ItemAdd(total_bb, id))
		return false;

	bool hovered, held;
	bool pressed = ImGui::ButtonBehavior(total_bb, id, &hovered, &held);
	if (pressed)
		*v = !(*v);

	if (*v)
		ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(198 / 400.f, 123 / 400.f, 219 / 400.f, 1.f));
	if (label_size.x > 0.0f)
		ImGui::RenderText(ImVec2(check_bb.GetTL().x + 12, check_bb.GetTL().y), Buf);
	if (*v)
		ImGui::PopStyleColor();

	return pressed;

}

void draw_keybind(const char* label, key_bind* key_bind, const char* unique_id)
{
	ImGui::NewLine(); ImGui::SameLine(42.f); ImGui::PushItemWidth(158.f);

	if (key_bind->key == KEY_ESCAPE)
		key_bind->key = KEY_NONE;

	auto clicked = false;
	auto text = (std::string)m_inputsys()->ButtonCodeToString(key_bind->key);

	if (key_bind->key <= KEY_NONE || key_bind->key >= KEY_MAX)
		text = ("Bind");

	if (hooks::input_shouldListen && hooks::input_receivedKeyval == &key_bind->key)
	{
		clicked = true;
		text = ("...");
	}

	auto textsize = ImGui::CalcTextSize(text.c_str()).x + 8;
	auto labelsize = ImGui::CalcTextSize(label);

	ImGui::Text(label);
	ImGui::SameLine();

	ImGui::SetCursorPosX(ImGui::GetWindowSize().x - (ImGui::GetWindowSize().x - ImGui::CalcItemWidth()) - max(50, textsize));
	ImGui::SetCursorPosY(ImGui::GetCursorPosY() - 3);

	if (ImGui::KeybindButton(text.c_str(), unique_id, ImVec2(max(50, textsize), 23), clicked, ImGuiButtonFlags_DontClosePopups))
		clicked = true;

	if (clicked)
	{
		hooks::input_shouldListen = true;
		hooks::input_receivedKeyval = &key_bind->key;
	}

	static auto hold = false, toggle = false;

	switch (key_bind->mode)
	{
	case HOLD:
		hold = true;
		toggle = false;
		break;
	case TOGGLE:
		toggle = true;
		hold = false;
		break;
	}

	if (ImGui::BeginPopup(unique_id))
	{
		ImGui::SetCursorPosY(ImGui::GetCursorPosY() + 6);
		ImGui::SetCursorPosX(ImGui::GetCursorPosX() + ((ImGui::GetCurrentWindow()->Size.x / 2) - (ImGui::CalcTextSize(("Hold")).x / 2)));
		ImGui::SetCursorPosX(ImGui::GetCursorPosX() - 11);

		if (LabelClick(("Hold"), &hold, unique_id))
		{
			if (hold)
			{
				toggle = false;
				key_bind->mode = HOLD;
			}
			else if (toggle)
			{
				hold = false;
				key_bind->mode = TOGGLE;
			}
			else
			{
				toggle = false;
				key_bind->mode = HOLD;
			}

			ImGui::CloseCurrentPopup();
		}

		ImGui::SetCursorPosX(ImGui::GetCursorPosX() + ((ImGui::GetCurrentWindow()->Size.x / 2) - (ImGui::CalcTextSize(("Toggle")).x / 2)));
		ImGui::SetCursorPosX(ImGui::GetCursorPosX() - 11);
		ImGui::SetCursorPosY(ImGui::GetCursorPosY() - 9);

		if (LabelClick(("Toggle"), &toggle, unique_id))
		{
			if (toggle)
			{
				hold = false;
				key_bind->mode = TOGGLE;
			}
			else if (hold)
			{
				toggle = false;
				key_bind->mode = HOLD;
			}
			else
			{
				hold = false;
				key_bind->mode = TOGGLE;
			}

			ImGui::CloseCurrentPopup();
		}

		ImGui::EndPopup();
	}
}

void draw_multicombo(std::string name, std::vector<int>& variable, const char* labels[], int count, std::string& preview)
{
	ImGui::Text(((" ") + name).c_str());

	auto hashname = ("##") + name; // we dont want to render name of combo

	for (auto i = 0, j = 0; i < count; i++)
	{
		if (variable[i])
		{

			if (j)
				//preview += (" "); //+ (std::string)labels[i];
				(std::string)labels[i];
			else
				preview = labels[i];

			j++;
		}
	}

	if (ImGui::BeginCombo(hashname.c_str(), preview.c_str())) // draw start
	{
		ImGui::BeginGroup();
		{
			for (auto i = 0; i < count; i++)
				ImGui::Selectable(labels[i], (bool*)&variable[i], ImGuiSelectableFlags_DontClosePopups);
		}
		ImGui::EndGroup();

		ImGui::EndCombo();
	}

	preview = ("None"); // reset preview to use later
}

void draw_semitabs(const char* labels[], int count, int& tab, ImGuiStyle& style)
{

}

__forceinline void tab_start()
{

}

__forceinline void tab_end()
{

}

void lua_edit(std::string window_name)
{
	std::string file_path;

	auto get_dir = [&]() -> void
	{
		static TCHAR path[MAX_PATH];

		if (SUCCEEDED(SHGetFolderPath(NULL, CSIDL_APPDATA, NULL, NULL, path)))
			file_path = std::string(path) + ("\\SunSet.CC\\Scripts\\");

		CreateDirectory(file_path.c_str(), NULL);
		file_path += window_name + (".lua");
	};

	get_dir();
	const char* child_name = (window_name + window_name).c_str();

	ImGui::SetNextWindowSize(ImVec2(700, 700), ImGuiCond_Once);
	ImGui::GetColorU32(ImVec4(40 / 255.f, 40 / 255.f, 40 / 255.f, 1.f));
	ImGui::Begin(window_name.c_str(), nullptr, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse);
	ImGui::PushStyleVar(ImGuiStyleVar_ScrollbarSize, 5.f);

	static TextEditor editor;

	if (!loaded_editing_script)
	{
		static auto lang = TextEditor::LanguageDefinition::Lua();

		editor.SetLanguageDefinition(lang);
		editor.SetReadOnly(false);

		std::ifstream t(file_path);

		if (t.good())
		{
			std::string str((std::istreambuf_iterator<char>(t)), std::istreambuf_iterator<char>());
			editor.SetText(str);
		}

		loaded_editing_script = true;
	}

	ImGui::SetWindowFontScale(1.f + ((1.0) * 0.5f));

	ImGui::SetWindowSize(ImVec2(ImFloor(500 * (1.f + ((1.0) * 0.5f))), ImFloor(500 * (1.f + ((1.0) * 0.5f)))));
	editor.Render(child_name, ImGui::GetWindowSize() - ImVec2(0, 66 * (1.f + ((1.0) * 0.5f))));

	ImGui::Separator();

	ImGui::SetCursorPosX(ImGui::GetCursorPosX() + ImGui::GetWindowSize().x - (16.f * (1.f + ((1.0) * 0.5f))) - (250.f * (1.f + ((1.0) * 0.5f))));
	ImGui::BeginGroup();

	if (ImGui::CustomButton(("Save"), (("Save") + window_name).c_str(), ImVec2(40 * (1.f + ((1.0) * 0.5f)), 0), true, c_menu::get().skeet_menu, ("")))
	{
		std::ofstream out;

		out.open(file_path);
		out << editor.GetText() << std::endl;
		out.close();
	}

	ImGui::SameLine();

	if (ImGui::CustomButton(("Close"), (("Close") + window_name).c_str(), ImVec2(40 * (1.f + ((1.0) * 0.5f)), 0), ImGuiItemFlags_ButtonRepeat, c_menu::get().skeet_menu, ("")))
	{
		g_ctx.globals.focused_on_input = false;
		loaded_editing_script = false;
		editing_script.clear();
	}

	ImGui::EndGroup();

	ImGui::PopStyleVar();
	ImGui::End();
}

static int tab = 0;

void c_menu::draw(bool is_open)
{
	static char config_name[64] = "\0";
	static float m_alpha = 0.0002f;
	m_alpha = math::clamp(m_alpha + (8.f * (is_open ? 1.f : -1.f)), 0.0001f, 1.f);

	public_alpha = m_alpha;

	if (m_alpha <= 0.0001f)
		return;

	ImGui::PushStyleVar(ImGuiStyleVar_Alpha, m_alpha);


	ImGui::PushStyleColor(ImGuiCol_ScrollbarGrab, ImVec4(ImGui::GetStyle().Colors[ImGuiCol_ScrollbarGrab].x, ImGui::GetStyle().Colors[ImGuiCol_ScrollbarGrab].y, ImGui::GetStyle().Colors[ImGuiCol_ScrollbarGrab].z, m_alpha));

	ImGui::Begin("Menu", nullptr, ImGuiWindowFlags_NoScrollWithMouse | ImGuiWindowFlags_NoBackground | ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_HorizontalScrollbar | ImGuiWindowFlags_NoBringToFrontOnFocus);
	{
		static int x = 600, y = 900;
		ImGui::SetWindowSize(ImVec2(ImFloor(x), ImFloor(900)));
		ImGui::SetCursorPos({ 0, 100 });




		/*if (logggo == nullptr)
			D3DXCreateTextureFromFileInMemoryEx(g_pd3dDevice, &Logo, sizeof(Logo), 462, 462, D3DX_DEFAULT, 0, D3DFMT_UNKNOWN, D3DPOOL_DEFAULT, D3DX_DEFAULT, D3DX_DEFAULT, 0, NULL, NULL, &logggo);

		if (RGBline == nullptr)
			D3DXCreateTextureFromFileInMemoryEx(g_pd3dDevice, &RGBLINE, sizeof(RGBLINE), 462, 462, D3DX_DEFAULT, 0, D3DFMT_UNKNOWN, D3DPOOL_DEFAULT, D3DX_DEFAULT, D3DX_DEFAULT, 0, NULL, NULL, &RGBline);*/

		ImDrawList* draw = ImGui::GetWindowDrawList();
		ImVec2 pos(ImGui::GetWindowPos());

		draw->AddRectFilled(pos, ImVec2(pos.x + 600, pos.y + 580), ImColor(3, 3, 3)); //BG
		draw->AddImage(logggo, ImVec2(pos.x + 5, pos.y + 5), ImVec2(pos.x + 55, pos.y + 50));

		draw->AddImage(RGBline, ImVec2(pos.x, pos.y + 49), ImVec2(pos.x + 600, pos.y + 51));
		ImGui::PushFont(two);
		draw->AddText(ImVec2(pos.x + 60, pos.y + 15), ImColor(255, 255, 255), "SunSet.CC");
		ImGui::PopFont();

		ImGui::SetCursorPos({ 0, 51 });
		ImGui::BeginGroup();
		{
			if (ImGui::Tab("Legit", CurrentTAB == 0, 100))
				CurrentTAB = 0;
			ImGui::SameLine();
			ImGui::SetCursorPosX(100);
			if (ImGui::Tab("Rage", CurrentTAB == 1, 100))
				CurrentTAB = 1;
			ImGui::SameLine();
			ImGui::SetCursorPosX(200);
			if (ImGui::Tab("ESP", CurrentTAB == 2, 100))
				CurrentTAB = 2;
			ImGui::SameLine();
			ImGui::SetCursorPosX(300);
			if (ImGui::Tab("Chams", CurrentTAB == 3, 100))
				CurrentTAB = 3;
			ImGui::SameLine();
			ImGui::SetCursorPosX(400);
			if (ImGui::Tab("Misc", CurrentTAB == 4, 100))
				CurrentTAB = 4;
			ImGui::SameLine();
			ImGui::SetCursorPosX(500);
			if (ImGui::Tab("Config", CurrentTAB == 5, 100))
				CurrentTAB = 5;
		}
		ImGui::EndGroup();

		if (CurrentTAB == 0)
		{
			Legit();
		}
		if (CurrentTAB == 1)
		{
			Aimbot();
		}
		if (CurrentTAB == 2)
		{
			Visuals();
		}
		if (CurrentTAB == 3)
		{
			Misc();
		}
		if (CurrentTAB == 4)
		{
			Skins();
		}
		if (CurrentTAB == 5)
		{
			Configurations();
		}
	}
	ImGui::End();
}

void c_menu::Legit()
{
	const char* legit_weapons[6] = { crypt_str("Deagle"), crypt_str("Pistols"), crypt_str("Rifles"), crypt_str("SMGs"), crypt_str("Snipers"), crypt_str("Heavy") };
	const char* hitbox_legit[3] = { crypt_str("Nearest"), crypt_str("Head"), crypt_str("Body") };

	ImGui::BeginChildcustom("pinto21", ImVec2(290, 490), true);
	{
		padding(0, 6);
		ImGui::Checkbox(crypt_str("Enable"), &g_cfg.legitbot.enabled);

		ImGui::SameLine();
		draw_keybind(crypt_str(""), &g_cfg.legitbot.key, crypt_str("##HOTKEY_LGBT_KEY"));


		if (g_cfg.legitbot.enabled)
			g_cfg.ragebot.enable = false;

		ImGui::Checkbox(crypt_str("Friendly fire"), &g_cfg.legitbot.friendly_fire);

		ImGui::Checkbox(crypt_str("Automatic pistols"), &g_cfg.legitbot.autopistol);


		ImGui::Checkbox(crypt_str("Automatic scope"), &g_cfg.legitbot.autoscope);


		if (g_cfg.legitbot.autoscope)
			ImGui::Checkbox(crypt_str("Unscope after shot"), &g_cfg.legitbot.unscope);

		ImGui::Checkbox(crypt_str("Snipers in zoom only"), &g_cfg.legitbot.sniper_in_zoom_only);

		ImGui::Checkbox(crypt_str("Automatic stop"), &g_cfg.legitbot.weapon[hooks::legit_weapon].auto_stop);

		ImGui::Checkbox(crypt_str("Enable in air"), &g_cfg.legitbot.do_if_local_in_air);

		ImGui::Checkbox(crypt_str("Enable if flashed"), &g_cfg.legitbot.do_if_local_flashed);

		ImGui::Checkbox(crypt_str("Enable in smoke"), &g_cfg.legitbot.do_if_enemy_in_smoke);

		draw_keybind(crypt_str("Automatic fire key"), &g_cfg.legitbot.autofire_key, crypt_str("##HOTKEY_AUTOFIRE_LGBT_KEY"));
		ImGui::Spacing();
		ImGui::SliderInt(crypt_str("Automatic fire delay"), &g_cfg.legitbot.autofire_delay, 0, 12, ("%d ticks"));

	}
	ImGui::EndChild();
	ImGui::SameLine(300);
	ImGui::BeginChildcustom("xerereca2121", ImVec2(290, 490), true);
	{

		draw_combo(crypt_str("Current weapon"), hooks::legit_weapon, legit_weapons, ARRAYSIZE(legit_weapons));

		draw_combo(crypt_str("Hitbox"), g_cfg.legitbot.weapon[hooks::legit_weapon].priority, hitbox_legit, ARRAYSIZE(hitbox_legit));
		ImGui::Spacing();
		ImGui::Spacing();
		draw_combo(crypt_str("Field of view type"), g_cfg.legitbot.weapon[hooks::legit_weapon].fov_type, LegitFov, ARRAYSIZE(LegitFov));
		ImGui::Spacing();
		ImGui::Spacing();
		ImGui::SliderFloat(crypt_str("Field of view amount"), &g_cfg.legitbot.weapon[hooks::legit_weapon].fov, 0.f, 30.f, crypt_str("%.2f"));


		ImGui::Spacing();

		ImGui::SliderFloat(crypt_str("Silent field of view"), &g_cfg.legitbot.weapon[hooks::legit_weapon].silent_fov, 0.f, 30.f, (!g_cfg.legitbot.weapon[hooks::legit_weapon].silent_fov ? crypt_str("None") : crypt_str("%.2f"))); //-V550

		ImGui::Spacing();


		draw_combo(crypt_str("Smooth type"), g_cfg.legitbot.weapon[hooks::legit_weapon].smooth_type, LegitSmooth, ARRAYSIZE(LegitSmooth));
		ImGui::Spacing();

		ImGui::SliderFloat(crypt_str("Smooth amount"), &g_cfg.legitbot.weapon[hooks::legit_weapon].smooth, 1.f, 12.f, crypt_str("%.1f"));

		ImGui::Spacing();


		draw_combo(crypt_str("RCS type"), g_cfg.legitbot.weapon[hooks::legit_weapon].rcs_type, RCSType, ARRAYSIZE(RCSType));
		ImGui::Spacing();

		ImGui::SliderFloat(crypt_str("RCS amount"), &g_cfg.legitbot.weapon[hooks::legit_weapon].rcs, 0.f, 100.f, crypt_str("%.0f%%"), 1.f);
		ImGui::Spacing();


		if (g_cfg.legitbot.weapon[hooks::legit_weapon].rcs > 0)
		{
			ImGui::SliderFloat(crypt_str("RCS custom field of view"), &g_cfg.legitbot.weapon[hooks::legit_weapon].custom_rcs_fov, 0.f, 30.f, (!g_cfg.legitbot.weapon[hooks::legit_weapon].custom_rcs_fov ? crypt_str("None") : crypt_str("%.2f"))); //-V550
			ImGui::Spacing();

			ImGui::SliderFloat(crypt_str("RCS Custom smooth"), &g_cfg.legitbot.weapon[hooks::legit_weapon].custom_rcs_smooth, 0.f, 12.f, (!g_cfg.legitbot.weapon[hooks::legit_weapon].custom_rcs_smooth ? crypt_str("None") : crypt_str("%.1f"))); //-V550
			ImGui::Spacing();

		}

		ImGui::Spacing();

		ImGui::SliderInt(crypt_str("Automatic wall damage"), &g_cfg.legitbot.weapon[hooks::legit_weapon].awall_dmg, 0, 100, crypt_str("%d"));
		ImGui::Spacing();
		ImGui::SliderInt(crypt_str("Automatic fire hitchance"), &g_cfg.legitbot.weapon[hooks::legit_weapon].autofire_hitchance, 0, 100, crypt_str("%d"));
		ImGui::Spacing();
		ImGui::SliderFloat(crypt_str("Target switch delay"), &g_cfg.legitbot.weapon[hooks::legit_weapon].target_switch_delay, 0.f, 1.f);
		ImGui::Spacing();
		ImGui::Spacing();
	}
	ImGui::EndChild();
}

void c_menu::Aimbot()
{
	static auto type = 0;
	const char* rage_weapons[8] = { crypt_str("Revolver / Deagle"), crypt_str("Pistols"), crypt_str("SMGs"), crypt_str("Rifles"), crypt_str("Auto"), crypt_str("Scout"), crypt_str("AWP"), crypt_str("Heavy") };
	//	child_title(crypt_str("Ragebot"));

		//ImGui::SetCursorPos({ 10, 100 });
		//ImGui::BeginChild(2, { 166,276 });
		//{




		//}
		//ImGui::EndChild();

		//ImGui::SetCursorPos(ImVec2(10, 100));
	ImGui::BeginChildcustom("pinto", ImVec2(290, 490), true);
	{
		ImGui::Checkbox(crypt_str("Enable"), &g_cfg.ragebot.enable);

		ImGui::Spacing();
		ImGui::Spacing();

		if (g_cfg.ragebot.enable)
			g_cfg.legitbot.enabled = false;

		ImGui::SliderInt(crypt_str("Field of view"), &g_cfg.ragebot.field_of_view, 1, 180, ("%d°"));

		ImGui::Checkbox(crypt_str("Silent aim"), &g_cfg.ragebot.silent_aim);
		ImGui::Spacing;

		ImGui::Checkbox(crypt_str("Automatic wall"), &g_cfg.ragebot.autowall);


		ImGui::Checkbox(crypt_str("Aimbot with zeus"), &g_cfg.ragebot.zeus_bot);


		ImGui::Checkbox(crypt_str("Aimbot with knife"), &g_cfg.ragebot.knife_bot);


		ImGui::Checkbox(crypt_str("Automatic fire"), &g_cfg.ragebot.autoshoot);


		//  ImGui::Checkbox(crypt_str("Automatic scope"), g_cfg.ragebot.weapon.autoscope);
		//	ImGui::Checkbox(crypt_str("Pitch desync correction"), &g_cfg.ragebot.pitch_antiaim_correction);


		ImGui::Checkbox(crypt_str("Double tap"), &g_cfg.ragebot.double_tap);

		if (g_cfg.ragebot.double_tap)
		{

			ImGui::SameLine();
			draw_keybind(crypt_str(""), &g_cfg.ragebot.double_tap_key, crypt_str("##HOTKEY_DT"));
			ImGui::Checkbox(crypt_str("Slow teleport"), &g_cfg.ragebot.slow_teleport);
		}

		ImGui::Checkbox(crypt_str("Hide shots"), &g_cfg.antiaim.hide_shots);



		if (g_cfg.antiaim.hide_shots)
		{

			ImGui::SameLine();
			draw_keybind(crypt_str(""), &g_cfg.antiaim.hide_shots_key, crypt_str("##HOTKEY_HIDESHOTS"));
		}



		////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


	}
	ImGui::EndChild();
	ImGui::SameLine(300);
	ImGui::BeginChildcustom("xerereca", ImVec2(290, 490), true);
	{
		ImGui::Checkbox(crypt_str("Enable Anti Aim"), &g_cfg.antiaim.enable);
		if (g_cfg.antiaim.enable)
		{
			ImGui::Spacing();
			ImGui::Spacing();
			draw_combo(crypt_str("Anti-aim type"), g_cfg.antiaim.antiaim_type, antiaim_type, ARRAYSIZE(antiaim_type));

			if (g_cfg.antiaim.antiaim_type)
			{
				padding(0, 3);
				draw_combo(crypt_str("Desync"), g_cfg.antiaim.desync, desync, ARRAYSIZE(desync));
				ImGui::Spacing();
				ImGui::Spacing();
				if (g_cfg.antiaim.desync)
				{
					padding(0, 3);
					draw_combo(crypt_str("LBY type"), g_cfg.antiaim.legit_lby_type, lby_type, ARRAYSIZE(lby_type));
					ImGui::Spacing();
					ImGui::Spacing();
					if (g_cfg.antiaim.desync == 1)
					{
						draw_keybind(crypt_str("Invert desync"), &g_cfg.antiaim.flip_desync, crypt_str("##HOTKEY_INVERT_DESYNC"));
					}
				}
			}
			else
			{
				draw_combo(crypt_str("Movement type"), type, movement_type, ARRAYSIZE(movement_type));
				ImGui::Spacing();
				ImGui::Spacing();
				padding(0, 3);
				draw_combo(crypt_str("Pitch"), g_cfg.antiaim.type[type].pitch, pitch, ARRAYSIZE(pitch));
				ImGui::Spacing();
				ImGui::Spacing();
				padding(0, 3);
				draw_combo(crypt_str("Yaw"), g_cfg.antiaim.type[type].yaw, yaw, ARRAYSIZE(yaw));
				ImGui::Spacing();
				ImGui::Spacing();
				padding(0, 3);
				draw_combo(crypt_str("Base angle"), g_cfg.antiaim.type[type].base_angle, baseangle, ARRAYSIZE(baseangle));
				ImGui::Spacing();
				ImGui::Spacing();

				if (g_cfg.antiaim.type[type].yaw)
				{
					ImGui::Spacing();
					ImGui::Spacing();
					ImGui::SliderInt(g_cfg.antiaim.type[type].yaw == 1 ? crypt_str("Jitter range") : crypt_str("Spin range"), &g_cfg.antiaim.type[type].range, 1, 180);

					if (g_cfg.antiaim.type[type].yaw == 2)
						ImGui::SliderInt(crypt_str("Spin speed"), &g_cfg.antiaim.type[type].speed, 1, 15);
					ImGui::Spacing();
					ImGui::Spacing();
					padding(0, 3);
				}

				draw_combo(crypt_str("Desync"), g_cfg.antiaim.type[type].desync, desync, ARRAYSIZE(desync));
				ImGui::Spacing();
				ImGui::Spacing();
				if (g_cfg.antiaim.type[type].desync)
				{
					if (type == ANTIAIM_STAND)
					{
						padding(0, 3);
						draw_combo(crypt_str("LBY type"), g_cfg.antiaim.lby_type, lby_type, ARRAYSIZE(lby_type));
						ImGui::Spacing();
						ImGui::Spacing();
					}

					if (type != ANTIAIM_STAND || !g_cfg.antiaim.lby_type)
					{
						ImGui::SliderInt(crypt_str("Desync range"), &g_cfg.antiaim.type[type].desync_range, 1, 60);
						ImGui::Spacing();
						ImGui::Spacing();
						ImGui::SliderInt(crypt_str("Inverted desync range"), &g_cfg.antiaim.type[type].inverted_desync_range, 1, 60);
						ImGui::Spacing();
						ImGui::Spacing();
						ImGui::SliderInt(crypt_str("Body lean"), &g_cfg.antiaim.type[type].body_lean, 0, 100);
						ImGui::Spacing();
						ImGui::Spacing();
						ImGui::SliderInt(crypt_str("Inverted body lean"), &g_cfg.antiaim.type[type].inverted_body_lean, 0, 100);
						ImGui::Spacing();
						ImGui::Spacing();
					}

					if (g_cfg.antiaim.type[type].desync == 1)
					{
						ImGui::Spacing();
						ImGui::Spacing();
						draw_keybind(crypt_str("Invert desync"), &g_cfg.antiaim.flip_desync, crypt_str("##HOTKEY_INVERT_DESYNC"));
					}
				}
				ImGui::Spacing();
				ImGui::Spacing();
				draw_keybind(crypt_str("Manual back"), &g_cfg.antiaim.manual_back, crypt_str("##HOTKEY_INVERT_BACK"));
				ImGui::Spacing();
				ImGui::Spacing();
				draw_keybind(crypt_str("Manual left"), &g_cfg.antiaim.manual_left, crypt_str("##HOTKEY_INVERT_LEFT"));
				ImGui::Spacing();
				ImGui::Spacing();
				draw_keybind(crypt_str("Manual right"), &g_cfg.antiaim.manual_right, crypt_str("##HOTKEY_INVERT_RIGHT"));
				ImGui::Spacing();
				ImGui::Spacing();

				if (g_cfg.antiaim.manual_back.key > KEY_NONE && g_cfg.antiaim.manual_back.key < KEY_MAX || g_cfg.antiaim.manual_left.key > KEY_NONE && g_cfg.antiaim.manual_left.key < KEY_MAX || g_cfg.antiaim.manual_right.key > KEY_NONE && g_cfg.antiaim.manual_right.key < KEY_MAX)
				{
					ImGui::Spacing();
					ImGui::Spacing();
					ImGui::Checkbox(crypt_str("Manuals indicator"), &g_cfg.antiaim.flip_indicator);
					ImGui::SameLine();
					ImGui::ColorEdit(crypt_str("##invc"), &g_cfg.antiaim.flip_indicator_color, ALPHA);
				}
			}

		}
		

		ImGui::Spacing();
		ImGui::Spacing();
		draw_combo(crypt_str("Current weapon"), hooks::rage_weapon, rage_weapons, ARRAYSIZE(rage_weapons));
		ImGui::Spacing();
		ImGui::Spacing();
		draw_combo(crypt_str("Target selection"), g_cfg.ragebot.weapon[hooks::rage_weapon].selection_type, selection, ARRAYSIZE(selection));
		ImGui::Spacing();
		ImGui::Spacing();
		padding(0, 3);
		draw_multicombo(crypt_str("Hitboxes"), g_cfg.ragebot.weapon[hooks::rage_weapon].hitboxes, hitboxes, ARRAYSIZE(hitboxes), preview);
		ImGui::Spacing();
		ImGui::Spacing();
		ImGui::Checkbox(crypt_str("Automatic stop"), &g_cfg.ragebot.weapon[hooks::rage_weapon].autostop);


		if (g_cfg.ragebot.weapon[hooks::rage_weapon].autostop)
			draw_multicombo(crypt_str("Modifiers"), g_cfg.ragebot.weapon[hooks::rage_weapon].autostop_modifiers, autostop_modifiers, ARRAYSIZE(autostop_modifiers), preview);

		ImGui::Checkbox(crypt_str("Hitchance"), &g_cfg.ragebot.weapon[hooks::rage_weapon].hitchance);


		if (g_cfg.ragebot.weapon[hooks::rage_weapon].hitchance)
			ImGui::SliderInt(crypt_str("Hitchance amount"), &g_cfg.ragebot.weapon[hooks::rage_weapon].hitchance_amount, 1, 100);

		if (g_cfg.ragebot.double_tap && hooks::rage_weapon <= 4)
		{
			//ImGui::Checkbox(crypt_str("Double tap hitchance"), &g_cfg.ragebot.weapon[hooks::rage_weapon].double_tap_hitchance);

			//if (g_cfg.ragebot.weapon[hooks::rage_weapon].double_tap_hitchance)
			//	ImGui::SliderInt(crypt_str("Double tap hitchance amount"), &g_cfg.ragebot.weapon[hooks::rage_weapon].double_tap_hitchance_amount, 1, 100);
		}

		ImGui::SliderInt(crypt_str("Minimum visible damage"), &g_cfg.ragebot.weapon[hooks::rage_weapon].minimum_visible_damage, 1, 120);

		if (g_cfg.ragebot.autowall)
			ImGui::SliderInt(crypt_str("Minimum wall damage"), &g_cfg.ragebot.weapon[hooks::rage_weapon].minimum_damage, 1, 120);
		ImGui::Spacing();
		ImGui::Spacing();
		draw_keybind(crypt_str("Damage override"), &g_cfg.ragebot.weapon[hooks::rage_weapon].damage_override_key, crypt_str("##HOTKEY__DAMAGE_OVERRIDE"));

		if (g_cfg.ragebot.weapon[hooks::rage_weapon].damage_override_key.key > KEY_NONE && g_cfg.ragebot.weapon[hooks::rage_weapon].damage_override_key.key < KEY_MAX)
			ImGui::SliderInt(crypt_str("Minimum override damage"), &g_cfg.ragebot.weapon[hooks::rage_weapon].minimum_override_damage, 1, 120);
		ImGui::Spacing();
		ImGui::Spacing();

		ImGui::Checkbox(crypt_str("Static point scale"), &g_cfg.ragebot.weapon[hooks::rage_weapon].static_point_scale);
		ImGui::Spacing();

		if (g_cfg.ragebot.weapon[hooks::rage_weapon].static_point_scale)
		{

			ImGui::SliderFloat(crypt_str("Head scale"), &g_cfg.ragebot.weapon[hooks::rage_weapon].head_scale, 0.0f, 1.0f, g_cfg.ragebot.weapon[hooks::rage_weapon].head_scale ? crypt_str("%.2f") : crypt_str("None"));
			ImGui::Spacing();

			ImGui::SliderFloat(crypt_str("Body scale"), &g_cfg.ragebot.weapon[hooks::rage_weapon].body_scale, 0.0f, 1.0f, g_cfg.ragebot.weapon[hooks::rage_weapon].body_scale ? crypt_str("%.2f") : crypt_str("None"));
			ImGui::Spacing();

		}


		ImGui::Checkbox(crypt_str("Enable max misses"), &g_cfg.ragebot.weapon[hooks::rage_weapon].max_misses);
		ImGui::Spacing();

		if (g_cfg.ragebot.weapon[hooks::rage_weapon].max_misses)
			ImGui::SliderInt(crypt_str("Max misses"), &g_cfg.ragebot.weapon[hooks::rage_weapon].max_misses_amount, 0, 6);


		ImGui::Checkbox(crypt_str("Prefer safe points"), &g_cfg.ragebot.weapon[hooks::rage_weapon].prefer_safe_points);

		ImGui::Checkbox(crypt_str("Prefer body aim"), &g_cfg.ragebot.weapon[hooks::rage_weapon].prefer_body_aim);


		draw_keybind(crypt_str("Force safe points"), &g_cfg.ragebot.safe_point_key, crypt_str("##HOKEY_FORCE_SAFE_POINTS"));
		ImGui::Spacing();
		ImGui::Spacing();
		draw_keybind(crypt_str("Force body aim"), &g_cfg.ragebot.body_aim_key, crypt_str("##HOKEY_FORCE_BODY_AIM"));
		ImGui::Spacing();
		ImGui::Spacing();
	}
	ImGui::EndChild();
	//ImGui::SameLine();




}

void c_menu::Antiaim()
{
	ImGui::PushFont(skeet_menu);

	ImGuiStyle* style = &ImGui::GetStyle();
	InsertSpacer("Top Spacer");

	ImGui::Columns(2, NULL, false);
	{
		InsertGroupBoxLeft("Anti-aim", 506.f);
		{
			static auto type = 0;

			style->ItemSpacing = ImVec2(4, 2);
			style->WindowPadding = ImVec2(4, 4);
			style->ButtonTextAlign = ImVec2(0.5f, -2.0f);
			ImGui::CustomSpacing(9.f);

			InsertCheckbox(("Enable"), g_cfg.antiaim.enable);

			InsertCombo(("Anti-aim type"), g_cfg.antiaim.antiaim_type, antiaim_type, ARRAYSIZE(antiaim_type));

			if (g_cfg.antiaim.antiaim_type)
			{
				InsertCombo(("Desync"), g_cfg.antiaim.desync, desync, ARRAYSIZE(desync));

				if (g_cfg.antiaim.desync)
				{
					InsertCombo(("LBY type"), g_cfg.antiaim.legit_lby_type, lby_type, ARRAYSIZE(lby_type));

					if (g_cfg.antiaim.desync == 1)
					{
						draw_keybind(("Invert desync"), &g_cfg.antiaim.flip_desync, ("##HOTKEY_INVERT_DESYNC"));
					}
				}
			}
			else
			{
				InsertCombo(("Movement type"), type, movement_type, ARRAYSIZE(movement_type));
				InsertCombo(("Pitch"), g_cfg.antiaim.type[type].pitch, pitch, ARRAYSIZE(pitch));
				InsertCombo(("Yaw"), g_cfg.antiaim.type[type].yaw, yaw, ARRAYSIZE(yaw));
				InsertCombo(("Base angle"), g_cfg.antiaim.type[type].base_angle, baseangle, ARRAYSIZE(baseangle));

				if (g_cfg.antiaim.type[type].yaw)
				{
					InsertSliderInt(g_cfg.antiaim.type[type].yaw == 1 ? (" Jitter range") : (" Spin range"), g_cfg.antiaim.type[type].range, 1, 180, ("%d°"));

					if (g_cfg.antiaim.type[type].yaw == 2)
						InsertSliderInt((" Spin speed"), g_cfg.antiaim.type[type].speed, 1, 15, ("%d°"));

				}

				InsertCombo(("Desync"), g_cfg.antiaim.type[type].desync, desync, ARRAYSIZE(desync));

				if (g_cfg.antiaim.type[type].desync)
				{
					if (type == ANTIAIM_STAND)
					{
						InsertCombo(("LBY type"), g_cfg.antiaim.lby_type, lby_type, ARRAYSIZE(lby_type));
					}

					if (type != ANTIAIM_STAND || !g_cfg.antiaim.lby_type)
					{
						InsertSliderInt((" Desync range"), g_cfg.antiaim.type[type].desync_range, 1, 60, ("%d°"));
						InsertSliderInt((" Inverted desync range"), g_cfg.antiaim.type[type].inverted_desync_range, 1, 60, ("%d°"));
						InsertSliderInt((" Body lean"), g_cfg.antiaim.type[type].body_lean, 0, 100, ("%d°"));
						InsertSliderInt((" Inverted body lean"), g_cfg.antiaim.type[type].inverted_body_lean, 0, 100, ("%d°"));
					}

					if (g_cfg.antiaim.type[type].desync == 1)
					{
						ImGui::NewLine(); ImGui::SameLine(42.f); ImGui::PushItemWidth(158.f);
						draw_keybind(("Invert desync"), &g_cfg.antiaim.flip_desync, ("##HOTKEY_INVERT_DESYNC"));
					}
				}

				ImGui::NewLine(); ImGui::SameLine(42.f); ImGui::PushItemWidth(158.f);
				draw_keybind(("Manual back"), &g_cfg.antiaim.manual_back, ("##HOTKEY_INVERT_BACK"));

				ImGui::NewLine(); ImGui::SameLine(42.f); ImGui::PushItemWidth(158.f);
				draw_keybind(("Manual left"), &g_cfg.antiaim.manual_left, ("##HOTKEY_INVERT_LEFT"));

				ImGui::NewLine(); ImGui::SameLine(42.f); ImGui::PushItemWidth(158.f);
				draw_keybind(("Manual right"), &g_cfg.antiaim.manual_right, ("##HOTKEY_INVERT_RIGHT"));


				if (g_cfg.antiaim.manual_back.key > KEY_NONE && g_cfg.antiaim.manual_back.key < KEY_MAX || g_cfg.antiaim.manual_left.key > KEY_NONE && g_cfg.antiaim.manual_left.key < KEY_MAX || g_cfg.antiaim.manual_right.key > KEY_NONE && g_cfg.antiaim.manual_right.key < KEY_MAX)
				{
					InsertCheckbox(("Manuals indicator"), g_cfg.antiaim.flip_indicator);
					ImGui::SameLine();
					ImGui::ColorEdit(("##invc"), &g_cfg.antiaim.flip_indicator_color, ALPHA);
				}

	}
		} InsertEndGroupBoxLeft("Anti-aim Cover", "Anti-aim");

		InsertGroupBoxRight("Other", 153.f);
		{

			style->ItemSpacing = ImVec2(4, 2);
			style->WindowPadding = ImVec2(4, 4);
			style->ButtonTextAlign = ImVec2(0.5f, -2.0f);
			ImGui::CustomSpacing(9.f);;
			ImGui::NewLine(); ImGui::SameLine(42.f); ImGui::PushItemWidth(158.f);
			draw_multicombo(crypt_str("Inverted condition"), g_cfg.antiaim.inverted_condition, condition_switch, ARRAYSIZE(condition_switch), preview);

		}
		InsertEndGroupBoxRight("Other Cover", "Other");

	}
	ImGui::PopFont();
}

void c_menu::Visuals()
{
	ImGui::BeginChildcustom("pinto13231", ImVec2(290, 490), true);
	{
	ImGui::Checkbox(crypt_str("Enable"), &g_cfg.player.enable);

	draw_multicombo(crypt_str("Indicators"), g_cfg.esp.indicators, indicators, ARRAYSIZE(indicators), preview);
	padding(0, 3);
	ImGui::Spacing();

	draw_multicombo(crypt_str("Removals"), g_cfg.esp.removals, removals, ARRAYSIZE(removals), preview);
	ImGui::Spacing();

	if (g_cfg.esp.removals[REMOVALS_ZOOM])
		ImGui::Checkbox(crypt_str("Fix zoom sensivity"), &g_cfg.esp.fix_zoom_sensivity);

	ImGui::Checkbox(crypt_str("Grenade prediction"), &g_cfg.esp.grenade_prediction);
	ImGui::SameLine(0); ImGui::PushItemWidth(400.f);
	ImGui::ColorEdit(crypt_str("##grenpredcolor"), &g_cfg.esp.grenade_prediction_color, ALPHA);
	if (g_cfg.esp.grenade_prediction)
	{
		ImGui::Checkbox(crypt_str("On click"), &g_cfg.esp.on_click);
		ImGui::Text(crypt_str("Tracer color "));
		ImGui::SameLine(0); ImGui::PushItemWidth(400.f);
		ImGui::ColorEdit(crypt_str("##tracergrenpredcolor"), &g_cfg.esp.grenade_prediction_tracer_color, ALPHA);

	}

	ImGui::Checkbox(crypt_str("Grenade projectiles"), &g_cfg.esp.projectiles);

	ImGui::Spacing();
	if (g_cfg.esp.projectiles)
		draw_multicombo(crypt_str("Grenade ESP"), g_cfg.esp.grenade_esp, proj_combo, ARRAYSIZE(proj_combo), preview);

	ImGui::Spacing();
	if (g_cfg.esp.grenade_esp[GRENADE_ICON] || g_cfg.esp.grenade_esp[GRENADE_TEXT])
	{
		ImGui::Text(crypt_str("Color "));
		ImGui::SameLine(0); ImGui::PushItemWidth(400.f);
		ImGui::ColorEdit(crypt_str("##projectcolor"), &g_cfg.esp.projectiles_color, ALPHA);
	}

	if (g_cfg.esp.grenade_esp[GRENADE_BOX])
	{
		ImGui::Text(crypt_str("Box color "));
		ImGui::SameLine(0); ImGui::PushItemWidth(400.f);
		ImGui::ColorEdit(crypt_str("##grenade_box_color"), &g_cfg.esp.grenade_box_color, ALPHA);

	}

	if (g_cfg.esp.grenade_esp[GRENADE_GLOW])
	{
		ImGui::Text(crypt_str("Glow color "));
		ImGui::SameLine(0); ImGui::PushItemWidth(400.f);
		ImGui::ColorEdit(crypt_str("##grenade_glow_color"), &g_cfg.esp.grenade_glow_color, ALPHA);
	}

	ImGui::Checkbox(crypt_str("Fire timer"), &g_cfg.esp.molotov_timer);
	ImGui::SameLine(0); ImGui::PushItemWidth(400.f);
	ImGui::ColorEdit(crypt_str("##molotovcolor"), &g_cfg.esp.molotov_timer_color, ALPHA);
	ImGui::Checkbox(crypt_str("Smoke timer"), &g_cfg.esp.smoke_timer);
	ImGui::SameLine(0); ImGui::PushItemWidth(400.f);
	ImGui::ColorEdit(crypt_str("##smokecolor"), &g_cfg.esp.smoke_timer_color, ALPHA);

	ImGui::Checkbox(crypt_str("Bomb indicator"), &g_cfg.esp.bomb_timer);

	ImGui::Spacing();
	draw_multicombo(crypt_str("Weapon ESP"), g_cfg.esp.weapon, weaponesp, ARRAYSIZE(weaponesp), preview);
	ImGui::Spacing();


	if (g_cfg.esp.weapon[WEAPON_ICON] || g_cfg.esp.weapon[WEAPON_TEXT] || g_cfg.esp.weapon[WEAPON_DISTANCE])
	{
		ImGui::Text(crypt_str("Color "));
		ImGui::SameLine(0); ImGui::PushItemWidth(400.f);
		ImGui::ColorEdit(crypt_str("##weaponcolor"), &g_cfg.esp.weapon_color, ALPHA);

	}

	if (g_cfg.esp.weapon[WEAPON_BOX])
	{
		ImGui::Text(crypt_str("Box color "));
		ImGui::SameLine(0); ImGui::PushItemWidth(400.f);
		ImGui::ColorEdit(crypt_str("##weaponboxcolor"), &g_cfg.esp.box_color, ALPHA);

	}

	if (g_cfg.esp.weapon[WEAPON_GLOW])
	{
		ImGui::Text(crypt_str("Glow color "));
		ImGui::SameLine(0); ImGui::PushItemWidth(400.f);
		ImGui::ColorEdit(crypt_str("##weaponglowcolor"), &g_cfg.esp.weapon_glow_color, ALPHA);

	}

	if (g_cfg.esp.weapon[WEAPON_AMMO])
	{
		ImGui::Text(crypt_str("Ammo bar color "));
		ImGui::SameLine(0); ImGui::PushItemWidth(400.f);
		ImGui::ColorEdit(crypt_str("##weaponammocolor"), &g_cfg.esp.weapon_ammo_color, ALPHA);

	}

	ImGui::Checkbox(crypt_str("Client bullet impacts"), &g_cfg.esp.client_bullet_impacts);
	ImGui::SameLine(0); ImGui::PushItemWidth(400.f);
	ImGui::ColorEdit(crypt_str("##clientbulletimpacts"), &g_cfg.esp.client_bullet_impacts_color, ALPHA);

	ImGui::Checkbox(crypt_str("Server bullet impacts"), &g_cfg.esp.server_bullet_impacts);
	ImGui::SameLine(0); ImGui::PushItemWidth(400.f);
	ImGui::ColorEdit(crypt_str("##serverbulletimpacts"), &g_cfg.esp.server_bullet_impacts_color, ALPHA);

	ImGui::Checkbox(crypt_str("Local bullet tracers"), &g_cfg.esp.bullet_tracer);
	ImGui::SameLine(0); ImGui::PushItemWidth(400.f);
	ImGui::ColorEdit(crypt_str("##bulltracecolor"), &g_cfg.esp.bullet_tracer_color, ALPHA);
	ImGui::Checkbox(crypt_str("Enemy bullet tracers"), &g_cfg.esp.enemy_bullet_tracer);
	ImGui::SameLine(0); ImGui::PushItemWidth(400.f);

	ImGui::ColorEdit(crypt_str("##enemybulltracecolor"), &g_cfg.esp.enemy_bullet_tracer_color, ALPHA);

	ImGui::Spacing();
	draw_multicombo(crypt_str("Hit marker"), g_cfg.esp.hitmarker, hitmarkers, ARRAYSIZE(hitmarkers), preview);
	ImGui::Spacing();

	ImGui::Checkbox(crypt_str("Damage marker"), &g_cfg.esp.damage_marker);

	ImGui::Checkbox(crypt_str("Kill effect"), &g_cfg.esp.kill_effect);

	if (g_cfg.esp.kill_effect)
		ImGui::SliderFloat(crypt_str("Duration"), &g_cfg.esp.kill_effect_duration, 0.01f, 3.0f);

	ImGui::SliderInt(crypt_str("Field of view"), &g_cfg.esp.fov, 0, 89);

	ImGui::Checkbox(crypt_str("Taser range"), &g_cfg.esp.taser_range);

	ImGui::Checkbox(crypt_str("Show spread"), &g_cfg.esp.show_spread);
	ImGui::SameLine(20);
	ImGui::ColorEdit(crypt_str("##spredcolor"), &g_cfg.esp.show_spread_color, ALPHA);

	ImGui::Checkbox(crypt_str("Penetration crosshair"), &g_cfg.esp.penetration_reticle);

	ImGui::Checkbox(crypt_str("Rare animations"), &g_cfg.skins.rare_animations);

	ImGui::SliderInt(crypt_str("Viewmodel field of view"), &g_cfg.esp.viewmodel_fov, 0, 89);
	ImGui::Spacing();

	ImGui::SliderInt(crypt_str("Viewmodel X"), &g_cfg.esp.viewmodel_x, -50, 50);
	ImGui::Spacing();

	ImGui::SliderInt(crypt_str("Viewmodel Y"), &g_cfg.esp.viewmodel_y, -50, 50);
	ImGui::Spacing();

	ImGui::SliderInt(crypt_str("Viewmodel Z"), &g_cfg.esp.viewmodel_z, -50, 50);
	ImGui::Spacing();

	ImGui::SliderInt(crypt_str("Viewmodel roll"), &g_cfg.esp.viewmodel_roll, -180, 180);
	ImGui::Spacing();

	}
	ImGui::EndChild();
	ImGui::SameLine(300);
	ImGui::BeginChildcustom("xerereca31231", ImVec2(290, 490), true);
	{

		ImGui::Checkbox(crypt_str("Arms chams"), &g_cfg.esp.arms_chams);
		ImGui::SameLine(0); ImGui::PushItemWidth(2000.f);
		ImGui::ColorEdit(crypt_str("##armscolor"), &g_cfg.esp.arms_chams_color, ALPHA);
		ImGui::Spacing();


		draw_combo(crypt_str("Arms chams material"), g_cfg.esp.arms_chams_type, chamstype, ARRAYSIZE(chamstype));
		ImGui::Spacing();

		if (g_cfg.esp.arms_chams_type != 6)
		{
			ImGui::Checkbox(crypt_str("Arms double material "), &g_cfg.esp.arms_double_material);
			ImGui::SameLine(0); ImGui::PushItemWidth(2000.f);
			ImGui::ColorEdit(crypt_str("##armsdoublematerial"), &g_cfg.esp.arms_double_material_color, ALPHA);

		}

		ImGui::Checkbox(crypt_str("Arms animated material "), &g_cfg.esp.arms_animated_material);
		ImGui::SameLine(0); ImGui::PushItemWidth(2000.f);
		ImGui::ColorEdit(crypt_str("##armsanimatedmaterial"), &g_cfg.esp.arms_animated_material_color, ALPHA);

		ImGui::Checkbox(crypt_str("Weapon chams"), &g_cfg.esp.weapon_chams);
		ImGui::SameLine(0); ImGui::PushItemWidth(2000.f);
		ImGui::ColorEdit(crypt_str("##weaponchamscolors"), &g_cfg.esp.weapon_chams_color, ALPHA);

		ImGui::Spacing();
		draw_combo(crypt_str("Weapon chams material"), g_cfg.esp.weapon_chams_type, chamstype, ARRAYSIZE(chamstype));
		ImGui::Spacing();

		if (g_cfg.esp.weapon_chams_type != 6)
		{
			ImGui::Checkbox(crypt_str("Double material "), &g_cfg.esp.weapon_double_material);
			ImGui::SameLine(0); ImGui::PushItemWidth(2000.f);
			ImGui::ColorEdit(crypt_str("##weapondoublematerial"), &g_cfg.esp.weapon_double_material_color, ALPHA);
		}

		ImGui::Checkbox(crypt_str("Animated material "), &g_cfg.esp.weapon_animated_material);
		ImGui::SameLine(0); ImGui::PushItemWidth(2000.f);
		ImGui::ColorEdit(crypt_str("##weaponanimatedmaterial"), &g_cfg.esp.weapon_animated_material_color, ALPHA);

		ImGui::Checkbox(crypt_str("Rain"), &g_cfg.esp.rain);

		ImGui::Checkbox(crypt_str("Full bright"), &g_cfg.esp.bright);

		draw_combo(crypt_str("Skybox"), g_cfg.esp.skybox, skybox, ARRAYSIZE(skybox));

		ImGui::Text(crypt_str("Color "));
		ImGui::SameLine(0); ImGui::PushItemWidth(2000.f);
		ImGui::ColorEdit(crypt_str("##skyboxcolor"), &g_cfg.esp.skybox_color, NOALPHA);

		if (g_cfg.esp.skybox == 21)
		{
			static char sky_custom[64] = "\0";

			if (!g_cfg.esp.custom_skybox.empty())
				strcpy_s(sky_custom, sizeof(sky_custom), g_cfg.esp.custom_skybox.c_str());

			ImGui::Text(crypt_str("Name"));
			ImGui::PushStyleVar(ImGuiStyleVar_FrameBorderSize, 1.f);

			if (ImGui::InputText(crypt_str("##customsky"), sky_custom, sizeof(sky_custom)))
				g_cfg.esp.custom_skybox = sky_custom;

			ImGui::PopStyleVar();
		}

		ImGui::Checkbox(crypt_str("Color modulation"), &g_cfg.esp.nightmode);


		if (g_cfg.esp.nightmode)
		{

			ImGui::Text(crypt_str("World color "));
			ImGui::SameLine(0); ImGui::PushItemWidth(2000.f);
			ImGui::ColorEdit(crypt_str("##worldcolor"), &g_cfg.esp.world_color, ALPHA);

			ImGui::Text(crypt_str("Props color "));
			ImGui::SameLine(0); ImGui::PushItemWidth(2000.f);
			ImGui::ColorEdit(crypt_str("##propscolor"), &g_cfg.esp.props_color, ALPHA);

		}

		ImGui::Checkbox(crypt_str("World modulation"), &g_cfg.esp.world_modulation);


		if (g_cfg.esp.world_modulation)
		{
			ImGui::SliderFloat(crypt_str("Bloom"), &g_cfg.esp.bloom, 0.0f, 750.0f);
			ImGui::Spacing();

			ImGui::SliderFloat(crypt_str("Exposure"), &g_cfg.esp.exposure, 0.0f, 2000.0f);
			ImGui::Spacing();

			ImGui::SliderFloat(crypt_str("Ambient"), &g_cfg.esp.ambient, 0.0f, 1500.0f);
			ImGui::Spacing();

		}

		ImGui::Checkbox(crypt_str("Fog modulation"), &g_cfg.esp.fog);

		if (g_cfg.esp.fog)
		{

			ImGui::Spacing();
			ImGui::SliderInt(crypt_str("Distance"), &g_cfg.esp.fog_distance, 0, 2500);

			ImGui::Spacing();
			ImGui::SliderInt(crypt_str("Density"), &g_cfg.esp.fog_density, 0, 100);

			ImGui::Spacing();

			ImGui::Text(crypt_str("Color "));

			ImGui::SameLine(0); ImGui::PushItemWidth(2000.f);
			ImGui::ColorEdit(crypt_str("##fogcolor"), &g_cfg.esp.fog_color, NOALPHA);
			ImGui::Spacing();

		}

	}
	ImGui::EndChild();
}

void c_menu::Misc()
{

	static int model_type = 0;
	auto player = players_section;


	ImGui::BeginChildcustom("pinto1233231", ImVec2(290, 490), true);
	{

		ImGui::Checkbox(crypt_str("Enable"), &g_cfg.player.enable);
		ImGui::Spacing();
		ImGui::Spacing();

		static const char* tsadt[] = { "Test","Enforcer",
										"Soldier",
										"Ground Rebel",
										"Maximus",
										"Osiris",
										"Slingshot",
										"Dragomir",
										"Blackwolf",
										"Prof. Shahmat",
										"Rezan The Ready",
										"Doctor Romanov",
										"Mr. Muhlik"
										"Seal Team 6",
										"3rd Commando",
										"Operator FBI",
										"Squadron Officer",
										"Markus Delrow",
										"Buckshot",
										"McCoy",
										"Commander Ricksaw",
										"Agent Ava" };

		draw_combo(crypt_str("Model type show"), model_type, tsadt, ARRAYSIZE(tsadt));

		ImGui::Spacing();
		ImGui::Spacing();






		if (player == ENEMY)
		{
			ImGui::Checkbox(crypt_str("OOF arrows"), &g_cfg.player.arrows);
			ImGui::SameLine(0); ImGui::PushItemWidth(2000.f);
			ImGui::ColorEdit(crypt_str("##arrowscolor"), &g_cfg.player.arrows_color, ALPHA);

			if (g_cfg.player.arrows)
			{

				ImGui::SliderInt(crypt_str("Arrows distance"), &g_cfg.player.distance, 1, 100, "");
				ImGui::SliderInt(crypt_str("Arrows size"), &g_cfg.player.size, 1, 100, "");
			}
		}

		ImGui::Checkbox(crypt_str("Bounding box"), &g_cfg.player.type[player].box);

		ImGui::SameLine(0); ImGui::PushItemWidth(2000.f);
		ImGui::ColorEdit(crypt_str("##boxcolor"), &g_cfg.player.type[player].box_color, ALPHA);
		ImGui::Checkbox(crypt_str("Name"), &g_cfg.player.type[player].name);
		ImGui::SameLine(0); ImGui::PushItemWidth(2000.f);
		ImGui::ColorEdit(crypt_str("##namecolor"), &g_cfg.player.type[player].name_color, ALPHA);
		ImGui::Checkbox(crypt_str("Health bar"), &g_cfg.player.type[player].health);
		ImGui::Checkbox(crypt_str("Health color"), &g_cfg.player.type[player].custom_health_color);
		ImGui::SameLine(0); ImGui::PushItemWidth(2000.f);
		ImGui::ColorEdit(crypt_str("##healthcolor"), &g_cfg.player.type[player].health_color, ALPHA);

		for (auto i = 0, j = 0; i < ARRAYSIZE(flags); i++)
		{
			if (g_cfg.player.type[player].flags[i])
			{
				if (j)
					preview += crypt_str(", ") + (std::string)flags[i];
				else
					preview = flags[i];

				j++;
			}
		}
		draw_multicombo(crypt_str("Flags"), g_cfg.player.type[player].flags, flags, ARRAYSIZE(flags), preview);
		draw_multicombo(crypt_str("Weapon"), g_cfg.player.type[player].weapon, weaponplayer, ARRAYSIZE(weaponplayer), preview);


		if (g_cfg.player.type[player].weapon[WEAPON_ICON] || g_cfg.player.type[player].weapon[WEAPON_TEXT])
		{
			ImGui::Text(crypt_str("Color "));
			ImGui::SameLine(0); ImGui::PushItemWidth(2000.f);
			ImGui::ColorEdit(crypt_str("##weapcolor"), &g_cfg.player.type[player].weapon_color, ALPHA);
		}

		ImGui::Checkbox(crypt_str("Skeleton"), &g_cfg.player.type[player].skeleton);
		ImGui::SameLine(0); ImGui::PushItemWidth(2000.f);
		ImGui::ColorEdit(crypt_str("##skeletoncolor"), &g_cfg.player.type[player].skeleton_color, ALPHA);
		ImGui::Checkbox(crypt_str("Ammo bar"), &g_cfg.player.type[player].ammo);
		ImGui::SameLine(0); ImGui::PushItemWidth(2000.f);
		ImGui::ColorEdit(crypt_str("##ammocolor"), &g_cfg.player.type[player].ammobar_color, ALPHA);
		ImGui::Checkbox(crypt_str("Footsteps"), &g_cfg.player.type[player].footsteps);
		ImGui::SameLine(0); ImGui::PushItemWidth(2000.f);
		ImGui::ColorEdit(crypt_str("##footstepscolor"), &g_cfg.player.type[player].footsteps_color, ALPHA);
		if (g_cfg.player.type[player].footsteps)
		{
			ImGui::SliderInt(crypt_str("Thickness"), &g_cfg.player.type[player].thickness, 1, 10);
			ImGui::SliderInt(crypt_str("Radius"), &g_cfg.player.type[player].radius, 50, 500);
		}

		if (player == ENEMY || player == TEAM)
		{

			ImGui::Checkbox(crypt_str("Snap lines"), &g_cfg.player.type[player].snap_lines);
			ImGui::SameLine(0); ImGui::PushItemWidth(2000.f);
			ImGui::ColorEdit(crypt_str("##snapcolor"), &g_cfg.player.type[player].snap_lines_color, ALPHA);

			if (player == ENEMY)
			{
				if (g_cfg.ragebot.enable)
				{
					ImGui::Checkbox(crypt_str("Aimbot points"), &g_cfg.player.show_multi_points);
					ImGui::SameLine(0); ImGui::PushItemWidth(2000.f);
					ImGui::ColorEdit(crypt_str("##showmultipointscolor"), &g_cfg.player.show_multi_points_color, ALPHA);
				}
				ImGui::Checkbox(crypt_str("Aimbot hitboxes"), &g_cfg.player.lag_hitbox);
				ImGui::SameLine(0); ImGui::PushItemWidth(2000.f);
				ImGui::ColorEdit(crypt_str("##lagcompcolor"), &g_cfg.player.lag_hitbox_color, ALPHA);
			}
		}
		else
		{

			draw_combo(crypt_str("Player model T"), g_cfg.player.player_model_t, player_model_t, ARRAYSIZE(player_model_t));
			padding(0, 3);
			draw_combo(crypt_str("Player model CT"), g_cfg.player.player_model_ct, player_model_ct, ARRAYSIZE(player_model_ct));

		}

	}
	ImGui::EndChild();
	ImGui::SameLine(300);
	ImGui::BeginChildcustom("xerereca3132231", ImVec2(290, 490), true);
	{

		if (player == LOCAL)
			draw_combo(crypt_str("Type"), g_cfg.player.local_chams_type, local_chams_type, ARRAYSIZE(local_chams_type));
		if (player != LOCAL || !g_cfg.player.local_chams_type)
			draw_multicombo(crypt_str("Chams"), g_cfg.player.type[player].chams, g_cfg.player.type[player].chams[PLAYER_CHAMS_VISIBLE] ? chamsvisact : chamsvis, g_cfg.player.type[player].chams[PLAYER_CHAMS_VISIBLE] ? ARRAYSIZE(chamsvisact) : ARRAYSIZE(chamsvis), preview);
		ImGui::Spacing();
		ImGui::Spacing();
		if (g_cfg.player.type[player].chams[PLAYER_CHAMS_VISIBLE] || player == LOCAL && g_cfg.player.local_chams_type) //-V648
		{
			if (player == LOCAL && g_cfg.player.local_chams_type)
			{
				ImGui::Spacing();
				ImGui::Checkbox(crypt_str("Enable desync chams"), &g_cfg.player.fake_chams_enable);
				ImGui::Checkbox(crypt_str("Visualize lag"), &g_cfg.player.visualize_lag);
				ImGui::Checkbox(crypt_str("Layered"), &g_cfg.player.layered);

				draw_combo(crypt_str("Player chams material"), g_cfg.player.fake_chams_type, chamstype, ARRAYSIZE(chamstype));
				ImGui::Text(crypt_str("Color "));
				ImGui::SameLine(0); ImGui::PushItemWidth(2000.f);
				ImGui::ColorEdit(crypt_str("##fakechamscolor"), &g_cfg.player.fake_chams_color, ALPHA);
				if (g_cfg.player.fake_chams_type != 6)
				{
					ImGui::Checkbox(crypt_str("Double material "), &g_cfg.player.fake_double_material);
					ImGui::SameLine(0); ImGui::PushItemWidth(2000.f);
					ImGui::ColorEdit(crypt_str("##doublematerialcolor"), &g_cfg.player.fake_double_material_color, ALPHA);
				}

				ImGui::Checkbox(crypt_str("Animated material"), &g_cfg.player.fake_animated_material);
				ImGui::SameLine(0); ImGui::PushItemWidth(2000.f);
				ImGui::ColorEdit(crypt_str("##animcolormat"), &g_cfg.player.fake_animated_material_color, ALPHA);
			}
			else
			{
				ImGui::Spacing();
				draw_combo(crypt_str("Player chams material"), g_cfg.player.type[player].chams_type, chamstype, ARRAYSIZE(chamstype));

				if (g_cfg.player.type[player].chams[PLAYER_CHAMS_VISIBLE])
				{
					ImGui::Spacing;
					ImGui::Text(crypt_str("Visible "));
					ImGui::SameLine(0); ImGui::PushItemWidth(2000.f);
					ImGui::ColorEdit(crypt_str("##chamsvisible"), &g_cfg.player.type[player].chams_color, ALPHA);
				}

				if (g_cfg.player.type[player].chams[PLAYER_CHAMS_VISIBLE] && g_cfg.player.type[player].chams[PLAYER_CHAMS_INVISIBLE])
				{

					ImGui::Text(crypt_str("Invisible "));
					ImGui::SameLine(0); ImGui::PushItemWidth(2000.f);
					ImGui::ColorEdit(crypt_str("##chamsinvisible"), &g_cfg.player.type[player].xqz_color, ALPHA);
				}

				if (g_cfg.player.type[player].chams_type != 6)
				{
					ImGui::Checkbox(crypt_str("Double material "), &g_cfg.player.type[player].double_material);
					ImGui::SameLine(0); ImGui::PushItemWidth(2000.f);
					ImGui::ColorEdit(crypt_str("##doublematerialcolor"), &g_cfg.player.type[player].double_material_color, ALPHA);
				}

				ImGui::Checkbox(crypt_str("Animated material"), &g_cfg.player.type[player].animated_material);
				ImGui::SameLine(0); ImGui::PushItemWidth(2000.f);
				ImGui::ColorEdit(crypt_str("##animcolormat"), &g_cfg.player.type[player].animated_material_color, ALPHA);

				if (player == ENEMY)
				{
					ImGui::Checkbox(crypt_str("Backtrack chams"), &g_cfg.player.backtrack_chams);

					if (g_cfg.player.backtrack_chams)
					{
						draw_combo(crypt_str("Backtrack chams material"), g_cfg.player.backtrack_chams_material, chamstype, ARRAYSIZE(chamstype));
						ImGui::Spacing();
						ImGui::Text(crypt_str("Color "));
						ImGui::SameLine(0); ImGui::PushItemWidth(2000.f);
						ImGui::ColorEdit(crypt_str("##backtrackcolor"), &g_cfg.player.backtrack_chams_color, ALPHA);
						ImGui::Spacing();
						ImGui::Spacing();
					}
				}
			}
		}

		if (player == ENEMY || player == TEAM)
		{
			ImGui::Checkbox(crypt_str("Ragdoll chams"), &g_cfg.player.type[player].ragdoll_chams);
			ImGui::Spacing();
			if (g_cfg.player.type[player].ragdoll_chams)
			{
				draw_combo(crypt_str("Ragdoll chams material"), g_cfg.player.type[player].ragdoll_chams_material, chamstype, ARRAYSIZE(chamstype));
				ImGui::Spacing();
				ImGui::Text(crypt_str("Color "));
				ImGui::SameLine(0); ImGui::PushItemWidth(2000.f);
				ImGui::ColorEdit(crypt_str("##ragdollcolor"), &g_cfg.player.type[player].ragdoll_chams_color, ALPHA);
			}
		}
		else if (!g_cfg.player.local_chams_type)
		{
			ImGui::Checkbox(crypt_str("Transparency in scope"), &g_cfg.player.transparency_in_scope);
			ImGui::Spacing();
			if (g_cfg.player.transparency_in_scope)
				ImGui::SliderFloat(crypt_str("Alpha"), &g_cfg.player.transparency_in_scope_amount, 0.0f, 1.0f);
		}

		ImGui::Spacing();

		ImGui::Checkbox(crypt_str("Glow"), &g_cfg.player.type[player].glow);
		ImGui::Spacing();
		if (g_cfg.player.type[player].glow)
		{
			draw_combo(crypt_str("Glow type"), g_cfg.player.type[player].glow_type, glowtype, ARRAYSIZE(glowtype));
			ImGui::Spacing();
			ImGui::Text(crypt_str("Color "));
			ImGui::SameLine(0); ImGui::PushItemWidth(2000.f);
			ImGui::ColorEdit(crypt_str("##glowcolor"), &g_cfg.player.type[player].glow_color, ALPHA);


		}
	ImGui::Spacing;

	ImGui::Checkbox(crypt_str("Local trails"), &g_cfg.esp.trails);
	ImGui::SameLine();
	ImGui::ColorEdit(crypt_str("##Localtrailscolor"), &g_cfg.esp.trails_color, ALPHA);

	}
	ImGui::EndChild();
}

void c_menu::Skins()
{




	ImGui::BeginChildcustom("pinto123323231", ImVec2(290, 490), true);
	{
		draw_keybind(crypt_str("Thirdperson"), &g_cfg.misc.thirdperson_toggle, crypt_str("##TPKEY__HOTKEY"));

		ImGui::Checkbox(crypt_str("Thirdperson when spectating"), &g_cfg.misc.thirdperson_when_spectating);
		if (g_cfg.misc.thirdperson_toggle.key > KEY_NONE && g_cfg.misc.thirdperson_toggle.key < KEY_MAX)
			ImGui::SliderInt(crypt_str("Thirdperson distance"), &g_cfg.misc.thirdperson_distance, 100, 300);
		ImGui::Spacing;
		ImGui::Checkbox(crypt_str("Anti-untrusted"), &g_cfg.misc.anti_untrusted);
		ImGui::Checkbox(crypt_str("Rank reveal"), &g_cfg.misc.rank_reveal);
		ImGui::Checkbox(crypt_str("Unlock inventory access"), &g_cfg.misc.inventory_access);
		ImGui::Checkbox(crypt_str("Gravity ragdolls"), &g_cfg.misc.ragdolls);
		ImGui::Checkbox(crypt_str("Preserve killfeed"), &g_cfg.esp.preserve_killfeed);
		ImGui::Checkbox(crypt_str("Aspect ratio"), &g_cfg.misc.aspect_ratio);

		if (g_cfg.misc.aspect_ratio)
		{
			padding(0, -5);
			ImGui::SliderFloat(crypt_str("Amount"), &g_cfg.misc.aspect_ratio_amount, 1.0f, 2.0f);
		}
		ImGui::Spacing;
		ImGui::Checkbox(crypt_str("Watermark"), &g_cfg.menu.watermark);
		ImGui::Checkbox(crypt_str("Spectators list"), &g_cfg.misc.spectators_list);
		draw_combo(crypt_str("Hitsound"), g_cfg.esp.hitsound, sounds, ARRAYSIZE(sounds));
		ImGui::Checkbox(crypt_str("Killsound"), &g_cfg.esp.killsound);
		draw_multicombo(crypt_str("Logs"), g_cfg.misc.events_to_log, events, ARRAYSIZE(events), preview);
		padding(0, 3);
		draw_multicombo(crypt_str("Logs output"), g_cfg.misc.log_output, events_output, ARRAYSIZE(events_output), preview);

		if (g_cfg.misc.events_to_log[EVENTLOG_HIT] || g_cfg.misc.events_to_log[EVENTLOG_ITEM_PURCHASES] || g_cfg.misc.events_to_log[EVENTLOG_BOMB])
		{
			ImGui::Text(crypt_str("Color "));
			ImGui::SameLine();
			ImGui::ColorEdit(crypt_str("##logcolor"), &g_cfg.misc.log_color, ALPHA);
		}

		ImGui::Checkbox(crypt_str("Show CS:GO logs"), &g_cfg.misc.show_default_log);
	}
	ImGui::EndChild();
	ImGui::SameLine(300);
	ImGui::BeginChildcustom("xerereca31322231", ImVec2(290, 490), true);
	{

		ImGui::Checkbox(crypt_str("Automatic jump"), &g_cfg.misc.bunnyhop);
		draw_combo(crypt_str("Automatic strafes"), g_cfg.misc.airstrafe, strafes, ARRAYSIZE(strafes));
		ImGui::Checkbox(crypt_str("Crouch in air"), &g_cfg.misc.crouch_in_air);
		ImGui::Checkbox(crypt_str("Fast stop"), &g_cfg.misc.fast_stop);
		ImGui::Checkbox(crypt_str("Slide walk"), &g_cfg.misc.slidewalk);
		ImGui::Checkbox(crypt_str("Ping Spike"), &g_cfg.misc.extended_backtracking);
		if (g_cfg.misc.extended_backtracking)
		ImGui::SliderInt(crypt_str("Amount"), &g_cfg.misc.extended_backtracking_value, 0, 200);
		ImGui::Spacing;

		ImGui::Checkbox(("Enable Fake lag"), &g_cfg.antiaim.fakelag);
		if (g_cfg.antiaim.fakelag)
		{
			draw_combo(("Fake lag type"), g_cfg.antiaim.fakelag_type, fakelags, ARRAYSIZE(fakelags));

			ImGui::NewLine(); ImGui::SameLine(42.f); ImGui::PushItemWidth(158.f);
			draw_multicombo(("Fake lag triggers"), g_cfg.antiaim.fakelag_enablers, lagstrigger, ARRAYSIZE(lagstrigger), preview);

			ImGui::SliderInt((" Limit"), &g_cfg.antiaim.fakelag_limit, 1, 16, ("%d"));

			auto enabled_fakelag_triggers = false;

			for (auto i = 0; i < ARRAYSIZE(lagstrigger); i++)
				if (g_cfg.antiaim.fakelag_enablers[i])
					enabled_fakelag_triggers = true;

			ImGui::SliderInt((" Triggers limit"), &g_cfg.antiaim.triggers_fakelag_limit, 1, 16, ("%d"));
			ImGui::Spacing;
		}


		ImGui::Checkbox(crypt_str("Fake Flick"), &g_cfg.antiaim.flick);
		if (g_cfg.antiaim.flick)
			ImGui::SliderInt((" Fake flick range"), &g_cfg.antiaim.flicktick, 1, 100, ("%d°"));
		ImGui::Spacing;
		ImGui::Checkbox(crypt_str("No duck cooldown"), &g_cfg.misc.noduck);

		if (g_cfg.misc.noduck)
			draw_keybind(crypt_str("Fake duck"), &g_cfg.misc.fakeduck_key, crypt_str("##FAKEDUCK__HOTKEY"));
		draw_keybind(crypt_str("Slow walk"), &g_cfg.misc.slowwalk_key, crypt_str("##SLOWWALK__HOTKEY"));
		draw_keybind(crypt_str("Auto peek"), &g_cfg.misc.automatic_peek, crypt_str("##AUTOPEEK__HOTKEY"));
		draw_keybind(crypt_str("Edge jump"), &g_cfg.misc.edge_jump, crypt_str("##EDGEJUMP__HOTKEY"));
		ImGui::Checkbox(crypt_str("Anti-screenshot"), &g_cfg.misc.anti_screenshot);
		ImGui::Checkbox(crypt_str("Clantag"), &g_cfg.misc.clantag_spammer);
		ImGui::Checkbox(crypt_str("Chat spam"), &g_cfg.misc.chat);
		ImGui::Checkbox(crypt_str("Enable buybot"), &g_cfg.misc.buybot_enable);

		if (g_cfg.misc.buybot_enable)
		{
			draw_combo(crypt_str("Snipers"), g_cfg.misc.buybot1, mainwep, ARRAYSIZE(mainwep));
			padding(0, 3);
			draw_combo(crypt_str("Pistols"), g_cfg.misc.buybot2, secwep, ARRAYSIZE(secwep));
			padding(0, 3);
			draw_multicombo(crypt_str("Other"), g_cfg.misc.buybot3, grenades, ARRAYSIZE(grenades), preview);
		}

	}
	ImGui::EndChild();
}


void c_menu::Configurations()
{

	ImGui::BeginChildcustom("pinto213123323231", ImVec2(290, 490), true);
	{
		{

			//ui::SingleSelect("Menu change", &g_ctx.menu_type, { "Skeet", "Evolve", "Onetap v3", "Nemesis", "Neverlose" });
			static auto should_update = true;

			if (should_update)
			{
				should_update = false;

				cfg_manager->config_files();
				files = cfg_manager->files;

				for (auto& current : files)
					if (current.size() > 2)
						current.erase(current.size() - 3, 3);
			}

			if (ImGui::Button(crypt_str("Open configs folder"), ImVec2(200, 15)))
			{
				std::string folder;

				auto get_dir = [&folder]() -> void
				{
					static TCHAR path[MAX_PATH];

					if (SUCCEEDED(SHGetFolderPath(NULL, CSIDL_APPDATA, NULL, NULL, path)))
						folder = std::string(path) + crypt_str("\\SunSet.CC\\Configs");

					CreateDirectory(folder.c_str(), NULL);
				};

				get_dir();
				ShellExecute(NULL, crypt_str("open"), folder.c_str(), NULL, NULL, SW_SHOWNORMAL);
			}
			if (ImGui::Button(crypt_str("Refresh configs"), ImVec2(200, 15)))
			{
				cfg_manager->config_files();
				files = cfg_manager->files;

				for (auto& current : files)
					if (current.size() > 2)
						current.erase(current.size() - 3, 3);
			}

			static char config_name[64] = "\0";

			ImGui::PushStyleVar(ImGuiStyleVar_FrameBorderSize, 1.f);
			ImGui::Text("Cfg name");
			ImGui::InputText(crypt_str("##configname"), config_name, sizeof(config_name));
			ImGui::PopStyleVar();

			if (ImGui::Button(crypt_str("Create config"), ImVec2(200, 15)))
			{
				g_cfg.new_config_name = config_name;
				add_config();
			}

			static auto next_save = false;
			static auto prenext_save = false;
			static auto clicked_sure = false;
			static auto save_time = m_globals()->m_realtime;
			static auto save_alpha = 1.0f;

			save_alpha = math::clamp(save_alpha + (4.f * ImGui::GetIO().DeltaTime * (!prenext_save ? 1.f : -1.f)), 0.01f, 1.f);
			//ui::PushStyleVar(ImGuiStyleVar_Alpha, save_alpha * public_alpha * (1.f - preview_alpha));

			if (!next_save)
			{
				clicked_sure = false;

				if (prenext_save && save_alpha <= 0.01f)
					next_save = true;

				if (ImGui::Button(crypt_str("Save config"), ImVec2(200, 15)))
				{
					save_time = m_globals()->m_realtime;
					prenext_save = true;
				}
			}
			else
			{
				if (prenext_save && save_alpha <= 0.01f)
				{
					prenext_save = false;
					next_save = !clicked_sure;
				}

				if (ImGui::Button(crypt_str("Are you sure?"), ImVec2(200, 15)))
				{
					save_config();
					prenext_save = true;
					clicked_sure = true;
				}

				if (!clicked_sure && m_globals()->m_realtime > save_time + 1.5f)
				{
					prenext_save = true;
					clicked_sure = true;
				}
			}

			//ui::PopStyleVar();

			if (ImGui::Button(crypt_str("Load config"), ImVec2(200, 15)))
				load_config();

			static auto next_delete = false;
			static auto prenext_delete = false;
			static auto clicked_sure_del = false;
			static auto delete_time = m_globals()->m_realtime;
			static auto delete_alpha = 1.0f;

			delete_alpha = math::clamp(delete_alpha + (4.f * ImGui::GetIO().DeltaTime * (!prenext_delete ? 1.f : -1.f)), 0.01f, 1.f);
			//ui::PushStyleVar(ImGuiStyleVar_Alpha, delete_alpha * public_alpha * (1.f - preview_alpha));

			if (!next_delete)
			{
				clicked_sure_del = false;

				if (prenext_delete && delete_alpha <= 0.01f)
					next_delete = true;

				if (ImGui::Button(crypt_str("Remove config"), ImVec2(200, 15)))
				{
					delete_time = m_globals()->m_realtime;
					prenext_delete = true;
				}
			}
			else
			{
				if (prenext_delete && delete_alpha <= 0.01f)
				{
					prenext_delete = false;
					next_delete = !clicked_sure_del;
				}

				if (ImGui::Button(crypt_str("Are you sure?"), ImVec2(200, 15)))
				{
					remove_config();
					prenext_delete = true;
					clicked_sure_del = true;
				}

				if (!clicked_sure_del && m_globals()->m_realtime > delete_time + 1.5f)
				{
					prenext_delete = true;
					clicked_sure_del = true;
				}
			}

			/*ImGui::Spacing();
			ImGui::Spacing();*/

			//ImGui::PushStyleVar(ImGuiStyleVar_FrameBorderSize, 1.f);
			//ImGui::ListBoxConfigArray(crypt_str("##CONFIGS"), &g_cfg.selected_config, files, 7);
			config_listbox::ListBox(("##CONFIGS"), &g_cfg.selected_config, files, 7);
			//ImGui::PopStyleVar();


			//ui::PopStyleVar();


		}
	}
	//ImGui::EndChild();
	//ImGui::SameLine(300);
	//ImGui::BeginChildcustom("xerereca123131322231", ImVec2(290, 700), true);
	//{
	//	
	//}
	//ImGui::EndChild();

}

void c_menu::Luas()
{
	ImGui::PushFont(skeet_menu);

	ImGuiStyle* style = &ImGui::GetStyle();
	InsertSpacer("Top Spacer");

	ImGui::Columns(2, NULL, false);
	{

		InsertGroupBoxLeft("Scripts", 506.f);
		{
			style->ItemSpacing = ImVec2(4, 2);
			style->WindowPadding = ImVec2(4, 4);
			style->ButtonTextAlign = ImVec2(0.5f, -2.0f);
			ImGui::CustomSpacing(9.f);

			static auto should_update = true;

			if (should_update)
			{
				should_update = false;
				scripts = c_lua::get().scripts;

				for (auto& current : scripts)
				{
					if (current.size() >= 5 && current.at(current.size() - 1) == 'c')
						current.erase(current.size() - 5, 5);
					else if (current.size() >= 4)
						current.erase(current.size() - 4, 4);
				}
			}

			ImGui::NewLine(); ImGui::SameLine(42.f); ImGui::PushItemWidth(158.f);
			if (ImGui::Button(("Open scripts folder")))
			{
				std::string folder;

				auto get_dir = [&folder]() -> void
				{
					static TCHAR path[MAX_PATH];

					if (SUCCEEDED(SHGetFolderPath(NULL, CSIDL_APPDATA, NULL, NULL, path)))
						folder = std::string(path) + ("\\SunSet.CC\\Scripts\\");

					CreateDirectory(folder.c_str(), NULL);
				};

				get_dir();
				ShellExecute(NULL, ("open"), folder.c_str(), NULL, NULL, SW_SHOWNORMAL);
			}

			ImGui::PushStyleVar(ImGuiStyleVar_FrameBorderSize, 1.f);

			InsertSpacer("Top Spacer");

			if (scripts.empty())
				config_listbox::ListBox(("##LUAS"), &selected_script, scripts, 7);
			else
			{
				auto backup_scripts = scripts;

				for (auto& script : scripts)
				{
					auto script_id = c_lua::get().get_script_id(script + (".lua"));

					if (script_id == -1)
						continue;

					if (c_lua::get().loaded.at(script_id))
						scripts.at(script_id) += (" [loaded]");
				}

				config_listbox::ListBox(("##LUAS"), &selected_script, scripts, 7);
				scripts = std::move(backup_scripts);
			}

			InsertSpacer("Top Spacer 2");

			ImGui::PopStyleVar();

			ImGui::NewLine(); ImGui::SameLine(42.f); ImGui::PushItemWidth(158.f);
			if (ImGui::Button(("Refresh scripts")))
			{
				c_lua::get().refresh_scripts();
				scripts = c_lua::get().scripts;

				if (selected_script >= scripts.size())
					selected_script = scripts.size() - 1; //-V103

				for (auto& current : scripts)
				{
					if (current.size() >= 5 && current.at(current.size() - 1) == 'c')
						current.erase(current.size() - 5, 5);
					else if (current.size() >= 4)
						current.erase(current.size() - 4, 4);
				}
			}

			InsertSpacer("Top Spacer 3");

			ImGui::NewLine(); ImGui::SameLine(42.f); ImGui::PushItemWidth(158.f);
			if (ImGui::Button(("Edit script")))
			{
				loaded_editing_script = false;
				editing_script = scripts.at(selected_script);
			}

			InsertSpacer("Top Spacer 4");

			ImGui::NewLine(); ImGui::SameLine(42.f); ImGui::PushItemWidth(158.f);
			if (ImGui::Button(("Load script")))
			{
				c_lua::get().load_script(selected_script);
				c_lua::get().refresh_scripts();

				scripts = c_lua::get().scripts;

				if (selected_script >= scripts.size())
					selected_script = scripts.size() - 1; //-V103

				for (auto& current : scripts)
				{
					if (current.size() >= 5 && current.at(current.size() - 1) == 'c')
						current.erase(current.size() - 5, 5);
					else if (current.size() >= 4)
						current.erase(current.size() - 4, 4);
				}

				eventlogs::get().add(("Loaded ") + scripts.at(selected_script) + (" script"), false); //-V106
			}

			InsertSpacer("Top Spacer 5");

			ImGui::NewLine(); ImGui::SameLine(42.f); ImGui::PushItemWidth(158.f);
			if (ImGui::Button(("Unload script")))
			{
				c_lua::get().unload_script(selected_script);
				c_lua::get().refresh_scripts();

				scripts = c_lua::get().scripts;

				if (selected_script >= scripts.size())
					selected_script = scripts.size() - 1; //-V103

				for (auto& current : scripts)
				{
					if (current.size() >= 5 && current.at(current.size() - 1) == 'c')
						current.erase(current.size() - 5, 5);
					else if (current.size() >= 4)
						current.erase(current.size() - 4, 4);
				}

				eventlogs::get().add(("Unloaded ") + scripts.at(selected_script) + (" script"), false); //-V106
			}

			InsertSpacer("Top Spacer 6");

			ImGui::NewLine(); ImGui::SameLine(42.f); ImGui::PushItemWidth(158.f);
			if (ImGui::Button(("Reload all scripts")))
			{
				c_lua::get().reload_all_scripts();
				c_lua::get().refresh_scripts();

				scripts = c_lua::get().scripts;

				if (selected_script >= scripts.size())
					selected_script = scripts.size() - 1; //-V103

				for (auto& current : scripts)
				{
					if (current.size() >= 5 && current.at(current.size() - 1) == 'c')
						current.erase(current.size() - 5, 5);
					else if (current.size() >= 4)
						current.erase(current.size() - 4, 4);
				}
			}

			InsertSpacer("Top Spacer 7");

			ImGui::NewLine(); ImGui::SameLine(42.f); ImGui::PushItemWidth(158.f);
			if (ImGui::Button(("Unload all scripts")))
			{
				c_lua::get().unload_all_scripts();
				c_lua::get().refresh_scripts();

				scripts = c_lua::get().scripts;

				if (selected_script >= scripts.size())
					selected_script = scripts.size() - 1; //-V103

				for (auto& current : scripts)
				{
					if (current.size() >= 5 && current.at(current.size() - 1) == 'c')
						current.erase(current.size() - 5, 5);
					else if (current.size() >= 4)
						current.erase(current.size() - 4, 4);
				}
			}

		} InsertEndGroupBoxLeft("Scripts Cover", "Scripts");
	}
	ImGui::NextColumn();
	{

		InsertGroupBoxRight("Script elements", 506.f);
		{
			style->ItemSpacing = ImVec2(4, 2);
			style->WindowPadding = ImVec2(4, 4);
			style->ButtonTextAlign = ImVec2(0.5f, -2.0f);
			ImGui::CustomSpacing(9.f);

			//InsertCheckbox(("Developer mode"), g_cfg.scripts.developer_mode);
			//InsertCheckbox(("Allow HTTP requests"), g_cfg.scripts.allow_http);
			//InsertCheckbox(("Allow files read or write"), g_cfg.scripts.allow_file);

			auto previous_check_box = false;

			for (auto& current : c_lua::get().scripts)
			{
				auto& items = c_lua::get().items.at(c_lua::get().get_script_id(current));

				for (auto& item : items)
				{
					std::string item_name;

					auto first_point = false;
					auto item_str = false;

					for (auto& c : item.first)
					{
						if (c == '.')
						{
							if (first_point)
							{
								item_str = true;
								continue;
							}
							else
								first_point = true;
						}

						if (item_str)
							item_name.push_back(c);
					}

					switch (item.second.type)
					{
					case NEXT_LINE:
						previous_check_box = false;
						break;
					case CHECK_BOX:
						previous_check_box = true;
						InsertCheckbox(item_name.c_str(), item.second.check_box_value);
						break;
					case COMBO_BOX:
						previous_check_box = false;
						ImGui::Spacing(); ImGui::NewLine(); ImGui::NewLine(); ImGui::SameLine(42.f); ImGui::PushItemWidth(158.f); ImGui::PopItemWidth(); ImGui::CustomSpacing(1.f);
						draw_combo(item_name.c_str(), item.second.combo_box_value, [](void* data, int idx, const char** out_text)
							{
								auto labels = (std::vector <std::string>*)data;
								*out_text = labels->at(idx).c_str(); //-V106
								return true;
							}, &item.second.combo_box_labels, item.second.combo_box_labels.size());
						break;
					case SLIDER_INT:
						previous_check_box = false;
						ImGui::Spacing(); ImGui::Spacing(); ImGui::Spacing(); ImGui::Spacing(); ImGui::Spacing(); ImGui::Spacing(); ImGui::Spacing(); ImGui::Spacing(); ImGui::NewLine(); ImGui::SameLine(42.f); ImGui::PushItemWidth(159.f); ImGui::PopItemWidth();
						ImGui::SliderInt(item_name.c_str(), &item.second.slider_int_value, item.second.slider_int_min, item.second.slider_int_max);
						break;
					case SLIDER_FLOAT:
						previous_check_box = false;
						ImGui::Spacing(); ImGui::Spacing(); ImGui::Spacing(); ImGui::Spacing(); ImGui::Spacing(); ImGui::Spacing(); ImGui::Spacing(); ImGui::Spacing(); ImGui::NewLine(); ImGui::SameLine(42.f); ImGui::PushItemWidth(159.f); ImGui::PopItemWidth();
						ImGui::SliderFloat(item_name.c_str(), &item.second.slider_float_value, item.second.slider_float_min, item.second.slider_float_max);
						break;
					case COLOR_PICKER:
						if (previous_check_box)
							previous_check_box = false;
						else
							ImGui::Text((item_name + ' ').c_str());

						ImGui::SameLine();
						ImGui::ColorEdit((("##") + item_name).c_str(), &item.second.color_picker_value, ALPHA, true);
						break;
					}
				}
			}
		}
		InsertEndGroupBoxRight("Script elements Cover", "Script elements");
	}
	ImGui::PopFont();
}

void c_menu::Chams()
{
	ImGui::PushFont(skeet_menu);

	ImGuiStyle* style = &ImGui::GetStyle();
	InsertSpacer("Top Spacer");

	ImGui::Columns(2, NULL, false);
	{
		InsertGroupBoxLeft("Weapon/Arms chams", 506.f);
		{
			style->ItemSpacing = ImVec2(4, 2);
			style->WindowPadding = ImVec2(4, 4);
			style->ButtonTextAlign = ImVec2(0.5f, -2.0f);
			ImGui::CustomSpacing(9.f);

			InsertCheckbox(("Arms chams"), g_cfg.esp.arms_chams);
			ImGui::SameLine();
			ImGui::ColorEdit(("##armscolor"), &g_cfg.esp.arms_chams_color, ALPHA);


			InsertCombo(("Arms chams material"), g_cfg.esp.arms_chams_type, chamstype, ARRAYSIZE(chamstype));

			if (g_cfg.esp.arms_chams_type != 6)
			{
				InsertCheckbox(("Arms double material "), g_cfg.esp.arms_double_material);
				ImGui::SameLine();
				ImGui::ColorEdit(("##armsdoublematerial"), &g_cfg.esp.arms_double_material_color, ALPHA);
			}

			InsertCheckbox(("Arms animated material "), g_cfg.esp.arms_animated_material);
			ImGui::SameLine();
			ImGui::ColorEdit(("##armsanimatedmaterial"), &g_cfg.esp.arms_animated_material_color, ALPHA);

			ImGui::Spacing();

			InsertCheckbox(("Weapon chams"), g_cfg.esp.weapon_chams);
			ImGui::SameLine();
			ImGui::ColorEdit(("##weaponchamscolors"), &g_cfg.esp.weapon_chams_color, ALPHA);

			InsertCombo(("Weapon chams material"), g_cfg.esp.weapon_chams_type, chamstype, ARRAYSIZE(chamstype));

			if (g_cfg.esp.weapon_chams_type != 6)
			{
				InsertCheckbox(("Double material "), g_cfg.esp.weapon_double_material);
				ImGui::SameLine();
				ImGui::ColorEdit(("##weapondoublematerial"), &g_cfg.esp.weapon_double_material_color, ALPHA);
			}

			InsertCheckbox(("Animated material "), g_cfg.esp.weapon_animated_material);
			ImGui::SameLine();
			ImGui::ColorEdit(("##weaponanimatedmaterial"), &g_cfg.esp.weapon_animated_material_color, ALPHA);

		} InsertEndGroupBoxLeft("Weapon/Arms chams Cover", "Weapon/Arms chams");

		//InsertSpacer("Chams - Other Spacer");

		/*InsertGroupBoxLeft("Enemy", 244.f);
		{
			style->ItemSpacing = ImVec2(4, 2);
			style->WindowPadding = ImVec2(4, 4);
			style->ButtonTextAlign = ImVec2(0.5f, -2.0f);
			ImGui::CustomSpacing(9.f);



		} InsertEndGroupBoxLeft("Enemy Cover", "Enemy chams");*/

	}
	ImGui::NextColumn();
	{

		InsertGroupBoxRight("Local", 506.f);
		{
			style->ItemSpacing = ImVec2(4, 2);
			style->WindowPadding = ImVec2(4, 4);
			style->ButtonTextAlign = ImVec2(0.5f, -2.0f);
			ImGui::CustomSpacing(9.f);

			// 0 = Enemy
			// 1 = Team
			// 2 = Local

			const char* players_chams[3] = { ("Enemy"), ("Team"), ("Local") };

			//auto player = players_section;

			InsertCombo(("Current Chams"), hooks::players_chams, players_chams, ARRAYSIZE(players_chams));

			if (hooks::players_chams == 2)
			{
				InsertCombo(("Type"), g_cfg.player.local_chams_type, local_chams_type, ARRAYSIZE(local_chams_type));
			}

			if (!g_cfg.player.local_chams_type == 1)
			{
				ImGui::NewLine(); ImGui::SameLine(42.f); ImGui::PushItemWidth(158.f); draw_multicombo(("Chams"), g_cfg.player.type[hooks::players_chams].chams, g_cfg.player.type[hooks::players_chams].chams[PLAYER_CHAMS_VISIBLE] ? chamsvisact : chamsvis, g_cfg.player.type[hooks::players_chams].chams[PLAYER_CHAMS_VISIBLE] ? ARRAYSIZE(chamsvisact) : ARRAYSIZE(chamsvis), preview);
			}

			if (hooks::players_chams == 2 && !g_cfg.player.local_chams_type == 0)
			{
				InsertCheckbox(("Enable desync chams"), g_cfg.player.fake_chams_enable);
				InsertCheckbox(("Visualize lag"), g_cfg.player.visualize_lag);
				InsertCheckbox(("Layered"), g_cfg.player.layered);

				InsertCombo(("Player chams material"), g_cfg.player.fake_chams_type, chamstype, ARRAYSIZE(chamstype));

				ImGui::Spacing(); ImGui::NewLine(); ImGui::SameLine(42.f); ImGui::PushItemWidth(158.f);
				ImGui::Text((" Color "));
				ImGui::SameLine();
				ImGui::ColorEdit(("##fakechamscolor"), &g_cfg.player.fake_chams_color, ALPHA);
				ImGui::Spacing();

				InsertCheckbox(("Double material "), g_cfg.player.fake_double_material);
				ImGui::SameLine();
				ImGui::ColorEdit(("##doublematerialcolor"), &g_cfg.player.fake_double_material_color, ALPHA);

				InsertCheckbox(("Animated material"), g_cfg.player.fake_animated_material);
				ImGui::SameLine();
				ImGui::ColorEdit(("##animcolormat"), &g_cfg.player.fake_animated_material_color, ALPHA);
			}

			if (hooks::players_chams == 0 || hooks::players_chams == 1 || !g_cfg.player.local_chams_type == 1)
			{
				InsertCombo(("Player chams material"), g_cfg.player.type[hooks::players_chams].chams_type, chamstype, ARRAYSIZE(chamstype));

				ImGui::Spacing(); ImGui::NewLine(); ImGui::SameLine(42.f); ImGui::PushItemWidth(158.f);
				ImGui::Text((" Visible "));
				ImGui::SameLine();
				ImGui::ColorEdit(("##chamsvisible"), &g_cfg.player.type[hooks::players_chams].chams_color, ALPHA);
				ImGui::Spacing();

				ImGui::Spacing(); ImGui::NewLine(); ImGui::SameLine(42.f); ImGui::PushItemWidth(158.f);
				ImGui::Text((" Invisible "));
				ImGui::SameLine();
				ImGui::ColorEdit(("##chamsinvisible"), &g_cfg.player.type[hooks::players_chams].xqz_color, ALPHA);
				ImGui::Spacing();

				InsertCheckbox(("Double material "), g_cfg.player.type[hooks::players_chams].double_material);
				ImGui::SameLine();
				ImGui::ColorEdit(("##doublematerialcolor"), &g_cfg.player.type[hooks::players_chams].double_material_color, ALPHA);

				InsertCheckbox(("Animated material"), g_cfg.player.type[hooks::players_chams].animated_material);
				ImGui::SameLine();
				ImGui::ColorEdit(("##animcolormat"), &g_cfg.player.type[hooks::players_chams].animated_material_color, ALPHA);
			}

			if (hooks::players_chams == 0)
			{
				InsertCheckbox(("Backtrack chams"), g_cfg.player.backtrack_chams);

				InsertCombo(("Backtrack chams material"), g_cfg.player.backtrack_chams_material, chamstype, ARRAYSIZE(chamstype));

				ImGui::Spacing(); ImGui::NewLine(); ImGui::SameLine(42.f); ImGui::PushItemWidth(158.f);
				ImGui::Text((" Color "));
				ImGui::SameLine();
				ImGui::ColorEdit(("##backtrackcolor"), &g_cfg.player.backtrack_chams_color, ALPHA);
				ImGui::Spacing();

				InsertCheckbox(("Ragdoll chams"), g_cfg.player.type[hooks::players_chams].ragdoll_chams);

				InsertCombo(("Ragdoll chams material"), g_cfg.player.type[hooks::players_chams].ragdoll_chams_material, chamstype, ARRAYSIZE(chamstype));

				ImGui::Spacing(); ImGui::NewLine(); ImGui::SameLine(42.f); ImGui::PushItemWidth(158.f);
				ImGui::Text((" Color "));
				ImGui::SameLine();
				ImGui::ColorEdit(("##ragdollcolor"), &g_cfg.player.type[hooks::players_chams].ragdoll_chams_color, ALPHA);
				ImGui::Spacing();
			}

			if (hooks::players_chams == 2)
			{
				InsertCheckbox(("Transparency in scope"), g_cfg.player.transparency_in_scope);

				ImGui::NewLine(); ImGui::NewLine(); ImGui::SameLine(42.f); ImGui::PushItemWidth(158.f);
				ImGui::SliderFloat((" Alpha"), &g_cfg.player.transparency_in_scope_amount, 0.0f, 1.0f);
			}

			InsertCheckbox(("HitChams"), g_cfg.player.lag_hitbox);
			ImGui::SameLine();
			ImGui::ColorEdit(("##lagcompcolor"), &g_cfg.player.lag_hitbox_color, ALPHA);

			InsertCheckbox(("Glow"), g_cfg.player.type[hooks::players_chams].glow);

			InsertCombo(("Glow type"), g_cfg.player.type[hooks::players_chams].glow_type, glowtype, ARRAYSIZE(glowtype));

			ImGui::Spacing(); ImGui::NewLine(); ImGui::SameLine(42.f); ImGui::PushItemWidth(158.f);
			ImGui::Text((" Color "));
			ImGui::SameLine();
			ImGui::ColorEdit(("##glowcolor"), &g_cfg.player.type[hooks::players_chams].glow_color, ALPHA);
			ImGui::Spacing();

		} InsertEndGroupBoxRight("Local Cover", "Local");

		//InsertSpacer("Chams other - Other Spacer");

		/*InsertGroupBoxRight("Team", 244.f);
		{

			style->ItemSpacing = ImVec2(4, 2);
			style->WindowPadding = ImVec2(4, 4);
			style->ButtonTextAlign = ImVec2(0.5f, -2.0f);
			ImGui::CustomSpacing(9.f);



		} InsertEndGroupBoxRight("Team Cover", "Team");*/

	}
	ImGui::PopFont();
}