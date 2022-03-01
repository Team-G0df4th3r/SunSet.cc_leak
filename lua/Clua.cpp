// This is an independent project of an individual developer. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: http://www.viva64.com
#include "HTTPRequest.hpp"
#include "Clua.h"	
#include <ShlObj_core.h>
#include <Windows.h>
#include <any>
#include "..\..\cheats\visuals\other_esp.h"
#include "..\utils\csgo.hpp"
#include "..\cheats\misc\logs.h"
#include "..\includes.hpp"


void lua_panic(sol::optional <std::string> message)
{
	if (!message)
		return;

	auto log = ("Lua error: ") + message.value_or("unknown");
	eventlogs::get().add(log, false);
}

std::string get_current_script(sol::this_state s)
{
	sol::state_view lua_state(s);
	sol::table rs = lua_state["debug"]["getinfo"](2, ("S"));
	std::string source = rs["source"];
	std::string filename = std::filesystem::path(source.substr(1)).filename().string();

	return filename;
}

int get_current_script_id(sol::this_state s)
{
	return c_lua::get().get_script_id(get_current_script(s));
}

namespace ns_client
{
	void add_callback(sol::this_state s, std::string eventname, sol::protected_function func)
	{		
		if (eventname != ("on_paint") && eventname != ("on_createmove") && eventname != ("on_shot"))
		{
			eventlogs::get().add(("Lua error: invalid callback \"") + eventname + '\"', false);
			return;
		}

		if (c_lua::get().loaded.at(get_current_script_id(s)))//new
		c_lua::get().hooks.registerHook(eventname, get_current_script_id(s), func);
	}

	void load_script(std::string name)
	{
		c_lua::get().refresh_scripts();
		c_lua::get().load_script(c_lua::get().get_script_id(name));
	}

	void unload_script(std::string name)
	{
		c_lua::get().refresh_scripts();
		c_lua::get().unload_script(c_lua::get().get_script_id(name));
	}

	void log(std::string text)
	{
		eventlogs::get().add(text, false);
	}
}

std::vector <std::pair <std::string, menu_item>>::iterator find_item(std::vector <std::pair <std::string, menu_item>>& items, const std::string& name)
{
	for (auto it = items.begin(); it != items.end(); ++it)
		if (it->first == name)
			return it;

	return items.end();
}

menu_item find_item(std::vector <std::vector <std::pair <std::string, menu_item>>>& scripts, const std::string& name)
{
	for (auto& script : scripts)
	{
		for (auto& item : script)
		{
			std::string item_name;

			auto first_point = false;
			auto second_point = false;

			for (auto& c : item.first)
			{
				if (c == '.')
				{
					if (first_point)
					{
						second_point = true;
						continue;
					}
					else
					{
						first_point = true;
						continue;
					}
				}

				if (!second_point)
					continue;

				item_name.push_back(c);
			}

			if (item_name == name)
				return item.second;
		}
	}

	return menu_item();
}

namespace ns_menu
{
	bool get_visible()
	{
		return hooks::menu_open;
	}

	void set_visible(bool visible)
	{
		hooks::menu_open = visible;
	}

	auto next_line_counter = 0;

	void next_line(sol::this_state s)
	{
		c_lua::get().items.at(get_current_script_id(s)).emplace_back(std::make_pair(("next_line_") + std::to_string(next_line_counter), menu_item()));
		++next_line_counter;
	}

	void add_check_box(sol::this_state s, const std::string& name)
	{
		auto script = get_current_script(s);
		auto script_id = c_lua::get().get_script_id(script);

		auto& items = c_lua::get().items.at(script_id);
		auto full_name = script + '.' + name;

		if (find_item(items, full_name) != items.end())
			return;

		items.emplace_back(std::make_pair(full_name, menu_item(false)));
	}

	void add_combo_box(sol::this_state s, std::string name, std::vector <std::string> labels) //-V813
	{
		if (labels.empty())
			return;

		auto script = get_current_script(s);
		auto script_id = c_lua::get().get_script_id(script);

		auto& items = c_lua::get().items.at(script_id);
		auto full_name = script + '.' + name;

		if (find_item(items, full_name) != items.end())
			return;

		items.emplace_back(std::make_pair(full_name, menu_item(labels, 0)));
	}

	void add_slider_int(sol::this_state s, const std::string& name, int min, int max)
	{
		auto script = get_current_script(s);
		auto script_id = c_lua::get().get_script_id(script);

		auto& items = c_lua::get().items.at(script_id);
		auto full_name = script + '.' + name;

		if (find_item(items, full_name) != items.end())
			return;

		items.emplace_back(std::make_pair(full_name, menu_item(min, max, min)));
	}

	void add_slider_float(sol::this_state s, const std::string& name, float min, float max)
	{
		auto script = get_current_script(s);
		auto script_id = c_lua::get().get_script_id(script);

		auto& items = c_lua::get().items.at(script_id);
		auto full_name = script + '.' + name;

		if (find_item(items, full_name) != items.end())
			return;

		items.emplace_back(std::make_pair(full_name, menu_item(min, max, min)));
	}

	void add_color_picker(sol::this_state s, const std::string& name)
	{
		auto script = get_current_script(s);
		auto script_id = c_lua::get().get_script_id(script);

		auto& items = c_lua::get().items.at(script_id);
		auto full_name = script + '.' + name;

		if (find_item(items, full_name) != items.end())
			return;

		items.emplace_back(std::make_pair(full_name, menu_item(Color::White)));
	}

	std::unordered_map <std::string, bool> first_update;
	std::unordered_map <std::string, menu_item> stored_values;
	std::unordered_map <std::string, void*> config_items;

