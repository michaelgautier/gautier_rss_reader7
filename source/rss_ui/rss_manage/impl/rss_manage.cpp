/*
Copyright (C) 2019 Michael Gautier

This source code is free software; you can redistribute it and/or modify it under the terms of the GNU Lesser General Public License as published by the Free Software Foundation; either version 2.1 of the License, or (at your option) any later version.

This source code is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public License along with this library. If not, see <http://www.gnu.org/licenses/>.

Author: Michael Gautier <michaelgautier.wordpress.com>
*/

#include <iostream>

#include "rss_ui/application.hpp"
#include "rss_ui/rss_manage/rss_manage.hpp"

#include "rss_lib/rss/rss_reader.hpp"
#include "rss_lib/rss/rss_writer.hpp"
#include "rss_lib/rss/rss_feed.hpp"

static GtkWindow*
win = NULL;

static GtkWindow*
parent_win = NULL;

static std::queue<gautier_rss_data_read::rss_feed_mod>*
feed_changes;

static void
create_window (GtkApplication* app, GtkWindow* parent, int window_width, int window_height);

static void
layout_rss_feed_entry_area (GtkWidget* feed_entry_layout_row1, GtkWidget* feed_entry_layout_row2,
                            GtkWindow* win);

namespace ns_app = gautier_rss_ui_app;
namespace ns_read = gautier_rss_data_read;
namespace ns_write = gautier_rss_data_write;

/*
	RSS Table
*/
static int
row_id_before = 0;

static int
row_id_now = 0;

static int
col_pos_feed_name = 0;

static int
col_pos_feed_article_count = 1;

static int
col_pos_feed_retrieved = 2;

static int
col_pos_feed_retention_days = 3;

static int
col_pos_feed_retrieve_limit_hrs = 4;

static int
col_pos_feed_webaddress = 5;

static int
col_pos_stop = -1;

extern "C"
void
rss_tree_view_selected (GtkTreeSelection* tree_selection, gpointer user_data);

static void
layout_rss_tree_view (GtkWidget* rss_tree_view);

static void
populate_rss_tree_view (GtkWidget* rss_tree_view);

static void
select_rss_tree_row_by_rss_url (std::string rss_url);

GtkWidget* rss_tree_view;
GtkListStore* list_store;
GtkTreeSelection* rss_tree_selection_manager;

/*
	Update Button
*/
GtkWidget* update_configuration_button;

extern "C"
void
update_configuration_click (GtkButton* button, gpointer user_data);

/*
	Delete Button
*/
GtkWidget* delete_configuration_button;

extern "C"
void
delete_configuration_click (GtkButton* button, gpointer user_data);

/*
	Reset Button
*/
GtkWidget* reset_configuration_button;

extern "C"
void
reset_configuration_click (GtkButton* button, gpointer user_data);

static void
reset_data_entry();

/*
	Feed name/url
*/
GtkWidget* feed_name_entry;
GtkWidget* feed_url_entry;

/*
	Feed Name Entry - Callbacks

	These callbacks work together to "indicate"" a change has occurred.
*/
extern "C"
void
feed_name_preedit (GtkEntry* entry, gchar* preedit, gpointer user_data);

extern "C"
void
feed_name_inserted (GtkEntryBuffer* buffer, guint position, gchar* chars, guint n_chars, gpointer user_data);

extern "C"
void
feed_name_deleted (GtkEntryBuffer* buffer, guint position, guint n_chars, gpointer user_data);

/*
	Feed Url Entry - Callbacks

	These callbacks work together to "indicate"" a change has occurred.
*/
extern "C"
void
feed_url_preedit (GtkEntry* entry, gchar* preedit, gpointer user_data);

extern "C"
void
feed_url_inserted (GtkEntryBuffer* buffer, guint position, gchar* chars, guint n_chars, gpointer user_data);

extern "C"
void
feed_url_deleted (GtkEntryBuffer* buffer, guint position, guint n_chars, gpointer user_data);

/*
	Validate feed name and url.
*/
static void
check_feed_keys (GtkEntryBuffer* feed_name_buffer, GtkEntryBuffer* feed_url_buffer);

/*
	RSS configuration options.
*/
GtkWidget* feed_refresh_interval;
GtkWidget* feed_retention_option;

