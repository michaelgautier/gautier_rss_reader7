/*
Copyright (C) 2019 Michael Gautier

This source code is free software; you can redistribute it and/or modify it under the terms of the GNU Lesser General Public License as published by the Free Software Foundation; either version 2.1 of the License, or (at your option) any later version.

This source code is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public License along with this library. If not, see <http://www.gnu.org/licenses/>.

Author: Michael Gautier <michaelgautier.wordpress.com>
*/

#ifndef michael_gautier_rss_ui_app_h
#define michael_gautier_rss_ui_app_h

#include <string>

#include <gtk/gtk.h>

namespace gautier_rss_ui_app {
	int
	create();

	extern "C" void
	activate (GtkApplication* application, gpointer user_data);

	void
	set_css_class (GtkWidget* widget, std::string css_class_name);

	std::string
	get_gtk_class_type_name (GtkWidget* widget);

	std::string
	get_db_file_name();

	std::string
	get_application_name();

	std::string
	get_style_file_name();
}

#endif
