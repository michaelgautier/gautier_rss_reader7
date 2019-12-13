/*
Copyright (C) 2019 Michael Gautier

This source code is free software; you can redistribute it and/or modify it under the terms of the GNU Lesser General Public License as published by the Free Software Foundation; either version 2.1 of the License, or (at your option) any later version.

This source code is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public License along with this library. If not, see <http://www.gnu.org/licenses/>.

Author: Michael Gautier <michaelgautier.wordpress.com>
*/

#include "rss_ui/rss_manage/rss_manage.hpp"

static GtkWindow*
win = NULL;

static GtkWindow*
parent_win = NULL;

static void
create_window (GtkApplication* app, GtkWindow* parent);

static void
layout_rss_feed_entry_row (GtkWidget* feed_entry_layout, GtkWindow* win);

void
gautier_rss_win_rss_manage::show_dialog (GtkApplication* app, GtkWindow* parent)
{
	create_window (app, parent);

	GtkWidget* box_layout_feed_update_entry = gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 4);

	layout_rss_feed_entry_row (box_layout_feed_update_entry, win);

	gtk_container_add (GTK_CONTAINER (win), box_layout_feed_update_entry);

	//call this last
	gtk_widget_show_all ((GtkWidget*)win);

	return;
}

static void
create_window (GtkApplication* app, GtkWindow* parent)
{
	parent_win = parent;

	win = GTK_WINDOW (gtk_window_new (GTK_WINDOW_TOPLEVEL));

	gtk_window_set_title (win, "Manage Feeds");
	gtk_window_set_resizable (win, false);
	gtk_window_set_modal (win, true);
	gtk_window_set_transient_for (win, parent_win);
	gtk_widget_set_size_request ((GtkWidget*)win, 1900, 1040);
	gtk_window_set_position (win, GTK_WIN_POS_CENTER_ALWAYS);

	return;
}

static void
layout_rss_feed_entry_row (GtkWidget* feed_entry_layout, GtkWindow* win)
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
	GtkWidget* feed_option_keep_all = gtk_radio_button_new_with_label (NULL, "Keep all articles");
	gtk_widget_set_valign (feed_option_keep_all, GTK_ALIGN_START);

	GtkWidget* feed_option_keep_last7 = gtk_radio_button_new_with_label_from_widget (GTK_RADIO_BUTTON (
	                                        feed_option_keep_all), "Keep last 7 days");
	gtk_widget_set_valign (feed_option_keep_last7, GTK_ALIGN_START);

	GtkWidget* feed_option_keep_last24hr = gtk_radio_button_new_with_label_from_widget (GTK_RADIO_BUTTON (
	        feed_option_keep_all), "Keep for 1 day");
	gtk_widget_set_valign (feed_option_keep_last24hr, GTK_ALIGN_START);

	/*
		Layout and arrange
	*/
	gtk_container_add (GTK_CONTAINER (feed_entry_layout), feed_name_label);
	gtk_container_add (GTK_CONTAINER (feed_entry_layout), feed_name_entry);
	gtk_container_add (GTK_CONTAINER (feed_entry_layout), feed_url_label);
	gtk_container_add (GTK_CONTAINER (feed_entry_layout), feed_url_entry);
	gtk_container_add (GTK_CONTAINER (feed_entry_layout), feed_refresh_interval_label);
	gtk_container_add (GTK_CONTAINER (feed_entry_layout), feed_refresh_interval);
	gtk_container_add (GTK_CONTAINER (feed_entry_layout), feed_option_keep_all);
	gtk_container_add (GTK_CONTAINER (feed_entry_layout), feed_option_keep_last7);
	gtk_container_add (GTK_CONTAINER (feed_entry_layout), feed_option_keep_last24hr);

	return;
}