void
gautier_rss_win_rss_manage::set_modification_queue (std::queue<gautier_rss_data_read::rss_feed_mod>* updates)
{
	feed_changes = updates;

	return;
}

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
		Horizontal Layout 2: Buttons.
	*/
	GtkWidget* feed_entry_layout_row2 = gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 4);
	gtk_widget_set_halign (feed_entry_layout_row2, GTK_ALIGN_END);

	/*
		Setup the feed entry area.
	*/
	layout_rss_feed_entry_area (feed_entry_layout_row1, feed_entry_layout_row2, win);

	/*
		Add feed entry area to main layout.
	*/
	bool content_expands = false;
	bool content_fills = false;

	gtk_box_pack_start (GTK_BOX (window_layout), feed_entry_layout_row1, content_expands, content_fills, 4);
	gtk_box_pack_start (GTK_BOX (window_layout), feed_entry_layout_row2, content_expands, content_fills, 4);

	/*
		RSS Configuration Table
	*/
	rss_tree_view = gtk_tree_view_new();
	gtk_widget_set_size_request (rss_tree_view, -1, window_height * 2);
	rss_tree_selection_manager = gtk_tree_view_get_selection (GTK_TREE_VIEW (rss_tree_view));
	gtk_tree_selection_set_mode (rss_tree_selection_manager, GTK_SELECTION_SINGLE);
	g_signal_connect (rss_tree_selection_manager, "changed", G_CALLBACK (rss_tree_view_selected), NULL);

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
                            GtkWindow* win)
{
	/*
		Feed name
	*/
	GtkWidget* feed_name_label = gtk_label_new ("Feed name");

	feed_name_entry = gtk_entry_new();
	gtk_entry_set_max_length (GTK_ENTRY (feed_name_entry), 100);
	gtk_widget_set_size_request (feed_name_entry, 240, 24);
	g_signal_connect (feed_name_entry, "preedit-changed", G_CALLBACK (feed_name_preedit), feed_name_entry);
	g_signal_connect (gtk_entry_get_buffer (GTK_ENTRY (feed_name_entry)), "inserted-text",
	                  G_CALLBACK (feed_name_inserted), feed_name_entry);
	g_signal_connect (gtk_entry_get_buffer (GTK_ENTRY (feed_name_entry)), "deleted-text",
	                  G_CALLBACK (feed_name_deleted), feed_name_entry);

	/*
		Feed url
	*/
	GtkWidget* feed_url_label = gtk_label_new ("Feed web address");

	feed_url_entry = gtk_entry_new();
	gtk_entry_set_max_length (GTK_ENTRY (feed_url_entry), 512);
	gtk_widget_set_size_request (feed_url_entry, 330, 24);
	g_signal_connect (feed_url_entry, "preedit-changed", G_CALLBACK (feed_url_preedit), feed_url_entry);
	g_signal_connect (gtk_entry_get_buffer (GTK_ENTRY (feed_url_entry)), "inserted-text",
	                  G_CALLBACK (feed_url_inserted), feed_url_entry);
	g_signal_connect (gtk_entry_get_buffer (GTK_ENTRY (feed_url_entry)), "deleted-text",
	                  G_CALLBACK (feed_url_deleted), feed_url_entry);

	/*
		Feed refresh interval
	*/
	GtkWidget* feed_refresh_interval_label = gtk_label_new ("Refresh interval");

	feed_refresh_interval = gtk_spin_button_new_with_range (1, 4, 1);
	gtk_widget_set_size_request (feed_refresh_interval, 80, 24);

	/*
		Feed retention period
	*/
	GtkWidget* feed_retention_option_label = gtk_label_new ("Keep feeds");

	feed_retention_option = gtk_combo_box_text_new();
	gtk_combo_box_text_append (GTK_COMBO_BOX_TEXT (feed_retention_option), "1", "Forever");
	gtk_combo_box_text_append (GTK_COMBO_BOX_TEXT (feed_retention_option), "2", "7 days");
	gtk_combo_box_text_append (GTK_COMBO_BOX_TEXT (feed_retention_option), "3", "1 day");
	gtk_combo_box_set_active (GTK_COMBO_BOX (feed_retention_option), 0);

	/*
		Insert/Update/Delete
	*/
	update_configuration_button = gtk_button_new_with_label ("Update");
	/*Begin with the button disabled until a valid imput is made.*/
	gtk_widget_set_sensitive (update_configuration_button, false);

	g_signal_connect (update_configuration_button, "clicked", G_CALLBACK (update_configuration_click), NULL);

	delete_configuration_button = gtk_button_new_with_label ("delete");
	gtk_widget_set_sensitive (delete_configuration_button, false);

	g_signal_connect (delete_configuration_button, "clicked", G_CALLBACK (delete_configuration_click), NULL);

	/*
		Reset
	*/
	reset_configuration_button = gtk_button_new_with_label ("Reset");
	gtk_widget_set_sensitive (reset_configuration_button, true);

	g_signal_connect (reset_configuration_button, "clicked", G_CALLBACK (reset_configuration_click), NULL);


	/*
		Layout and arrange
	*/
	gtk_container_add (GTK_CONTAINER (feed_entry_layout_row1), feed_name_label);
	gtk_container_add (GTK_CONTAINER (feed_entry_layout_row1), feed_name_entry);
	gtk_container_add (GTK_CONTAINER (feed_entry_layout_row1), feed_url_label);
	gtk_container_add (GTK_CONTAINER (feed_entry_layout_row1), feed_url_entry);
	gtk_container_add (GTK_CONTAINER (feed_entry_layout_row1), feed_refresh_interval_label);
	gtk_container_add (GTK_CONTAINER (feed_entry_layout_row1), feed_refresh_interval);
	gtk_container_add (GTK_CONTAINER (feed_entry_layout_row1), feed_retention_option_label);
	gtk_container_add (GTK_CONTAINER (feed_entry_layout_row1), feed_retention_option);

	gtk_container_add (GTK_CONTAINER (feed_entry_layout_row2), reset_configuration_button);
	gtk_container_add (GTK_CONTAINER (feed_entry_layout_row2), delete_configuration_button);
	gtk_container_add (GTK_CONTAINER (feed_entry_layout_row2), update_configuration_button);

	return;
}

