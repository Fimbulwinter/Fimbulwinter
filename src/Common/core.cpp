#include "core.hpp"

#include <show_message.hpp>
#include <database_helper.h>

std::string database_helper::driver_;

void core_display_title()
{
	ShowMessage(""CL_WTBL"          (=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=)"CL_CLL""CL_NORMAL"\n");
	ShowMessage(""CL_XXBL"          ("CL_BT_YELLOW"           Fimbulvetr Developer team presents:         "CL_XXBL")"CL_CLL""CL_NORMAL"\n");
	ShowMessage(""CL_XXBL"          ("CL_BOLD"   ______ _           _           _           _        "CL_XXBL")"CL_CLL""CL_NORMAL"\n");
	ShowMessage(""CL_XXBL"          ("CL_BOLD"   |  ___(_)         | |         | |         | |	  "CL_XXBL")"CL_CLL""CL_NORMAL"\n");
	ShowMessage(""CL_XXBL"          ("CL_BOLD"   | |_   _ _ __ ___ | |__  _   _| |_   _____| |_ _ __ "CL_XXBL")"CL_CLL""CL_NORMAL"\n");
	ShowMessage(""CL_XXBL"          ("CL_BOLD"   |  _| | | '_ ` _ \\| '_ \\| | | | \\ \\ / / _ \\ __| '__|"CL_XXBL")"CL_CLL""CL_NORMAL"\n");
	ShowMessage(""CL_XXBL"          ("CL_BOLD"   | |   | | | | | | | |_) | |_| | |\\ V \\/  __\\/ |_| | "CL_XXBL")"CL_CLL""CL_NORMAL"\n");
	ShowMessage(""CL_XXBL"          ("CL_BOLD"   \\_|   |_|_| |_| |_|_.__/ \\__,_|_| \\_/ \\___|\\__|_|   "CL_XXBL")"CL_CLL""CL_NORMAL"\n");
	ShowMessage(""CL_XXBL"          ("CL_BT_GREEN"                 Ragnarok Online Emulator              "CL_XXBL")"CL_CLL""CL_NORMAL"\n");
	ShowMessage(""CL_XXBL"          ("CL_BT_RED"                     DEVELOPER VERSION                 "CL_XXBL")"CL_CLL""CL_NORMAL"\n");
	ShowMessage(""CL_XXBL"          ("CL_BOLD"                                                       "CL_XXBL")"CL_CLL""CL_NORMAL"\n");
	ShowMessage(""CL_WTBL"          (=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=)"CL_CLL""CL_NORMAL"\n\n");
}