	bool find_config_item(std::string name, std::string type)
	{
		if (config_items.find(name) == config_items.end())
		{
			auto found = false;

			for (auto item : cfg_manager->items)
			{
				if (item->name == name)
				{
					if (item->type != type)
					{
						eventlogs::get().add(("Lua error: invalid config item type, must be ") + type, false);
						return false;
					}

					found = true;
					config_items[name] = item->pointer;
					break;
				}
			}

			if (!found)
			{
				eventlogs::get().add(("Lua error: cannot find config variable \"") + name + '\"', false);
				return false;
			}
		}

		return true;
	}

	bool get_bool(std::string name)
	{
		if (first_update.find(name) == first_update.end())
			first_update[name] = false;

		if (!hooks::menu_open && first_update[name])
		{
			if (stored_values.find(name) != stored_values.end())
				return stored_values[name].check_box_value;
			else if (config_items.find(name) != config_items.end())
				return *(bool*)config_items[name];
			else
				return false;
		}

		auto& it = find_item(c_lua::get().items, name);

		if (it.type == NEXT_LINE)
		{
			if (find_config_item(name, ("bool")))
				return *(bool*)config_items[name];

			eventlogs::get().add(("Lua error: cannot find menu variable \"") + name + '\"', false);
			return false;
		}

		first_update[name] = true;
		stored_values[name] = it;

		return it.check_box_value;
	}

	int get_int(std::string name)
	{
		if (first_update.find(name) == first_update.end())
			first_update[name] = false;

		if (!hooks::menu_open && first_update[name])
		{
			if (stored_values.find(name) != stored_values.end())
				return stored_values[name].type == COMBO_BOX ? stored_values[name].combo_box_value : stored_values[name].slider_int_value;
			else if (config_items.find(name) != config_items.end())
				return *(int*)config_items[name]; //-V206
			else
				return 0;
		}

		auto& it = find_item(c_lua::get().items, name);

		if (it.type == NEXT_LINE)
		{
			if (find_config_item(name, ("int")))
				return *(int*)config_items[name]; //-V206

			eventlogs::get().add(("Lua error: cannot find menu variable \"") + name + '\"', false);
			return 0;
		}

		first_update[name] = true;
		stored_values[name] = it;

		return it.type == COMBO_BOX ? it.combo_box_value : it.slider_int_value;
	}

	float get_float(std::string name)
	{
		if (first_update.find(name) == first_update.end())
			first_update[name] = false;

		if (!hooks::menu_open && first_update[name])
		{
			if (stored_values.find(name) != stored_values.end())
				return stored_values[name].slider_float_value;
			else if (config_items.find(name) != config_items.end())
				return *(float*)config_items[name];
			else
				return 0.0f;
		}

		auto& it = find_item(c_lua::get().items, name);

		if (it.type == NEXT_LINE)
		{
			if (find_config_item(name, ("float")))
				return *(float*)config_items[name];

			eventlogs::get().add(("Lua error: cannot find menu variable \"") + name + '\"', false);
			return 0.0f;
		}

		first_update[name] = true;
		stored_values[name] = it;

		return it.slider_float_value;
	}

	Color get_color(std::string name)
	{
		if (first_update.find(name) == first_update.end())
			first_update[name] = false;

		if (!hooks::menu_open && first_update[name])
		{
			if (stored_values.find(name) != stored_values.end())
				return stored_values[name].color_picker_value;
			else if (config_items.find(name) != config_items.end())
				return *(Color*)config_items[name];
			else
				return Color::White;
		}

		auto& it = find_item(c_lua::get().items, name);

		if (it.type == NEXT_LINE)
		{
			if (find_config_item(name, ("Color")))
				return *(Color*)config_items[name];

			eventlogs::get().add(("Lua error: cannot find menu variable \"") + name + '\"', false);
			return Color::White;
		}

		first_update[name] = true;
		stored_values[name] = it;

		return it.color_picker_value;
	}

	bool get_key_bind_state(int key_bind)
	{
		return key_binds::get().get_key_bind_state_lua(key_bind);
	}

	int get_key_bind_mode(int key_bind)
	{
		return key_binds::get().get_key_bind_mode(key_bind);
	}
	
	void set_bool(std::string name, bool value)
	{
		if (!find_config_item(name, ("bool")))
			return;

		*(bool*)config_items[name] = value;
	}

	void set_int(std::string name, int value)
	{
		if (!find_config_item(name, ("int")))
			return;

		*(int*)config_items[name] = value; //-V206
	}

	void set_float(std::string name, float value)
	{
		if (!find_config_item(name, ("float")))
			return;

		*(float*)config_items[name] = value;
	}

	void set_color(std::string name, Color value)
	{
		if (!find_config_item(name, ("Color")))
			return;

		*(Color*)config_items[name] = value;
	}
}

namespace ns_globals
{
	int get_framerate()
	{
		return g_ctx.globals.framerate;
	}

	int get_ping()
	{
		return g_ctx.globals.ping;
	}

	std::string get_server_address()
	{
		if (!m_engine()->IsInGame())
			return "Unknown";

		auto nci = m_engine()->GetNetChannelInfo();

		if (!nci)
			return "Unknown";

		auto server = nci->GetAddress();

		if (!strcmp(server, "loopback"))
			server = "Local server";
		else if (m_gamerules()->m_bIsValveDS())
			server = "Valve server";

		return server;
	}

	std::string get_time()
	{
		return g_ctx.globals.time;
	}

	std::string get_username()
	{
		return g_ctx.username;
	}

	float get_realtime()
	{
		return m_globals()->m_realtime;
	}

	float get_curtime()
	{
		return m_globals()->m_curtime;
	}

	float get_frametime()
	{
		return m_globals()->m_frametime;
	}

	int get_tickcount()
	{
		return m_globals()->m_tickcount;
	}

	int get_framecount()
	{
		return m_globals()->m_framecount;
	}

