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
	GtkApplication* application = gtk_application_new ("michael.gautier.rss.v7", G_APPLICATION_FLAGS_NONE);

	g_signal_connect (application, "activate", G_CALLBACK (gautier_rss_ui_app::activate), NULL);

	status = g_application_run (G_APPLICATION (application), 0, NULL);

	/*
		Clean-up.
	*/
	{
		g_object_unref (application);

		std::string db_file_name = get_db_file_name();
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
gautier_rss_ui_app::set_css_class (GtkWidget* widget, std::string css_class_name)
{
	std::string css_style_file_name = gautier_rss_ui_app::get_style_file_name();

	GtkCssProvider* css_provider = gtk_css_provider_new();

	gtk_css_provider_load_from_path (css_provider, css_style_file_name.data(), NULL);

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

	std::string widget_type_name = g_type_name (class_type);

	return widget_type_name;
}
/*
	Database

	Primary file name.
*/
std::string
gautier_rss_ui_app::get_db_file_name()
{
	return "rss.db";
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
	CSS
*/
std::string
gautier_rss_ui_app::get_style_file_name()
{
	return "app.css";
}

