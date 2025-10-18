#pragma once

#include <any>
#include <chrono>
#include <algorithm>
#include <unordered_map>
#include <string>
#include <vector>

#define private public // CKeybindManager::{switchToWindow, spawn}
#include <hyprlang.hpp>
#include <hyprland/src/Compositor.hpp>
#include <hyprland/src/plugins/PluginAPI.hpp>
#include <hyprland/src/managers/KeybindManager.hpp>
#include <hyprland/src/desktop/Window.hpp>
#include <hyprland/src/helpers/Monitor.hpp>
#undef private

inline HANDLE PHANDLE = nullptr;

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

class EvenWindowsHasThis {
	/// Stores the app IDs and launch commands for quick access (e.g., with
	/// super-number).
	std::array<QuickAccessApp, NUM_QUICK_ACCESS_APPS>           quick_access_apps;
	/// Stores the window list for each tracked app ID.
	std::unordered_map<std::string_view, MRUList<PHLWINDOWREF>> app_id_to_windows_map;

public:
	/// Note: Must call `EvenWindowsHasThis::load_config()` manually in the
	/// plugin init function.
	EvenWindowsHasThis() { seed_from_existing_windows(); }
	/// Load configuration.
	void load_config();
	/// Update a window's last used status.
	void touch_window(const PHLWINDOW &window);
	/// Remove a window.
	void close_window(const PHLWINDOW &window);
	/// Focus the last used window of the `n`-th quick access app or launch it.
	void focus_or_launch(int n) const;

private:
	/// Set state using currently open windows.
	void seed_from_existing_windows();
};

template <typename... Args>
static void debug_notification(std::string_view fmt_string, Args &&...fmt_args)
{
#ifdef DEBUG
	HyprlandAPI::addNotification(
		PHANDLE,
		std::format("[focusorlaunch] {}", std::vformat(fmt_string, std::make_format_args(fmt_args...))),
		CHyprColor{1.0, 0.2, 0.2, 1.0},
		5000
	);
#endif
}