	float get_intervalpertick()
	{
		return m_globals()->m_intervalpertick;
	}

	int get_maxclients()
	{
		return m_globals()->m_maxclients;
	}
}

namespace ns_engine
{
	static int width, height;

	int get_screen_width()
	{
		m_engine()->GetScreenSize(width, height);
		return width;
	}

	int get_screen_height()
	{
		m_engine()->GetScreenSize(width, height);
		return height;
	}

	player_info_t get_player_info(int i)
	{
		player_info_t player_info;
		m_engine()->GetPlayerInfo(i, &player_info);

		return player_info;
	}

	int get_player_for_user_id(int i) 
	{
		return m_engine()->GetPlayerForUserID(i);
	}

	int get_local_player_index() 
	{
		return m_engine()->GetLocalPlayer();
	}

	Vector get_view_angles() 
	{
		Vector view_angles;
		m_engine()->GetViewAngles(view_angles);

		return view_angles;
	}

	void set_view_angles(Vector view_angles)
	{
		math::normalize_angles(view_angles);
		m_engine()->SetViewAngles(view_angles);
	}

	bool is_in_game() 
	{
		return m_engine()->IsInGame();
	}

	bool is_connected() 
	{
		return m_engine()->IsConnected();
	}

	std::string get_level_name() 
	{
		return m_engine()->GetLevelName();
	}

	std::string get_level_name_short() 
	{
		return m_engine()->GetLevelNameShort();
	}

	std::string get_map_group_name() 
	{
		return m_engine()->GetMapGroupName();
	}

	bool is_playing_demo()
	{
		return m_engine()->IsPlayingDemo();
	}

	bool is_recording_demo()
	{
		return m_engine()->IsRecordingDemo();
	}

	bool is_paused() 
	{
		return m_engine()->IsPaused();
	}

	bool is_taking_screenshot() 
	{
		return m_engine()->IsTakingScreenshot();
	}

	bool is_hltv() 
	{
		return m_engine()->IsHLTV();
	}
}

namespace ns_render
{
	Vector world_to_screen(const Vector& world)
	{
		Vector screen;

		if (!math::world_to_screen(world, screen))
			return ZERO;

		return screen;
	}

	int get_text_width(vgui::HFont font, const std::string& text)
	{
		return render::get().text_width(font, text.c_str());
	}

	vgui::HFont create_font(const std::string& name, float size, float weight, std::optional <bool> antialias, std::optional <bool> dropshadow, std::optional <bool> outline)
	{
		DWORD flags = FONTFLAG_NONE;

		if (antialias.value_or(false))
			flags |= FONTFLAG_ANTIALIAS;

		if (dropshadow.value_or(false))
			flags |= FONTFLAG_DROPSHADOW;

		if (outline.value_or(false))
			flags |= FONTFLAG_OUTLINE;

		g_ctx.last_font_name = name;

		auto font = m_surface()->FontCreate();
		m_surface()->SetFontGlyphSet(font, name.c_str(), (int)size, (int)weight, 0, 0, flags);

		return font;
	}

	void draw_text(vgui::HFont font, float x, float y, Color color, const std::string& text)
	{
		render::get().text(font, (int)x, (int)y, color, HFONT_CENTERED_NONE, text.c_str());
	}

	void draw_text_centered(vgui::HFont font, float x, float y, Color color, bool centered_x, bool centered_y, const std::string& text)
	{
		DWORD centered_flags = HFONT_CENTERED_NONE;

		if (centered_x)
		{
			centered_flags &= ~HFONT_CENTERED_NONE; //-V753
			centered_flags |= HFONT_CENTERED_X;
		}

		if (centered_y)
		{
			centered_flags &= ~HFONT_CENTERED_NONE;
			centered_flags |= HFONT_CENTERED_Y;
		}

		render::get().text(font, (int)x, (int)y, color, centered_flags, text.c_str());
	}

	void draw_line(float x, float y, float x2, float y2, Color color)
	{
		render::get().line((int)x, (int)y, (int)x2, (int)y2, color);
	}

	void draw_rect(float x, float y, float w, float h, Color color)
	{
		render::get().rect((int)x, (int)y, (int)w, (int)h, color);
	}

	void draw_rect_filled(float x, float y, float w, float h, Color color)
	{
		render::get().rect_filled((int)x, (int)y, (int)w, (int)h, color);
	}

	void draw_rect_filled_gradient(float x, float y, float w, float h, Color color, Color color2, int gradient_type)
	{
		gradient_type = math::clamp(gradient_type, 0, 1);
		render::get().gradient((int)x, (int)y, (int)w, (int)h, color, color2, (GradientType)gradient_type);
	}

	void draw_circle(float x, float y, float points, float radius, Color color)
	{
		render::get().circle((int)x, (int)y, (int)points, (int)radius, color);
	}

	void draw_circle_filled(float x, float y, float points, float radius, Color color)
	{
		render::get().circle_filled((int)x, (int)y, (int)points, (int)radius, color);
	}

	void draw_triangle(float x, float y, float x2, float y2, float x3, float y3, Color color)
	{
		render::get().triangle(Vector2D(x, y), Vector2D(x2, y2), Vector2D(x3, y3), color);
	}
}
namespace ns_indicator
{
	void add_indicator(std::string& text, Color color)
	{
		if (!g_ctx.available())
			return;
		if (!g_ctx.local()->is_alive()) //-V807
			return;
  

		if (otheresp::get().m_indicators.empty())
		{
			otheresp::get().m_indicators.insert(otheresp::get().m_indicators.begin(), m_indicator(text, color));
		}
		else {
			otheresp::get().m_indicators.insert(otheresp::get().m_indicators.begin(), m_indicator(text, color));
		}


	}
	void add_indicator_with_pos(std::string& text, Color color, int pos)
	{
		if (!g_ctx.available())
			return;
		if (!g_ctx.local()->is_alive()) //-V807
			return;
		if (otheresp::get().m_indicators.empty())
		{
			otheresp::get().m_indicators.insert(otheresp::get().m_indicators.begin(), m_indicator(text, color));
		}
		else 
		{
			auto iter1 = otheresp::get().m_indicators.cbegin();
			otheresp::get().m_indicators.insert(iter1 + (pos - 1), m_indicator(text, color));
		}
	}
}
namespace ns_console
{
	void execute(std::string& command)
	{
		if (command.empty())
			return;

		m_engine()->ExecuteClientCmd(command.c_str());
	}

