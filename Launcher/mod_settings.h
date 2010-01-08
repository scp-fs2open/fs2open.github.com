#ifndef __MOD_SETTINGS_H__
#define __MOD_SETTINGS_H__

class ModSettings
{
	public:
		static bool load_mod();
		static bool load_user();
		static bool save_user();

		static char *get_mod_parameters();

		static char *get_standard_parameters();
		static void set_standard_parameters(const char *standard_params);

		static char *get_custom_parameters();
		static void set_custom_parameters(const char *custom_params);

	private:
		static bool load_user_fso();
		static bool load_user_retail();

		static char m_mod_params[1024];
		static char m_standard_params[1024];
		static char m_custom_params[1024];
};

#endif
