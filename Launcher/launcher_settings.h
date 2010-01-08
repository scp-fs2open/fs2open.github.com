#ifndef __LAUNCHER_SETTINGS_H__
#define __LAUNCHER_SETTINGS_H__

class LauncherSettings
{
	public:
		static bool load();
		static bool save();

		static char *get_exe_pathonly();
		static char *get_exe_nameonly();
		static char *get_exe_filepath();
		static bool is_exe_path_valid();
		static void set_exe_filepath(const char *path);

		static char *get_reg_path();
		static int get_exe_type();
		static void set_reg_path(const char *reg_path, int exe_type);

		static char *get_active_mod();
		static void set_active_mod(const char *active_mod);

		static bool get_showed_pilot_warning();
		static void set_showed_pilot_warning(bool showed_pilot_warning);

		static bool is_openal_build();
		static void set_openal_build(bool flag);

	private:
		static char m_exe_filepath[MAX_PATH];
		static char m_exe_pathonly[MAX_PATH];
		static char m_exe_nameonly[MAX_PATH];
		static bool m_is_exe_path_valid;

		static char m_reg_path[MAX_PATH];
		static int m_exe_type;

		static char m_active_mod[MAX_PATH];

		static bool m_showed_pilot_warning;

		static bool m_is_openal_build;
};

#endif
