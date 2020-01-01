/*
Copyright (C) 2019 Michael Gautier

This source code is free software; you can redistribute it and/or modify it under the terms of the GNU Lesser General Public License as published by the Free Software Foundation; either version 2.1 of the License, or (at your option) any later version.

This source code is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public License along with this library. If not, see <http://www.gnu.org/licenses/>.

Author: Michael Gautier <michaelgautier.wordpress.com>
*/

#include "rss_ui/application.hpp"
#include "rss_ui/app_win/headlines_frame.hpp"

#include <stdio.h>
#include <iostream>
#include <thread>

static int
headlines_section_width = 0;

void
gautier_rss_win_main_headlines_frame::initialize_headline_view (GtkWidget* headlines_view, int monitor_width,
        int monitor_height)
{
	double monitor_width_d = monitor_width;
	double monitor_width_part = 1.7;

	headlines_section_width = monitor_width_d / monitor_width_part;

	//int article_section_width = monitor_width - headlines_section_width;

	gtk_notebook_set_scrollable (GTK_NOTEBOOK (headlines_view), true);

	return;
}

void
gautier_rss_win_main_headlines_frame::add_headline_page (GtkWidget* headlines_view, std::string& feed_name,
        void (*connect_headline_list_box_select_row) (GtkWidget*))
{
	GtkWidget* scroll_win = gtk_scrolled_window_new (NULL, NULL);

	gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (scroll_win), GTK_POLICY_ALWAYS, GTK_POLICY_ALWAYS);

	GtkWidget* list_box = gtk_list_box_new();
	gtk_widget_set_size_request (list_box, headlines_section_width, -1);

	gtk_list_box_set_selection_mode (GTK_LIST_BOX (list_box), GTK_SELECTION_BROWSE);

	gtk_container_add (GTK_CONTAINER (scroll_win), list_box);

	connect_headline_list_box_select_row (list_box);

	gtk_notebook_append_page (GTK_NOTEBOOK (headlines_view), scroll_win, gtk_label_new (feed_name.data()));

	gtk_widget_show_all (scroll_win);

	return;
}

void
gautier_rss_win_main_headlines_frame::show_headlines (GtkWidget* headlines_view, std::string feed_name,
        int headline_index_start, std::vector<std::string>& headlines)
{
	/*
		Tab Contents (in this case a scroll window containing a list box)

		Tab > Scroll Window > List Box > individual labels (headlines)
	*/
	int headlines_count_new = headlines.size();

	if (headlines_count_new > -1) {
		GtkWidget* tab = NULL;

		std::string tab_label;
		int tab_n = -1;

		gint page_count = gtk_notebook_get_n_pages (GTK_NOTEBOOK (headlines_view));

		for (int tab_i = 0; tab_i < page_count; tab_i++) {
			tab = gtk_notebook_get_nth_page (GTK_NOTEBOOK (headlines_view), tab_i);

			const gchar* tab_text = gtk_notebook_get_tab_label_text (GTK_NOTEBOOK (headlines_view), tab);

			tab_label = tab_text;

			if (feed_name == tab_label) {
				tab_n = tab_i;
				break;
			} else {
				tab = NULL;
			}
		}

		/*
			Get the list box
		*/
		GtkWidget* list_box = NULL;

		if (tab != NULL) {
			GtkScrolledWindow* scroll_win = GTK_SCROLLED_WINDOW (tab);

			if (scroll_win) {
				GtkWidget* viewport = gtk_bin_get_child (GTK_BIN (scroll_win));

				if (viewport) {
					list_box = gtk_bin_get_child (GTK_BIN (viewport));
				}
			}
		}

		/*
			Populate list box.
		*/

		if (list_box != NULL) {
			gtk_widget_hide (list_box);

			for (int i = headline_index_start; i < headlines_count_new; i++) {
				//Each line should be displayed in the order stored.
				std::string headline_text = headlines.at (i);

				GtkWidget* headline_label = gtk_label_new (headline_text.data());

				gtk_label_set_selectable (GTK_LABEL (headline_label), false);
				gtk_label_set_single_line_mode (GTK_LABEL (headline_label), true);
				gtk_label_set_line_wrap (GTK_LABEL (headline_label), false);

				gtk_widget_set_halign (headline_label, GTK_ALIGN_START);

				gtk_list_box_insert (GTK_LIST_BOX (list_box), headline_label, i);
			}

			gtk_widget_show_all (list_box);
		}
	}

	return;
}

void
gautier_rss_win_main_headlines_frame::select_headline (gautier_rss_data_read::rss_article& rss_data,
        GtkListBoxRow* headline_row)
{
	GtkWidget* headline_label = NULL;
	{
		GtkBin* bin = GTK_BIN (headline_row);

		if (bin) {
			headline_label = gtk_bin_get_child (bin);
		}
	}

	if (headline_label) {
		const gchar*
		headline_text = gtk_label_get_text (GTK_LABEL (headline_label));

		rss_data.headline = headline_text;

		std::string db_file_name = gautier_rss_ui_app::get_db_file_name();

		gautier_rss_data_read::get_feed_article_summary (db_file_name, rss_data.feed_name,
		        rss_data.headline,
		        rss_data);
	}

	return;
}

void
gautier_rss_win_main_headlines_frame::select_headline_row (GtkWidget* headlines_view, std::string feed_name,
        int headline_row_index)
{
	if (headline_row_index > -1) {
		GtkWidget* tab = NULL;

		std::string tab_label;
		int tab_n = -1;

		gint page_count = gtk_notebook_get_n_pages (GTK_NOTEBOOK (headlines_view));

		for (int tab_i = 0; tab_i < page_count; tab_i++) {
			tab = gtk_notebook_get_nth_page (GTK_NOTEBOOK (headlines_view), tab_i);

			const gchar* tab_text = gtk_notebook_get_tab_label_text (GTK_NOTEBOOK (headlines_view), tab);

			tab_label = tab_text;

			if (feed_name == tab_label) {
				tab_n = tab_i;
				break;
			} else {
				tab = NULL;
			}
		}

		/*
			Get the list box
		*/
		GtkWidget* list_box = NULL;

		if (tab != NULL) {
			GtkScrolledWindow* scroll_win = GTK_SCROLLED_WINDOW (tab);

			if (scroll_win) {
				GtkWidget* viewport = gtk_bin_get_child (GTK_BIN (scroll_win));

				if (viewport) {
					list_box = gtk_bin_get_child (GTK_BIN (viewport));
				}
			}
		}

		/*
			Populate list box.
		*/

		if (list_box != NULL) {
			GtkListBoxRow* headline_row = gtk_list_box_get_row_at_index (GTK_LIST_BOX (list_box), headline_row_index);

			gtk_list_box_select_row (GTK_LIST_BOX (list_box), headline_row);
		}
	}

	return;
}

int
gautier_rss_win_main_headlines_frame::get_default_headlines_view_content_width()
{
	return headlines_section_width;
}


