/*
Copyright (C) 2020 Michael Gautier

This source code is free software; you can redistribute it and/or modify it under the terms of the GNU Lesser General Public License as published by the Free Software Foundation; either version 2.1 of the License, or (at your option) any later version.

This source code is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public License along with this library. If not, see <http://www.gnu.org/licenses/>.

Author: Michael Gautier <michaelgautier.wordpress.com>
*/

#include <iostream>
#include <queue>
#include <thread>
#include <unordered_map>

#include "rss_ui/application.hpp"
#include "rss_ui/app_win/app_win.hpp"
#include "rss_ui/app_win/article_header.hpp"
#include "rss_ui/app_win/article_frame.hpp"
#include "rss_ui/app_win/headlines_frame.hpp"

#include "rss_lib/rss/rss_reader.hpp"
#include "rss_lib/rss/rss_writer.hpp"
#include "rss_lib/rss/rss_feed_mod.hpp"
#include "rss_lib/rss/rss_util.hpp"

#include "rss_ui/rss_manage/rss_manage.hpp"

#include "rss_lib/rss_download/feed_download.hpp"

#include <webkit2/webkit2.h>

namespace ns_data_read = gautier_rss_data_read;
namespace ns_data_write = gautier_rss_data_write;

/*
	RSS Concurrent Modification
*/
static
std::unordered_map<std::string, ns_data_read::rss_feed>
feed_index;

static
std::unordered_map<std::string, std::vector<std::string>>
        feeds_articles;

/*Start-up	Load first n lines*/
static
guint
notebook_concurrent_init_interval_milliseconds = 120;

extern "C"
gboolean
notebook_concurrent_init (gpointer data);

static
gint
notebook_concurrent_init_id = -1;

static
gint
next_notebook_tab_index = -1;

/*Load new lines either following start-up or on database refresh (download)*/
static
guint
headlines_list_refresh_interval_milliseconds = 40;

extern "C"
gboolean
headlines_list_refresh (gpointer data);

static
gint
headlines_list_refresh_id = -1;

/*Feed download checks	Most of this might disappear.*/
static int
data_download_thread_wait_in_seconds = 6;

static
std::thread thread_download_data;

static void
initialize_data_threads();

static void
download_data_noop();

static void
download_data();

static std::queue<ns_data_read::rss_feed_mod>
feed_changes;

static
bool shutting_down = false;

static
bool download_running = false;

/*Do not set to true unless testing.*/
static
bool pre_download_pause_enabled = false;

static
bool feed_expire_time_enabled = false;

/*May evolve into a compile flag #define DIAG*/
static
bool diagnostics_enabled = false;

/*
	Signal response functions.
*/
gulong headline_view_switch_page_signal_id = -1;

extern "C"
void
headline_view_switch_page (GtkNotebook* headlines_view,
                           GtkWidget*   page,
                           guint        page_num,
                           gpointer     user_data);

extern "C"
void
headline_view_select_row (GtkTreeSelection* tree_selection, gpointer user_data);

extern "C"
void
rss_operation_click (GtkButton* button, gpointer user_data);

extern "C"
void
manage_feeds_click (GtkButton* button, gpointer user_data);

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

/*
	Application GUI Entry Point. The program starts here.
*/
extern "C"
void
window_size_allocate (GtkWidget* widget, GdkRectangle* allocation, gpointer user_data);

