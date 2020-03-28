/*
Copyright (C) 2020 Michael Gautier

This source code is free software; you can redistribute it and/or modify it under the terms of the GNU Lesser General Public License as published by the Free Software Foundation; either version 2.1 of the License, or (at your option) any later version.

This source code is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public License along with this library. If not, see <http://www.gnu.org/licenses/>.

Author: Michael Gautier <michaelgautier.wordpress.com>
*/

#include <cstddef>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <ctime>
#include <cuchar>
#include <cwchar>
#include <iostream>
#include <map>
#include <string>
#include <vector>

#include "rss_lib/rss/rss_reader.hpp"
#include "rss_lib/rss/rss_writer.hpp"
#include "rss_lib/rss/rss_feed.hpp"
#include "rss_lib/rss/rss_util.hpp"

#include "rss_ui/application.hpp"
#include "rss_ui/rss_manage/rss_manage.hpp"

namespace ns_app = gautier_rss_ui_app;
namespace ns_data_read = gautier_rss_data_read;
namespace ns_data_write = gautier_rss_data_write;

using feed_by_name_type = std::map<std::string, ns_data_read::rss_feed>;
using feed_mod_by_name_type = std::map<std::string, ns_data_read::rss_feed_mod>;

static
feed_mod_by_name_type feed_changes;

static
feed_by_name_type feed_model_original;

static
feed_by_name_type feed_model_updated;

static GtkWindow*
win = NULL;

static GtkWindow*
parent_win = NULL;

static
gautier_rss_win_rss_manage::feed_mod_cb_type* feed_mod_cb;

static
void
finalize_rss_modifications();

static void
create_window (GtkApplication* app, GtkWindow* parent, const int window_width, const int window_height);

static void
layout_rss_feed_entry_area (GtkWidget* feed_entry_layout_row1, GtkWidget* feed_entry_layout_row2,
                            GtkWindow* window);

/*
	RSS Table
*/
static const int
col_pos_feed_name = 0;

static const int
col_pos_feed_article_count = 1;

static const int
col_pos_feed_retrieved = 2;

static const int
col_pos_feed_retention_days = 3;

static const int
col_pos_feed_retrieve_limit_hrs = 4;

static const int
col_pos_feed_webaddress = 5;

static const int
col_pos_feed_rowid = 6;

static const int
col_pos_stop = -1;

extern "C"
void
rss_tree_view_selected (GtkTreeSelection* tree_selection, gpointer user_data);

static gulong
rss_tree_view_selected_signal_id = -1UL;

static void
layout_rss_tree_view (GtkWidget* rss_tree_view);

static void
populate_rss_tree_view (GtkWidget* rss_tree_view);

static void
select_rss_tree_row();

static GtkWidget* rss_tree_view;
static GtkListStore* list_store;
static GtkTreeSelection* rss_tree_selection_manager;

static int64_t
feed_row_id = -1;

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
static GtkWidget* delete_configuration_button;

extern "C"
void
delete_configuration_click (GtkButton* button, gpointer user_data);

/*
	Reset Button
*/
static GtkWidget* reset_configuration_button;

extern "C"
void
reset_configuration_click (GtkButton* button, gpointer user_data);

static void
reset_data_entry();

/*
	Feed name/url
*/
static GtkWidget* feed_name_entry;
static GtkWidget* feed_url_entry;

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

static bool
validate_values (const std::string feed_name, const std::string feed_url, const std::string retrieve_limit_hrs,
                 const std::string retention_days);

/*
	RSS configuration options.
*/
static GtkWidget* feed_refresh_interval;
static GtkWidget* feed_retention_option;

/*
	UI Window Shutdown
*/
extern "C"
void
rss_manage_window_destroy (GtkWidget* window, gpointer user_data);

/*
	RSS Modification Operations
*/
static void
modify_feed (ns_data_read::rss_feed* feed, const std::string feed_name, const std::string feed_url,
             const std::string retrieve_limit_hrs, const std::string retention_days);

static void
update_feed_config (const std::string feed_name, const std::string feed_url,
                    const std::string retrieve_limit_hrs, const std::string retention_days);

static void
delete_feed (const std::string feed_url);

/*
	RSS Feed Retrieval
*/
static bool
check_feed_exist_by_url (feed_by_name_type feed_model,
                         const std::string feed_url);

