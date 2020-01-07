/*
Copyright (C) 2019 Michael Gautier

This source code is free software; you can redistribute it and/or modify it under the terms of the GNU Lesser General Public License as published by the Free Software Foundation; either version 2.1 of the License, or (at your option) any later version.

This source code is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public License along with this library. If not, see <http://www.gnu.org/licenses/>.

Author: Michael Gautier <michaelgautier.wordpress.com>
*/

#include <iostream>
#include <queue>
#include <chrono>
#include <thread>

#include "rss_ui/application.hpp"
#include "rss_ui/app_win/app_win.hpp"
#include "rss_ui/app_win/article_header.hpp"
#include "rss_ui/app_win/article_frame.hpp"
#include "rss_ui/app_win/headlines_frame.hpp"

#include "rss_lib/rss/rss_reader.hpp"
#include "rss_lib/rss/rss_writer.hpp"
#include "rss_lib/rss/rss_feed_mod.hpp"

#include "rss_ui/rss_manage/rss_manage.hpp"

#include <webkit2/webkit2.h>

namespace ns_data_read = gautier_rss_data_read;
namespace ns_data_write = gautier_rss_data_write;

/*
	RSS Concurrent Modification
*/
static
bool rss_mod_running = true;

static
std::thread thread_rss_mod;

static void
process_rss_modifications();

static void
process_feeds();

static std::queue<ns_data_read::rss_feed_mod>
feed_changes;

static void
update_tab (ns_data_read::rss_feed_mod& modification);

static
void
download_feed (std::string& db_file_name,
               std::vector<std::pair<ns_data_read::rss_feed, ns_data_read::rss_feed>>& changed_feeds);

/*Do not set to true unless testing.*/
static
bool pre_download_pause_enabled = false;

/*
	Signal response functions.
*/
extern "C"
void
headline_view_switch_page (GtkNotebook* headlines_view,
                           GtkWidget*   page,
                           guint        page_num,
                           gpointer     user_data);

extern "C"
void
headline_view_select_row (GtkListBox*    box,
                          GtkListBoxRow* row,
                          gpointer       user_data);

extern "C"
void
connect_headline_list_box_select_row (GtkWidget* list_box);

extern "C"
void
rss_operation_click (GtkButton* button, gpointer user_data);

extern "C"
void
manage_feeds_click (GtkButton* button, gpointer user_data);

extern "C"
void
refresh_feed_click (GtkButton* button, gpointer user_data);

/*
	Operations.
*/
//C++ style enumeration.
enum class
rss_operation_enum
{
	view_article
};
/*
	Session data.
*/
static ns_data_read::rss_article
_feed_data;

/*
	Session UI data.
*/
static GtkWidget*
headlines_view = NULL;

static GtkWidget*
header_bar = NULL;

static GtkWidget*
article_date = NULL;

static GtkWidget*
article_summary = NULL;

static GtkWidget*
article_details = NULL;

static GtkWidget*
info_bar = NULL;

static GtkWidget*
view_article_button = NULL;

static GtkWidget*
manage_feeds_button = NULL;

static GtkWidget*
refresh_feed_button = NULL;

static int
headline_row_index = -1;

static int
monitor_width = 0;

static int
monitor_height = 0;

static int
monitor_x = 0;

static int
monitor_y = 0;

static int
window_width = 0;

static int
window_height = 0;

static rss_operation_enum
rss_op_view_article = rss_operation_enum::view_article;

static GtkWindow*
win = NULL;

static GtkWidget*
layout_pane = NULL;

static
bool shutting_down = false;
/*
	Application GUI Entry Point. The program starts here.
*/
extern "C"
void
window_size_allocate (GtkWidget* widget, GdkRectangle* allocation, gpointer user_data);

extern "C"
void
window_destroy (GtkWidget* window, gpointer user_data);

static void
get_screen_dimensions (GtkWindow* window);

static void
set_window_attributes (GtkWidget* window, std::string title, int width, int height);

static void
layout_rss_view (GtkWidget* layout_pane, GtkWidget* headlines_view, GtkWidget* article_frame);

static void
refresh_feeds (std::string feed_name, int headline_index_start);

static void
make_user_note (std::string note);