extern "C"
void
window_destroy (GtkWidget* window, gpointer user_data);

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
populate_rss_tabs();

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
	g_object_ref_sink (header_bar);
	gautier_rss_ui_app::set_css_class (header_bar, "header_bar");
	{
		namespace ns = gautier_rss_win_main_article_header;
		ns::initialize_article_header_bar (header_bar);
	}

	/*
		Article Summary
	*/
	article_summary = gtk_text_view_new();
	g_object_ref_sink (article_summary);
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
		g_object_ref_sink (article_details);
		webkit_web_view_set_settings (WEBKIT_WEB_VIEW (article_details), settings);
	}

	/*
		Article Date
	*/
	article_date = gtk_label_new (NULL);
	g_object_ref_sink (article_date);
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
		gtk_widget_set_sensitive (manage_feeds_button, false);

		gautier_rss_ui_app::set_css_class (manage_feeds_button, "button");

		g_signal_connect (manage_feeds_button, "clicked", G_CALLBACK (manage_feeds_click), NULL);

		gtk_container_add (GTK_CONTAINER (primary_function_buttons), view_article_button);
		gtk_container_add (GTK_CONTAINER (primary_function_buttons), manage_feeds_button);
	}

	/*
		Info Bar
	*/
	info_bar = gtk_label_new ("Status");
	g_object_ref_sink (info_bar);
	gautier_rss_ui_app::set_css_class (info_bar, "feed_status");
	gtk_widget_set_halign (info_bar, GTK_ALIGN_START);

	/*
		RSS Headlines Tab
	*/
	headlines_view = gtk_notebook_new();
	g_object_ref_sink (headlines_view);
	{
		gautier_rss_win_main_headlines_frame::initialize_headline_view (headlines_view, monitor_width, monitor_height);

		populate_rss_tabs();

		int tab_count = gtk_notebook_get_n_pages (GTK_NOTEBOOK (headlines_view));

		if (tab_count > 0) {
			next_notebook_tab_index = 0;
		}

		/*Approximate ideal "perceptual" rate of load per tab based on max 100 items per tab.*/
		notebook_concurrent_init_interval_milliseconds = ((tab_count + 2) * 100) / 10;

		notebook_concurrent_init_id = gdk_threads_add_timeout (notebook_concurrent_init_interval_milliseconds,
		                              notebook_concurrent_init, headlines_view);
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
	g_object_ref_sink (layout_pane);
	{
		layout_rss_view (layout_pane, headlines_view, article_frame);

		gtk_container_add (GTK_CONTAINER (window), layout_pane);
	}

	gtk_widget_show_all (window);

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

		webkit_web_view_load_plain_text (WEBKIT_WEB_VIEW (article_details), article_summary.data());
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

	if (feed_name.empty() == false) {
		gautier_rss_win_main_headlines_frame::select_headline_row (GTK_WIDGET (headlines_view), feed_name, 0);
	}

	gtk_header_bar_set_title (GTK_HEADER_BAR (header_bar), feed_name.data());

	ns_data_read::rss_feed* feed = &feed_index[feed_name];

	if (feed) {
		const int64_t headline_count = feed->article_count;

		make_user_note (std::to_string (headline_count) + " articles since " +
		                gautier_rss_util::get_current_date_time_local());
	}

	return;
}

/*
	RSS Headlines List Box

	Choose a new headline.
*/
void
headline_view_select_row (GtkTreeSelection* tree_selection, gpointer user_data)
{
	if (tree_selection) {
		/*
			Clear feed headline/article data.
		*/
		ns_data_read::clear_feed_data_keep_name (_feed_data);

		gautier_rss_win_main_headlines_frame::select_headline (_feed_data, tree_selection);

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

			if (article_text.empty() == false) {
				webkit_web_view_load_html (WEBKIT_WEB_VIEW (article_details), article_text.data(), NULL);
			} else {
				webkit_web_view_load_plain_text (WEBKIT_WEB_VIEW (article_details), article_text.data());
			}
		}

		gtk_widget_set_tooltip_text (view_article_button, _feed_data.url.data());
	}

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
	/**
	 * g_signal_handlers_disconnect_by_func:
	 * @instance: The instance to remove handlers from.
	 * @func: The C closure callback of the handlers (useless for non-C closures).
	 * @data: The closure data of the handlers' closures.
	 *
	 * Disconnects all handlers on an instance that match @func and @data.
	 *
	 * Returns: The number of handlers that matched.
	 */
	g_signal_handler_disconnect (headlines_view, headline_view_switch_page_signal_id);

	gautier_rss_win_rss_manage::set_modification_queue (&feed_changes);
	gautier_rss_win_rss_manage::show_dialog (NULL, win, window_width, window_height);

	headline_view_switch_page_signal_id = g_signal_connect (headlines_view, "switch-page",
	                                      G_CALLBACK (headline_view_switch_page), NULL);

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
	download_running = false;

	thread_download_data.join();

	/*
		These use GObject explicit ref counting.
		Ref counting enables explicit clean-up sequence.
	*/
	gtk_widget_destroy (article_date);
	gtk_widget_destroy (article_details);
	gtk_widget_destroy (article_summary);
	gtk_widget_destroy (header_bar);
	gtk_widget_destroy (info_bar);
	gtk_widget_destroy (manage_feeds_button);
	gtk_widget_destroy (view_article_button);
	gtk_widget_destroy (headlines_view);
	gtk_widget_destroy (layout_pane);

	return;
}

