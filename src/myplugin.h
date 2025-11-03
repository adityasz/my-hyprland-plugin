#pragma once

#include <any>
#include <chrono>
#include <algorithm>
#include <hyprutils/math/Vector2D.hpp>
#include <src/SharedDefs.hpp>
#include <src/debug/Log.hpp>
#include <unordered_map>
#include <string>
#include <vector>

#define private public // CKeybindManager::{switchToWindow, bringActiveToTop, spawn}
#include <hyprlang.hpp>
#include <hyprland/src/Compositor.hpp>
#include <hyprland/src/plugins/PluginAPI.hpp>
#include <hyprland/src/managers/KeybindManager.hpp>
#include <hyprland/src/desktop/Window.hpp>
#include <hyprland/src/helpers/Monitor.hpp>
#undef private

static constexpr int NUM_QUICK_ACCESS_APPS = 47;

struct QuickAccessApp {
	/// The Wayland application ID of the app's windows, used to group them
	/// together.
	///
	/// My understanding is that Hyprland's `class` is the same thing, and
	/// hence, in the config file, I call it `class`.
	std::string_view app_id;
	/// The command used to launch the app.
	std::string_view command;
};

template <typename T>
struct MRUList {
	std::vector<T> data;

	MRUList() { data.reserve(10); }

	void touch(const T &v)
	{
		if (auto it = std::ranges::find(data, v); it == data.end())
			data.insert(data.begin(), v);
		else if (it != data.begin())
			std::rotate(data.begin(), it, it + 1);
	}

	void erase(const T &v) { std::erase(data, v); }

	[[nodiscard]] std::optional<T> top() const
	{
		if (!data.empty())
			return data.front();
		return {};
	}
};

class MyPlugin {
	/// Stores the app IDs and launch commands for quick access
	/// (e.g., with super-number).
	std::array<QuickAccessApp, NUM_QUICK_ACCESS_APPS>           quick_access_apps;
	/// Stores the window list for each tracked app ID.
	std::unordered_map<std::string_view, MRUList<PHLWINDOWREF>> app_id_to_windows_map;

public:
	HANDLE phandle = nullptr;

	/// Note: Must call `EvenWindowsHasThis::load_config()` manually in the
	/// plugin init function.
	MyPlugin() { seed_from_existing_windows(); }

	/// Load configuration.
	///
	/// This is configured differently from the moveorexec dispatcher because
	/// passing around strings and finding commas in them is a lot slower than
	/// converting (usually just) one ascii character to an integer and using
	/// that to index into an array.
	void load_config();

	/// Update a window's last used status.
	void        touch_window(const PHLWINDOW &window);
	/// Remove a window.
	void        close_window(const PHLWINDOW &window);
	/// Create a group if the window is tiled and not in a group.
	/// Destroy the group if the window is floating and in a group.
	static void window_update_rules(const PHLWINDOW &window);

	/// Launch (a new window of) the `n`-th quick access app.
	SDispatchResult exec(int n) const;
	/// Focus the last used window of the `n`-th quick access app or launch it.
	SDispatchResult focus_or_exec(int n) const;
	/// Focus the last used window of the `n`-th quick access app after moving it to the current workspace if needed, or launch it.
	SDispatchResult move_or_exec(int n) const;
#ifdef TILES_ARE_GROUPS
	/// Focus the last used window of the `n`-th quick access app after inserting it into the current group, or launch it.
	SDispatchResult        move_into_group_or_exec(int n) const;
#endif

private:
	/// Set state using currently open windows.
	void seed_from_existing_windows();
};

#ifdef DEBUG_LOGS
template <typename... Args>
static void log(eLogLevel level, std::format_string<Args...> fmt_string, Args &&...fmt_args)
{
	Debug::log(level, "[myplugin] {}", std::format(fmt_string, std::forward<Args>(fmt_args)...));
}
#else
template <typename... Args>
static void
log([[maybe_unused]] eLogLevel                   level,
    [[maybe_unused]] std::format_string<Args...> fmt,
    [[maybe_unused]] Args &&...fmt_args)
{}
#endif
