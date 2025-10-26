#include "myplugin.h"
#include <src/managers/KeybindManager.hpp>


void MyPlugin::touch_window(const PHLWINDOW &window)
{
	debug_notification("Touching window with class {}", window->m_class);
	auto it = app_id_to_windows_map.find(window->m_class);
	if (it == app_id_to_windows_map.end())
		return; // window of an untracked app
	it->second.touch(window);
}

void MyPlugin::close_window(const PHLWINDOW &window)
{
	debug_notification("Closing window with class {}", window->m_class);
	auto it = app_id_to_windows_map.find(window->m_class);
	if (it == app_id_to_windows_map.end())
		return; // window of an untracked app
	it->second.erase(window);
}

void MyPlugin::focus_or_exec(int n) const
{
	debug_notification("focus or exec {}", n);

	if (quick_access_apps[n].app_id.empty() || quick_access_apps[n].command.empty()) {
		debug_notification("OOB");
		return;
	}

	if (auto window = app_id_to_windows_map.at(quick_access_apps[n].app_id).top()) {
		debug_notification("Focusing {}", quick_access_apps[n].app_id);
		g_pKeybindManager->switchToWindow(window->lock());
		// FIXME(deprecated): Replace with alterZOrder (with the right arguments)
		// FIXME: - (kitty, zed) in a group.
		//        - zed focused.
		//        - Focus zen in a scratch workspace.
		//        - Focus kitty with keybind: zed still on top.
		//        Groups are screwed up. Figure it out.
		g_pKeybindManager->bringActiveToTop("");
		return;
	}

	debug_notification("Executing {}", quick_access_apps[n].command);
	g_pKeybindManager->spawn(std::string(quick_access_apps[n].command));
}

void MyPlugin::load_config()
{
	for (size_t i = 0; i < NUM_QUICK_ACCESS_APPS; i++) {
		quick_access_apps[i] = QuickAccessApp{
		    .app_id = std::string_view(
		        *reinterpret_cast<const Hyprlang::STRING *>(HyprlandAPI::getConfigValue(
		            PHANDLE, std::format("plugin:myplugin:app_{}:class", i)
		        )->getDataStaticPtr()) // SAFETY: default "", so it cannot be nullptr
		    ),
		    .command = std::string_view(
		        *reinterpret_cast<const Hyprlang::STRING *>(HyprlandAPI::getConfigValue(
		            PHANDLE, std::format("plugin:myplugin:app_{}:command", i)
		        )->getDataStaticPtr()) // SAFETY: default "", so it cannot be nullptr
		    ),
		};
		if (auto app_id = quick_access_apps[i].app_id; !app_id.empty())
			app_id_to_windows_map.emplace(app_id, MRUList<PHLWINDOWREF>{});
		if (auto &[app_id, command] = quick_access_apps[i]; !(app_id.empty() && command.empty()))
			debug_notification("Quick access app {}: class = {}, command = {}", i, app_id, command);
	}
	seed_from_existing_windows(); // to track windows of any new apps added to config
}

void MyPlugin::seed_from_existing_windows()
{
	for (const auto &window : g_pCompositor->m_windowFocusHistory | std::views::reverse) {
		if (auto it = app_id_to_windows_map.find(window->m_class);
		    it != app_id_to_windows_map.end()) {
			it->second.touch(window);
		}
	}
}