static void
populate_rss_tabs()
{
	namespace ns = gautier_rss_win_main_headlines_frame;

	/*
		Make a tab for each feed name.
	*/
	std::vector<ns_data_read::rss_feed> feed_names;

	std::string db_file_name = gautier_rss_ui_app::get_db_file_name();

	ns_data_read::get_feeds (db_file_name, feed_names);

	for (ns_data_read::rss_feed feed : feed_names) {
		std::string feed_name = feed.feed_name;

		feed_index.insert_or_assign (feed_name, feed);

		ns_data_read::rss_feed* feed_clone = &feed_index[feed_name];
		feed_clone->revised_index_start = -1;
		feed_clone->revised_index_end = -1;
		feed_clone->last_index = 0;

		feeds_articles.insert_or_assign (feed_name, std::vector<std::string>());

		ns::add_headline_page (headlines_view, feed_name, -1, headline_view_select_row);
	}

	return;
}

gboolean
notebook_concurrent_init (gpointer data)
{
	namespace ns = gautier_rss_win_main_headlines_frame;

	gboolean still_active = false;

	int tab_count = gtk_notebook_get_n_pages (GTK_NOTEBOOK (headlines_view));

	if (tab_count > 0) {
		gint tab_i = next_notebook_tab_index;

		if (tab_i > -1) {
			GtkWidget* tab = gtk_notebook_get_nth_page (GTK_NOTEBOOK (headlines_view), tab_i);

			if (tab) {
				std::string feed_name = gtk_notebook_get_tab_label_text (GTK_NOTEBOOK (headlines_view), tab);

				std::string db_file_name = gautier_rss_ui_app::get_db_file_name();

				/*
					RSS headlines.
				*/
				std::vector < std::string > headlines;
				{
					const int64_t max_lines = 32;

					std::vector < std::string > all_headlines;

					ns_data_read::get_feed_headlines (db_file_name, feed_name, all_headlines);

					feeds_articles[feed_name] = all_headlines;

					ns_data_read::rss_feed* feed_index_entry = &feed_index[feed_name];

					int64_t last_index = 0;

					for (int64_t i = 0; i < max_lines && i < all_headlines.size(); i++) {
						std::string headline = all_headlines.at (i);

						headlines.push_back (headline);

						last_index = i;
					}

					feed_index_entry->last_index = last_index;
				}

				if (headlines.size() > 0) {
					int64_t headline_index_start = 0;

					ns::show_headlines (headlines_view, feed_name, headline_index_start, headlines.size(), headlines);
				}

				next_notebook_tab_index++;

				/*
					CONCURRENT BRANCH #1
					Present the first few lines from the first feed listed in the database.
				*/
				if (tab_i == 0) {
					gboolean sensitive_value = gtk_widget_get_sensitive (tab);

					if (sensitive_value == false) {
						gtk_widget_set_sensitive (tab, true);
					}

					ns::select_headline_row (GTK_WIDGET (headlines_view), feed_name, 0);

					/*
						CONCURRENT BRANCH #2
						Sets up a recursive call to notebook_concurrent_init

						First pass: notebook_concurrent_init has still_active == false
						Terminating the thread of execution frees up the UI to show a screen of information.
						However, there is more information to show (other tabs).

						The process repeats by establishing a new call to notebook_concurrent_init
						where still_active == true until all remaining tabs have been processed.

						Those tabs may be locked for the duration of their processing. The critical method
						is to feed enough data into each tab on each cycle in a short a time possible
						while preserving the end-user's ability to click through the tabs while they are
						still loading.
					*/
					notebook_concurrent_init_id = gdk_threads_add_timeout (notebook_concurrent_init_interval_milliseconds,
					                              notebook_concurrent_init, headlines_view);

					/*
						CONCURRENT BRANCH #4

						The #1 goal of a UI is to show that UI first, everything else is secondary.
						After the UI is established (conclusion of CONCURRENT BRANCH #1), only then should subsequent actions can be initiated.

						This call will begin the process of acquiring data and setting the conditions for showing new data in the UI.
						The threads initiated by this will not modify the UI, these threads will manage the data only.
					*/
					initialize_data_threads();
				}
				/*
					CONCURRENT BRANCH #2
					Terminate the thread on the final tab in process.
				*/
				else {
					still_active = (next_notebook_tab_index < tab_count);
				}
			}
		}
	}

	/*
		Applies only to CONCURRENT BRANCH #2
	*/
	if (still_active == false && next_notebook_tab_index >= tab_count) {
		for (int tab_i = 1; tab_i < tab_count; tab_i++) {
			GtkWidget* tab = gtk_notebook_get_nth_page (GTK_NOTEBOOK (headlines_view), tab_i);

			if (tab) {
				gboolean sensitive_value = gtk_widget_get_sensitive (tab);

				if (sensitive_value == false) {
					gtk_widget_set_sensitive (tab, true);
				}

				gtk_widget_show_all (tab);
			}
		}

		/*
			Tab page switch signal. Show news headlines for the chosen tab.
		*/
		headline_view_switch_page_signal_id = g_signal_connect (headlines_view, "switch-page",
		                                      G_CALLBACK (headline_view_switch_page), NULL);

		gtk_widget_set_sensitive (manage_feeds_button, true);

		/*
			CONCURRENT BRANCH #3
			Initiate the process of populating remaining data from database.
		*/
		next_notebook_tab_index = 0;

		headlines_list_refresh_id = gdk_threads_add_timeout (headlines_list_refresh_interval_milliseconds,
		                            headlines_list_refresh, NULL);

		/*
			CONCURRENT BRANCH #4		Depends on CONCURRENT BRANCH #1 initiating CONCURRENT BRANCH #4
			Provide signal for active data download thread to begin contacting servers.
		*/
		data_download_thread_wait_in_seconds = 4;
		download_running = true;
	}

	return still_active;
}

