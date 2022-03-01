// This is an independent project of an individual developer. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: http://www.viva64.com

#include "logs.h"

void eventlogs::paint_traverse()
{
	if (logs.empty())
		return;

	while (logs.size() > 10)
		logs.pop_back();

	auto last_y = 146;
	
	for (size_t i = 0; i < logs.size(); i++)
	{
		auto& log = logs.at(i);

		if (util::epoch_time() - log.log_time > 4600) 
		{
			auto factor = log.log_time + 5000.0f - (float)util::epoch_time();
			factor *= 0.001f;

			auto opacity = (int)(255.0f * factor);

			if (opacity < 2)
			{
				logs.erase(logs.begin() + i);
				continue;
			}

			log.color.SetAlpha(opacity);
			log.y -= factor * 1.25f;
		}

		last_y -= 14;

		auto logs_size_inverted = 10 - logs.size();
		render::get().text(fonts[LOGS], log.x, last_y + log.y - logs_size_inverted * 14, log.color, HFONT_CENTERED_NONE, log.message.c_str());
	}
}

void eventlogs::events(IGameEvent* event) 
{
	static auto get_hitgroup_name = [](int hitgroup) -> std::string
	{
		switch (hitgroup)
		{
		case HITGROUP_HEAD:
			return ("head");
		case HITGROUP_CHEST:
			return ("chest");
		case HITGROUP_STOMACH:
			return ("stomach");
		case HITGROUP_LEFTARM:
			return ("left arm");
		case HITGROUP_RIGHTARM:
			return ("right arm");
		case HITGROUP_LEFTLEG:
			return ("left leg");
		case HITGROUP_RIGHTLEG:
			return ("right leg");
		default:
			return ("generic");
		}
	};

	if (g_cfg.misc.events_to_log[EVENTLOG_HIT] && !strcmp(event->GetName(), ("player_hurt")))
	{
		auto userid = event->GetInt(("userid")), attacker = event->GetInt(("attacker"));

		if (!userid || !attacker) 
			return;

		auto userid_id = m_engine()->GetPlayerForUserID(userid), attacker_id = m_engine()->GetPlayerForUserID(attacker); //-V807

		player_info_t userid_info, attacker_info;

		if (!m_engine()->GetPlayerInfo(userid_id, &userid_info)) 
			return;

		if (!m_engine()->GetPlayerInfo(attacker_id, &attacker_info)) 
			return;

		auto m_victim = static_cast<player_t *>(m_entitylist()->GetClientEntity(userid_id));

		std::stringstream ss;

		if (attacker_id == m_engine()->GetLocalPlayer() && userid_id != m_engine()->GetLocalPlayer()) 
		{
			ss << ("Hit ") << userid_info.szName << (" in the ") << get_hitgroup_name(event->GetInt(("hitgroup"))) << (" for ") << event->GetInt(("dmg_health"));
			ss << (" damage (") << event->GetInt(("health")) << (" health remaining)");

			add(ss.str());
		}
		else if (userid_id == m_engine()->GetLocalPlayer() && attacker_id != m_engine()->GetLocalPlayer()) 
		{
			ss << ("Take ") << event->GetInt(("dmg_health")) << (" damage from ") << attacker_info.szName << (" in the ") << get_hitgroup_name(event->GetInt(("hitgroup")));
			ss << (" (") << event->GetInt(("health")) << (" health remaining)");

			add(ss.str());
		}
	}

	if (g_cfg.misc.events_to_log[EVENTLOG_ITEM_PURCHASES] && !strcmp(event->GetName(), ("item_purchase")))
	{
		auto userid = event->GetInt(("userid"));

		if (!userid) 
			return;

		auto userid_id = m_engine()->GetPlayerForUserID(userid);

		player_info_t userid_info;

		if (!m_engine()->GetPlayerInfo(userid_id, &userid_info)) 
			return;

		auto m_player = static_cast<player_t *>(m_entitylist()->GetClientEntity(userid_id));

		if (!g_ctx.local() || !m_player) 
			return;

		if (g_ctx.local() == m_player)
			g_ctx.globals.should_buy = 0;

		if (m_player->m_iTeamNum() == g_ctx.local()->m_iTeamNum()) 
			return;

		std::string weapon = event->GetString(("weapon"));
		
		std::stringstream ss;
		ss << userid_info.szName << (" bought ") << weapon;

		add(ss.str());
	}

	if (g_cfg.misc.events_to_log[EVENTLOG_BOMB] && !strcmp(event->GetName(), ("bomb_beginplant")))
	{
		auto userid = event->GetInt(("userid"));

		if (!userid) 
			return;

		auto userid_id = m_engine()->GetPlayerForUserID(userid);

		player_info_t userid_info;

		if (!m_engine()->GetPlayerInfo(userid_id, &userid_info)) 
			return;

		auto m_player = static_cast<player_t *>(m_entitylist()->GetClientEntity(userid_id));

		if (!m_player) 
			return;

		std::stringstream ss;
		ss << userid_info.szName << (" the bomb was started to being planted ");

		add(ss.str());
	}

	if (g_cfg.misc.events_to_log[EVENTLOG_BOMB] && !strcmp(event->GetName(), ("bomb_begindefuse")))
	{
		auto userid = event->GetInt(("userid"));

		if (!userid) 
			return;

		auto userid_id = m_engine()->GetPlayerForUserID(userid);

		player_info_t userid_info;

		if (!m_engine()->GetPlayerInfo(userid_id, &userid_info)) 
			return;

		auto m_player = static_cast<player_t *>(m_entitylist()->GetClientEntity(userid_id));

		if (!m_player) 
			return;

		std::stringstream ss;
		ss << userid_info.szName << (" the bomb was started to being defused ") << (event->GetBool(("haskit")) ? ("with defuse kit") : ("without defuse kit"));

		add(ss.str());
	}
}

void eventlogs::add(std::string text, bool full_display)
{
	logs.emplace_front(loginfo_t(util::epoch_time(), text, g_cfg.misc.log_color));

	if (!full_display)
		return;

	if (g_cfg.misc.log_output[EVENTLOG_OUTPUT_CONSOLE])
	{
		last_log = true;

#if RELEASE
#if BETA
		m_cvar()->ConsoleColorPrintf(g_cfg.misc.log_color, ("[ SunSet.CC ] ")); //-V807
#else
		m_cvar()->ConsoleColorPrintf(g_cfg.misc.log_color, ("[ goodtap ] "));
#endif
#else
		m_cvar()->ConsoleColorPrintf(g_cfg.misc.log_color, ("[ SunSet.CC ] ")); //-V807
#endif

		m_cvar()->ConsoleColorPrintf(Color::White, text.c_str());
		m_cvar()->ConsolePrintf(("\n"));
	}

	if (g_cfg.misc.log_output[EVENTLOG_OUTPUT_CHAT])
	{
		static CHudChat* chat = nullptr;

		if (!chat)
			chat = util::FindHudElement <CHudChat> (("CHudChat"));

#if RELEASE
#if BETA
		auto log = ("[ \x0Cgoodtap beta \x01] ") + text;
		chat->chat_print(log.c_str());
#else
		auto log = ("[ \x0Cgoodtap \x01] ") + text;
		chat->chat_print(log.c_str());
#endif
#else
		auto log = ("[ \x0CSunSet.CC \x01] ") + text;
		chat->chat_print(log.c_str());
#endif
	}
}