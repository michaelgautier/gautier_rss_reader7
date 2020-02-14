/*
Copyright (C) 2020 Michael Gautier

This source code is free software; you can redistribute it and/or modify it under the terms of the GNU Lesser General Public License as published by the Free Software Foundation; either version 2.1 of the License, or (at your option) any later version.

This source code is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public License along with this library. If not, see <http://www.gnu.org/licenses/>.

Author: Michael Gautier <michaelgautier.wordpress.com>
*/

#ifndef michael_gautier_rss_win_main_headlines_frame_h
#define michael_gautier_rss_win_main_headlines_frame_h

#include <string>
#include <vector>

#include <gtk/gtk.h>

#include "rss_lib/rss/rss_reader.hpp"

namespace gautier_rss_win_main_headlines_frame {
	void
	initialize_headline_view (GtkWidget* headlines_view, int monitor_width, int monitor_height);

	void
	add_headline_page (GtkWidget* headlines_view, std::string& feed_name, int position,
	                   void (*headline_view_select_row) (GtkTreeSelection*, gpointer));

	void
	show_headlines (GtkWidget* headlines_view, std::string feed_name, int headline_index_start,
	                int headline_index_end, std::vector<std::string>& headlines);

	void
	select_headline (gautier_rss_data_read::rss_article& rss_data, GtkTreeSelection* headline_row);

	void
	select_headline_row (GtkWidget* headlines_view, std::string feed_name, int headline_row_index);

	int
	get_default_headlines_view_content_width();

	int
	get_tab_contents_container_by_feed_name (GtkNotebook* headlines_view, std::string& feed_name,
	        GtkWidget** notebook_tab);
}
#endif