gboolean
headlines_list_refresh (gpointer data)
{
	namespace ns = gautier_rss_win_main_headlines_frame;

	gboolean still_active = (shutting_down == false);

	int tab_count = gtk_notebook_get_n_pages (GTK_NOTEBOOK (headlines_view));

	if (still_active && tab_count > 0) {
		gint tab_i = next_notebook_tab_index++;

		if (next_notebook_tab_index > tab_count) {
			next_notebook_tab_index = 0;
		}

		if (tab_i > -1) {
			GtkWidget* tab = gtk_notebook_get_nth_page (GTK_NOTEBOOK (headlines_view), tab_i);

			if (tab) {
				std::string feed_name = gtk_notebook_get_tab_label_text (GTK_NOTEBOOK (headlines_view), tab);

				ns_data_read::rss_feed* feed_index_entry = &feed_index[feed_name];

				const int64_t max_lines = 32;

				const int64_t headline_count = feed_index_entry->article_count;

				const int64_t index_start = (feed_index_entry->last_index) + 1;

				if (headline_count > 0 && index_start > -1 && index_start < headline_count) {
					std::vector < std::string > all_headlines = feeds_articles[feed_name];

					int64_t index_end = index_start + max_lines;

					if (index_end > headline_count) {
						index_end = all_headlines.size() - 1;
					}

					feed_index_entry->last_index = index_end;

					ns::show_headlines (headlines_view, feed_name, index_start, index_end, all_headlines);

					gtk_widget_show_all (tab);
				}
			}
		}
	}

	return still_active;
}

static void
make_user_note (std::string note)
{
	gtk_label_set_text (GTK_LABEL (info_bar), note.data());

	return;
}

/*
	Database threads - NO UI
*/
static void
initialize_data_threads()
{
	thread_download_data = std::thread (download_data);

	return;
}