	std::unordered_map <std::string, ConVar*> convars;

	bool get_bool(const std::string& convar_name)
	{
		if (convars.find(convar_name) == convars.end())
		{
			convars[convar_name] = m_cvar()->FindVar(convar_name.c_str());

			if (!convars[convar_name])
			{
				eventlogs::get().add(("Lua error: cannot find ConVar \"") + convar_name + '\"', false);
				return false;
			}
		}

		if (!convars[convar_name])
			return false;

		return convars[convar_name]->GetBool();
	}

	int get_int(const std::string& convar_name)
	{
		if (convars.find(convar_name) == convars.end())
		{
			convars[convar_name] = m_cvar()->FindVar(convar_name.c_str());

			if (!convars[convar_name])
			{
				eventlogs::get().add(("Lua error: cannot find ConVar \"") + convar_name + '\"', false);
				return 0;
			}
		}

		if (!convars[convar_name])
			return 0;

		return convars[convar_name]->GetInt();
	}

	float get_float(const std::string& convar_name)
	{
		if (convars.find(convar_name) == convars.end())
		{
			convars[convar_name] = m_cvar()->FindVar(convar_name.c_str());

			if (!convars[convar_name])
			{
				eventlogs::get().add(("Lua error: cannot find ConVar \"") + convar_name + '\"', false);
				return 0.0f;
			}
		}

		if (!convars[convar_name])
			return 0.0f;

		return convars[convar_name]->GetFloat();
	}

	std::string get_string(const std::string& convar_name)
	{
		if (convars.find(convar_name) == convars.end())
		{
			convars[convar_name] = m_cvar()->FindVar(convar_name.c_str());

			if (!convars[convar_name])
			{
				eventlogs::get().add(("Lua error: cannot find ConVar \"") + convar_name + '\"', false);
				return ("");
			}
		}

		if (!convars[convar_name])
			return "";

		return convars[convar_name]->GetString();
	}

	void set_bool(const std::string& convar_name, bool value)
	{
		if (convars.find(convar_name) == convars.end())
		{
			convars[convar_name] = m_cvar()->FindVar(convar_name.c_str());

			if (!convars[convar_name])
			{
				eventlogs::get().add(("Lua error: cannot find ConVar \"") + convar_name + '\"', false);
				return;
			}
		}

		if (!convars[convar_name])
			return;

		if (convars[convar_name]->GetBool() != value)
			convars[convar_name]->SetValue(value);
	}

	void set_int(const std::string& convar_name, int value)
	{
		if (convars.find(convar_name) == convars.end())
		{
			convars[convar_name] = m_cvar()->FindVar(convar_name.c_str());

			if (!convars[convar_name])
			{
				eventlogs::get().add(("Lua error: cannot find ConVar \"") + convar_name + '\"', false);
				return;
			}
		}

		if (!convars[convar_name])
			return;

		if (convars[convar_name]->GetInt() != value)
			convars[convar_name]->SetValue(value);
	}

	void set_float(const std::string& convar_name, float value)
	{
		if (convars.find(convar_name) == convars.end())
		{
			convars[convar_name] = m_cvar()->FindVar(convar_name.c_str());

			if (!convars[convar_name])
			{
				eventlogs::get().add(("Lua error: cannot find ConVar \"") + convar_name + '\"', false);
				return;
			}
		}

		if (!convars[convar_name])
			return;

		if (convars[convar_name]->GetFloat() != value) //-V550
			convars[convar_name]->SetValue(value);
	}

	void set_string(const std::string& convar_name, const std::string& value)
	{
		if (convars.find(convar_name) == convars.end())
		{
			convars[convar_name] = m_cvar()->FindVar(convar_name.c_str());

			if (!convars[convar_name])
			{
				eventlogs::get().add(("Lua error: cannot find ConVar \"") + convar_name + '\"', false);
				return;
			}
		}

		if (!convars[convar_name])
			return;

		if (convars[convar_name]->GetString() != value)
			convars[convar_name]->SetValue(value.c_str());
	}
}

namespace ns_events
{
	void register_event(sol::this_state s, std::string event_name, sol::protected_function function)
	{
		if (std::find(g_ctx.globals.events.begin(), g_ctx.globals.events.end(), event_name) == g_ctx.globals.events.end())
		{
			m_eventmanager()->AddListener(&hooks::hooked_events, event_name.c_str(), false);
			g_ctx.globals.events.emplace_back(event_name);
		}

		c_lua::get().events[get_current_script_id(s)][event_name] = function;
	}
}

namespace ns_entitylist
{
	sol::optional <player_t*> get_local_player()
	{
		if (!m_engine()->IsInGame())
			return sol::optional <player_t*> (sol::nullopt);

		return (player_t*)m_entitylist()->GetClientEntity(m_engine()->GetLocalPlayer());
	}

	sol::optional <player_t*> get_player_by_index(int i)
	{
		if (!m_engine()->IsInGame())
			return sol::optional <player_t*> (sol::nullopt);

		return (player_t*)m_entitylist()->GetClientEntity(i);
	}