/*
	RSS Management Entry Points
*/
void
gautier_rss_win_rss_manage::set_feed_model (feed_by_name_type feed_model)
{
	feed_model_original.clear();
	feed_model_updated.clear();

	const std::string db_file_name = gautier_rss_ui_app::get_db_file_name();

	for (std::pair<std::string, ns_data_read::rss_feed> feed_entry : feed_model) {
		const std::string feed_url = feed_entry.second.feed_url;

		const int64_t row_id = ns_data_read::get_row_id (db_file_name, feed_url);

		feed_model_original.try_emplace (feed_url, ns_data_read::rss_feed());
		feed_model_updated.try_emplace (feed_url, ns_data_read::rss_feed());

		ns_data_read::rss_feed* feed_original = &feed_model_original[feed_url];
		ns_data_read::rss_feed* feed_updated = &feed_model_updated[feed_url];

		feed_entry.second.row_id = row_id;

		ns_data_read::copy_feed (&feed_entry.second, feed_original);
		ns_data_read::copy_feed (&feed_entry.second, feed_updated);
	}

	return;
}

void
gautier_rss_win_rss_manage::set_modification_callback (feed_mod_cb_type* feed_mod_callback)
{
	feed_mod_cb = feed_mod_callback;

	return;
}

void
gautier_rss_win_rss_manage::show_dialog (GtkApplication* app, GtkWindow* parent, const int window_width,
        const int window_height)
{
	/*
		RSS Management Window.
	*/
	create_window (app, parent, window_width, window_height);

	/*
		Vertical layout for the window's contents.
	*/
	GtkWidget* window_layout = gtk_box_new (GTK_ORIENTATION_VERTICAL, 0);
	gtk_container_add (GTK_CONTAINER (win), window_layout);

	/*
		Horizontal Layout 1: RSS entry fields.
	*/
	GtkWidget* feed_entry_layout_row1 = gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 0);

	/*
		Horizontal Layout 2: Buttons.
	*/
	GtkWidget* feed_entry_layout_row2 = gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 0);
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

	gtk_box_pack_start (GTK_BOX (window_layout), feed_entry_layout_row1, content_expands, content_fills, 0);
	gtk_box_pack_start (GTK_BOX (window_layout), feed_entry_layout_row2, content_expands, content_fills, 0);

	/*
		RSS Configuration Table
	*/
	rss_tree_view = gtk_tree_view_new();
	gtk_widget_set_size_request (rss_tree_view, -1, window_height * 2);
	rss_tree_selection_manager = gtk_tree_view_get_selection (GTK_TREE_VIEW (rss_tree_view));
	gtk_tree_selection_set_mode (rss_tree_selection_manager, GTK_SELECTION_SINGLE);
	rss_tree_view_selected_signal_id = g_signal_connect (rss_tree_selection_manager, "changed",
	                                   G_CALLBACK (rss_tree_view_selected), NULL);

	GtkWidget* scroll_win = gtk_scrolled_window_new (NULL, NULL);
	gtk_widget_set_valign (scroll_win, GTK_ALIGN_FILL);

	gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (scroll_win), GTK_POLICY_ALWAYS, GTK_POLICY_ALWAYS);
	gtk_container_add (GTK_CONTAINER (scroll_win), rss_tree_view);

	layout_rss_tree_view (rss_tree_view);

	content_expands = false;
	content_fills = true;

	gtk_widget_set_hexpand (scroll_win, true);
	gtk_widget_set_vexpand (scroll_win, true);

	gtk_box_pack_start (GTK_BOX (window_layout), scroll_win, content_expands, content_fills, 0);

	/*
		Window Presentation
	*/
	gtk_widget_show_all ((GtkWidget*)win);

	return;
}

static void
create_window (GtkApplication* app, GtkWindow* parent, const int window_width, const int window_height)
{
	/*
		Operating in a valid instance of a GTK application.
	*/
	if (app && parent) {
		std::cout << "RSS Management UI Initialized: " << gautier_rss_util::get_current_date_time_utc() << "\n";
	} else {
		std::cout << "ERROR Initializing RSS Management UI (no application or parent window)\n";
	}

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

	if (win) {
		g_signal_connect (win, "destroy", G_CALLBACK (rss_manage_window_destroy), NULL);
	}

	return;
}

/*
	Finalization
*/
void
rss_manage_window_destroy (GtkWidget* window, gpointer user_data)
{
	if (window) {
		if (user_data) {
			std::cout << __func__ << " called with user_data\n";
		}

		finalize_rss_modifications();
	}

	return;
}

