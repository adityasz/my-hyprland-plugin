#include "myplugin.h"
#include <src/SharedDefs.hpp>
#include <src/managers/KeybindManager.hpp>
#include <src/managers/LayoutManager.hpp>

#define LOG_TRACE(fmt, ...) log(TRACE, "{}: " fmt, __func__, __VA_ARGS__)

static SDispatchResult oob(int n)
{
	auto error =
	    std::format("app_{} is not registered or has an empty application ID or command", n);
	log(ERR, "{}", error);
	return {.success = false, .error = error};
}

void MyPlugin::touch_window(const PHLWINDOW &window)
{
	LOG_TRACE("{}", window->m_class);
	auto it = app_id_to_windows_map.find(window->m_class);
	if (it == app_id_to_windows_map.end())
		return; // window of an untracked app
	it->second.touch(window);
}

void MyPlugin::close_window(const PHLWINDOW &window)
{
	LOG_TRACE("{}", window->m_class);
	auto it = app_id_to_windows_map.find(window->m_class);
	if (it == app_id_to_windows_map.end())
		return; // window of an untracked app
	it->second.erase(window);
}

void MyPlugin::window_update_rules(const PHLWINDOW &window)
{
	LOG_TRACE("{}", window->m_class);
	if (!window->m_isFloating && window->m_groupData.pNextWindow.expired())
		window->createGroup();
	if (window->m_isFloating && !window->m_groupData.pNextWindow.expired())
		window->destroyGroup();
}

SDispatchResult MyPlugin::exec(int n) const
{
	LOG_TRACE("{}", n);

	if (quick_access_apps[n].app_id.empty() || quick_access_apps[n].command.empty())
		return oob(n);

	log(INFO, "executing {}", quick_access_apps[n].command);
	CKeybindManager::spawn(std::string(quick_access_apps[n].command));
	return {};
}

SDispatchResult MyPlugin::focus_or_exec(int n) const
{
	LOG_TRACE("{}", n);

	if (quick_access_apps[n].app_id.empty() || quick_access_apps[n].command.empty())
		return oob(n);

	if (auto window = app_id_to_windows_map.at(quick_access_apps[n].app_id)
	                      .top()
	                      .value_or(PHLWINDOWREF{})
	                      .lock()) {
		log(INFO, "focusing {} (class={})", static_cast<void *>(window.get()), window->m_class);
		auto last_monitor = g_pCompositor->m_lastMonitor;
		if (last_monitor && last_monitor->m_activeSpecialWorkspace
		    && window->m_workspace != last_monitor->m_activeSpecialWorkspace) {
			last_monitor->setSpecialWorkspace(nullptr);
		}
		g_pCompositor->focusWindow(window);
		if (!window->m_groupData.pNextWindow.expired())
			window->setGroupCurrent(window);
		if (window->m_isFloating)
			g_pCompositor->changeWindowZOrder(window, true);
		return {};
	}

	log(INFO, "executing {}", quick_access_apps[n].command);
	CKeybindManager::spawn(std::string(quick_access_apps[n].command));
	return {};
}

SDispatchResult MyPlugin::move_or_exec(int n) const
{
	LOG_TRACE("{}", n);

	if (quick_access_apps[n].app_id.empty() || quick_access_apps[n].command.empty())
		return oob(n);

	if (auto window = app_id_to_windows_map.at(quick_access_apps[n].app_id)
	                      .top()
	                      .value_or(PHLWINDOWREF{})
	                      .lock()) {
		if (auto active_workspace = g_pCompositor->m_lastMonitor->m_activeWorkspace;
		    window->m_workspace != active_workspace) {
			log(INFO,
			    "moving {} (class={}) from workspace '{}' to the active workspace '{}'",
			    static_cast<void *>(window.get()),
			    window->m_class,
			    window->m_workspace->m_name,
			    active_workspace->m_name);
			if (!window->m_groupData.pNextWindow.expired())
				CKeybindManager::moveWindowOutOfGroup(window);
			g_pCompositor->moveWindowToWorkspaceSafe(window, active_workspace);
#ifdef TILES_ARE_GROUPS
			if (!window->m_isFloating)
				window->createGroup();
#endif
		} else {
			g_pCompositor->warpCursorTo(window->middle());
		}
		log(INFO, "focusing {} (class={})", static_cast<void *>(window.get()), window->m_class);
		g_pCompositor->focusWindow(window);
		if (window->m_isFloating)
			g_pCompositor->changeWindowZOrder(window, true);
		return {};
	}

	log(INFO, "executing {}", quick_access_apps[n].command);
	CKeybindManager::spawn(std::string(quick_access_apps[n].command));
	return {};
}

#ifdef TILES_ARE_GROUPS
SDispatchResult MyPlugin::move_into_group_or_exec(int n) const
{
	LOG_TRACE("{}", n);

	if (quick_access_apps[n].app_id.empty() || quick_access_apps[n].command.empty())
		return oob(n);

	auto active_window = g_pCompositor->m_lastWindow.lock();
	auto window =
	    app_id_to_windows_map.at(quick_access_apps[n].app_id).top().value_or(PHLWINDOWREF{}).lock();
	if (!active_window || active_window->m_isFloating || !window || window->m_isFloating)
		return move_or_exec(n);

	// window is tiled and grouped
	log(INFO, "moving and focusing {}", quick_access_apps[n].app_id);
	CKeybindManager::moveWindowOutOfGroup(window);
	if (window->m_workspace != active_window->m_workspace)
		g_pCompositor->moveWindowToWorkspaceSafe(window, active_window->m_workspace);
	CKeybindManager::moveWindowIntoGroup(window, active_window);
	window->setGroupCurrent(window);
	return {};
}
#endif

void MyPlugin::load_config()
{
	for (int i = 0; i < NUM_QUICK_ACCESS_APPS; i++) {
		quick_access_apps[i] = QuickAccessApp{
		    .app_id = std::string_view(
		        *reinterpret_cast<const Hyprlang::STRING *>(
		            HyprlandAPI::getConfigValue(
		                phandle, std::format("plugin:myplugin:app_{}:class", i)
		            )
		                ->getDataStaticPtr()
		        ) // SAFETY: default "", so it cannot be nullptr
		    ),
		    .command = std::string_view(
		        *reinterpret_cast<const Hyprlang::STRING *>(
		            HyprlandAPI::getConfigValue(
		                phandle, std::format("plugin:myplugin:app_{}:command", i)
		            )
		                ->getDataStaticPtr()
		        ) // SAFETY: default "", so it cannot be nullptr
		    ),
		};
		if (auto app_id = quick_access_apps[i].app_id; !app_id.empty())
			app_id_to_windows_map.emplace(app_id, MRUList<PHLWINDOWREF>{});
		if (auto &[app_id, command] = quick_access_apps[i]; !(app_id.empty() && command.empty()))
			log(INFO, "app {}: class={}, command={}", i, app_id, command);
	}
	log(INFO, "config loaded");
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