	sol::optional <weapon_t*> get_weapon_by_player(sol::optional <player_t*> player)
	{
		if (!m_engine()->IsInGame())
			return sol::optional <weapon_t*> (sol::nullopt);

		if (!player.value())
			return sol::optional <weapon_t*>(sol::nullopt);

		return player.value()->m_hActiveWeapon().Get();
	}

}

namespace ns_cmd
{
	bool get_send_packet()
	{
		if (!g_ctx.get_command())
			return true;

		return g_ctx.send_packet;
	}

	void set_send_packet(bool send_packet)
	{
		if (!g_ctx.get_command())
			return;

		g_ctx.send_packet = send_packet;
	}

	int get_choke()
	{
		if (!g_ctx.get_command())
			return 0;

		return m_clientstate()->iChokedCommands;
	}

	bool get_button_state(int button)
	{
		if (!g_ctx.get_command())
			return false;

		return g_ctx.get_command()->m_buttons & button;
	}

	void set_button_state(int button, bool state)
	{
		if (!g_ctx.get_command())
			return;

		if (state)
			g_ctx.get_command()->m_buttons |= button;
		else
			g_ctx.get_command()->m_buttons &= ~button;
	}
}

namespace ns_utils 
{
	uint64_t find_signature(const std::string& szModule, const std::string& szSignature)
	{
		return util::FindSignature(szModule.c_str(), szSignature.c_str());
	}
}

namespace ns_http //new
{
	std::string get(sol::this_state s, std::string& link)
	{
		if (!g_cfg.scripts.allow_http)
		{
			c_lua::get().unload_script(get_current_script_id(s));
			eventlogs::get().add(("Please, allow HTTP requests"), false);
			return "";
		}

		try
		{		
			http::Request request(link);

			const http::Response response = request.send(("GET"));
			return std::string(response.body.begin(), response.body.end()); 
		}
		catch (const std::exception& e)
		{
			eventlogs::get().add(("Request failed, error: ") + std::string(e.what()), false);
			return "";
		}
	}

	std::string post(sol::this_state s, std::string& link, std::string& params)
	{
		if (!g_cfg.scripts.allow_http)
		{
			c_lua::get().unload_script(get_current_script_id(s));
			eventlogs::get().add(("Please, allow HTTP requests"), false);
			return "";
		}
		try
		{
			http::Request request(link);
			const http::Response response = request.send(("POST"), params, {("Content-Type: application/x-www-form-urlencoded")});
			return std::string(response.body.begin(), response.body.end()); 
		}
		catch (const std::exception& e)
		{
			eventlogs::get().add(("Request failed, error: ") + std::string(e.what()), false);
			return "";
		}
	}
}