void
gautier_rss_win_main::create (
    GtkApplication* application, gpointer user_data)
{
	/*
		Window
	*/
	GtkWidget* window = gtk_application_window_new (application);

	g_signal_connect (window, "size-allocate", G_CALLBACK (window_size_allocate), NULL);
	g_signal_connect (window, "destroy", G_CALLBACK (window_destroy), NULL);

	win = GTK_WINDOW (window);
	get_screen_dimensions (win);
	set_window_attributes (window, gautier_rss_ui_app::get_application_name(), monitor_width, monitor_height);

	/*
		Header Bar
	*/
	header_bar = gtk_header_bar_new();
	gautier_rss_ui_app::set_css_class (header_bar, "header_bar");
	{
		namespace ns = gautier_rss_win_main_article_header;
		ns::initialize_article_header_bar (header_bar);
	}

	/*
		Article Summary
	*/
	article_summary = gtk_text_view_new();
	gautier_rss_ui_app::set_css_class (article_summary, "article_summary");
	{
		gtk_text_view_set_editable (GTK_TEXT_VIEW (article_summary), false);
		gtk_text_view_set_cursor_visible (GTK_TEXT_VIEW (article_summary), false);
		gtk_text_view_set_wrap_mode (GTK_TEXT_VIEW (article_summary), GTK_WRAP_WORD_CHAR);
		gtk_text_view_set_accepts_tab (GTK_TEXT_VIEW (article_summary), true);
	}

	/*
		Article Text
	*/
	{
		WebKitSettings* settings = webkit_settings_new();

		/*Do not want images, just HTML*/
		webkit_settings_set_auto_load_images (settings, false);

		/*The objective is to render HTML, but no JavaScript, local HTML databases, etc*/
		webkit_settings_set_enable_html5_database (settings, false);
		webkit_settings_set_enable_html5_local_storage (settings, false);
		webkit_settings_set_enable_java (settings, false);
		webkit_settings_set_enable_javascript (settings, false);
		webkit_settings_set_enable_offline_web_application_cache (settings, false);
		webkit_settings_set_enable_plugins (settings, false);
		webkit_settings_set_enable_webaudio (settings, false);
		webkit_settings_set_enable_webgl (settings, false);
		webkit_settings_set_enable_page_cache (settings, false);
		webkit_settings_set_enable_media_stream (settings, false);

		WebKitWebContext* web_context = webkit_web_context_new_ephemeral();
		article_details = webkit_web_view_new_with_context (web_context);
		webkit_web_view_set_settings (WEBKIT_WEB_VIEW (article_details), settings);
	}

	/*
		Article Date
	*/
	article_date = gtk_label_new (NULL);
	gautier_rss_ui_app::set_css_class (article_date, "article_date");

	gtk_widget_set_halign (article_date, GTK_ALIGN_END);

	/*
		Primary Functions
	*/
	GtkWidget* primary_function_buttons = gtk_button_box_new (GTK_ORIENTATION_HORIZONTAL);
	gautier_rss_ui_app::set_css_class (primary_function_buttons, "rss_main_button_container");
	gtk_button_box_set_layout (GTK_BUTTON_BOX (primary_function_buttons), GTK_BUTTONBOX_START);
	{
		/*
			View Article
		*/
		view_article_button = gtk_button_new_with_label ("View Article");
		gautier_rss_ui_app::set_css_class (view_article_button, "button");

		g_signal_connect (view_article_button, "clicked", G_CALLBACK (rss_operation_click), &rss_op_view_article);

		/*
			Manage Feeds
		*/
		manage_feeds_button = gtk_button_new_with_label ("Manage Feeds");
		gautier_rss_ui_app::set_css_class (manage_feeds_button, "button");

		g_signal_connect (manage_feeds_button, "clicked", G_CALLBACK (manage_feeds_click), NULL);

		/*
			Refesh Feed
		*/
		refresh_feed_button = gtk_button_new_with_label ("Refresh Feed");
		gautier_rss_ui_app::set_css_class (refresh_feed_button, "button");

		g_signal_connect (refresh_feed_button, "clicked", G_CALLBACK (refresh_feed_click), NULL);

		gtk_container_add (GTK_CONTAINER (primary_function_buttons), view_article_button);
		gtk_container_add (GTK_CONTAINER (primary_function_buttons), manage_feeds_button);
		gtk_container_add (GTK_CONTAINER (primary_function_buttons), refresh_feed_button);
	}

	/*
		Info Bar
	*/
	info_bar = gtk_label_new ("Status");
	gautier_rss_ui_app::set_css_class (info_bar, "feed_status");
	gtk_widget_set_halign (info_bar, GTK_ALIGN_START);

	/*
		RSS Headlines Tab
	*/
	headlines_view = gtk_notebook_new();
	{
		/*
			Tab page switch signal. Show news headlines for the chosen tab.
		*/
		g_signal_connect (headlines_view, "switch-page", G_CALLBACK (headline_view_switch_page), NULL);

		namespace ns = gautier_rss_win_main_headlines_frame;
		ns::initialize_headline_view (headlines_view, monitor_width, monitor_height);

		/*
			Make a tab for each feed name.
		*/
		std::vector<ns_data_read::rss_feed> feed_names;

		std::string db_file_name = gautier_rss_ui_app::get_db_file_name();

		ns_data_read::get_feed_names (db_file_name, feed_names);

		for (ns_data_read::rss_feed feed : feed_names) {
			std::string feed_name = feed.feed_name;

			ns::add_headline_page (headlines_view, feed_name, -1, connect_headline_list_box_select_row);

			/*
				RSS headlines.
			*/
			std::vector < std::string > headlines;

			gautier_rss_data_read::get_feed_headlines (db_file_name, feed_name, headlines);

			if (headlines.size() > 0) {
				int headline_index_start = 0;

				ns::show_headlines (headlines_view, feed_name, headline_index_start, headlines);
			}
		}

		if (feed_names.size() > 0) {
			ns_data_read::rss_feed feed = feed_names.front();

			std::string feed_name = feed.feed_name;

			namespace ns = gautier_rss_win_main_headlines_frame;

			ns::select_headline_row (GTK_WIDGET (headlines_view), feed_name, 0);
		}
	}
	/*
		Article Frame
	*/
	GtkWidget* article_frame = gtk_box_new (GTK_ORIENTATION_VERTICAL, 0);
	gautier_rss_ui_app::set_css_class (article_frame, "article_frame");
	{
		namespace ns = gautier_rss_win_main_article_frame;
		ns::initialize_article_frame (article_frame);
		ns::layout_article_frame (article_frame, header_bar, article_summary, article_details, article_date,
		                          primary_function_buttons,
		                          info_bar);
	}
	/*
		Window Layout
	*/
	layout_pane = gtk_paned_new (GTK_ORIENTATION_HORIZONTAL);
	{
		layout_rss_view (layout_pane, headlines_view, article_frame);

		gtk_container_add (GTK_CONTAINER (window), layout_pane);
	}

	gtk_widget_show_all (window);

	/*
		RSS Modifications
	*/
	thread_rss_mod = std::thread (process_rss_modifications);

	return;
}