void
finalize_rss_modifications()
{
	if (win && feed_mod_cb) {
		const std::string db_file_name = ns_app::get_db_file_name();

		/*
			Reduce feed changes down to actual changes.

			Implement actual modifications.
		*/
		for (std::pair<std::string, ns_data_read::rss_feed> feed_entry : feed_model_updated) {
			ns_data_read::rss_feed feed_item = feed_entry.second;

			const int64_t row_id = feed_item.row_id;

			const std::string feed_name = feed_item.feed_name;
			const std::string feed_url = feed_item.feed_url;
			const std::string retrieve_limit_hrs = feed_item.retrieve_limit_hrs;
			const std::string retention_days = feed_item.retention_days;

			/*
				Compare the existing database snapshot of the feed to the proposed changes.
				Only update the database if there is a change.
			*/
			if (row_id > 0) {
				ns_data_read::rss_feed old_feed;

				ns_data_read::get_feed_by_row_id (db_file_name, row_id, old_feed);

				const bool changed = (old_feed.feed_name != feed_name ||
				                      old_feed.feed_url != feed_url ||
				                      old_feed.retrieve_limit_hrs != retrieve_limit_hrs ||
				                      old_feed.retention_days != retention_days);

				if (changed) {
					const std::string old_feed_name = old_feed.feed_name;

					ns_data_write::update_feed_config (db_file_name, row_id, feed_name, feed_url, retrieve_limit_hrs,
					                                   retention_days);

					feed_changes.insert_or_assign (old_feed_name, ns_data_read::rss_feed_mod());

					ns_data_read::rss_feed_mod* modification = &feed_changes[feed_name];

					modification->feed_name = old_feed_name;
					modification->status = ns_data_read::rss_feed_mod_status::change;
					modification->row_id = row_id;
				}
			}
			/*
				New feed metadata entry.
			*/
			else {
				ns_data_write::set_feed_config (db_file_name, feed_name, feed_url, retrieve_limit_hrs, retention_days);

				const int64_t new_row_id = ns_data_read::get_row_id (db_file_name, feed_url);

				if (new_row_id > 0) {
					feed_changes.insert_or_assign (feed_name, ns_data_read::rss_feed_mod());

					ns_data_read::rss_feed_mod* modification = &feed_changes[feed_name];

					modification->status = ns_data_read::rss_feed_mod_status::insert;
					modification->feed_name = feed_name;
					modification->row_id = new_row_id;
				}
			}
		}

		/*
			Deletions
		*/
		for (std::pair<std::string, ns_data_read::rss_feed> feed_entry : feed_model_original) {
			ns_data_read::rss_feed feed_item_original = feed_entry.second;

			bool exists_in_update = false;
			const int64_t orig_rowid = feed_item_original.row_id;

			for (std::pair<std::string, ns_data_read::rss_feed> feed_entry_updated : feed_model_updated) {
				ns_data_read::rss_feed feed_item_update = feed_entry_updated.second;

				const int64_t updt_rowid = feed_item_update.row_id;

				exists_in_update = (orig_rowid == updt_rowid);

				if (exists_in_update) {
					break;
				}
			}

			if (exists_in_update == false) {
				ns_data_write::delete_feed (db_file_name, feed_item_original.feed_url);

				const std::string feed_name = feed_item_original.feed_name;

				feed_changes.insert_or_assign (feed_name, ns_data_read::rss_feed_mod());

				ns_data_read::rss_feed_mod* modification = &feed_changes[feed_name];

				modification->status = ns_data_read::rss_feed_mod_status::remove;
				modification->feed_name = feed_name;
				modification->row_id = orig_rowid;
			}
		}

		feed_mod_cb (feed_changes);
	}

	/*
		Remove residual data
	*/
	feed_model_original.clear();

	return;
}

static void
layout_rss_feed_entry_area (GtkWidget* feed_entry_layout_row1, GtkWidget* feed_entry_layout_row2,
                            GtkWindow* window)
{
	if (window) {
		/*
			Feed name
		*/
		GtkWidget* feed_name_label = gtk_label_new ("Feed name");
		gautier_rss_ui_app::set_css_class (feed_name_label, "input_container");

		feed_name_entry = gtk_entry_new();
		gautier_rss_ui_app::set_css_class (feed_name_entry, "input_container");
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
		gautier_rss_ui_app::set_css_class (feed_url_label, "input_container");

		feed_url_entry = gtk_entry_new();
		gautier_rss_ui_app::set_css_class (feed_url_entry, "input_container");
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
		gautier_rss_ui_app::set_css_class (feed_refresh_interval_label, "input_container");

		feed_refresh_interval = gtk_spin_button_new_with_range (1, 4, 1);
		gautier_rss_ui_app::set_css_class (feed_refresh_interval, "input_container");
		gtk_widget_set_size_request (feed_refresh_interval, 80, 24);

		/*
			Feed retention period
		*/
		GtkWidget* feed_retention_option_label = gtk_label_new ("Keep feeds");
		gautier_rss_ui_app::set_css_class (feed_retention_option_label, "input_container");

		feed_retention_option = gtk_combo_box_text_new();
		gautier_rss_ui_app::set_css_class (feed_retention_option, "input_container");
		gtk_combo_box_text_append (GTK_COMBO_BOX_TEXT (feed_retention_option), "1", "Forever");
		gtk_combo_box_text_append (GTK_COMBO_BOX_TEXT (feed_retention_option), "2", "7 days");
		gtk_combo_box_text_append (GTK_COMBO_BOX_TEXT (feed_retention_option), "3", "1 day");
		gtk_combo_box_set_active (GTK_COMBO_BOX (feed_retention_option), 0);

		/*
			Insert/Update/Delete
		*/
		update_configuration_button = gtk_button_new_with_label ("Save");
		gautier_rss_ui_app::set_css_class (update_configuration_button, "button");

		/*Begin with the button disabled until a valid imput is made.*/
		gtk_widget_set_sensitive (update_configuration_button, false);

		g_signal_connect (update_configuration_button, "clicked", G_CALLBACK (update_configuration_click), NULL);

		delete_configuration_button = gtk_button_new_with_label ("Delete");
		gautier_rss_ui_app::set_css_class (delete_configuration_button, "button");

		gtk_widget_set_sensitive (delete_configuration_button, false);

		g_signal_connect (delete_configuration_button, "clicked", G_CALLBACK (delete_configuration_click), NULL);

		/*
			Reset
		*/
		reset_configuration_button = gtk_button_new_with_label ("Clear");
		gautier_rss_ui_app::set_css_class (reset_configuration_button, "button");

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
	}

	return;
}

