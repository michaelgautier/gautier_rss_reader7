/*
Copyright (C) 2019 Michael Gautier

This source code is free software; you can redistribute it and/or modify it under the terms of the GNU Lesser General Public License as published by the Free Software Foundation; either version 2.1 of the License, or (at your option) any later version.

This source code is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public License along with this library. If not, see <http://www.gnu.org/licenses/>.

Author: Michael Gautier <michaelgautier.wordpress.com>
*/

#include "rss_ui/application.hpp"
#include "rss_ui/rss_manage/rss_manage.hpp"

#include "rss_lib/rss/rss_reader.hpp"

static GtkWindow*
win = NULL;

static GtkWindow*
parent_win = NULL;

static void
create_window (GtkApplication* app, GtkWindow* parent, int window_width, int window_height);

static void
layout_rss_feed_entry_area (GtkWidget* feed_entry_layout_row1, GtkWidget* feed_entry_layout_row2,
                            GtkWidget* feed_entry_layout_row3, GtkWindow* win);

static void
layout_rss_tree_view (GtkWidget* rss_tree_view);

extern "C"
void
update_configuration_click (GtkButton* button, gpointer user_data);

void
gautier_rss_win_rss_manage::show_dialog (GtkApplication* app, GtkWindow* parent, int window_width,
        int window_height)
{
	/*
		RSS Management Window.
	*/
	create_window (app, parent, window_width, window_height);

	/*
		Vertical layout for the window's contents.
	*/
	GtkWidget* window_layout = gtk_box_new (GTK_ORIENTATION_VERTICAL, 4);
	gtk_container_add (GTK_CONTAINER (win), window_layout);

	/*
		Horizontal Layout 1: RSS entry fields.
	*/
	GtkWidget* feed_entry_layout_row1 = gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 4);

	/*
		Horizontal Layout 2: RSS retention options.
	*/
	GtkWidget* feed_entry_layout_row2 = gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 4);

	/*
		Horizontal Layout 3: Buttons.
	*/
	GtkWidget* feed_entry_layout_row3 = gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 4);
	gtk_widget_set_halign (feed_entry_layout_row3, GTK_ALIGN_END);

	/*
		Setup the feed entry area.
	*/
	layout_rss_feed_entry_area (feed_entry_layout_row1, feed_entry_layout_row2, feed_entry_layout_row3, win);

	/*
		Add feed entry area to main layout.
	*/
	bool content_expands = false;
	bool content_fills = false;

	gtk_box_pack_start (GTK_BOX (window_layout), feed_entry_layout_row1, content_expands, content_fills, 4);
	gtk_box_pack_start (GTK_BOX (window_layout), feed_entry_layout_row2, content_expands, content_fills, 4);
	gtk_box_pack_start (GTK_BOX (window_layout), feed_entry_layout_row3, content_expands, content_fills, 4);

	/*
		RSS Configuration Table
	*/
	GtkWidget* rss_tree_view = gtk_tree_view_new();
	gtk_widget_set_size_request (rss_tree_view, -1, window_height * 2);

	GtkWidget* scroll_win = gtk_scrolled_window_new (NULL, NULL);
	gtk_widget_set_valign (scroll_win, GTK_ALIGN_FILL);

	gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (scroll_win), GTK_POLICY_ALWAYS, GTK_POLICY_ALWAYS);
	gtk_container_add (GTK_CONTAINER (scroll_win), rss_tree_view);

	layout_rss_tree_view (rss_tree_view);

	content_expands = false;
	content_fills = true;

	gtk_widget_set_hexpand (scroll_win, true);
	gtk_widget_set_vexpand (scroll_win, true);

	gtk_box_pack_start (GTK_BOX (window_layout), scroll_win, content_expands, content_fills, 4);

	/*
		Window Presentation
	*/
	gtk_widget_show_all ((GtkWidget*)win);

	return;
}

