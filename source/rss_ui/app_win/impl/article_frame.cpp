/*
Copyright (C) 2020 Michael Gautier

This source code is free software; you can redistribute it and/or modify it under the terms of the GNU Lesser General Public License as published by the Free Software Foundation; either version 2.1 of the License, or (at your option) any later version.

This source code is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public License along with this library. If not, see <http://www.gnu.org/licenses/>.

Author: Michael Gautier <michaelgautier.wordpress.com>
*/

#include <iostream>

#include "rss_ui/app_win/article_frame.hpp"

void
gautier_rss_win_main_article_frame:: initialize_article_frame (GtkWidget* article_frame)
{
	if (article_frame == nullptr) {
		std::cout << __func__ << " LINE: " << __LINE__ << "article_frame nullptr\n";
	}

	return;
}

void
gautier_rss_win_main_article_frame::layout_article_frame (GtkWidget* article_frame, GtkWidget* header_bar,
        GtkWidget* article_summary, GtkWidget* article_text, GtkWidget* article_date,
        GtkWidget* primary_function_buttons, GtkWidget* info_bar)
{
	bool box_child_is_expanded = false;
	bool box_child_is_fills = false;

	/*
		Header Bar
	*/
	gtk_box_pack_start (GTK_BOX (article_frame), header_bar, box_child_is_expanded, box_child_is_fills, 0);

	/*
		Article Summary
	*/
	gtk_box_pack_start (GTK_BOX (article_frame), article_summary, box_child_is_expanded, box_child_is_fills, 0);

	/*
		Article Text
	*/
	box_child_is_expanded = false;
	box_child_is_fills = true;

	/*
		Always wrap a text label or text area in a scroll window to control text resize.
	*/
	GtkWidget* scroll_win = gtk_scrolled_window_new (nullptr, nullptr);
	gtk_widget_set_hexpand (scroll_win, true);
	gtk_widget_set_vexpand (scroll_win, true);
	gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (scroll_win), GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);

	if (article_text) {
		gtk_container_add (GTK_CONTAINER (scroll_win), article_text);
	}

	gtk_box_pack_start (GTK_BOX (article_frame), scroll_win, box_child_is_expanded, box_child_is_fills, 0);

	box_child_is_expanded = false;
	box_child_is_fills = false;

	/*
		Article Date
	*/
	gtk_box_pack_start (GTK_BOX (article_frame), article_date, box_child_is_expanded, box_child_is_fills, 0);

	/*
		Primary Function Buttons
	*/
	gtk_box_pack_start (GTK_BOX (article_frame), primary_function_buttons, box_child_is_expanded,
	                    box_child_is_fills, 0);

	/*
		Info Bar
	*/
	gtk_box_pack_start (GTK_BOX (article_frame), info_bar, box_child_is_expanded, box_child_is_fills, 0);

	return;
}