/*
	Window and Screen
*/
void
get_screen_dimensions (GtkWindow* window)
{
	GdkScreen* screen = gtk_window_get_screen (window);
	GdkWindow* screen_window = gdk_screen_get_root_window (screen);
	GdkDisplay* display = gdk_screen_get_display (screen);
	GdkMonitor* monitor = gdk_display_get_monitor_at_window (display, screen_window);

	GdkRectangle geometry;

	gdk_monitor_get_geometry (monitor, &geometry);

	int scale_factor = gdk_window_get_scale_factor (screen_window);

	monitor_width = geometry.width * scale_factor;
	monitor_height = geometry.height * scale_factor;

	monitor_x = geometry.x;
	monitor_y = geometry.y;

	return;
}

void
set_window_attributes (GtkWidget* window, std::string title, int width, int height)
{
	gtk_window_set_title (GTK_WINDOW (window), title.data());
	gtk_window_set_default_size (GTK_WINDOW (window), width, height);
	gtk_window_set_resizable (GTK_WINDOW (window), true);

	return;
}

void
layout_rss_view (GtkWidget* layout_pane, GtkWidget* headlines_view, GtkWidget* article_frame)
{
	bool can_resize = true;
	bool can_shrink = true;

	/*
		Headlines Section
	*/
	gtk_paned_pack1 (GTK_PANED (layout_pane), headlines_view, can_resize, can_shrink);

	/*
		Article Section
	*/
	gtk_paned_pack2 (GTK_PANED (layout_pane), article_frame, can_resize, can_shrink);

	int
	default_width = gautier_rss_win_main_headlines_frame::get_default_headlines_view_content_width();

	gtk_paned_set_position (GTK_PANED (layout_pane), default_width);

	return;
}

