#pragma once

// Contains macros that define compile-time options.

#define REDEN_VERSION_NUMBER "0.0.0"

// Whether the keyboard shortcuts to switch windows should be ignored while the overlay is visible.
#define OVERLAY_PREVENTS_SWAPPING

// The name of the global variable in which the plugin instance is stored within a shared library.
constexpr const char *PLUGIN_GLOBAL_VARIABLE_NAME = "ext_plugin";

// The name of the folder in the user's home directory in which data is stored.
constexpr const char *DEFAULT_DATA_DIR  = ".config/reden";
constexpr const char *DEFAULT_CONFIG_DB = "client.conf";
constexpr const char *DEFAULT_ALIAS_DB  = "aliases.conf";

// Whether to disable performance testing. Saves a bit of memory and CPU usage.
#ifndef DISABLE_PERFORMANCE
#define DISABLE_PERFORMANCE
#endif
