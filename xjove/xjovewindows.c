/***************************************************************************
 * This program is Copyright (C) 1991 by C.H.Lindsey, University of        *
 * Manchester.  (X)JOVETOOL is provided to you without charge, and with no *
 * warranty.  You may give away copies of (X)JOVETOOL, including sources,  *
 * provided that this notice is included in all the files, except insofar  *
 * as a more specific copyright notices attaches to the file (x)jovetool.c *
 ***************************************************************************/

#include <xview/xview.h>
#include <xview/font.h>
#include <xview/tty.h>
#include <stdio.h>
#include "exts.h"
#include "xjovewindows.h"

private char command_prefix[] = "\033X";	/* ESC-X */
#define command_prefix_length	(sizeof(command_prefix) - 1)

Menu	main_menu;

private Menu
	file_menu,
	mode_menu,
	help_menu,
	move_menu,
	bind_menu,
	macro_menu,
	buffer_menu,
	window_menu,
	mark_menu,
	edit_menu,
	directory_menu,
	compile_menu,
	format_menu,
	search_menu,
	process_menu,
	abbrev_menu,
	misc_menu,
	print_variables_menu,
	set_variables_menu,
	variables_menu,
	another_help_menu,
	commands_menu,
	var_files_menu,
	var_modes_menu,
	var_move_menu,
	var_search_menu,
	var_process_menu,
	var_compile_menu,
	var_format_menu,
	var_misc_menu,
	var_abbrev_menu,
	empty_menu,
	on_off_menu;

private Menu_item	empty_menu_item;

private int
	describing,
	printing,
	climbing,
	falling;

caddr_t
do_ancestor(m, mi)
	Menu		m;
	Menu_item	mi;
{	Menu		parent_menu;
	Menu_item	parent_item;
	caddr_t		(*proc)();

	parent_item = xv_get(m, MENU_PARENT, NULL);
	if (parent_item != NULL) {
		parent_menu = xv_get(parent_item, MENU_PARENT, NULL);
		proc = (caddr_t (*)())xv_get(parent_item, MENU_NOTIFY_PROC, NULL);
		if (proc == NULL)
			proc = (caddr_t (*)())xv_get(parent_menu, MENU_NOTIFY_PROC, NULL);
		if (proc == NULL) do_ancestor(parent_menu, parent_item);
		else (*proc)(parent_menu, parent_item);
	}
}

caddr_t
do_notify(m, mi, p)
	Menu		m;
	Menu_item	mi;
	void		(*p)();
{
	Menu		pull;
	Menu_item	deft;
	caddr_t		(*proc)();
	int		local_climbing,
			local_falling;

	if (!(local_climbing = climbing))
		ttysw_input (ttysw, command_prefix, command_prefix_length);
	climbing = 1;
	if (!(local_falling = falling)) do_ancestor(m, mi);
	(*p)(m, mi);
	if (!local_climbing && (pull = xv_get(mi, MENU_PULLRIGHT, NULL)) != NULL) {
		falling = 1;
		xv_set(pull, MENU_PARENT, mi, NULL);
		deft = xv_get(pull, MENU_DEFAULT_ITEM, NULL);
		proc = (caddr_t (*)())xv_get(deft, MENU_NOTIFY_PROC, NULL);
		if (proc == NULL)
			proc = (caddr_t (*)())xv_get(pull, MENU_NOTIFY_PROC, NULL);
		(*proc)(pull, deft);
		falling = local_falling;
	}
	climbing = local_climbing;
	if (!climbing) ttysw_input (ttysw, "\n", 1);
}


void
do_print_client_data(m, mi)
	Menu		m;
	Menu_item	mi;
{
	char		*menu_string;

	menu_string = (char *)xv_get(mi, MENU_CLIENT_DATA);
	ttysw_input(ttysw, menu_string, strlen(menu_string));
}

private caddr_t
print_client_data(m, mi)
	Menu		m;
	Menu_item	mi;
{
	return do_notify(m, mi, do_print_client_data);
}

private caddr_t
main_notify(m, mi)
	Menu		m;
	Menu_item	mi;
{
	char		*menu_string;

	print_client_data(m, mi);
	/* ??? what should this function return */
}

void
do_sp_printit(m, mi)
	Menu		m;
	Menu_item	mi;
{
	char		*menu_string;

	menu_string = (char *)xv_get(mi, MENU_CLIENT_DATA);
	if (mi != empty_menu_item)
		ttysw_input(ttysw, " ", 1);
	ttysw_input(ttysw, menu_string, strlen(menu_string));
}

private caddr_t
sp_printit(m, mi)
	Menu		m;
	Menu_item	mi;
{
	return do_notify(m, mi, do_sp_printit);
}

private Menu_item
commands_proc(item, operation)
	Menu_item	item;
	Menu_generate	operation;
{
	return commands_menu;
}

private Menu_item
on_off_proc(item, operation)
	Menu_item	item;
	Menu_generate	operation;
{
	switch (operation) {
	    case MENU_DISPLAY:
		if (!describing & !printing) {
			xv_set(item, MENU_PULLRIGHT, on_off_menu, 0);
		} else {
			xv_set(item, MENU_PULLRIGHT, 0, 0);
		}
		break;
	    case MENU_DISPLAY_DONE:
	    case MENU_NOTIFY:
	    case MENU_NOTIFY_DONE:
		break;
	}
	return item;
}

private Menu
do_set_proc(mi, operation, menu)
	Menu_item	mi;
	Menu_generate	operation;
	Menu		menu;
{
	if (!describing) {
		switch (operation) {
			case MENU_DISPLAY:
			if (!strcmp((char *)xv_get(mi, MENU_CLIENT_DATA), "print"))
			printing = 1;
			return menu;
			break;
			case MENU_DISPLAY_DONE:
			printing = 0;
			case MENU_NOTIFY:
			case MENU_NOTIFY_DONE:
			return empty_menu;
			break;
			}
	} else return empty_menu;
}

private Menu
do_describing_proc(m, operation)
	Menu		m;
	Menu_generate	operation;
{
	switch (operation) {
	    case MENU_DISPLAY:
		describing = 1;
		break;
	    case MENU_DISPLAY_DONE:
		describing = 0;
		break;
	    case MENU_NOTIFY:
	    case MENU_NOTIFY_DONE:
		break;
	}
	return m;
}

#define X

#include "jovemenu.c"