static void
create_window (GtkApplication* app, GtkWindow* parent, int window_width, int window_height)
{
	parent_win = parent;

	win = GTK_WINDOW (gtk_window_new (GTK_WINDOW_TOPLEVEL));

	/*
		Default size of the window.
		Exists in case main application window size is unavailable.
		These values should be overriden at runtime by the values
			provided by the main application window.
	*/
	int win_width = 720;
	int win_height = 440;

	/*
		Determine actual window size based on the size of the
			main application window that spawned this one.
	*/
	if (window_width > 0) {
		win_width = window_width - 120;
	}

	if (window_height > 0) {
		win_height = window_height - 120;
	}

	gtk_window_set_skip_taskbar_hint (win, true); /*Do not show in the taskbar*/
	gtk_window_set_urgency_hint (win, true); /*Flash the window*/
	gtk_window_set_type_hint (win, GDK_WINDOW_TYPE_HINT_DIALOG);/*Show only the close button*/
	gtk_window_set_title (win, "Manage Feeds");
	gtk_window_set_resizable (win, false);
	gtk_window_set_modal (win, true);
	gtk_window_set_transient_for (win, parent_win);
	gtk_window_set_default_size (win, win_width, win_height);
	gtk_window_set_position (win, GTK_WIN_POS_CENTER_ALWAYS);

	return;
}

static void
layout_rss_feed_entry_area (GtkWidget* feed_entry_layout_row1, GtkWidget* feed_entry_layout_row2,
                            GtkWidget* feed_entry_layout_row3, GtkWindow* win)
{
	/*
		Feed name
	*/
	GtkWidget* feed_name_label = gtk_label_new ("Feed name");

	GtkWidget* feed_name_entry = gtk_entry_new();
	gtk_widget_set_size_request (feed_name_entry, 240, 24);

	/*
		Feed url
	*/
	GtkWidget* feed_url_label = gtk_label_new ("Feed web address");

	GtkWidget* feed_url_entry = gtk_entry_new();
	gtk_widget_set_size_request (feed_url_entry, 330, 24);

	/*
		Feed refresh interval
	*/
	GtkWidget* feed_refresh_interval_label = gtk_label_new ("Refresh interval");

	GtkWidget* feed_refresh_interval = gtk_spin_button_new_with_range (1, 4, 1);
	gtk_widget_set_size_request (feed_refresh_interval, 80, 24);

	/*
		Feed retention period
	*/
	GtkWidget* feed_retention_option_label = gtk_label_new ("Keep feeds");

	GtkWidget* feed_option_keep_all = gtk_radio_button_new_with_label (NULL, "Forever");

	GtkWidget* feed_option_keep_last7 = gtk_radio_button_new_with_label_from_widget (GTK_RADIO_BUTTON (
	                                        feed_option_keep_all), "7 days");

	GtkWidget* feed_option_keep_last24hr = gtk_radio_button_new_with_label_from_widget (GTK_RADIO_BUTTON (
	        feed_option_keep_all), "1 day");

	/*
		Insert/Update/Delete
	*/
	GtkWidget* update_configuration_button = gtk_button_new_with_label ("Update");

	g_signal_connect (update_configuration_button, "clicked", G_CALLBACK (update_configuration_click), NULL);

	/*
		Layout and arrange
	*/
	gtk_container_add (GTK_CONTAINER (feed_entry_layout_row1), feed_name_label);
	gtk_container_add (GTK_CONTAINER (feed_entry_layout_row1), feed_name_entry);
	gtk_container_add (GTK_CONTAINER (feed_entry_layout_row1), feed_url_label);
	gtk_container_add (GTK_CONTAINER (feed_entry_layout_row1), feed_url_entry);
	gtk_container_add (GTK_CONTAINER (feed_entry_layout_row1), feed_refresh_interval_label);
	gtk_container_add (GTK_CONTAINER (feed_entry_layout_row1), feed_refresh_interval);

	gtk_container_add (GTK_CONTAINER (feed_entry_layout_row2), feed_retention_option_label);
	gtk_container_add (GTK_CONTAINER (feed_entry_layout_row2), feed_option_keep_all);
	gtk_container_add (GTK_CONTAINER (feed_entry_layout_row2), feed_option_keep_last7);
	gtk_container_add (GTK_CONTAINER (feed_entry_layout_row2), feed_option_keep_last24hr);

	gtk_container_add (GTK_CONTAINER (feed_entry_layout_row3), update_configuration_button);

	return;
}

void
update_configuration_click (GtkButton* button, gpointer user_data)
{
	return;
}