void
update_configuration_click (GtkButton* button, gpointer user_data)
{
	if (button) {
		if (user_data) {
			std::cout << __func__ << " called with user_data\n";
		}

		const std::string feed_name = gtk_entry_get_text (GTK_ENTRY (feed_name_entry));
		const std::string feed_url = gtk_entry_get_text (GTK_ENTRY (feed_url_entry));

		const int retrieve_limit = gtk_spin_button_get_value_as_int (GTK_SPIN_BUTTON (feed_refresh_interval));

		const std::string retrieve_limit_hrs = std::to_string (retrieve_limit);
		std::string retention_days;

		const std::string active_id = gtk_combo_box_get_active_id (GTK_COMBO_BOX (feed_retention_option));

		if (active_id.empty() == false) {
			const int_fast32_t retention_id = (int_fast32_t) (std::stoi (active_id));

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
		}

		if (feed_name.empty() == false && feed_url.empty() == false) {
			update_feed_config (feed_name, feed_url, retrieve_limit_hrs, retention_days);

			g_signal_handler_disconnect (rss_tree_selection_manager, rss_tree_view_selected_signal_id);

			populate_rss_tree_view (rss_tree_view);

			rss_tree_view_selected_signal_id = g_signal_connect (rss_tree_selection_manager, "changed",
			                                   G_CALLBACK (rss_tree_view_selected), NULL);

			select_rss_tree_row();
		}
	}

	return;
}

void
delete_configuration_click (GtkButton* button, gpointer user_data)
{
	if (button) {
		if (user_data) {
			std::cout << __func__ << " called with user_data\n";
		}

		const std::string feed_url = gtk_entry_get_text (GTK_ENTRY (feed_url_entry));

		delete_feed (feed_url);

		populate_rss_tree_view (rss_tree_view);

		reset_data_entry();
	}

	return;
}

void
reset_configuration_click (GtkButton* button, gpointer user_data)
{
	if (button) {
		if (user_data != NULL) {
			std::cout << __func__ << " called with user_data\n";
		}

		gint row_count = gtk_tree_selection_count_selected_rows (rss_tree_selection_manager);

		if (row_count > 0) {
			gtk_tree_selection_unselect_all (rss_tree_selection_manager);
		} else {
			reset_data_entry();
		}
	}

	return;
}

void
reset_data_entry()
{
	gtk_widget_set_sensitive (delete_configuration_button, false);

	GtkEntryBuffer* b1 = gtk_entry_get_buffer (GTK_ENTRY (feed_name_entry));
	GtkEntryBuffer* b2 = gtk_entry_get_buffer (GTK_ENTRY (feed_url_entry));

	gtk_entry_buffer_delete_text (b1, 0, -1);
	gtk_entry_buffer_delete_text (b2, 0, -1);

	gtk_spin_button_set_value (GTK_SPIN_BUTTON (feed_refresh_interval), 1);
	gtk_combo_box_set_active_id (GTK_COMBO_BOX (feed_retention_option), "1");

	gtk_widget_grab_focus (feed_name_entry);

	gtk_button_set_label (GTK_BUTTON (update_configuration_button), "Save");

	return;
}

