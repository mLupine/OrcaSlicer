#include "PresetBundleAdapter.hpp"
#include "AppConfig.hpp"
#include "libslic3r/PrintConfig.hpp"

#include <boost/algorithm/string.hpp>
#include <boost/range/adaptor/transformed.hpp>
#include <boost/lexical_cast.hpp>

namespace Slic3r {

static inline std::string remove_ini_suffix(const std::string &name)
{
    std::string ret = name;
    const char *suffix = ".ini";
    if (boost::algorithm::iends_with(ret, suffix))
        ret.erase(ret.end() - strlen(suffix), ret.end());
    return ret;
}

void set_preset_visible_from_appconfig(Preset& preset, const AppConfig& config)
{
    PresetVisibilityConfig visibility_config;
    visibility_config.installed_variants = config.vendors();
    if (config.has_section(AppConfig::SECTION_FILAMENTS)) {
        for (const auto &kv : config.get_section(AppConfig::SECTION_FILAMENTS))
            if (!kv.second.empty())
                visibility_config.installed_filaments.insert(kv.first);
    }
    if (config.has_section(AppConfig::SECTION_MATERIALS)) {
        for (const auto &kv : config.get_section(AppConfig::SECTION_MATERIALS))
            if (!kv.second.empty())
                visibility_config.installed_materials.insert(kv.first);
    }
    preset.set_visible_from_config(visibility_config);
}

PresetsConfigSubstitutions load_preset_bundle_presets(
    PresetBundle& bundle,
    AppConfig& config,
    ForwardCompatibilitySubstitutionRule rule,
    const PresetBundle::PresetPreferences& prefs)
{
    PresetsConfigSubstitutions substitutions;
    std::string errors_cummulative;

    std::tie(substitutions, errors_cummulative) = bundle.load_system_presets_from_json(rule);

    std::string dir_user_presets = config.get("preset_folder");
    if (dir_user_presets.empty()) {
        bundle.load_user_presets(DEFAULT_USER_FOLDER_NAME, rule);
    } else {
        bundle.load_user_presets(dir_user_presets, rule);
    }

    bundle.update_multi_material_filament_presets();
    bundle.update_compatible(PresetSelectCompatibleType::Never);

    load_preset_bundle_selections(bundle, config, prefs);

    bundle.set_calibrate_printer("");

    return substitutions;
}

void load_preset_bundle_selections(
    PresetBundle& bundle,
    AppConfig& config,
    const PresetBundle::PresetPreferences& prefs)
{
    load_preset_bundle_installed_printers(bundle, config);
    load_preset_bundle_installed_filaments(bundle, config);
    load_preset_bundle_installed_sla_materials(bundle, config);

    std::string initial_printer_profile_name = remove_ini_suffix(config.get("presets", PRESET_PRINTER_NAME));

    PrinterSettingsConfig settings_config;
    settings_config.printer_name = initial_printer_profile_name;
    settings_config.print_profile = config.get_printer_setting(initial_printer_profile_name, PRESET_PRINT_NAME);
    settings_config.primary_filament = config.get_printer_setting(initial_printer_profile_name, PRESET_FILAMENT_NAME);

    for (unsigned int i = 1; i < 1000; ++i) {
        char name[64];
        sprintf(name, "filament_%02u", i);
        auto f_name = config.get_printer_setting(initial_printer_profile_name, name);
        if (f_name.empty())
            break;
        settings_config.additional_filaments.push_back(f_name);
    }

    auto f_colors = config.get_printer_setting(initial_printer_profile_name, "filament_colors");
    if (!f_colors.empty()) {
        boost::algorithm::split(settings_config.filament_colors, f_colors, boost::algorithm::is_any_of(","));
    }

    if (config.has_printer_setting(initial_printer_profile_name, "filament_multi_colors")) {
        boost::algorithm::split(settings_config.filament_multi_colors,
            config.get_printer_setting(initial_printer_profile_name, "filament_multi_colors"),
            boost::algorithm::is_any_of(","));
    }

    if (config.has_printer_setting(initial_printer_profile_name, "filament_color_types")) {
        boost::algorithm::split(settings_config.filament_color_types,
            config.get_printer_setting(initial_printer_profile_name, "filament_color_types"),
            boost::algorithm::is_any_of(","));
    }

    std::vector<std::string> extruder_ams_count_str;
    if (config.has_printer_setting(initial_printer_profile_name, "extruder_ams_count")) {
        boost::algorithm::split(extruder_ams_count_str,
            config.get_printer_setting(initial_printer_profile_name, "extruder_ams_count"),
            boost::algorithm::is_any_of(","));
    }
    settings_config.extruder_ams_counts = get_extruder_ams_count(extruder_ams_count_str);

    std::vector<std::string> matrix;
    if (config.has_printer_setting(initial_printer_profile_name, "flush_volumes_matrix")) {
        boost::algorithm::split(matrix, config.get_printer_setting(initial_printer_profile_name, "flush_volumes_matrix"), boost::algorithm::is_any_of("|"));
        auto flush_volumes_matrix = matrix | boost::adaptors::transformed(boost::lexical_cast<double, std::string>);
        settings_config.flush_volumes_matrix = std::vector<double>(flush_volumes_matrix.begin(), flush_volumes_matrix.end());
    }
    if (config.has_printer_setting(initial_printer_profile_name, "flush_volumes_vector")) {
        boost::algorithm::split(matrix, config.get_printer_setting(initial_printer_profile_name, "flush_volumes_vector"), boost::algorithm::is_any_of("|"));
        auto flush_volumes_vector = matrix | boost::adaptors::transformed(boost::lexical_cast<double, std::string>);
        settings_config.flush_volumes_vector = std::vector<double>(flush_volumes_vector.begin(), flush_volumes_vector.end());
    }
    if (config.has_printer_setting(initial_printer_profile_name, "flush_multiplier")) {
        boost::algorithm::split(matrix, config.get_printer_setting(initial_printer_profile_name, "flush_multiplier"), boost::algorithm::is_any_of("|"));
        auto flush_multipliers = matrix | boost::adaptors::transformed(boost::lexical_cast<double, std::string>);
        settings_config.flush_multipliers = std::vector<double>(flush_multipliers.begin(), flush_multipliers.end());
    }

    bundle.load_selections(settings_config, prefs);

    const Preset& current_printer = bundle.printers.get_selected_preset();
    const Preset* base_printer = bundle.printers.get_preset_base(current_printer);
    bool use_default_nozzle_volume_type = true;
    if (base_printer) {
        std::string prev_nozzle_volume_type = config.get_nozzle_volume_types_from_config(base_printer->name);
        if (!prev_nozzle_volume_type.empty()) {
            ConfigOptionEnumsGeneric* nozzle_volume_type_option = bundle.project_config.option<ConfigOptionEnumsGeneric>("nozzle_volume_type");
            if (nozzle_volume_type_option->deserialize(prev_nozzle_volume_type)) {
                use_default_nozzle_volume_type = false;
            }
        }
    }

    if (use_default_nozzle_volume_type) {
        bundle.project_config.option<ConfigOptionEnumsGeneric>("nozzle_volume_type")->values = current_printer.config.option<ConfigOptionEnumsGeneric>("default_nozzle_volume_type")->values;
    }

    std::string initial_physical_printer_name = remove_ini_suffix(config.get("presets", "physical_printer"));
    if (!initial_physical_printer_name.empty())
        bundle.physical_printers.select_printer(initial_physical_printer_name);
}

PresetsConfigSubstitutions load_preset_bundle_user_presets(
    PresetBundle& bundle,
    AppConfig& config,
    std::map<std::string, std::map<std::string, std::string>>& my_presets,
    ForwardCompatibilitySubstitutionRule rule)
{
    std::string user_sub_folder = DEFAULT_USER_FOLDER_NAME;
    if (!config.get("preset_folder").empty())
        user_sub_folder = config.get("preset_folder");

    return bundle.load_user_presets(user_sub_folder, rule);
}

void save_preset_bundle_user_presets(
    PresetBundle& bundle,
    AppConfig& config,
    std::vector<std::string>& need_to_delete_list)
{
    std::string user_sub_folder = DEFAULT_USER_FOLDER_NAME;
    if (!config.get("preset_folder").empty())
        user_sub_folder = config.get("preset_folder");

    bundle.save_user_presets(user_sub_folder, need_to_delete_list);
}

void remove_preset_bundle_users_preset(
    PresetBundle& bundle,
    AppConfig& config,
    std::map<std::string, std::map<std::string, std::string>>* my_presets)
{
    std::string preset_folder_user_id = config.get("preset_folder");
    bundle.remove_users_preset(preset_folder_user_id, my_presets);
}

void export_preset_bundle_selections(PresetBundle& bundle, AppConfig& config)
{
    config.clear_section("presets");

    auto exported = bundle.export_selections();
    exported.current_bed_type = config.get("curr_bed_type");

    config.set("presets", PRESET_PRINTER_NAME, exported.printer_name);

    config.clear_printer_settings(exported.printer_name);
    config.set_printer_setting(exported.printer_name, PRESET_PRINTER_NAME, exported.printer_name);
    config.set_printer_setting(exported.printer_name, PRESET_PRINT_NAME, exported.print_profile);
    config.set_printer_setting(exported.printer_name, PRESET_FILAMENT_NAME, exported.filament_presets.front());
    config.set_printer_setting(exported.printer_name, "curr_bed_type", exported.current_bed_type);

    for (unsigned i = 1; i < exported.filament_presets.size(); ++i) {
        char name[64];
        assert(!exported.filament_presets[i].empty());
        sprintf(name, "filament_%02u", i);
        config.set_printer_setting(exported.printer_name, name, exported.filament_presets[i]);
    }

    std::string filament_colors = boost::algorithm::join(exported.filament_colors, ",");
    config.set_printer_setting(exported.printer_name, "filament_colors", filament_colors);

    std::string filament_multi_colors = boost::algorithm::join(exported.filament_multi_colors, ",");
    config.set_printer_setting(exported.printer_name, "filament_multi_colors", filament_multi_colors);

    std::string filament_color_types = boost::algorithm::join(exported.filament_color_types, ",");
    config.set_printer_setting(exported.printer_name, "filament_color_types", filament_color_types);

    std::string extruder_ams_count_str = boost::algorithm::join(save_extruder_ams_count_to_string(exported.extruder_ams_counts), ",");
    config.set_printer_setting(exported.printer_name, "extruder_ams_count", extruder_ams_count_str);

    std::string flush_volumes_matrix = boost::algorithm::join(exported.flush_volumes_matrix |
                                                             boost::adaptors::transformed(static_cast<std::string (*)(double)>(std::to_string)),
                                                         "|");
    config.set_printer_setting(exported.printer_name, "flush_volumes_matrix", flush_volumes_matrix);

    std::string flush_volumes_vector = boost::algorithm::join(exported.flush_volumes_vector |
                                                             boost::adaptors::transformed(static_cast<std::string (*)(double)>(std::to_string)),
                                                         "|");
    config.set_printer_setting(exported.printer_name, "flush_volumes_vector", flush_volumes_vector);

    std::string flush_multiplier_str = boost::algorithm::join(exported.flush_multipliers |
                                                                  boost::adaptors::transformed(static_cast<std::string (*)(double)>(std::to_string)),
                                                              "|");
    config.set_printer_setting(exported.printer_name, "flush_multiplier", flush_multiplier_str);
}

void update_preset_bundle_selections(PresetBundle& bundle, AppConfig& config)
{
    std::string initial_printer_profile_name = bundle.printers.get_selected_preset_name();

    PrinterSettingsConfig settings_config;
    settings_config.printer_name = initial_printer_profile_name;
    settings_config.print_profile = config.get_printer_setting(initial_printer_profile_name, PRESET_PRINT_NAME);
    settings_config.primary_filament = config.get_printer_setting(initial_printer_profile_name, PRESET_FILAMENT_NAME);

    for (unsigned int i = 1; i < 1000; ++i) {
        char name[64];
        sprintf(name, "filament_%02u", i);
        auto f_name = config.get_printer_setting(initial_printer_profile_name, name);
        if (f_name.empty())
            break;
        settings_config.additional_filaments.push_back(f_name);
    }

    auto f_colors = config.get_printer_setting(initial_printer_profile_name, "filament_colors");
    if (!f_colors.empty()) {
        boost::algorithm::split(settings_config.filament_colors, f_colors, boost::algorithm::is_any_of(","));
    }

    if (config.has_printer_setting(initial_printer_profile_name, "filament_multi_colors")) {
        boost::algorithm::split(settings_config.filament_multi_colors,
            config.get_printer_setting(initial_printer_profile_name, "filament_multi_colors"),
            boost::algorithm::is_any_of(","));
    }

    if (config.has_printer_setting(initial_printer_profile_name, "filament_color_types")) {
        boost::algorithm::split(settings_config.filament_color_types,
            config.get_printer_setting(initial_printer_profile_name, "filament_color_types"),
            boost::algorithm::is_any_of(","));
    }

    std::vector<std::string> extruder_ams_count_str;
    if (config.has_printer_setting(initial_printer_profile_name, "extruder_ams_count")) {
        boost::algorithm::split(extruder_ams_count_str,
            config.get_printer_setting(initial_printer_profile_name, "extruder_ams_count"),
            boost::algorithm::is_any_of(","));
    }
    settings_config.extruder_ams_counts = get_extruder_ams_count(extruder_ams_count_str);

    std::vector<std::string> matrix;
    if (config.has_printer_setting(initial_printer_profile_name, "flush_volumes_matrix")) {
        boost::algorithm::split(matrix, config.get_printer_setting(initial_printer_profile_name, "flush_volumes_matrix"), boost::algorithm::is_any_of("|"));
        auto flush_volumes_matrix = matrix | boost::adaptors::transformed(boost::lexical_cast<double, std::string>);
        settings_config.flush_volumes_matrix = std::vector<double>(flush_volumes_matrix.begin(), flush_volumes_matrix.end());
    }
    if (config.has_printer_setting(initial_printer_profile_name, "flush_volumes_vector")) {
        boost::algorithm::split(matrix, config.get_printer_setting(initial_printer_profile_name, "flush_volumes_vector"), boost::algorithm::is_any_of("|"));
        auto flush_volumes_vector = matrix | boost::adaptors::transformed(boost::lexical_cast<double, std::string>);
        settings_config.flush_volumes_vector = std::vector<double>(flush_volumes_vector.begin(), flush_volumes_vector.end());
    }
    if (config.has_printer_setting(initial_printer_profile_name, "flush_multiplier")) {
        boost::algorithm::split(matrix, config.get_printer_setting(initial_printer_profile_name, "flush_multiplier"), boost::algorithm::is_any_of("|"));
        auto flush_multipliers = matrix | boost::adaptors::transformed(boost::lexical_cast<double, std::string>);
        settings_config.flush_multipliers = std::vector<double>(flush_multipliers.begin(), flush_multipliers.end());
    }

    bundle.update_selections(settings_config);
}

void load_preset_bundle_installed_printers(PresetBundle& bundle, const AppConfig& config)
{
    PresetVisibilityConfig visibility_config;
    visibility_config.installed_variants = config.vendors();

    bundle.load_installed_printers(visibility_config);
}

void load_preset_bundle_installed_filaments(PresetBundle& bundle, AppConfig& config)
{
    InstalledPresets installed;
    if (config.has_section(AppConfig::SECTION_FILAMENTS)) {
        installed.filament_section = config.get_section(AppConfig::SECTION_FILAMENTS);
        for (const auto &kv : installed.filament_section) {
            if (!kv.second.empty())
                installed.visibility.installed_filaments.insert(kv.first);
        }
    }

    auto update = bundle.load_installed_filaments(installed);

    for (const auto &filament_name : update.filaments_to_install) {
        config.set(AppConfig::SECTION_FILAMENTS, filament_name, "true");
    }
}

void load_preset_bundle_installed_sla_materials(PresetBundle& bundle, AppConfig& config)
{
    bool has_materials_section = config.has_section(AppConfig::SECTION_MATERIALS);
    auto update = bundle.load_installed_sla_materials(has_materials_section);

    for (const auto &material_name : update.materials_to_install) {
        config.set(AppConfig::SECTION_MATERIALS, material_name, "true");
    }

    for (auto &preset : bundle.sla_materials)
        set_preset_visible_from_appconfig(preset, config);
}

}
