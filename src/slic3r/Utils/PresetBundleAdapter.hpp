#ifndef slic3r_PresetBundleAdapter_hpp_
#define slic3r_PresetBundleAdapter_hpp_

#include "libslic3r/Preset.hpp"
#include "libslic3r/PresetBundle.hpp"

namespace Slic3r {

class AppConfig;

void set_preset_visible_from_appconfig(Preset& preset, const AppConfig& config);

PresetsConfigSubstitutions load_preset_bundle_presets(
    PresetBundle& bundle,
    AppConfig& config,
    ForwardCompatibilitySubstitutionRule rule,
    const PresetBundle::PresetPreferences& prefs = PresetBundle::PresetPreferences());

void load_preset_bundle_selections(
    PresetBundle& bundle,
    AppConfig& config,
    const PresetBundle::PresetPreferences& prefs = PresetBundle::PresetPreferences());

PresetsConfigSubstitutions load_preset_bundle_user_presets(
    PresetBundle& bundle,
    AppConfig& config,
    std::map<std::string, std::map<std::string, std::string>>& my_presets,
    ForwardCompatibilitySubstitutionRule rule);

void save_preset_bundle_user_presets(
    PresetBundle& bundle,
    AppConfig& config,
    std::vector<std::string>& need_to_delete_list);

void remove_preset_bundle_users_preset(
    PresetBundle& bundle,
    AppConfig& config,
    std::map<std::string, std::map<std::string, std::string>>* my_presets = nullptr);

void export_preset_bundle_selections(PresetBundle& bundle, AppConfig& config);

void update_preset_bundle_selections(PresetBundle& bundle, AppConfig& config);

void load_preset_bundle_installed_printers(PresetBundle& bundle, const AppConfig& config);

void load_preset_bundle_installed_filaments(PresetBundle& bundle, AppConfig& config);

void load_preset_bundle_installed_sla_materials(PresetBundle& bundle, AppConfig& config);

}

#endif