void
layout_rss_tree_view (GtkWidget* tree_view)
{
	if (tree_view) {
		/*
			Tree Model to describe the columns
		*/
		list_store = gtk_list_store_new (7 /*7 columns*/,
		                                 G_TYPE_STRING,/*feed name*/
		                                 G_TYPE_STRING,/*article count*/
		                                 G_TYPE_STRING,/*last retrieved*/
		                                 G_TYPE_STRING,/*retention in days*/
		                                 G_TYPE_STRING,/*retrieve limit*/
		                                 G_TYPE_STRING,/*web url*/
		                                 G_TYPE_STRING /*row_id*/);

		/*
			Column: Feed Name
		*/
		GtkCellRenderer* column_renderer_feed_name = gtk_cell_renderer_text_new();
		GtkTreeViewColumn* column_feed_name = gtk_tree_view_column_new_with_attributes ("Feed",
		                                      column_renderer_feed_name, "text", col_pos_feed_name, NULL);
		gtk_tree_view_append_column (GTK_TREE_VIEW (tree_view), column_feed_name);

		/*
			Column: Article Count
		*/
		GtkCellRenderer* column_renderer_feed_article_count = gtk_cell_renderer_text_new();
		GtkTreeViewColumn* column_feed_article_count = gtk_tree_view_column_new_with_attributes ("Article Count",
		        column_renderer_feed_article_count, "text", col_pos_feed_article_count, NULL);
		gtk_tree_view_append_column (GTK_TREE_VIEW (tree_view), column_feed_article_count);

		/*
			Column: Last Retrieved
		*/
		GtkCellRenderer* column_renderer_feed_last_retrieved = gtk_cell_renderer_text_new();
		GtkTreeViewColumn* column_feed_last_retrieved = gtk_tree_view_column_new_with_attributes ("Last updated",
		        column_renderer_feed_last_retrieved, "text", col_pos_feed_retrieved, NULL);
		gtk_tree_view_append_column (GTK_TREE_VIEW (tree_view), column_feed_last_retrieved);

		/*
			Column: Retention Period
		*/
		GtkCellRenderer* column_renderer_feed_retention_period = gtk_cell_renderer_text_new();
		GtkTreeViewColumn* column_feed_retention_period = gtk_tree_view_column_new_with_attributes ("Retention (days)",
		        column_renderer_feed_retention_period, "text", col_pos_feed_retention_days, NULL);
		gtk_tree_view_append_column (GTK_TREE_VIEW (tree_view), column_feed_retention_period);

		/*
			Column: Retrieve Limit
		*/
		GtkCellRenderer* column_renderer_feed_retrieve_limit_hrs = gtk_cell_renderer_text_new();
		GtkTreeViewColumn* column_feed_retrieve_limit_hrs = gtk_tree_view_column_new_with_attributes ("Refresh (hrs)",
		        column_renderer_feed_retrieve_limit_hrs, "text", col_pos_feed_retrieve_limit_hrs, NULL);
		gtk_tree_view_append_column (GTK_TREE_VIEW (tree_view), column_feed_retrieve_limit_hrs);


		/*
			Column: Feed Url
		*/
		GtkCellRenderer* column_renderer_feed_url = gtk_cell_renderer_text_new();
		GtkTreeViewColumn* column_feed_url = gtk_tree_view_column_new_with_attributes ("Web address",
		                                     column_renderer_feed_url, "text", col_pos_feed_webaddress, NULL);
		gtk_tree_view_append_column (GTK_TREE_VIEW (tree_view), column_feed_url);

		/*
			Populate tree
		*/
		gtk_tree_view_set_model (GTK_TREE_VIEW (tree_view), GTK_TREE_MODEL (list_store));

		populate_rss_tree_view (tree_view);
	}

	return;
}

void
populate_rss_tree_view (GtkWidget* tree_view)
{
	if (tree_view) {
		gtk_list_store_clear (list_store);

		GtkTreeIter iter;

		std::map<std::string, std::string> name_to_url;

		for (std::pair<std::string, ns_data_read::rss_feed> feed_entry : feed_model_updated) {
			ns_data_read::rss_feed feed = feed_entry.second;

			name_to_url[feed.feed_name] = feed.feed_url;
		}

		for (std::pair<std::string, std::string> name_to_url_entry : name_to_url) {
			std::string feed_url_value = name_to_url_entry.second;

			ns_data_read::rss_feed feed = feed_model_updated[feed_url_value];

			std::string article_count_text = std::to_string (feed.article_count);
			std::string row_id_text = std::to_string (feed.row_id);

			gchar* feed_name = feed.feed_name.data();
			gchar* article_count = article_count_text.data();
			gchar* last_retrieved = feed.last_retrieved.data();
			gchar* retrieve_limit_hrs = feed.retrieve_limit_hrs.data();
			gchar* feed_url = feed.feed_url.data();
			gchar* rowid = row_id_text.data();

			std::string retention_period;

			if (feed.retention_days.empty() == false) {
				const int_fast32_t retention_days = std::stoi (feed.retention_days);

				if (retention_days == 1) {
					retention_period = "1 day";
				} else if (retention_days == 7) {
					retention_period = "7 days";
				} else {
					retention_period = "Forever";
				}
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
			                    col_pos_feed_rowid, rowid,
			                    col_pos_stop);
		}
	}

	return;
}