void
update_configuration_click (GtkButton* button, gpointer user_data)
{
	std::string db_file_name = ns_app::get_db_file_name();

	std::string feed_name = gtk_entry_get_text (GTK_ENTRY (feed_name_entry));
	std::string feed_url = gtk_entry_get_text (GTK_ENTRY (feed_url_entry));

	int retrieve_limit = gtk_spin_button_get_value_as_int (GTK_SPIN_BUTTON (feed_refresh_interval));

	std::string retrieve_limit_hrs = std::to_string (retrieve_limit);
	std::string retention_days;

	std::string active_id = gtk_combo_box_get_active_id (GTK_COMBO_BOX (feed_retention_option));

	int retention_id = std::stoi (active_id);

	switch (retention_id) {
		case 1:
			retention_days = "-1";
			break;

		case 2:
			retention_days = "7";
			break;

		case 3:
			retention_days = "1";
			break;
	}

	if (feed_name.empty() == false && feed_url.empty() == false) {
		bool changed = false;
		bool added = false;

		std::string existing_row_id = ns_read::get_row_id (db_file_name, feed_url);

		if (row_id_now > 0) {
			std::string row_id = std::to_string (row_id_now);

			ns_read::rss_feed old_feed;

			/*
				Compare the existing database snapshot of the feed to the proposed changes.
				Only update the database if there is a change.
			*/
			ns_read::get_feed_by_row_id (db_file_name, row_id, old_feed);

			if (old_feed.feed_name != feed_name ||
			        old_feed.feed_url != feed_url ||
			        old_feed.retrieve_limit_hrs != retrieve_limit_hrs ||
			        old_feed.retention_days != retention_days) {
				changed = (existing_row_id.empty() == true || existing_row_id == row_id);
			}

			/*
				Block duplicate feed url.
				Existing_Row_Id can be empty if the url doesn't exist in the database.
				Otherwise, it should be the same url.
			*/
			if (changed) {
				ns_write::update_feed_config (db_file_name, row_id, feed_name, feed_url, retrieve_limit_hrs, retention_days);
			} else {
				std::cout << "EXISTING rss data changed. Failed update due to failed duplicate url check\n";
			}
		} else {
			/*
				Block duplicate feed url
			*/
			if (existing_row_id.empty() == true) {
				ns_write::set_feed_config (db_file_name, feed_name, feed_url, retrieve_limit_hrs, retention_days);

				changed = true;
				added = true;
			} else {
				std::cout << "NEW rss data. Failed insert due to failed duplicate url check\n";
			}
		}

		existing_row_id = ns_read::get_row_id (db_file_name, feed_url);

		if (changed && existing_row_id.empty() == false) {
			feed_changes->emplace (ns_read::rss_feed_mod());

			ns_read::rss_feed_mod* modification = &feed_changes->back();

			modification->status = ns_read::rss_feed_mod_status::none;
			modification->feed_name = feed_name;
			modification->row_id = std::stoi (existing_row_id);

			if (changed) {
				modification->status = ns_read::rss_feed_mod_status::change;
			} else if (added) {
				modification->status = ns_read::rss_feed_mod_status::insert;
			}

			populate_rss_tree_view (rss_tree_view);

			select_rss_tree_row_by_rss_url (feed_url);
		}
	}

	return;
}

