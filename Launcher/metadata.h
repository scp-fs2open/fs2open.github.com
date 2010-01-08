const int FLAG_TYPE_LEN = 16;

typedef struct
{
	char *exe_name;
	char *company;
	char *regname;
	int   flags;

} ExeType;

typedef struct
{
	char  name[20];				// The actual flag
	char  desc[40];				// The text that will appear in the launcher
	bool  fso_only;				// true if this is a fs2_open only feature
	int   on_flags;				// Easy flag which will turn this feature on
	int   off_flags;			// Easy flag which will turn this feature off
	char  type[FLAG_TYPE_LEN];	// Launcher uses this to put flags under different headings
	char  web_url[256];			// Link to documentation of feature (please use wiki or somewhere constant)

} Flag;

typedef struct
{
	char name[32];

} EasyFlag;


extern char *exe_types_string[];
extern ExeType exe_types[];

extern Flag retail_params_FS2[];
extern int Num_retail_params_FS2;
extern Flag retail_params_FS1[];
extern int Num_retail_params_FS1;