void
select_rss_tree_row()
{
	if (feed_row_id > 0) {
		GtkTreeModel* tree_model = gtk_tree_view_get_model (GTK_TREE_VIEW (rss_tree_view));

		GtkTreeIter tree_iterator;

		gboolean iter_is_valid = gtk_tree_model_get_iter_first (tree_model, &tree_iterator);

		while (iter_is_valid) {
			gchar* row_id_chars;

			gtk_tree_model_get (tree_model, &tree_iterator, col_pos_feed_rowid, &row_id_chars, -1);

			const std::string row_id_text = row_id_chars;

			const int64_t id = std::stoll (row_id_text);

			if (id == feed_row_id) {
				gtk_tree_selection_select_iter (rss_tree_selection_manager, &tree_iterator);

				break;
			} else {
				iter_is_valid = gtk_tree_model_iter_next (tree_model, &tree_iterator);
			}
		}
	}

	return;
}

void
rss_tree_view_selected (GtkTreeSelection* tree_selection, gpointer user_data)
{
	bool row_selected = false;

	feed_row_id = -1;

	gtk_button_set_label (GTK_BUTTON (reset_configuration_button), "Clear");

	if (tree_selection) {
		if (user_data != NULL) {
			std::cout << __func__ << " called with user_data\n";
		}

		/*
			GTK documentation says this will not work if
				selection mode is GTK_SELECTION_MULTIPLE.
		*/
		GtkTreeModel* tree_model;
		GtkTreeIter tree_iterator;

		row_selected = gtk_tree_selection_get_selected (tree_selection, &tree_model, &tree_iterator);

		if (row_selected) {
			gtk_button_set_label (GTK_BUTTON (update_configuration_button), "Update");

			std::string feed_name;
			std::string last_retrieved;
			std::string retention_days;
			int_fast32_t retrieve_limit_hrs;
			std::string feed_url;
			int64_t rowid;

			const gint column_count = gtk_tree_model_get_n_columns (tree_model);

			for (int i = 0; i < column_count; i++) {
				GValue gvalue = G_VALUE_INIT;

				gtk_tree_model_get_value (tree_model, &tree_iterator, i, &gvalue);

				const gchar* value = g_value_get_string (&gvalue);

				if (i == col_pos_feed_name) {
					feed_name.insert (0, value);
				} else if (i == col_pos_feed_retrieved) {
					last_retrieved.insert (0, value);
				} else if (i == col_pos_feed_retention_days) {
					retention_days.insert (0, value);
				} else if (i == col_pos_feed_retrieve_limit_hrs) {
					retrieve_limit_hrs = gautier_rss_util::convert_chars_to_number (value, -1);
				} else if (i == col_pos_feed_webaddress) {
					feed_url.insert (0, value);
				} else if (i == col_pos_feed_rowid) {
					rowid = gautier_rss_util::convert_chars_to_number (value, -1);
				}

				g_value_unset (&gvalue);
			}

			GtkEntryBuffer* b1 = gtk_entry_get_buffer (GTK_ENTRY (feed_name_entry));
			GtkEntryBuffer* b2 = gtk_entry_get_buffer (GTK_ENTRY (feed_url_entry));

			gtk_entry_buffer_delete_text (b1, 0, -1);
			gtk_entry_buffer_delete_text (b2, 0, -1);

			gtk_entry_buffer_set_text (b1, feed_name.data(), -1);
			gtk_entry_buffer_set_text (b2, feed_url.data(), -1);

			gtk_spin_button_set_value (GTK_SPIN_BUTTON (feed_refresh_interval), (gdouble) (retrieve_limit_hrs));

			std::string active_id;
			const std::string retention_days_text = retention_days;

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

			const gboolean row_found = gtk_combo_box_set_active_id (GTK_COMBO_BOX (feed_retention_option),
			                           active_id.data());

			if (row_found == false) {
				gtk_combo_box_set_active_id (GTK_COMBO_BOX (feed_retention_option), "1");
			}

			feed_row_id = rowid;

			gtk_widget_set_sensitive (delete_configuration_button, row_selected);

			gtk_button_set_label (GTK_BUTTON (reset_configuration_button), "New");
		}
	}

	if (row_selected == false) {
		reset_data_entry();
	}

	return;
}