/*
	RSS Headlines Tab

	Tab page switch signal. Show news headlines for the chosen tab.
*/
void
headline_view_switch_page (GtkNotebook* headlines_view,
                           GtkWidget*   content,
                           guint        page_num,
                           gpointer     user_data)
{
	/*
		Clear feed data.
	*/
	ns_data_read::clear_feed_data_all (_feed_data);

	/*
		Clear content.
	*/
	std::string date_status = "";

	gtk_label_set_text (GTK_LABEL (article_date), date_status.data());

	{
		GtkTextBuffer* text_buffer = gtk_text_view_get_buffer (GTK_TEXT_VIEW (article_summary));

		std::string article_summary = "";
		size_t article_summary_l = article_summary.size();

		gtk_text_buffer_set_text (text_buffer, article_summary.data(), article_summary_l);

		webkit_web_view_load_html (WEBKIT_WEB_VIEW (article_details), NULL, NULL);
	}
	{
		std::string url = "no feed data";

		gtk_widget_set_tooltip_text (view_article_button, url.data());
	}
	/*
		Setup tab.
	*/
	std::string feed_name = gtk_notebook_get_tab_label_text (headlines_view, content);
	_feed_data.feed_name = feed_name;

	gtk_header_bar_set_title (GTK_HEADER_BAR (header_bar), feed_name.data());

	std::string db_file_name = gautier_rss_ui_app::get_db_file_name();

	int article_count = ns_data_read::get_feed_headline_count (db_file_name, feed_name);

	make_user_note (std::to_string (article_count) + " articles since " +
	                ns_data_read::get_current_date_time_local());

	refresh_feeds (feed_name, article_count);

	return;
}

/*
	RSS Headlines List Box

	Choose a new headline.
*/
void
connect_headline_list_box_select_row (GtkWidget* list_box)
{
	g_signal_connect (list_box, "row-selected", G_CALLBACK (headline_view_select_row), NULL);

	return;
}

void
headline_view_select_row (GtkListBox*    list_box,
                          GtkListBoxRow* headline_row,
                          gpointer       user_data)
{
	headline_row_index = gtk_list_box_row_get_index (headline_row);

	/*
		Clear feed headline/article data.
	*/
	ns_data_read::clear_feed_data_keep_name (_feed_data);

	gautier_rss_win_main_headlines_frame::select_headline (_feed_data, headline_row);

	/*
		Article date.
	*/
	std::string date_status = "Published -- " + _feed_data.article_date;
	gtk_label_set_text (GTK_LABEL (article_date), date_status.data());

	/*
		Article summary.
	*/
	{
		GtkTextBuffer* text_buffer = gtk_text_view_get_buffer (GTK_TEXT_VIEW (article_summary));

		bool indicates_html = ns_data_read::indicates_html (_feed_data.article_summary);

		if (_feed_data.article_summary.empty() == false && indicates_html == false) {
			std::string article_summary = _feed_data.article_summary;
			size_t article_summary_l = article_summary.size();

			gtk_text_buffer_set_text (text_buffer, article_summary.data(), article_summary_l);
		} else {
			std::string article_summary = _feed_data.headline;
			size_t article_summary_l = article_summary.size();

			gtk_text_buffer_set_text (text_buffer, article_summary.data(), article_summary_l);
		}
	}

	/*
		Article text.
	*/
	{
		std::string article_text = _feed_data.article_text;

		if (article_text.empty()) {
			article_text = _feed_data.article_summary;
		}

		webkit_web_view_load_html (WEBKIT_WEB_VIEW (article_details), article_text.data(), NULL);
	}

	gtk_widget_set_tooltip_text (view_article_button, _feed_data.url.data());
	gtk_widget_set_tooltip_text (GTK_WIDGET (headline_row), _feed_data.url.data());

	return;
}