void
delete_configuration_click (GtkButton* button, gpointer user_data)
{
	if (row_id_now > 0) {
		std::string db_file_name = ns_app::get_db_file_name();

		ns_read::rss_feed feed;

		/*
			Need the feed info as stored in the database.
		*/
		std::string row_id = std::to_string (row_id_now);

		ns_read::get_feed_by_row_id (db_file_name, row_id, feed);

		ns_write::delete_feed (db_file_name, feed.feed_url);

		/*
			Make sure it is deleted before invalidating the UI.
		*/
		std::string existing_row_id = ns_read::get_row_id (db_file_name, feed.feed_url);

		if (existing_row_id.empty() == true) {
			feed_changes->emplace (ns_read::rss_feed_mod());

			ns_read::rss_feed_mod* modification = &feed_changes->back();

			modification->status = ns_read::rss_feed_mod_status::none;
			modification->feed_name = feed.feed_name;
			modification->row_id = std::stoi (existing_row_id);

			modification->status = ns_read::rss_feed_mod_status::remove;

			populate_rss_tree_view (rss_tree_view);

			reset_data_entry();
		}
	}

	return;
}

void
reset_configuration_click (GtkButton* button, gpointer user_data)
{
	gint row_count = gtk_tree_selection_count_selected_rows (rss_tree_selection_manager);

	if (row_count > 0) {
		gtk_tree_selection_unselect_all (rss_tree_selection_manager);
	} else {
		reset_data_entry();
	}

	return;
}

void
reset_data_entry()
{
	gtk_widget_set_sensitive (delete_configuration_button, false);

	gtk_entry_set_text (GTK_ENTRY (feed_name_entry), "");
	gtk_entry_set_text (GTK_ENTRY (feed_url_entry), "");
	gtk_spin_button_set_value (GTK_SPIN_BUTTON (feed_refresh_interval), 1);
	gtk_combo_box_set_active_id (GTK_COMBO_BOX (feed_retention_option), "1");

	gtk_widget_grab_focus (feed_name_entry);

	return;
}

void
layout_rss_tree_view (GtkWidget* rss_tree_view)
{
	/*
		Tree Model to describe the columns
	*/
	list_store = gtk_list_store_new (6 /*6 columns*/,
	                                 G_TYPE_STRING,/*feed name*/
	                                 G_TYPE_STRING,/*article count*/
	                                 G_TYPE_STRING,/*last retrieved*/
	                                 G_TYPE_STRING,/*retention in days*/
	                                 G_TYPE_STRING,/*retrieve limit*/
	                                 G_TYPE_STRING /*web url*/);

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
	GtkTreeViewColumn* column_feed_retention_period = gtk_tree_view_column_new_with_attributes ("Retention (days)",
	        column_renderer_feed_retention_period, "text", col_pos_feed_retention_days, NULL);
	gtk_tree_view_append_column (GTK_TREE_VIEW (rss_tree_view), column_feed_retention_period);

	/*
		Column: Retrieve Limit
	*/
	GtkCellRenderer* column_renderer_feed_retrieve_limit_hrs = gtk_cell_renderer_text_new();
	GtkTreeViewColumn* column_feed_retrieve_limit_hrs = gtk_tree_view_column_new_with_attributes ("Refresh (hrs)",
	        column_renderer_feed_retrieve_limit_hrs, "text", col_pos_feed_retrieve_limit_hrs, NULL);
	gtk_tree_view_append_column (GTK_TREE_VIEW (rss_tree_view), column_feed_retrieve_limit_hrs);


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

	populate_rss_tree_view (rss_tree_view);

	return;
}