namespace ns_file //new
{
	void append(sol::this_state s, std::string& path, std::string& data)
	{
		if (!g_cfg.scripts.allow_file)
		{
			c_lua::get().unload_script(get_current_script_id(s));
			eventlogs::get().add(("Please, allow files read or write"), false);
			return;
		}

		std::ofstream out(path, std::ios::app | std::ios::binary);

		if (out.is_open())
			out << data;
		else
			eventlogs::get().add(("Can't append to file: ") + path, false);
		
		out.close();
	}
	void write(sol::this_state s, std::string& path, std::string& data)
	{
		if (!g_cfg.scripts.allow_file)
		{
			c_lua::get().unload_script(get_current_script_id(s));
			eventlogs::get().add(("Please, allow files read or write"), false);
			return;
		}

		std::ofstream out(path, std::ios::binary);

		if (out.is_open())
			out << data;

		else
			eventlogs::get().add(("Can't write to file: ") + path, false);

		out.close();
	}
	std::string read(sol::this_state s, std::string& path)
	{
		if (!g_cfg.scripts.allow_file)
		{
			c_lua::get().unload_script(get_current_script_id(s));
			eventlogs::get().add(("Please, allow files read or write"), false);
			return "";
		}

		std::ifstream file(path, std::ios::binary);

		if (file.is_open())
		{
			std::string content((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
			file.close();
			return content;
		}
		else
		{
			eventlogs::get().add(("Can't read file: ") + path, false);
			file.close();
			return "";
		}
	}
}

sol::state lua;
void c_lua::initialize()
{
	lua = sol::state(sol::c_call<decltype(&lua_panic), &lua_panic>);
	lua.open_libraries(sol::lib::base, sol::lib::string, sol::lib::math, sol::lib::table, sol::lib::debug, sol::lib::package);

	lua[("collectgarbage")] = sol::nil;
	lua[("dofilsse")] = sol::nil;
	lua[("load")] = sol::nil;
	lua[("loadfile")] = sol::nil;
	lua[("pcall")] = sol::nil;
	lua[("print")] = sol::nil;
	lua[("xpcall")] = sol::nil;
	lua[("getmetatable")] = sol::nil;
	lua[("setmetatable")] = sol::nil;
	lua[("__nil_callback")] = [](){};
	
	lua.new_enum(("key_binds"),
		("legit_automatic_fire"), 0,
		("legit_enable"), 1,
		("double_tap"), 2,
		("safe_points"), 3,
		("damage_override"), 4,
		("hide_shots"), 12,
		("manual_back"), 13,
		("manual_left"), 14,
		("manual_right"), 15,
		("flip_desync"), 16,
		("thirdperson"), 17,
		("automatic_peek"), 18,
		("edge_jump"), 19,
		("fakeduck"), 20,
		("slowwalk"), 21,
		("body_aim"), 22
	);
	
	lua.new_enum(("key_bind_mode"),
		("hold"), 0,
		("toggle"), 1
	);
	
	lua.new_usertype<entity_t>(("entity"), // new
		(std::string)("get_prop_int"), &entity_t::GetPropInt,
		(std::string)("get_prop_float"), &entity_t::GetPropFloat,
		(std::string)("get_prop_bool"), &entity_t::GetPropBool,
		(std::string)("get_prop_string"), &entity_t::GetPropString,
		(std::string)("set_prop_int"), &entity_t::SetPropInt,
		(std::string)("set_prop_float"), &entity_t::SetPropFloat,
		(std::string)("set_prop_bool"), &entity_t::SetPropBool		
	);
	
	lua.new_usertype <Color> (("color"), sol::constructors <Color(), Color(int, int, int), Color(int, int, int, int)> (), 
		(std::string)("r"), &Color::r, 
		(std::string)("g"), &Color::g,
		(std::string)("b"), &Color::b, 
		(std::string)("a"), &Color::a
	);
	
	lua.new_usertype <Vector> (("vector"), sol::constructors <Vector(), Vector(float, float, float)> (),
		(std::string)("x"), &Vector::x,
		(std::string)("y"), &Vector::y,
		(std::string)("z"), &Vector::z,
		(std::string)("length"), &Vector::Length,
		(std::string)("length_sqr"), &Vector::LengthSqr,
		(std::string)("length_2d"), &Vector::Length2D,
		(std::string)("length_2d_sqr"), &Vector::Length2DSqr,
		(std::string)("is_zero"), &Vector::IsZero,
		(std::string)("is_valid"), &Vector::IsValid,
		(std::string)("zero"), &Vector::Zero,
		(std::string)("dist_to"), &Vector::DistTo,
		(std::string)("dist_to_sqr"), &Vector::DistToSqr,
		(std::string)("cross_product"), &Vector::Cross,
		(std::string)("normalize"), &Vector::Normalize
	);
	
	//lua.new_usertype <player_info_t> (("player_info"),
	//	(std::string)("bot"), &player_info_t::fakeplayer,
	//	(std::string)("name"), &player_info_t::szName,
	//	(std::string)("steam_id"), &player_info_t::szSteamID
	//
	//);
	//
	lua.new_usertype <IGameEvent> (("game_event"),
		(std::string)("get_bool"), &IGameEvent::GetBool,
		(std::string)("get_int"), &IGameEvent::GetInt,
		(std::string)("get_float"), &IGameEvent::GetFloat,
		(std::string)("get_string"), &IGameEvent::GetString,
		(std::string)("set_bool"), &IGameEvent::SetBool,
		(std::string)("set_int"), &IGameEvent::SetInt,
		(std::string)("set_float"), &IGameEvent::SetFloat,
		(std::string)("set_string"), &IGameEvent::SetString
	);
	
	lua.new_enum(("hitboxes"),
		("head"), HITBOX_HEAD,
		("neck"), HITBOX_NECK,
		("pelvis"), HITBOX_PELVIS,
		("stomach"), HITBOX_STOMACH,
		("lower_chest"), HITBOX_LOWER_CHEST,
		("chest"), HITBOX_CHEST,
		("upper_chest"), HITBOX_UPPER_CHEST,
		("right_thigh"), HITBOX_RIGHT_THIGH,
		("left_thigh"), HITBOX_LEFT_THIGH,
		("right_calf"), HITBOX_RIGHT_CALF,
		("left_calf"), HITBOX_LEFT_CALF,
		("right_foot"), HITBOX_RIGHT_FOOT,
		("left_foot"), HITBOX_LEFT_FOOT,
		("right_hand"), HITBOX_RIGHT_HAND,
		("left_hand"), HITBOX_LEFT_HAND,
		("right_upper_arm"), HITBOX_RIGHT_UPPER_ARM,
		("right_forearm"), HITBOX_RIGHT_FOREARM,
		("left_upper_arm"), HITBOX_LEFT_UPPER_ARM,
		("left_forearm"), HITBOX_LEFT_FOREARM
	);
	
	lua.new_usertype <player_t> (("player"), sol::base_classes, sol::bases<entity_t>(), //new
		(std::string)("get_index"), &player_t::EntIndex,
		(std::string)("get_dormant"), &player_t::IsDormant,
		(std::string)("get_team"), &player_t::m_iTeamNum,
		(std::string)("get_alive"), &player_t::is_alive,
		(std::string)("get_velocity"), &player_t::m_vecVelocity,
		(std::string)("get_origin"), &player_t::GetAbsOrigin,
		(std::string)("get_angles"), &player_t::m_angEyeAngles,
		(std::string)("get_hitbox_position"), &player_t::hitbox_position,
		(std::string)("has_helmet"), &player_t::m_bHasHelmet,
		(std::string)("has_heavy_armor"), &player_t::m_bHasHeavyArmor,
		(std::string)("is_scoped"), &player_t::m_bIsScoped,
		(std::string)("get_health"), &player_t::m_iHealth
	);
	
	lua.new_usertype <weapon_t> (("weapon"), sol::base_classes, sol::bases<entity_t>(),
		(std::string)("is_empty"), &weapon_t::is_empty,
		(std::string)("can_fire"), &weapon_t::can_fire,
		(std::string)("is_non_aim"), &weapon_t::is_non_aim,
		(std::string)("can_double_tap"), &weapon_t::can_double_tap,
		(std::string)("get_name"), &weapon_t::get_name,
		(std::string)("get_inaccuracy"), &weapon_t::get_inaccuracy,
		(std::string)("get_spread"), &weapon_t::get_spread
	);
	
	lua.new_enum(("buttons"),
		("in_attack"), IN_ATTACK,
		("in_jump"), IN_JUMP,
		("in_duck"), IN_DUCK,
		("in_forward"), IN_FORWARD,
		("in_back"), IN_BACK,
		("in_use"), IN_USE,
		("in_cancel"), IN_CANCEL,
		("in_left"), IN_LEFT,
		("in_right"), IN_RIGHT,
		("in_moveleft"), IN_MOVELEFT,
		("in_moveright"), IN_MOVERIGHT,
		("in_attack2"), IN_ATTACK2,
		("in_run"), IN_RUN,
		("in_reload"), IN_RELOAD,
		("in_alt1"), IN_ALT1,
		("in_alt2"), IN_ALT2,
		("in_score"), IN_SCORE,
		("in_speed"), IN_SPEED,
		("in_walk"), IN_WALK,
		("in_zoom"), IN_ZOOM,
		("in_weapon1"), IN_WEAPON1,
		("in_weapon2"), IN_WEAPON2,
		("in_bullrush"), IN_BULLRUSH,
		("in_grenade1"), IN_GRENADE1,
		("in_grenade2"), IN_GRENADE2,
		("in_lookspin"), IN_LOOKSPIN
	);
	
	lua.new_usertype <shot_info> (("shot_info"), sol::constructors <> (), 
		(std::string)("target_name"), &shot_info::target_name, 
		(std::string)("result"), &shot_info::result, 
		(std::string)("client_hitbox"), &shot_info::client_hitbox, 
		(std::string)("server_hitbox"), &shot_info::server_hitbox,
		(std::string)("client_damage"), &shot_info::client_damage,
		(std::string)("server_damage"), &shot_info::server_damage,
		(std::string)("hitchance"), &shot_info::hitchance,
		(std::string)("backtrack_ticks"), &shot_info::backtrack_ticks,
		(std::string)("aim_point"), &shot_info::aim_point
	);
	
	auto client = lua.create_table();
	client[("add_callback")] = ns_client::add_callback;
	client[("load_script")] = ns_client::load_script;
	client[("unload_script")] = ns_client::unload_script;
	client[("log")] = ns_client::log;
	
	auto menu = lua.create_table();
	menu[("next_line")] = ns_menu::next_line;
	menu[("add_check_box")] = ns_menu::add_check_box;
	menu[("add_combo_box")] = ns_menu::add_combo_box;
	menu[("add_slider_int")] = ns_menu::add_slider_int;
	menu[("add_slider_float")] = ns_menu::add_slider_float;
	menu[("add_color_picker")] = ns_menu::add_color_picker;
	menu[("get_bool")] = ns_menu::get_bool;
	menu[("get_int")] = ns_menu::get_int;
	menu[("get_float")] = ns_menu::get_float;
	menu[("get_color")] = ns_menu::get_color;
	menu[("get_key_bind_state")] = ns_menu::get_key_bind_state;
	menu[("get_key_bind_mode")] = ns_menu::get_key_bind_mode;
	menu[("set_bool")] = ns_menu::set_bool;
	menu[("set_int")] = ns_menu::set_int;
	menu[("set_float")] = ns_menu::set_float;
	menu[("set_color")] = ns_menu::set_color;
	
	auto globals = lua.create_table();
	globals[("get_framerate")] = ns_globals::get_framerate;
	globals[("get_ping")] = ns_globals::get_ping;
	globals[("get_server_address")] = ns_globals::get_server_address;
	globals[("get_time")] = ns_globals::get_time;
	globals[("get_username")] = ns_globals::get_username;
	globals[("get_realtime")] = ns_globals::get_realtime;
	globals[("get_curtime")] = ns_globals::get_curtime;
	globals[("get_frametime")] = ns_globals::get_frametime;
	globals[("get_tickcount")] = ns_globals::get_tickcount;
	globals[("get_framecount")] = ns_globals::get_framecount;
	globals[("get_intervalpertick")] = ns_globals::get_intervalpertick;
	globals[("get_maxclients")] = ns_globals::get_maxclients;
	
	auto engine = lua.create_table();
	engine[("get_screen_width")] = ns_engine::get_screen_width;
	engine[("get_screen_height")] = ns_engine::get_screen_height;
	engine[("get_level_name")] = ns_engine::get_level_name;
	engine[("get_level_name_short")] = ns_engine::get_level_name_short;
	engine[("get_local_player_index")] = ns_engine::get_local_player_index;
	engine[("get_map_group_name")] = ns_engine::get_map_group_name;
	engine[("get_player_for_user_id")] = ns_engine::get_player_for_user_id;
	engine[("get_player_info")] = ns_engine::get_player_info;
	engine[("get_view_angles")] = ns_engine::get_view_angles;
	engine[("is_connected")] = ns_engine::is_connected;
	engine[("is_hltv")] = ns_engine::is_hltv;
	engine[("is_in_game")] = ns_engine::is_in_game;
	engine[("is_paused")] = ns_engine::is_paused;
	engine[("is_playing_demo")] = ns_engine::is_playing_demo;
	engine[("is_recording_demo")] = ns_engine::is_recording_demo;
	engine[("is_taking_screenshot")] = ns_engine::is_taking_screenshot;
	engine[("set_view_angles")] = ns_engine::set_view_angles;
	
	auto render = lua.create_table();
	render[("world_to_screen")] = ns_render::world_to_screen;
	render[("get_text_width")] = ns_render::get_text_width;
	render[("create_font")] = ns_render::create_font;
	render[("draw_text")] = ns_render::draw_text;
	render[("draw_text_centered")] = ns_render::draw_text_centered;
	render[("draw_line")] = ns_render::draw_line;
	render[("draw_rect")] = ns_render::draw_rect;
	render[("draw_rect_filled")] = ns_render::draw_rect_filled;
	render[("draw_rect_filled_gradient")] = ns_render::draw_rect_filled_gradient;
	render[("draw_circle")] = ns_render::draw_circle;
	render[("draw_circle_filled")] = ns_render::draw_circle_filled;
	render[("draw_triangle")] = ns_render::draw_triangle;
	
	auto console = lua.create_table();
	console[("execute")] = ns_console::execute;
	console[("get_int")] = ns_console::get_int;
	console[("get_float")] = ns_console::get_float;
	console[("get_string")] = ns_console::get_string;
	console[("set_int")] = ns_console::set_int;
	console[("set_float")] = ns_console::set_float;
	console[("set_string")] = ns_console::set_string;
	
	auto events = lua.create_table(); //-V688
	events[("register_event")] = ns_events::register_event;
	
	auto entitylist = lua.create_table();
	entitylist[("get_local_player")] = ns_entitylist::get_local_player;
	entitylist[("get_player_by_index")] = ns_entitylist::get_player_by_index;
	entitylist[("get_weapon_by_player")] = ns_entitylist::get_weapon_by_player;
	
	auto cmd = lua.create_table();
	cmd[("get_send_packet")] = ns_cmd::get_send_packet;
	cmd[("set_send_packet")] = ns_cmd::set_send_packet;
	cmd[("get_choke")] = ns_cmd::get_choke;
	cmd[("get_button_state")] = ns_cmd::get_button_state;
	cmd[("set_button_state")] = ns_cmd::set_button_state;
	
	auto utils = lua.create_table();//new
	utils[("find_signature")] = ns_utils::find_signature;//new
	
	auto indicators = lua.create_table();//new
	indicators[("add")] = ns_indicator::add_indicator;//new
	indicators[("add_position")] = ns_indicator::add_indicator_with_pos;//new
	
	auto http = lua.create_table();//new
	http[("get")] = ns_http::get;//new
	http[("post")] = ns_http::post;//new
	
	auto file = lua.create_table();//new
	file[("append")] = ns_file::append; //new
	file[("write")] = ns_file::write;//new
	file[("read")] = ns_file::read;//new
	
	lua[("client")] = client;
	lua[("menu")] = menu;
	lua[("globals")] = globals;
	lua[("engine")] = engine;
	lua[("render")] = render;
	lua[("console")] = console;
	lua[("events")] = events;
	lua[("entitylist")] = entitylist;
	lua[("cmd")] = cmd;
	lua[("utils")] = utils;//new
	lua[("indicators")] = indicators;//new
	lua[("http")] = http;//new
	lua[("file")] = file;//new

	refresh_scripts();
}

int c_lua::get_script_id(const std::string& name)
{
	for (auto i = 0; i < scripts.size(); i++)
		if (scripts.at(i) == name) //-V106
			return i;

	return -1;
}

int c_lua::get_script_id_by_path(const std::string& path)
{
	for (auto i = 0; i < pathes.size(); i++)
		if (pathes.at(i).string() == path) //-V106
			return i;

	return -1;
}

void c_lua::refresh_scripts()
{
	auto oldLoaded = loaded;
	auto oldScripts = scripts;

	loaded.clear();
	pathes.clear();
	scripts.clear();
	ns_console::convars.clear();

	std::string folder;
	static TCHAR path[MAX_PATH];

	if (SUCCEEDED(SHGetFolderPath(NULL, CSIDL_APPDATA, NULL, NULL, path)))
	{
		folder = std::string(path) + ("\\SunSet.CC\\Scripts\\");
		CreateDirectory(folder.c_str(), NULL);

		auto i = 0;

		for (auto& entry : std::filesystem::directory_iterator(folder))
		{
			if (entry.path().extension() == (".lua") || entry.path().extension() == (".luac"))
			{
				auto path = entry.path();
				auto filename = path.filename().string();

				auto didPut = false;

				for (auto i = 0; i < oldScripts.size(); i++)
				{
					if (filename == oldScripts.at(i)) //-V106
					{
						loaded.emplace_back(oldLoaded.at(i)); //-V106
						didPut = true;
					}
				}

				if (!didPut)
					loaded.emplace_back(false);

				pathes.emplace_back(path);
				scripts.emplace_back(filename);

				items.emplace_back(std::vector <std::pair <std::string, menu_item>> ());
				++i;
			}
		}
	}
}

void c_lua::load_script(int id)
{
	if (id == -1)
		return;

	if (loaded.at(id)) //-V106
		return;

	auto path = get_script_path(id);

	if (path == (""))
		return;

	auto error_load = false;
	loaded.at(id) = true;
	lua.script_file(path, 
		[&error_load](lua_State*, sol::protected_function_result result)
		{
			if (!result.valid())
			{
				sol::error error = result;
				auto log = ("Lua error: ") + (std::string)error.what();

				eventlogs::get().add(log, false);
				error_load = true;
				
			}

			return result;
		}
	);

	if (error_load | loaded.at(id) == false)
	{
		loaded.at(id) = false;
		return;
	}
		

	 //-V106
	g_ctx.globals.loaded_script = true;
}

void c_lua::unload_script(int id)
{
	if (id == -1)
		return;

	if (!loaded.at(id)) //-V106
		return;

	items.at(id).clear(); //-V106

	if (c_lua::get().events.find(id) != c_lua::get().events.end()) //-V807
		c_lua::get().events.at(id).clear();

	hooks.unregisterHooks(id);
	loaded.at(id) = false; //-V106
}

void c_lua::reload_all_scripts()
{
	for (auto current : scripts)
	{
		if (!loaded.at(get_script_id(current))) //-V106
			continue;

		unload_script(get_script_id(current));
		load_script(get_script_id(current));
	}
}

void c_lua::unload_all_scripts()
{
	for (auto s : scripts)
		unload_script(get_script_id(s));
}

std::string c_lua::get_script_path(const std::string& name)
{
	return get_script_path(get_script_id(name));
}

std::string c_lua::get_script_path(int id)
{
	if (id == -1)
		return ("");

	return pathes.at(id).string(); //-V106
}