void
rss_operation_click (GtkButton* button,
                     gpointer   user_data)
{
	rss_operation_enum operation = * (rss_operation_enum*)user_data;

	switch (operation) {
		case rss_operation_enum::view_article: {
				gtk_show_uri_on_window (win, _feed_data.url.data(), GDK_CURRENT_TIME, NULL);
			}
			break;
	}

	return;
}

void
manage_feeds_click (GtkButton* button,
                    gpointer   user_data)
{
	gautier_rss_win_rss_manage::set_modification_queue (&feed_changes);
	gautier_rss_win_rss_manage::show_dialog (NULL, win, window_width, window_height);

	return;
}

void
refresh_feed_click (GtkButton* button,
                    gpointer   user_data)
{
	refresh_feeds (_feed_data.feed_name, 0);

	return;
}

void
window_size_allocate (GtkWidget* widget, GdkRectangle* allocation, gpointer user_data)
{
	window_width = 0;
	window_height = 0;

	/*
		GtkAllocation is a typedef for GdkRectangle
		The rectangle struct is defined as

			int x,y,width,height;
	*/
	if (allocation) {
		window_width = allocation->width;
		window_height = allocation->height;
	}

	return;
}

void
window_destroy (GtkWidget* window, gpointer user_data)
{
	shutting_down = true;

	thread_rss_mod.join();

	return;
}

static void
process_rss_modifications()
{
	while (shutting_down == false && rss_mod_running == true) {
		rss_mod_running = false;

		std::this_thread::sleep_for (std::chrono::milliseconds (777));

		//Keep this part until the code is finished. It gives us a reliable indicator that the background thread is still in operation.
		std::string datetime = ns_data_read::get_current_date_time_utc();

		std::cout << "RSS configuration update: \t" << datetime << "\n";

		int change_count = feed_changes.size();

		if (change_count > 0) {
			make_user_note (std::to_string (change_count) + " changes pending.");

			ns_data_read::rss_feed_mod modification = feed_changes.front();

			ns_data_read::rss_feed_mod_status status = modification.status;

			if (status != ns_data_read::rss_feed_mod_status::none) {
				update_tab (modification);
			}
		} else {
			process_feeds();
		}

		rss_mod_running = true;
	}

	return;
}

static void
update_tab (ns_data_read::rss_feed_mod& modification)
{
	ns_data_read::rss_feed_mod_status status = modification.status;

	std::string feed_name = modification.feed_name;
	int row_id_now = modification.row_id;

	bool is_insert = status == ns_data_read::rss_feed_mod_status::insert;

	if (is_insert == false) {
		gint page_count = gtk_notebook_get_n_pages (GTK_NOTEBOOK (headlines_view));

		GtkWidget* tab = NULL;
		std::string tab_label;
		int tab_n = -1;

		for (int tab_i = 0; tab_i < page_count; tab_i++) {
			tab = gtk_notebook_get_nth_page (GTK_NOTEBOOK (headlines_view), tab_i);

			if (tab == NULL) {
				continue;
			}

			const gchar* tab_text = gtk_notebook_get_tab_label_text (GTK_NOTEBOOK (headlines_view), tab);

			tab_label = tab_text;

			if (feed_name == tab_label) {
				tab_n = tab_i;
				break;
			} else {
				tab = NULL;
			}
		}

		if (tab_n > -1 && tab != NULL && tab_label.empty() == false && tab_label == feed_name) {
			switch (status) {
				case ns_data_read::rss_feed_mod_status::remove: {
						gtk_widget_hide (tab);

						make_user_note (feed_name + " DELETED.");

						gtk_notebook_remove_page (GTK_NOTEBOOK (headlines_view), tab_n);

						feed_changes.pop();
					}
					break;

				case ns_data_read::rss_feed_mod_status::change: {
						std::string row_id = std::to_string (row_id_now);

						ns_data_read::rss_feed updated_feed;

						/*
							Use database snapshot to determine what has changed.
						*/
						std::string db_file_name = gautier_rss_ui_app::get_db_file_name();

						ns_data_read::get_feed_by_row_id (db_file_name, row_id, updated_feed);

						std::string updated_feed_name = updated_feed.feed_name;

						if (updated_feed_name != feed_name) {
							gtk_notebook_set_tab_label_text (GTK_NOTEBOOK (headlines_view), tab, updated_feed_name.data());
							/*Only changing name -- other changes will be cached to another queue and processed as appropriate.*/
							make_user_note (feed_name + " updated to " + updated_feed_name + ".");

						}

						feed_changes.pop();
					}
					break;
			}
		}
	} else if (is_insert == true) {
		namespace ns = gautier_rss_win_main_headlines_frame;

		int page_count = gtk_notebook_get_n_pages (GTK_NOTEBOOK (headlines_view));

		ns::add_headline_page (headlines_view, feed_name, page_count + 1, connect_headline_list_box_select_row);

		page_count = gtk_notebook_get_n_pages (GTK_NOTEBOOK (headlines_view));

		for (int tab_i = 0; tab_i < page_count; tab_i++) {
			GtkWidget* tab = gtk_notebook_get_nth_page (GTK_NOTEBOOK (headlines_view), tab_i);

			if (tab == NULL) {
				continue;
			}

			std::string tab_label = gtk_notebook_get_tab_label_text (GTK_NOTEBOOK (headlines_view), tab);

			if (feed_name == tab_label) {
				gtk_widget_show_all (tab);

				break;
			}
		}

		feed_changes.pop();
	}

	return;
}

