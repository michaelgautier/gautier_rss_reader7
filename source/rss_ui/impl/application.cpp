/*
Copyright (C) 2020 Michael Gautier

This source code is free software; you can redistribute it and/or modify it under the terms of the GNU Lesser General Public License as published by the Free Software Foundation; either version 2.1 of the License, or (at your option) any later version.

This source code is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public License along with this library. If not, see <http://www.gnu.org/licenses/>.

Author: Michael Gautier <michaelgautier.wordpress.com>
*/

#include <cerrno>
#include <cstdlib>
#include <cstring>
#include <iostream>
#include <string>

#include "rss_lib/rss/rss_util.hpp"
#include "rss_lib/rss/rss_writer.hpp"

#include "rss_ui/application.hpp"
#include "rss_ui/app_resources.h"
#include "rss_ui/app_win/app_win.hpp"

#include <gio/gio.h>

/*
	GNU C Library extensions

	- Revise into platform specific modules if OS platform port needed.
*/
#include <sys/types.h>

/*
	- Using GNU C file system I/O to create directories to hold user data.
	- Opted for that over C++ filesystem (introduced in C++ 17).
	- Waiting until C++ filesystem has 0 defect reports for approx. 5 years.
*/
#include <sys/stat.h>
#include <dirent.h>

/*
	- Get the home directory for the currently logged in user.
	- Necessary since GNU C does not translate ~/ or expand it out.

	UNIX/LINUX specific
*/
#include <grp.h>
#include <pwd.h>

namespace {
/*
	- End all directory paths in this program with / to make concatenation trivial.
	- Referencing root_user_data_directory fails under the following conditions:
		* ./local/share does not exist
		* If ~/ is used as the root of the tree. Does not expand to /home/<you>.
	- user_home_directory is modified by calls to create_user_root_directory.
*/
std::string
user_home_directory = "";

const std::string
root_user_data_directory = ".local/share/newsreader/";

const std::string
app_name = "michael.gautier.rss.v7";

int
create_user_root_directory();

int
create_user_data_directory();

int
create_directory (const std::string directory_path);

void
load_application_icon();

int
create_directory (const std::string directory_path)
{
	const std::string directory_name = directory_path;

	int directory_status = mkdir (directory_name.data(), S_IRWXU | S_IRWXG | S_IRWXO);

	if (directory_status != 0) {
		/*
			Ignore errors (in 'this' context) where the directory already exist.
			Our objective is to have a directory with the given path. If it exists, that is good.
			Checking the error return code is in this case faster and simpler than 20 lines of directory scan code.
		*/
		const int c_error_no = errno;

		if (c_error_no == EEXIST) { //Directory exists when we tried to create it.
			directory_status = 0;
		} else { //All other errors are critical and must abort the program.
			/*
				Most common errors:
					Permissions
					Directory cannot be the same name as the executable when both would exist in the same working directory.
			*/
			std::cout << "Cannot create directory: " << directory_name << ". Error: " << strerror (
			              errno) << ", errno: " << errno << "\n";
		}
	}

	return directory_status;
}

int
create_user_root_directory()
{
	int status  = -1;

	const uid_t user_id = getuid();
	struct passwd* user_info = getpwuid (user_id);

	if (!user_info) {
		status = -1;
		std::cout << "Could not determine user directory\n";

		exit (EXIT_FAILURE);
	} else {
		status = 0;

		user_home_directory = user_info->pw_dir;
		user_home_directory = user_home_directory + "/";
	}

	if (status == 0) {
		const std::string directory_name = user_home_directory + root_user_data_directory;

		status = create_directory (directory_name);
	}

	return status;
}

int
create_user_data_directory()
{
	const std::string directory_name = gautier_rss_ui_app::get_user_directory_name();

	const int directory_status = create_directory (directory_name);

	return directory_status;
}

void
load_application_icon()
{
	const std::string app_icon_resource_path = "/newsreader/app_icon.png";

	GdkPixbuf* application_icon_pixbuf = gdk_pixbuf_new_from_resource (app_icon_resource_path.data(), nullptr);

	gtk_window_set_default_icon (application_icon_pixbuf);

	return;
}
}

