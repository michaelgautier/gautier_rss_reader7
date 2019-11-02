/*
Copyright (C) 2019 Michael Gautier

This source code is free software; you can redistribute it and/or modify it under the terms of the GNU Lesser General Public License as published by the Free Software Foundation; either version 2.1 of the License, or (at your option) any later version.

This source code is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public License along with this library. If not, see <http://www.gnu.org/licenses/>.

Author: Michael Gautier <michaelgautier.wordpress.com>
*/

#include "rss_ui/application.hpp"
#include "rss_ui/app_win/app_win.hpp"

#include "rss_lib/rss/rss_writer.hpp"

int
main (int argc, char** argv)
{
	int status = gautier_rss_ui_app::create();

	return status;
}

int
gautier_rss_ui_app::create()
{
	int status = 0;

	/*
		Database

		Create file to hold RSS feed data.
	*/
	{
		std::string db_file_name = get_db_file_name();
		{
			namespace ns_write = gautier_rss_data_write;
			ns_write::initialize_db (db_file_name);
		}
	}

	/*
		GUI

		Run the user interface.
	*/
	GtkApplication* application = gtk_application_new ("michael.gautier.rss", G_APPLICATION_FLAGS_NONE);

	g_signal_connect (application, "activate", G_CALLBACK (gautier_rss_ui_app::activate), NULL);

	status = g_application_run (G_APPLICATION (application), 0, NULL);

	g_object_unref (application);

	return status;
}

/*
	GUI

	Run the user interface. Create the main visual window.
*/
void
gautier_rss_ui_app::activate (GtkApplication* application, gpointer user_data)
{
	gautier_rss_win_main::create (application, user_data);

	return;
}

/*
	Database

	Primary file name.
*/
std::string
gautier_rss_ui_app::get_db_file_name()
{
	return "../data/rss.db";
}

/*
	UI Name

	Program name as seen in title bar.
*/
std::string
gautier_rss_ui_app::get_application_name()
{
	return "RSS Reader";
}

