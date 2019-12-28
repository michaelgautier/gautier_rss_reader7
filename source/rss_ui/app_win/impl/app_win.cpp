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
#include "rss_lib/rss/rss_feed_mod.hpp"

#include "rss_ui/rss_manage/rss_manage.hpp"

#include <webkit2/webkit2.h>

/*
	RSS Concurrent Modification
*/
static
bool rss_mod_running = true;

static
std::thread thread_rss_mod;

static void
process_rss_modifications();

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
static gautier_rss_data_read::rss_article
_feed_data;

static std::queue<gautier_rss_data_read::rss_feed_mod>
feed_changes;

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
	{
		namespace ns = gautier_rss_win_main_article_header;
		ns::initialize_article_header_bar (header_bar);
	}

	/*
		Article Summary
	*/
	article_summary = gtk_text_view_new();
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

	/*
		Primary Functions
	*/
	GtkWidget* primary_function_buttons = gtk_button_box_new (GTK_ORIENTATION_HORIZONTAL);
	gtk_button_box_set_layout (GTK_BUTTON_BOX (primary_function_buttons), GTK_BUTTONBOX_START);
	{
		/*
			View Article
		*/
		GtkWidget* view_article_button = gtk_button_new_with_label ("View Article");

		g_signal_connect (view_article_button, "clicked", G_CALLBACK (rss_operation_click), &rss_op_view_article);

		/*
			Manage Feeds
		*/
		GtkWidget* manage_feeds_button = gtk_button_new_with_label ("Manage Feeds");

		g_signal_connect (manage_feeds_button, "clicked", G_CALLBACK (manage_feeds_click), NULL);

		/*
			Refesh Feed
		*/
		GtkWidget* refresh_feed_button = gtk_button_new_with_label ("Refresh Feed");

		g_signal_connect (refresh_feed_button, "clicked", G_CALLBACK (refresh_feed_click), NULL);

		gtk_container_add (GTK_CONTAINER (primary_function_buttons), view_article_button);
		gtk_container_add (GTK_CONTAINER (primary_function_buttons), manage_feeds_button);
		gtk_container_add (GTK_CONTAINER (primary_function_buttons), refresh_feed_button);
	}

	/*
		Info Bar
	*/
	GtkWidget* info_bar = gtk_label_new ("Status");

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
		namespace rss_ns = gautier_rss_data_read;
		std::vector<rss_ns::rss_feed> feed_names;

		std::string db_file_name = gautier_rss_ui_app::get_db_file_name();

		rss_ns::get_feed_names (db_file_name, feed_names);

		for (rss_ns::rss_feed feed : feed_names) {
			ns::add_headline_page (headlines_view, feed.feed_name);
		}
	}
	/*
		Article Frame
	*/
	GtkWidget* article_frame = gtk_box_new (GTK_ORIENTATION_VERTICAL, 4);
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
	GtkWidget* layout_pane = gtk_paned_new (GTK_ORIENTATION_HORIZONTAL);
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
	gautier_rss_data_read::clear_feed_data_all (_feed_data);

	gautier_rss_win_main_headlines_frame::switch_page (_feed_data, headlines_view, content,
	        &connect_headline_list_box_select_row);

	std::string feed_name = gtk_notebook_get_tab_label_text (headlines_view, content);
	gtk_header_bar_set_title (GTK_HEADER_BAR (header_bar), feed_name.data());

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
	/*
		Clear feed headline/article data.
	*/
	gautier_rss_data_read::clear_feed_data_keep_name (_feed_data);

	gautier_rss_win_main_headlines_frame::select_headline (_feed_data, headline_row);

	/*
		Article date.
	*/
	gtk_label_set_text (GTK_LABEL (article_date), _feed_data.article_date.data());

	/*
		Article summary.
	*/
	{
		GtkTextBuffer* text_buffer = gtk_text_view_get_buffer (GTK_TEXT_VIEW (article_summary));

		bool indicates_html = gautier_rss_data_read::indicates_html (_feed_data.article_summary);

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
	//stub callback to implement later.

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
	rss_mod_running = false;

	thread_rss_mod.join();

	return;
}

static void
process_rss_modifications()
{
	while (rss_mod_running) {
		std::this_thread::sleep_for (std::chrono::seconds (1));

		//Keep this part until the code is finished. It gives us a reliable indicator that the background thread is still in operation.
		std::string datetime = gautier_rss_data_read::get_current_date_time_utc();

		std::cout << datetime << "\n";

		gint page_count = gtk_notebook_get_n_pages (GTK_NOTEBOOK (headlines_view));

		std::cout << "Tab count: " << page_count << "\n";

		for (int tab_i = 0; tab_i < page_count; tab_i++) {
			GtkWidget* tab = gtk_notebook_get_nth_page (GTK_NOTEBOOK (headlines_view), tab_i);

			const gchar* tab_text = gtk_notebook_get_tab_label_text (GTK_NOTEBOOK (headlines_view), tab);

			std::string tab_label = tab_text;

			std::cout << "\t\tTab " << tab_i << ": " << tab_label << "\n";
		}

	}

	return;
}