void
populate_rss_tree_view (GtkWidget* rss_tree_view)
{
	gtk_list_store_clear (list_store);

	row_id_before = 0;
	row_id_now = 0;

	GtkTreeIter iter;

	/*
		Make a row for each feed/url
	*/
	std::vector<ns_read::rss_feed> feed_info;

	/*
		std::string feed_name;
		std::string feed_url;
		std::string last_retrieved;
	*/

	std::string db_file_name = ns_app::get_db_file_name();

	ns_read::get_feed_names (db_file_name, feed_info);

	for (ns_read::rss_feed feed : feed_info) {
		gchar* feed_name = feed.feed_name.data();
		gchar* article_count = std::string ("0").data();
		gchar* last_retrieved = feed.last_retrieved.data();
		gchar* retrieve_limit_hrs = feed.retrieve_limit_hrs.data();
		gchar* feed_url = feed.feed_url.data();

		std::string retention_period;

		int retention_days = std::stoi (feed.retention_days);

		if (retention_days == 1) {
			retention_period = "1 day";
		} else if (retention_days == 7) {
			retention_period = "7 days";
		} else {
			retention_period = "Forever";
		}

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
		                    col_pos_feed_retention_days, retention_period.data(),
		                    col_pos_feed_retrieve_limit_hrs, retrieve_limit_hrs,
		                    col_pos_feed_webaddress, feed_url,
		                    col_pos_stop);
	}

	return;
}

void
select_rss_tree_row_by_rss_url (std::string rss_url)
{
	GtkTreeModel* tree_model = gtk_tree_view_get_model (GTK_TREE_VIEW (rss_tree_view));

	GtkTreeIter tree_iterator;

	gtk_tree_model_get_iter_first (tree_model, &tree_iterator);

	while (gtk_tree_model_iter_next (tree_model, &tree_iterator)) {
		gchar* feed_url;

		gtk_tree_model_get (tree_model, &tree_iterator, col_pos_feed_webaddress, &feed_url, -1);

		std::string rss_url_now = feed_url;

		if (feed_url == rss_url) {
			gtk_tree_selection_select_iter (rss_tree_selection_manager, &tree_iterator);

			break;
		}
	}

	return;
}

void
rss_tree_view_selected (GtkTreeSelection* tree_selection, gpointer user_data)
{
	/*
		GTK documentation says this will not work if the selection mode is GTK_SELECTION_MULTIPLE.
	*/
	GtkTreeModel* tree_model;
	GtkTreeIter tree_iterator;

	gtk_widget_set_sensitive (delete_configuration_button, false);

	bool row_selected = gtk_tree_selection_get_selected (tree_selection, &tree_model, &tree_iterator);

	row_id_before = row_id_now;
	row_id_now = 0;

	if (row_selected) {
		gchar* feed_name;
		gchar* feed_url;
		gchar* retrieve_limit_hrs;
		gchar* retention_days;

		gtk_tree_model_get (tree_model, &tree_iterator, col_pos_feed_name, &feed_name, -1);
		gtk_tree_model_get (tree_model, &tree_iterator, col_pos_feed_webaddress, &feed_url, -1);
		gtk_tree_model_get (tree_model, &tree_iterator, col_pos_feed_retrieve_limit_hrs, &retrieve_limit_hrs, -1);
		gtk_tree_model_get (tree_model, &tree_iterator, col_pos_feed_retention_days, &retention_days, -1);

		gtk_entry_set_text (GTK_ENTRY (feed_name_entry), feed_name);
		gtk_entry_set_text (GTK_ENTRY (feed_url_entry), feed_url);
		gtk_spin_button_set_value (GTK_SPIN_BUTTON (feed_refresh_interval), std::stoi (retrieve_limit_hrs));

		std::string active_id;
		std::string retention_days_text = retention_days;

		if (retention_days_text == "Forever") {
			//Forever
			active_id = "1";
		} else if (retention_days_text == "7 days") {
			//1 day
			active_id = "2";
		} else if (retention_days_text == "1 day") {
			//7 days
			active_id = "3";
		}

		gboolean row_found = gtk_combo_box_set_active_id (GTK_COMBO_BOX (feed_retention_option), active_id.data());

		if (row_found == false) {
			gtk_combo_box_set_active_id (GTK_COMBO_BOX (feed_retention_option), "1");
		}

		std::string rss_url = feed_url;

		if (rss_url.empty()) {
			row_id_now = 0;
		} else {
			std::string db_file_name = ns_app::get_db_file_name();

			std::string row_id = ns_read::get_row_id (db_file_name, rss_url);

			if (row_id.empty() == false) {
				row_id_now = std::stoi (row_id);
			}
		}
	} else if (row_id_before > 0) {
		reset_data_entry();
	}

	gtk_widget_set_sensitive (delete_configuration_button, row_id_now > 0);

	return;
}

