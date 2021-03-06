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
#include <map>
#include <vector>
#include <cstdint>

#include <gtk/gtk.h>

#include "rss_lib/rss/rss_reader.hpp"
#include "rss_lib/rss/rss_article.hpp"

namespace gautier_rss_win_main_headlines_frame {
	void
	initialize_headline_view (GtkWidget* headlines_view, const int monitor_width, const int monitor_height);

	void
	add_headline_page (GtkWidget* headlines_view, const std::string feed_name, const int position,
	                   void (*headline_view_select_row) (GtkTreeSelection*, gpointer));

	void
	show_headlines (GtkWidget* headlines_view, const std::string& feed_name,
	                const gautier_rss_data_read::headline_range_type& range,
	                const gautier_rss_data_read::headlines_list_type& headlines,
	                const bool prepend);

	void
	get_selected_headline_text (GtkTreeSelection* headline_row, std::string& headline_text);

	void
	set_headlines_title (GtkWidget* headlines_view, const gint tab_number, const std::string title);

	void
	select_headline_row (GtkWidget* headlines_view, const std::string feed_name, const std::string headline_text);

	void
	clear_row_selection (GtkWidget* headlines_view, const std::string feed_name);

	int
	get_default_headlines_view_content_width();

	bool
	is_tab_selected (GtkWidget* headlines_view, const std::string tab_name);

	gint
	get_selected_tab_numb (GtkWidget* headlines_view);

	std::string
	get_selected_tab_name (GtkWidget* headlines_view);
}
#endif
