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
layout_rss_feed_entry_row (GtkWidget* feed_entry_layout_row1, GtkWidget* feed_entry_layout_row2,
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
	create_window (app, parent, window_width, window_height);

	GtkWidget* window_layout = gtk_box_new (GTK_ORIENTATION_VERTICAL, 4);

	GtkWidget* feed_entry_layout_row1 = gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 4);
	GtkWidget* feed_entry_layout_row2 = gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 4);
	GtkWidget* feed_entry_layout_row3 = gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 4);

	layout_rss_feed_entry_row (feed_entry_layout_row1, feed_entry_layout_row2, feed_entry_layout_row3, win);

	gtk_container_add (GTK_CONTAINER (window_layout), feed_entry_layout_row1);
	gtk_container_add (GTK_CONTAINER (window_layout), feed_entry_layout_row2);
	gtk_container_add (GTK_CONTAINER (window_layout), feed_entry_layout_row3);

	GtkWidget* rss_tree_view = gtk_tree_view_new();

	layout_rss_tree_view (rss_tree_view);

	gtk_container_add (GTK_CONTAINER (window_layout), rss_tree_view);

	gtk_container_add (GTK_CONTAINER (win), window_layout);

	//call this last
	gtk_widget_show_all ((GtkWidget*)win);

	return;
}

static void
create_window (GtkApplication* app, GtkWindow* parent, int window_width, int window_height)
{
	parent_win = parent;

	win = GTK_WINDOW (gtk_window_new (GTK_WINDOW_TOPLEVEL));

	int win_width = 800;
	int win_height = 640;

	if (window_width > 0) {
		win_width = window_width - 120;
	}

	if (window_height > 0) {
		win_height = window_height - 120;
	}

	gtk_window_set_title (win, "Manage Feeds");
	gtk_window_set_resizable (win, false);
	gtk_window_set_modal (win, true);
	gtk_window_set_transient_for (win, parent_win);
	gtk_window_set_default_size (win, win_width, win_height);
	gtk_window_set_position (win, GTK_WIN_POS_CENTER_ALWAYS);

	return;
}

static void
layout_rss_feed_entry_row (GtkWidget* feed_entry_layout_row1, GtkWidget* feed_entry_layout_row2,
                           GtkWidget* feed_entry_layout_row3, GtkWindow* win)
{
	/*
		Feed name
	*/
	GtkWidget* feed_name_label = gtk_label_new ("Feed name");
	gtk_widget_set_valign (feed_name_label, GTK_ALIGN_START);

	GtkWidget* feed_name_entry = gtk_entry_new();
	gtk_widget_set_size_request (feed_name_entry, 240, 24);
	gtk_widget_set_valign (feed_name_entry, GTK_ALIGN_START);

	/*
		Feed url
	*/
	GtkWidget* feed_url_label = gtk_label_new ("Feed web address");
	gtk_widget_set_valign (feed_url_label, GTK_ALIGN_START);

	GtkWidget* feed_url_entry = gtk_entry_new();
	gtk_widget_set_size_request (feed_url_entry, 330, 24);
	gtk_widget_set_valign (feed_url_entry, GTK_ALIGN_START);

	/*
		Feed refresh interval
	*/
	GtkWidget* feed_refresh_interval_label = gtk_label_new ("Refresh interval");
	gtk_widget_set_valign (feed_refresh_interval_label, GTK_ALIGN_START);

	GtkWidget* feed_refresh_interval = gtk_spin_button_new_with_range (1, 4, 1);
	gtk_widget_set_size_request (feed_refresh_interval, 80, 24);
	gtk_widget_set_valign (feed_refresh_interval, GTK_ALIGN_START);

	/*
		Feed retention period
	*/
	GtkWidget* feed_retention_option_label = gtk_label_new ("Keep feeds");
	gtk_widget_set_valign (feed_retention_option_label, GTK_ALIGN_START);

	GtkWidget* feed_option_keep_all = gtk_radio_button_new_with_label (NULL, "Forever");
	gtk_widget_set_valign (feed_option_keep_all, GTK_ALIGN_START);

	GtkWidget* feed_option_keep_last7 = gtk_radio_button_new_with_label_from_widget (GTK_RADIO_BUTTON (
	                                        feed_option_keep_all), "7 days");
	gtk_widget_set_valign (feed_option_keep_last7, GTK_ALIGN_START);

	GtkWidget* feed_option_keep_last24hr = gtk_radio_button_new_with_label_from_widget (GTK_RADIO_BUTTON (
	        feed_option_keep_all), "1 day");
	gtk_widget_set_valign (feed_option_keep_last24hr, GTK_ALIGN_START);

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
	/*
		Tree Model to describe the columns
	*/
	GtkListStore* list_store = gtk_list_store_new (3 /*3 columns*/,
	                           G_TYPE_STRING,/*feed name*/
	                           G_TYPE_STRING,/*feed url*/
	                           G_TYPE_STRING /*last retrieved*/);


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
		gchar* feed_url = feed.feed_url.data();
		gchar* last_retrieved = feed.last_retrieved.data();

		/*
			Adds a new row in the Tree Model.
		*/
		gtk_list_store_append (list_store, &iter);

		/*
			Links specific data to column positions in the row.
		*/
		gtk_list_store_set (list_store, &iter,
		                    0 /*0 = feed name column*/, feed_name,
		                    1 /*1 = feed url column*/, feed_url,
		                    2 /*2 = last retrieved column*/, last_retrieved,
		                    -1);
	}

	/*
		Column: Feed Name
	*/
	GtkCellRenderer* column_renderer_feed_name = gtk_cell_renderer_text_new();
	GtkTreeViewColumn* column_feed_name = gtk_tree_view_column_new_with_attributes ("Feed",
	                                      column_renderer_feed_name, "text", 0, NULL);
	gtk_tree_view_append_column (GTK_TREE_VIEW (rss_tree_view), column_feed_name);

	/*
		Column: Feed Url
	*/
	GtkCellRenderer* column_renderer_feed_url = gtk_cell_renderer_text_new();
	GtkTreeViewColumn* column_feed_url = gtk_tree_view_column_new_with_attributes ("Web addres",
	                                     column_renderer_feed_url, "text", 1, NULL);
	gtk_tree_view_append_column (GTK_TREE_VIEW (rss_tree_view), column_feed_url);

	/*
		Column: Last Retrieved
	*/
	GtkCellRenderer* column_renderer_feed_last_retrieved = gtk_cell_renderer_text_new();
	GtkTreeViewColumn* column_feed_last_retrieved = gtk_tree_view_column_new_with_attributes ("Last updated",
	        column_renderer_feed_last_retrieved, "text", 2, NULL);
	gtk_tree_view_append_column (GTK_TREE_VIEW (rss_tree_view), column_feed_last_retrieved);

	gtk_tree_view_set_model (GTK_TREE_VIEW (rss_tree_view), GTK_TREE_MODEL (list_store));

	return;
}