/*
	Feed Name Entry - Callbacks

	These callbacks work together to "indicate"" a change has occurred.
*/
void
feed_name_preedit (GtkEntry* entry, gchar* preedit, gpointer user_data)
{
	if (entry) {
		if (preedit || user_data) {
			std::cout << __func__ << " called with preedit or user_data\n";
		}

		GtkEntryBuffer* feed_name_buffer = gtk_entry_buffer_new (preedit, -1);
		GtkEntryBuffer* feed_url_buffer = gtk_entry_get_buffer (GTK_ENTRY (feed_url_entry));

		check_feed_keys (feed_name_buffer, feed_url_buffer);
	}

	return;
}

void
feed_name_inserted (GtkEntryBuffer* buffer, guint position, gchar* chars, guint n_chars, gpointer user_data)
{
	if (buffer) {
		if (user_data == NULL) {
			std::cout << __func__ << " called without user_data\n";
		}

		GtkEntryBuffer* feed_name_buffer = buffer;
		GtkEntryBuffer* feed_url_buffer = gtk_entry_get_buffer (GTK_ENTRY (feed_url_entry));

		check_feed_keys (feed_name_buffer, feed_url_buffer);
	} else {
		std::cout << __func__ << " called with NULL BUFFER: pos " << position << " char count " << n_chars << " chars "
		          << chars << "\n";
	}

	return;
}

void
feed_name_deleted (GtkEntryBuffer* buffer, guint position, guint n_chars, gpointer user_data)
{
	if (buffer) {
		if (user_data == NULL) {
			std::cout << __func__ << " called without user_data\n";
		}

		GtkEntryBuffer* feed_name_buffer = buffer;
		GtkEntryBuffer* feed_url_buffer = gtk_entry_get_buffer (GTK_ENTRY (feed_url_entry));

		check_feed_keys (feed_name_buffer, feed_url_buffer);
	} else {
		std::cout << __func__ << " called with NULL BUFFER: pos " << position << " char count " << n_chars << "\n";
	}

	return;
}

/*
	Feed Url Entry - Callbacks

	These callbacks work together to "indicate"" a change has occurred.
*/
void
feed_url_preedit (GtkEntry* entry, gchar* preedit, gpointer user_data)
{
	if (entry) {
		if (user_data == NULL) {
			std::cout << __func__ << " called without user_data\n";
		}

		GtkEntryBuffer* feed_name_buffer = gtk_entry_get_buffer (GTK_ENTRY (feed_name_entry));
		GtkEntryBuffer* feed_url_buffer = gtk_entry_buffer_new (preedit, -1);

		check_feed_keys (feed_name_buffer, feed_url_buffer);
	}

	return;
}

void
feed_url_inserted (GtkEntryBuffer* buffer, guint position, gchar* chars, guint n_chars, gpointer user_data)
{
	if (buffer) {
		if (user_data == NULL) {
			std::cout << __func__ << " called without user_data\n";
		}

		GtkEntryBuffer* feed_name_buffer = gtk_entry_get_buffer (GTK_ENTRY (feed_name_entry));
		GtkEntryBuffer* feed_url_buffer = buffer;

		check_feed_keys (feed_name_buffer, feed_url_buffer);
	} else {
		std::cout << __func__ << " called with NULL BUFFER: pos " << position << " char count " << n_chars << " chars "
		          << chars << "\n";
	}

	return;
}

void
feed_url_deleted (GtkEntryBuffer* buffer, guint position, guint n_chars, gpointer user_data)
{
	if (buffer) {
		if (user_data == NULL) {
			std::cout << __func__ << " called without user_data\n";
		}

		GtkEntryBuffer* feed_name_buffer = gtk_entry_get_buffer (GTK_ENTRY (feed_name_entry));
		GtkEntryBuffer* feed_url_buffer = buffer;

		check_feed_keys (feed_name_buffer, feed_url_buffer);
	} else {
		std::cout << __func__ << " called with NULL BUFFER: pos " << position << " char count " << n_chars << "\n";
	}

	return;
}

/*
	Validate feed name and url.
*/
static void
check_feed_keys (GtkEntryBuffer* feed_name_buffer, GtkEntryBuffer* feed_url_buffer)
{
	if (feed_name_buffer && feed_url_buffer) {
		bool enabled = false;

		guint feed_name_length = gtk_entry_buffer_get_length (feed_name_buffer);
		guint feed_url_length = gtk_entry_buffer_get_length (feed_url_buffer);

		enabled = (feed_name_length > 2 && feed_url_length > 6);

		gtk_widget_set_sensitive (update_configuration_button, enabled);
	}

	return;
}

static bool
validate_values (const std::string feed_name, const std::string feed_url, const std::string retrieve_limit_hrs,
                 const std::string retention_days)
{
	const bool values_valid = (feed_name.empty() == false && feed_url.empty() == false &&
	                           retrieve_limit_hrs.empty() == false && retention_days.empty() == false);

	return values_valid;
}

