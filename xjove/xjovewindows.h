#include "tune.h"

#define FUNCTION(key_string, string)	MENU_ITEM, MENU_STRING, key_string, \
				MENU_CLIENT_DATA, string, 0,
#define GROUP(menu)		menu = menu_create( \
					MENU_NOTIFY_PROC, print_client_data,
#define GROUPNAME(menu, name)	MENU_ITEM, MENU_STRING, name, \
				MENU_CLIENT_DATA,  "", \
				MENU_PULLRIGHT, menu, 0,
#define FUNCPULL(key_string, string, menu) \
				MENU_ITEM, MENU_STRING, key_string, \
				MENU_CLIENT_DATA, string, \
				MENU_PULLRIGHT, menu, 0,
#define FUNCPULLPROC(key_string, string, menu_proc) \
				MENU_ITEM, MENU_STRING, key_string, \
				MENU_CLIENT_DATA, string, \
				MENU_GEN_PULLRIGHT, menu_proc, 0,
#define VARIABLE(string)	FUNCTION(string, string)
#define VARIBOOL(string)	MENU_ITEM, MENU_STRING,  string, \
				MENU_CLIENT_DATA, string, \
				MENU_GEN_PROC, on_off_proc, 0,
#define VARGROUP(menu)		menu = menu_create( \
					MENU_NOTIFY_PROC, sp_printit,
#define VARPROC(proc, menu)	Menu_item \
proc(item, operation) \
	Menu_item	item; \
	Menu_generate	operation; \
{	return do_set_proc(item, operation, menu); }