static void
process_feeds()
{
	//Keep this part until the code is finished. It gives us a reliable indicator that the background thread is still in operation.
	std::string datetime = ns_data_read::get_current_date_time_utc();

	std::string db_file_name = gautier_rss_ui_app::get_db_file_name();

	int pause_interval_in_seconds = 0;

	if (pre_download_pause_enabled) {
		/*
			Pause before download.
			-----------------------------------------------------------------
			Details:	d37564a59e58a324e0e02d03d76b4166c4120ed4
			Command:	git show d37564a59e58a324e0e02d03d76b4166c4120ed4
		*/

		pause_interval_in_seconds = 8;
	}

	std::vector<std::pair<ns_data_read::rss_feed, ns_data_read::rss_feed>> changed_feeds;

	ns_data_write::download_feeds (db_file_name, pause_interval_in_seconds, changed_feeds);

	/*
		Feeds with new data.
	*/
	bool changed = false;

	for (std::pair<ns_data_read::rss_feed, ns_data_read::rss_feed> feed_pair : changed_feeds) {
		ns_data_read::rss_feed feed_old = feed_pair.first;
		ns_data_read::rss_feed feed_new = feed_pair.second;

		std::string feed_name = feed_new.feed_name;
		int article_count = feed_old.article_count;

		int headline_count = ns_data_read::get_feed_headline_count (db_file_name, feed_name);

		if (changed == false) {
			changed = true;
		}

		if (headline_count > article_count) {
			int new_article_count = (headline_count - article_count);

			make_user_note (std::to_string (new_article_count) + " headline additions pending.");

			refresh_feeds (feed_name, article_count);

			make_user_note (feed_name + " has new articles.");
		}
	}

	if (changed == true) {
		gtk_widget_show_all (GTK_WIDGET (win));
	}

	return;
}

static void
refresh_feeds (std::string feed_name, int headline_index_start)
{
	int previous_headline_row_index = headline_row_index;

	namespace ns = gautier_rss_win_main_headlines_frame;

	std::string db_file_name = gautier_rss_ui_app::get_db_file_name();

	std::vector < std::string > headlines;

	gautier_rss_data_read::get_feed_headlines (db_file_name, feed_name, headlines);

	if (headlines.size() > 0) {
		ns::show_headlines (headlines_view, feed_name, headline_index_start, headlines);

		if (previous_headline_row_index > -1) {
			ns::select_headline_row (headlines_view, feed_name, previous_headline_row_index);
		}
	}

	int headline_count = ns_data_read::get_feed_headline_count (db_file_name, feed_name);

	make_user_note (std::to_string (headline_count) + " articles since " +
	                ns_data_read::get_current_date_time_local());

	return;
}

static void
make_user_note (std::string note)
{
	gtk_label_set_text (GTK_LABEL (info_bar), note.data());

	return;
}