/*
	RSS Feed Retrieval
*/
static bool
check_feed_exist_by_url (feed_by_name_type feed_model,
                         const std::string feed_url)
{
	const size_t feed_count = feed_model.count (feed_url);

	return (feed_count > 0);
}

/*
	RSS Modification Operations
*/
static void
modify_feed (ns_data_read::rss_feed* feed, const std::string feed_name, const std::string feed_url,
             const std::string retrieve_limit_hrs, const std::string retention_days)
{
	if (feed) {
		feed->feed_name = feed_name;
		feed->feed_url = feed_url;
		feed->retrieve_limit_hrs = retrieve_limit_hrs;
		feed->retention_days = retention_days;
	}

	return;
}

static void
update_feed_config (const std::string feed_name, const std::string feed_url,
                    const std::string retrieve_limit_hrs, const std::string retention_days)
{
	const bool values_valid = validate_values (feed_name, feed_url, retrieve_limit_hrs, retention_days);

	if (values_valid) {
		std::string feed_name_original;
		std::string feed_url_original;

		ns_data_read::rss_feed feed_to_update;

		const int64_t id = feed_row_id;

		for (std::pair<std::string, ns_data_read::rss_feed> feed_entry : feed_model_original) {
			ns_data_read::rss_feed feed_item = feed_entry.second;

			const std::string existing_url = feed_item.feed_url;
			const int64_t existing_id = feed_item.row_id;

			const bool name_eq = (feed_item.feed_name == feed_name);
			const bool url_eq = (existing_url == feed_url);

			const bool feed_is_new = (id < 1);
			const bool id_same = (existing_id == id);

			/*This should ALWAYS be false; Added this check as a defensive safeguard.*/
			const bool feed_new_is_orig_id = (feed_is_new && id_same && existing_id > 0);

			if ((feed_new_is_orig_id == false) &&
			        (id_same || (feed_is_new && url_eq) || (feed_is_new && name_eq))) {
				feed_name_original = feed_item.feed_name;
				feed_url_original = feed_item.feed_url;

				ns_data_read::copy_feed (&feed_item, &feed_to_update);

				/*
					New feed entry but values match existing feed (in database).
					Keep the existing id to preserve related data.
				*/
				if (feed_is_new) {
					feed_row_id = id;
				}

				break;
			}
		}

		modify_feed (&feed_to_update, feed_name, feed_url, retrieve_limit_hrs, retention_days);

		std::vector<std::string> urls_to_invalidate;

		/*
			No duplicates allowed.

			Ultimately, this will keep the database clean.

			This will also keep the entire application *sane*.
			Any duplicate url entry can cause network problems as
			well as indirectly lead to too many website requests.

			No 2 entries can have the same feed name.
			No 2 entries can have the same feed url.

			The approach here is to simply clear the list of *any*
			matching entries. A later step will re-add the entry.
		*/
		for (std::pair<std::string, ns_data_read::rss_feed> feed_entry : feed_model_updated) {
			ns_data_read::rss_feed feed_item = feed_entry.second;

			const std::string existing_url = feed_item.feed_url;
			const int64_t existing_id = feed_item.row_id;

			const bool name_eq = (feed_item.feed_name == feed_name);
			const bool url_eq = (existing_url == feed_url);

			const bool feed_is_new = (id < 1);
			const bool item_is_orig = (existing_id > 0);
			const bool id_same = (existing_id == id);

			if (name_eq || url_eq) {
				/*
					Feed entry to remove. Indicated by url.
				*/
				urls_to_invalidate.push_back (existing_url);

				/*
					Preserve the row id.
				*/
				if (feed_is_new && item_is_orig) {
					int64_t preserved_rowid = existing_id;

					if (feed_is_new == false) {
						preserved_rowid = id;
					}

					feed_to_update.row_id = preserved_rowid;
				}
			}

			if (feed_is_new == false && id_same) {
				/*
					Feed entry to remove. Indicated by url.
					Repopulate the entry after modifications are made.
				*/
				urls_to_invalidate.push_back (existing_url);
			}
		}

		for (const std::string invalid_feed_url : urls_to_invalidate) {
			delete_feed (invalid_feed_url);
		}

		feed_model_updated.try_emplace (feed_url, ns_data_read::rss_feed());

		ns_data_read::rss_feed* feed_final = &feed_model_updated[feed_url];

		ns_data_read::copy_feed (&feed_to_update, feed_final);
	}

	return;
}

static void
delete_feed (const std::string feed_url)
{
	const bool value_valid = (feed_url.empty() == false);

	if (value_valid) {
		const bool feed_exists = check_feed_exist_by_url (feed_model_updated, feed_url);

		if (feed_exists) {
			feed_model_updated.erase (feed_url);
		}
	}

	return;
}
