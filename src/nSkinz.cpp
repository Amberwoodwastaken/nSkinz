/* This file is part of nSkinz by namazso, licensed under the MIT license:
*
* MIT License
*
* Copyright (c) namazso 2018
*
* Permission is hereby granted, free of charge, to any person obtaining a copy
* of this software and associated documentation files (the "Software"), to deal
* in the Software without restriction, including without limitation the rights
* to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
* copies of the Software, and to permit persons to whom the Software is
* furnished to do so, subject to the following conditions:
*
* The above copyright notice and this permission notice shall be included in all
* copies or substantial portions of the Software.
*
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
* IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
* FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
* AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
* LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
* OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
* SOFTWARE.
*/
#include "nSkinz.hpp"
#include "Hooks/hooks.hpp"
#include "render.hpp"
#include "kit_parser.hpp"
#include "update_check.hpp"
#include "config.hpp"

sdk::IBaseClientDLL*		g_client;
sdk::IClientEntityList*		g_entity_list;
sdk::IVEngineClient*		g_engine;
sdk::IVModelInfoClient*		g_model_info;
sdk::IGameEventManager2*	g_game_event_manager;
sdk::ILocalize*				g_localize;

sdk::CBaseClientState**		g_client_state;

vmt_smart_hook*				g_client_hook;
vmt_smart_hook*				g_game_event_manager_hook;

recv_prop_hook*				g_sequence_hook;

template <class T>
auto get_interface(const char* module, const char* name) -> T*
{
	return reinterpret_cast<T*>(platform::get_interface(module, name));
}

auto initialize(void* instance) -> void
{
	g_client = get_interface<sdk::IBaseClientDLL>("client.dll", CLIENT_DLL_INTERFACE_VERSION);
	g_entity_list = get_interface<sdk::IClientEntityList>("client.dll", VCLIENTENTITYLIST_INTERFACE_VERSION);
	g_engine = get_interface<sdk::IVEngineClient>("engine.dll", VENGINE_CLIENT_INTERFACE_VERSION);
	g_model_info = get_interface<sdk::IVModelInfoClient>("engine.dll", VMODELINFO_CLIENT_INTERFACE_VERSION);
	g_game_event_manager = get_interface<sdk::IGameEventManager2>("engine.dll", INTERFACEVERSION_GAMEEVENTSMANAGER2);
	g_localize = get_interface<sdk::ILocalize>("localize.dll", ILOCALIZE_CLIENT_INTERFACE_VERSION);

	g_client_state = *reinterpret_cast<sdk::CBaseClientState***>(get_vfunc<std::uintptr_t>(g_engine, 12) + 0x10);

	run_update_check();

	// Get skins
	initialize_kits();

	g_config.load();

	render::initialize();

	g_client_hook = new vmt_smart_hook(g_client);
	g_client_hook->apply_hook<hooks::FrameStageNotify>(36);

	g_game_event_manager_hook = new vmt_smart_hook(g_game_event_manager);
	g_game_event_manager_hook->apply_hook<hooks::FireEventClientSide>(9);

	const auto sequence_prop = sdk::C_BaseViewModel::GetSequenceProp();

	g_sequence_hook = new recv_prop_hook(sequence_prop, &hooks::sequence_proxy_fn);
}

// If we aren't unloaded correctly (like when you close csgo)
// we should just leak the hooks, since the hooked instances
// might be already destroyed
auto uninitialize() -> void
{
	render::uninitialize();

	delete g_client_hook;
	delete g_game_event_manager_hook;

	delete g_sequence_hook;
}