/*
	RSS DATA INTEGRATION - The **PRIMARY** RSS function.
	Designed to run in a separate thread. All it does is update a database.
	The user interface is expected to have its own logic to detect database
	modifications that occur when function executes.

	This function does not interact with the user interface.
*/
static void
download_data()
{
	/*
		KEEP ALL the std::cout << calls. They are an intentional part of
		this function. That way, when the program is initiated from a
		command-line, the primary output to stdout is network/database
		processes that occur in this function.
	*/

	std::string db_file_name = gautier_rss_ui_app::get_db_file_name();

	while (shutting_down == false && download_running == false) {
		std::cout << __func__ << ":\tentering sleep\n";
		std::this_thread::sleep_for (std::chrono::seconds (data_download_thread_wait_in_seconds));
	}

	std::cout << __func__ << ":\tawake\n";

	int successful_download_attempts = 0;
	int failed_download_attempts = 0;
	const int max_failed_download_attempts = 9;
	bool failed_download_notify_was_output = false;
	int change_count = 0;
	std::string last_download_datetime = gautier_rss_util::get_current_date_time_utc();
	std::string last_failed_download_datetime;

	/*
		Execution is signaled by download_running == true
		When that condition changes, exit is defined in 3 key areas.
		If a download is in progress, that download will conclude but no other will initiate
		when the end-user signals the end of the application.
	*/
	int allow_process_output = true;

	data_download_thread_wait_in_seconds = 2;

	while (shutting_down == false && download_running) {
		if (allow_process_output) {
			std::cout << __func__ << ":\tPreparing to download\n";
		}

		std::this_thread::sleep_for (std::chrono::seconds (data_download_thread_wait_in_seconds));

		/*
			End this thread if the user closes the program.
		*/
		if (shutting_down || download_running == false) {
			std::cout << __func__ << ", LINE: " << __LINE__ << ";\tEXIT\n";
			break;
		}

		/*
			If downloads have already occurred within a hour window of time,
			stop further download processing until 12 minutes has passed.
		*/
		if (successful_download_attempts > 0) {
			std::string prep_download_datetime = gautier_rss_util::get_current_date_time_utc();

			int_fast32_t time_difference_in_seconds = gautier_rss_util::get_time_difference_in_seconds (
			            last_download_datetime, prep_download_datetime);

			bool allow_time_output = (time_difference_in_seconds < 10 || time_difference_in_seconds > 718);

			if (allow_time_output) {
				std::cout << __func__ << ", LINE: " << __LINE__ << ";\t\tTime check\n";

				std::cout << __func__ << ", LINE: " << __LINE__ << ";\t\t\tSeconds since last successful download: " <<
				          time_difference_in_seconds << "\n";
			}

			/*
				Any return to the loop entrance puts this just past 720s (12 minutes) once the time reaches 12 minutess.

				This is the *Guarantee that the application will not access the network too frequently but will achieve
				also check for data often enough in the event there is new data based on the feed's last retrieved date.

				Website operators tend to dislike programs that check the website every few seconds. Setting this
				to an hour guarantees that the program will not unintentionally violate minimum website access intervals.
			*/
			bool clock_still_running = (time_difference_in_seconds < 722);

			if (clock_still_running) {
				if (allow_time_output) {
					std::cout << __func__ << ", LINE: " << __LINE__ << ";\t\t\t\tSKIP until 720s have passed.\n";
				}

				continue;
			} else {
				allow_process_output = true;
			}
		}

		if (failed_download_attempts >= max_failed_download_attempts) {
			const int download_retry_wait_in_minutes = 5;
			int wait_time_in_seconds = (download_retry_wait_in_minutes * 60);

			std::string download_review_datetime = gautier_rss_util::get_current_date_time_utc();

			int_fast32_t time_difference_in_seconds = gautier_rss_util::get_time_difference_in_seconds (
			            last_failed_download_datetime, download_review_datetime);

			if (failed_download_notify_was_output == false) {
				std::cout << __func__ << ", LINE: " << __LINE__ << ";\tSeveral failed downloads (unable to connect).\n";
				std::cout << __func__ << ", LINE: " << __LINE__ << ";\t\tWill try again in " << download_retry_wait_in_minutes
				          << " minutes.\n";
			}

			bool clock_still_running = (time_difference_in_seconds < (wait_time_in_seconds + 2));

			if (clock_still_running) {
				if (failed_download_notify_was_output == false) {
					failed_download_notify_was_output = true;

					std::cout << __func__ << ", LINE: " << __LINE__ << ";\t\t\t\tSKIP until " << wait_time_in_seconds <<
					          "s have passed.\n";
				}

				continue;
			} else {
				allow_process_output = true;
				failed_download_attempts = 0;
			}
		}

		/*
			Feeds with new data.
		*/
		change_count = 0;
		successful_download_attempts = 0;

		std::vector <ns_data_read::rss_feed> feeds;

		ns_data_read::get_feeds (db_file_name, feeds);

		/*
			Cycles through all the feeds to download, 1 at a time.
		*/
		for (ns_data_read::rss_feed feed : feeds) {
			/*
				End this thread if the user closes the program.
			*/
			if (shutting_down || download_running == false) {
				std::cout << __func__ << ", LINE: " << __LINE__ << ";\tEXIT\n";
				break;
			}

			std::string feed_name = feed.feed_name;
			std::string feed_url = feed.feed_url;
			std::string retrieve_limit_hrs = feed.retrieve_limit_hrs;
			std::string retention_days = feed.retention_days;

			std::cout << __func__ << ", LINE: " << __LINE__ << ";\tFEED: " << feed_name << "\n";

			/*
				Aborts the download attempt if a download has already occured within the allowed time frame.
			*/
			bool is_feed_still_fresh = gautier_rss_data_read::is_feed_still_fresh (db_file_name, feed_name,
			                           feed_expire_time_enabled);

			/*
				Handles the situation where the program is relaunched within the normal minimum website access time frame.
				In that case, use the most recent retrieve date stored in the feed's record.
			*/
			if (is_feed_still_fresh) {
				last_download_datetime = feed.last_retrieved;
				successful_download_attempts++;
			}

			if (is_feed_still_fresh == false && (shutting_down == false && download_running)) {
				/*
					The download of a given feed will be retried a few times.
				*/
				const int max_download_attempts = 3;
				int download_attempts = 0;

				bool network_response_good = false;

				while (network_response_good == false && download_attempts < max_download_attempts) {
					download_attempts++;

					long response_code = ns_data_write::update_rss_db_from_network (db_file_name, feed_name, feed_url,
					                     retrieve_limit_hrs,
					                     retention_days);

					bool network_response_good = gautier_rss_data_read::is_network_response_ok (response_code);

					if (network_response_good) {
						successful_download_attempts++;

						std::cout << __func__ << ", LINE: " << __LINE__ << ";\tDOWNLOAD OK!\n";

						/*Although there is control logic in the loop, go ahead in this case and exit early.*/
						break;
					} else {
						if (download_attempts == max_download_attempts) {
							failed_download_attempts++;

							if (failed_download_attempts >= max_failed_download_attempts) {
								last_failed_download_datetime = gautier_rss_util::get_current_date_time_utc();

								allow_process_output = false;
								failed_download_notify_was_output = false;
							}

							std::cout << __func__ << ", LINE: " << __LINE__ << ";\tFAILED DOWNLOAD!!!!\n";
						}

						/*Skip this iteration since no data is expected*/
						continue;
					}
				}

				/*
					Determine the range of rowids to use to update the user interface.

					A 'UI thread valid for updating the UI' will pick up these values.
				*/
				ns_data_read::rss_feed feed_new;

				ns_data_read::get_feed (db_file_name, feed_name, feed_new);

				std::cout << __func__ << ", LINE: " << __LINE__ << ";\tCHECK FEED\n";
				bool new_updates = ns_data_read::check_feed_changed (feed, feed_new);

				if (new_updates) {
					change_count++;
					std::cout << __func__ << ", LINE: " << __LINE__ << ";\tChange count: " << change_count << "\n";

					ns_data_read::rss_feed* feed_in_use = &feed_index[feed_name];

					if (feed_in_use) {
						const int64_t in_use_count = feed_in_use->article_count;
						const int64_t record_count = ns_data_read::get_feed_headline_count (db_file_name, feed_name);

						/*
							CONCURRENT BRANCH - Mark Downloaded Data
							See: CONCURRENT BRANCK - Use Downloaded Data
						*/
						if (record_count > in_use_count) {
							int diff_count = (record_count - in_use_count);

							const int64_t feed_index_start = in_use_count - 1;
							const int64_t feed_index_end = feed_index_start + diff_count;

							/*
								Signals to the UI that new records are available.
								UI Checks for: end > start
							*/
							feed_in_use->revised_index_start = feed_index_start;
							feed_in_use->revised_index_end = feed_index_end;
						}
					}
				}
			}
		}

		/*
			A signal to this thread that no more downloads should be attempted for a while.
		*/
		if (successful_download_attempts > 0) {
			last_download_datetime = gautier_rss_util::get_current_date_time_utc();

			std::cout << __func__ << ", LINE: " << __LINE__ << ";\tdate/time last successful download:\t" <<
			          last_download_datetime << "\n";

			allow_process_output = false;
		}
	}

	return;
}
