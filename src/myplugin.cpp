#include "myplugin.h"
#include <src/managers/KeybindManager.hpp>
#include <src/managers/LayoutManager.hpp>

static void focus_window(PHLWINDOWREF window)
{
	g_pKeybindManager->switchToWindow(window.lock());
	if (!window->m_groupData.pNextWindow.expired())
		window->setGroupCurrent(window.lock());
	else if (!window->m_isFloating)
		window->createGroup();
	if (window->m_isFloating)
	    g_pCompositor->changeWindowZOrder(window.lock(), true);
	return;
}

void MyPlugin::touch_window(const PHLWINDOW &window)
{
	debug_notification("{}: TOUCH {}", window->m_isFloating, window->m_class);
	auto it = app_id_to_windows_map.find(window->m_class);
	if (it == app_id_to_windows_map.end())
		return; // window of an untracked app
	it->second.touch(window);
}

void MyPlugin::close_window(const PHLWINDOW &window)
{
	debug_notification("CLOSE {}", window->m_class);
	auto it = app_id_to_windows_map.find(window->m_class);
	if (it == app_id_to_windows_map.end())
		return; // window of an untracked app
	it->second.erase(window);
}

void MyPlugin::window_update_rules(const PHLWINDOW &window)
{
	debug_notification("WINDOWRULES {}", window->m_class);
	if (!window->m_isFloating && window->m_groupData.pNextWindow.expired())
		window->createGroup();
	if (window->m_isFloating && !window->m_groupData.pNextWindow.expired())
		window->destroyGroup();
}

void MyPlugin::focus_or_exec(int n) const
{
	debug_notification("FOCUSOREXEC {}", n);

	if (quick_access_apps[n].app_id.empty() || quick_access_apps[n].command.empty()) {
		debug_notification("OOB");
		return;
	}

	if (auto window =
	        app_id_to_windows_map.at(quick_access_apps[n].app_id).top().value_or(PHLWINDOWREF{})) {
		debug_notification("Focusing {}", quick_access_apps[n].app_id);
		focus_window(window);
		return;
	}

	debug_notification("Executing {}", quick_access_apps[n].command);
	g_pKeybindManager->spawn(std::string(quick_access_apps[n].command));
}

void MyPlugin::move_or_exec(int n) const
{
	debug_notification("MOVEOREXEC {}", n);

	if (quick_access_apps[n].app_id.empty() || quick_access_apps[n].command.empty()) {
		debug_notification("OOB");
		return;
	}

	if (auto window =
	        app_id_to_windows_map.at(quick_access_apps[n].app_id).top().value_or(PHLWINDOWREF{})) {
		if (auto active_workspace = g_pCompositor->m_lastMonitor->m_activeWorkspace;
		    window->m_workspace != active_workspace) {
			debug_notification("Moving {}", quick_access_apps[n].app_id);
			if (window->m_groupData.pNextWindow.lock())
				CKeybindManager::moveWindowOutOfGroup(window.lock());
#ifdef GROUP_TILED_WINDOWS
			if (!window->m_isFloating)
				window->createGroup();
#endif
			g_pCompositor->moveWindowToWorkspaceSafe(window.lock(), active_workspace);
		}
		debug_notification("Focusing {}", quick_access_apps[n].app_id);
		focus_window(window);
		return;
	}

	debug_notification("Executing {}", quick_access_apps[n].command);
	g_pKeybindManager->spawn(std::string(quick_access_apps[n].command));
}

void MyPlugin::exec(int n) const
{
	debug_notification("EXEC {}", n);

	if (quick_access_apps[n].app_id.empty() || quick_access_apps[n].command.empty()) {
		debug_notification("OOB");
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
		            phandle, std::format("plugin:myplugin:app_{}:class", i)
		        )->getDataStaticPtr()) // SAFETY: default "", so it cannot be nullptr
		    ),
		    .command = std::string_view(
		        *reinterpret_cast<const Hyprlang::STRING *>(HyprlandAPI::getConfigValue(
		            phandle, std::format("plugin:myplugin:app_{}:command", i)
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
