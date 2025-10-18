#define WLR_USE_UNSTABLE

#include "focusorlaunch.h"

#include <hyprland/src/Compositor.hpp>
#include <hyprland/src/desktop/Window.hpp>
#include <hyprland/src/config/ConfigManager.hpp>

EvenWindowsHasThis plugin;

APICALL EXPORT std::string PLUGIN_API_VERSION() { return HYPRLAND_API_VERSION; }

APICALL EXPORT PLUGIN_DESCRIPTION_INFO PLUGIN_INIT(HANDLE handle)
{
	PHANDLE = handle;

	if (__hyprland_api_get_hash() != std::string_view(GIT_COMMIT_HASH)) {
		HyprlandAPI::addNotification(
		    PHANDLE,
		    "[focusorlaunch] Failure in initialization: Version mismatch (headers ver != running ver)",
		    CHyprColor{1.0, 0.2, 0.2, 1.0},
		    5000
		);
		throw std::runtime_error("[focusorlaunch] version mismatch");
	}

	for (int i = 0; i < NUM_QUICK_ACCESS_APPS; i++) {
		HyprlandAPI::addConfigValue(
		    PHANDLE, std::format("plugin:focusorlaunch:app_{}:class", i), Hyprlang::STRING{""}
		);
		HyprlandAPI::addConfigValue(
		    PHANDLE, std::format("plugin:focusorlaunch:app_{}:command", i), Hyprlang::STRING{""}
		);
	}
	HyprlandAPI::reloadConfig();
	plugin.load_config();

	static auto PActive = HyprlandAPI::registerCallbackDynamic(
	    PHANDLE, "activeWindow", [&](void *, SCallbackInfo &, const std::any &data) {
	    	// Hyprland calls activeWindow with nullptr when switching workspaces
	    	if (auto window = std::any_cast<PHLWINDOW>(data))
				plugin.touch_window(window);
		}
	);
	static auto PClose = HyprlandAPI::registerCallbackDynamic(
	    PHANDLE, "closeWindow", [&](void *, SCallbackInfo &, const std::any &data) {
			if (auto window = std::any_cast<PHLWINDOW>(data))
				plugin.close_window(window);
	    }
	);
	static auto PConfig = HyprlandAPI::registerCallbackDynamic(
	    PHANDLE, "configReloaded", [&](void *, SCallbackInfo &, const std::any &) {
		    plugin.load_config();
	    }
	);

	bool success = true;
	for (int i = 0; i < NUM_QUICK_ACCESS_APPS; i++) {
		success = success
		          && HyprlandAPI::addDispatcherV2(
		              PHANDLE,
		              std::format("plugin:focusorlaunch:{}", i),
		              [i](const std::string &) -> SDispatchResult {
			              plugin.focus_or_launch(i);
			              return {};
		              }
		          );
	}
	if (!success) {
		HyprlandAPI::addNotification(
		    PHANDLE,
		    "[focusorlaunch] Failed to register dispatchers",
		    CHyprColor{1.0, 0.2, 0.2, 1.0},
		    5000
		);
		throw std::runtime_error("[focusorlaunch] Failed to register dispatchers");
	}

	debug_notification("Initialized");

	return {
	    "focusorlaunch", "A dispatcher to focus or launch an app.", "Aditya Singh", "0.1"
	};
}

APICALL EXPORT void PLUGIN_EXIT() {}