int
gautier_rss_ui_app::create()
{
	int status = 0;

	/*
		GUI

		Run the user interface.
	*/
	GtkApplication* application = gtk_application_new (app_name.data(), G_APPLICATION_FLAGS_NONE);

	g_signal_connect (application, "activate", G_CALLBACK (gautier_rss_ui_app::activate), nullptr);

	load_application_icon();

	status = g_application_run (G_APPLICATION (application), 0, nullptr);

	/*
		Clean-up.
	*/
	std::cout << "clean-up before exit.\n";
	{
		g_object_unref (application);

		const std::string db_file_name = get_db_file_name();
		{
			namespace ns_write = gautier_rss_data_write;
			ns_write::de_initialize_db (db_file_name);
		}
	}

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
	CSS Style
*/
void
gautier_rss_ui_app::set_css_class (GtkWidget* widget, const std::string css_class_name)
{
	const std::string css_style_resource_path = "/newsreader/app_style.css";

	GtkCssProvider* css_provider = gtk_css_provider_new();

	gtk_css_provider_load_from_resource (css_provider, css_style_resource_path.data());

	GtkStyleContext* style_context = gtk_widget_get_style_context (widget);

	gtk_style_context_add_provider (style_context, GTK_STYLE_PROVIDER (css_provider),
	                                GTK_STYLE_PROVIDER_PRIORITY_APPLICATION);

	gtk_style_context_add_class (style_context, css_class_name.data());

	return;
}

/*
	GTK Type
*/
std::string
gautier_rss_ui_app::get_gtk_class_type_name (GtkWidget* widget)
{
	GtkWidgetClass* widget_class = GTK_WIDGET_GET_CLASS (widget);
	GType class_type = G_OBJECT_CLASS_TYPE (widget_class);

	const std::string widget_type_name = g_type_name (class_type);

	return widget_type_name;
}

void
gautier_rss_ui_app::get_scroll_content_as_list_view (GtkScrolledWindow* scroll_win, GtkWidget** list_view)
{
	if (scroll_win) {
		GtkWidget* viewport = gtk_bin_get_child (GTK_BIN (scroll_win));

		const std::string gtk_widget_type_name = get_gtk_class_type_name (viewport);

		if (viewport) {
			if (gtk_widget_type_name == "GtkTextView" ||
			        gtk_widget_type_name == "GtkTreeView" ||
			        gtk_widget_type_name == "GtkIconView") {
				*list_view = viewport;
			} else {
				GtkWidget* bin_list_view = gtk_bin_get_child (GTK_BIN (viewport));

				if (bin_list_view) {
					*list_view = bin_list_view;
				}
			}
		}
	}

	return;
}

/*
	Database

	Primary file name.
*/
std::string
gautier_rss_ui_app::get_db_file_name()
{
	return get_user_directory_name() + "rss.db";
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

/*
	The functions here serve the following purposes:

		1.)	Avoid cluttering up the user's home directory.
		2.)	Establish per-user files.
		3.)	Operate the same as other GNOME software in storing per-user data.
		4.)	Observe the File Hierarchy Standards and separate global setup
			from per-user data.

		Item #4 was the catalyst for these changes. See the previous work on RPM setup.
*/

/*
	User's directory to keep files

	Program name as seen in title bar.
*/
std::string
gautier_rss_ui_app::get_user_directory_name()
{
	std::string base_directory;

	base_directory = user_home_directory + root_user_data_directory + "data/";

	return base_directory;
}

/*
	Program start-up.
*/
int
main (int argc, char** argv)
{
	const std::string program_time_started =  gautier_rss_util::get_current_date_time_utc();

	std::cout << "Gautier RSS Reader 7, compiled under C++ std " <<  __cplusplus << "\n";
	std::cout << "Originally created by Michael Gautier, https://michaelgautier.github.io/gautier_rss_reader7/ \n";
	std::cout << "This is free software under LGPLv2; see the source for copying conditions.\n";
	std::cout << "NO warranty; not even for MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.\n";
	std::cout << "--------------------------------------------------------------------------------\n";
	std::cout << "--------------------------------------------------------------------------------\n";
	std::cout << "--------------------------------------------------------------------------------\n";
	std::cout << "--------------------------------------------------------------------------------\n";
	std::cout << "--------------------------------------------------------------------------------\n";
	std::cout << "--------------------------------------------------------------------------------\n";
	std::cout << "--------------------------------------------------------------------------------\n";

	std::cout << "\n\nProgram started: " << program_time_started << ".\n";

	if (argc > 0 && argv) {
		/*
			Ignore passed args and print a message instead.
		*/
		std::cout << "Program does not accept any command arguments.\n";
	}

	namespace ns_ui_app = gautier_rss_ui_app;

	int status = 0;

	/*
		Initialize User's directory to hold data files.
	*/
	{
		status = create_user_root_directory();

		if (status == 0) {
			status = create_user_data_directory();
		}
	}

	/*
		Initialize database to hold RSS feed data.
	*/
	if (status == 0) {
		const std::string db_file_name = ns_ui_app::get_db_file_name();

		gautier_rss_data_write::initialize_db (db_file_name);

		gautier_rss_data_write::remove_expired_articles (db_file_name);
	}

	/*
		Initialize UI
	*/
	if (status == 0) {
		gtk_init (nullptr, nullptr);

		std::cout << "Creating user interface - main screen.\n";

		status = ns_ui_app::create();
	}

	const std::string program_time_ended_precleanup =  gautier_rss_util::get_current_date_time_utc();

	const int_fast32_t time_difference_in_seconds = gautier_rss_util::get_time_difference_in_seconds (
	            program_time_started, program_time_ended_precleanup);

	std::cout << "--------------------------------------------------------------------------------\n";
	std::cout << "--------------------------------------------------------------------------------\n";
	std::cout << "--------------------------------------------------------------------------------\n";
	std::cout << "--------------------------------------------------------------------------------\n";
	std::cout << "--------------------------------------------------------------------------------\n";
	std::cout << "--------------------------------------------------------------------------------\n";
	std::cout << "--------------------------------------------------------------------------------\n";
	std::cout << "Program ended " << program_time_ended_precleanup << ".\n";
	std::cout << "Program running time: " << time_difference_in_seconds << " seconds.\n";

	return status;
}