void
layout_rss_tree_view (GtkWidget* rss_tree_view)
{
	int col_pos_feed_name = 0;
	int col_pos_feed_article_count = 1;
	int col_pos_feed_retrieved = 2;
	int col_pos_feed_retention = 3;
	int col_pos_feed_webaddress = 4;
	int col_pos_stop = -1;

	/*
		Tree Model to describe the columns
	*/
	GtkListStore* list_store = gtk_list_store_new (5 /*5 columns*/,
	                           G_TYPE_STRING,/*feed name*/
	                           G_TYPE_STRING,/*article count*/
	                           G_TYPE_STRING,/*last retrieved*/
	                           G_TYPE_STRING,/*retention in days*/
	                           G_TYPE_STRING /*web url*/);

	GtkTreeIter iter;


	/*
		Make a row for each feed/url
	*/
	namespace rss_ns = gautier_rss_data_read;
	std::vector<rss_ns::rss_feed> feed_info;

	/*
		std::string feed_name;
		std::string feed_url;
		std::string last_retrieved;
	*/

	std::string db_file_name = gautier_rss_ui_app::get_db_file_name();

	rss_ns::get_feed_names (db_file_name, feed_info);

	/*
		TO-DO ITEM	Later on ... replace the literals 0,1,2 etc.
		with named constants to ensure proper coordination of positions
		to names / conceptual representations.
	*/

	for (rss_ns::rss_feed feed : feed_info) {
		gchar* feed_name = feed.feed_name.data();
		gchar* article_count = std::string ("0").data();
		gchar* last_retrieved = feed.last_retrieved.data();
		gchar* retention_period = std::string ("forever").data();
		gchar* feed_url = feed.feed_url.data();

		/*
			Adds a new row in the Tree Model.
		*/
		gtk_list_store_append (list_store, &iter);

		/*
			Links specific data to column positions in the row.
		*/
		gtk_list_store_set (list_store, &iter,
		                    col_pos_feed_name, feed_name,
		                    col_pos_feed_article_count, article_count,
		                    col_pos_feed_retrieved, last_retrieved,
		                    col_pos_feed_retention, retention_period,
		                    col_pos_feed_webaddress, feed_url,
		                    col_pos_stop);
	}

	/*
		Column: Feed Name
	*/
	GtkCellRenderer* column_renderer_feed_name = gtk_cell_renderer_text_new();
	GtkTreeViewColumn* column_feed_name = gtk_tree_view_column_new_with_attributes ("Feed",
	                                      column_renderer_feed_name, "text", col_pos_feed_name, NULL);
	gtk_tree_view_append_column (GTK_TREE_VIEW (rss_tree_view), column_feed_name);

	/*
		Column: Article Count
	*/
	GtkCellRenderer* column_renderer_feed_article_count = gtk_cell_renderer_text_new();
	GtkTreeViewColumn* column_feed_article_count = gtk_tree_view_column_new_with_attributes ("Article Count",
	        column_renderer_feed_article_count, "text", col_pos_feed_article_count, NULL);
	gtk_tree_view_append_column (GTK_TREE_VIEW (rss_tree_view), column_feed_article_count);

	/*
		Column: Last Retrieved
	*/
	GtkCellRenderer* column_renderer_feed_last_retrieved = gtk_cell_renderer_text_new();
	GtkTreeViewColumn* column_feed_last_retrieved = gtk_tree_view_column_new_with_attributes ("Last updated",
	        column_renderer_feed_last_retrieved, "text", col_pos_feed_retrieved, NULL);
	gtk_tree_view_append_column (GTK_TREE_VIEW (rss_tree_view), column_feed_last_retrieved);

	/*
		Column: Retention Period
	*/
	GtkCellRenderer* column_renderer_feed_retention_period = gtk_cell_renderer_text_new();
	GtkTreeViewColumn* column_feed_retention_period = gtk_tree_view_column_new_with_attributes ("Retention",
	        column_renderer_feed_retention_period, "text", col_pos_feed_retention, NULL);
	gtk_tree_view_append_column (GTK_TREE_VIEW (rss_tree_view), column_feed_retention_period);

	/*
		Column: Feed Url
	*/
	GtkCellRenderer* column_renderer_feed_url = gtk_cell_renderer_text_new();
	GtkTreeViewColumn* column_feed_url = gtk_tree_view_column_new_with_attributes ("Web address",
	                                     column_renderer_feed_url, "text", col_pos_feed_webaddress, NULL);
	gtk_tree_view_append_column (GTK_TREE_VIEW (rss_tree_view), column_feed_url);

	/*
		Populate tree
	*/
	gtk_tree_view_set_model (GTK_TREE_VIEW (rss_tree_view), GTK_TREE_MODEL (list_store));

	return;
}