/*
	Feed Name Entry - Callbacks

	These callbacks work together to "indicate"" a change has occurred.
*/
void
feed_name_preedit (GtkEntry* entry, gchar* preedit, gpointer user_data)
{
	GtkEntryBuffer* feed_name_buffer = gtk_entry_buffer_new (preedit, -1);
	GtkEntryBuffer* feed_url_buffer = gtk_entry_get_buffer (GTK_ENTRY (feed_url_entry));

	check_feed_keys (feed_name_buffer, feed_url_buffer);

	return;
}

void
feed_name_inserted (GtkEntryBuffer* buffer, guint position, gchar* chars, guint n_chars, gpointer user_data)
{
	GtkEntryBuffer* feed_name_buffer = buffer;
	GtkEntryBuffer* feed_url_buffer = gtk_entry_get_buffer (GTK_ENTRY (feed_url_entry));

	check_feed_keys (feed_name_buffer, feed_url_buffer);

	return;
}

void
feed_name_deleted (GtkEntryBuffer* buffer, guint position, guint n_chars, gpointer user_data)
{
	GtkEntryBuffer* feed_name_buffer = buffer;
	GtkEntryBuffer* feed_url_buffer = gtk_entry_get_buffer (GTK_ENTRY (feed_url_entry));

	check_feed_keys (feed_name_buffer, feed_url_buffer);

	return;
}

/*
	Feed Url Entry - Callbacks

	These callbacks work together to "indicate"" a change has occurred.
*/
void
feed_url_preedit (GtkEntry* entry, gchar* preedit, gpointer user_data)
{
	GtkEntryBuffer* feed_name_buffer = gtk_entry_get_buffer (GTK_ENTRY (feed_name_entry));
	GtkEntryBuffer* feed_url_buffer = gtk_entry_buffer_new (preedit, -1);

	check_feed_keys (feed_name_buffer, feed_url_buffer);

	return;
}

void
feed_url_inserted (GtkEntryBuffer* buffer, guint position, gchar* chars, guint n_chars, gpointer user_data)
{
	GtkEntryBuffer* feed_name_buffer = gtk_entry_get_buffer (GTK_ENTRY (feed_name_entry));
	GtkEntryBuffer* feed_url_buffer = buffer;

	check_feed_keys (feed_name_buffer, feed_url_buffer);

	return;
}

void
feed_url_deleted (GtkEntryBuffer* buffer, guint position, guint n_chars, gpointer user_data)
{
	GtkEntryBuffer* feed_name_buffer = gtk_entry_get_buffer (GTK_ENTRY (feed_name_entry));
	GtkEntryBuffer* feed_url_buffer = buffer;

	check_feed_keys (feed_name_buffer, feed_url_buffer);

	return;
}

/*
	Validate feed name and url.
*/
static void
check_feed_keys (GtkEntryBuffer* feed_name_buffer, GtkEntryBuffer* feed_url_buffer)
{
	bool enabled = false;

	gint feed_name_length = gtk_entry_buffer_get_length (feed_name_buffer);
	gint feed_url_length = gtk_entry_buffer_get_length (feed_url_buffer);

	enabled = (feed_name_length > 2 && feed_url_length > 6);

	gtk_widget_set_sensitive (update_configuration_button, enabled);

	return;
}

