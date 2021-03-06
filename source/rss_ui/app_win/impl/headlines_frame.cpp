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
#include <string>
#include <vector>

#include "rss_ui/application.hpp"
#include "rss_ui/app_win/headlines_frame.hpp"

namespace {
	int
	headlines_section_width = 0;

	/*
		Headline Model
	*/
	const int
	col_pos_headline_text = 0;

	const int
	col_pos_stop = -1;

	void
	initialize_headlines_list_view (GtkWidget* scroll_container, GtkWidget* headlines_list_view,
	                                void (*headline_view_select_row) (GtkTreeSelection*, gpointer));

	void
	get_row_data (GtkTreeSelection* tree_selection, std::string& headline_text);

	void
	initialize_headlines_list_view (GtkWidget* scroll_container, GtkWidget* headlines_list_view,
	                                void (*headline_view_select_row) (GtkTreeSelection*, gpointer))
	{
		gtk_widget_set_size_request (headlines_list_view, headlines_section_width, -1);

		/*
			Column: Feed Name
		*/
		GtkCellRenderer* column_renderer_headline_text = gtk_cell_renderer_text_new();
		GtkTreeViewColumn* column_headline_text = gtk_tree_view_column_new_with_attributes ("Headlines",
		        column_renderer_headline_text, "text", col_pos_headline_text, nullptr);
		gtk_tree_view_append_column (GTK_TREE_VIEW (headlines_list_view), column_headline_text);

		/*
			Tree Model to describe the columns
		*/
		GtkListStore* list_store = gtk_list_store_new (1 /*1 column*/,
		                           G_TYPE_STRING/*headline*/);

		gtk_tree_view_set_model (GTK_TREE_VIEW (headlines_list_view), GTK_TREE_MODEL (list_store));

		gtk_container_add (GTK_CONTAINER (scroll_container), headlines_list_view);

		if (headline_view_select_row) {
			GtkTreeSelection* rss_headline_selection_manager = gtk_tree_view_get_selection (GTK_TREE_VIEW (
			            headlines_list_view));
			gtk_tree_selection_set_mode (rss_headline_selection_manager, GTK_SELECTION_SINGLE);

			g_signal_connect (rss_headline_selection_manager, "changed", G_CALLBACK (headline_view_select_row), nullptr);
		}

		return;
	}

	void
	get_row_data (GtkTreeSelection* tree_selection, std::string& headline_text)
	{
		/*
			GTK documentation says this will not work if the selection mode is GTK_SELECTION_MULTIPLE.
		*/
		GtkTreeModel* tree_model;
		GtkTreeIter tree_iterator;

		bool row_selected = gtk_tree_selection_get_selected (tree_selection, &tree_model, &tree_iterator);

		if (row_selected) {
			gchar* headline_text_data;

			gtk_tree_model_get (tree_model, &tree_iterator, col_pos_headline_text, &headline_text_data, -1);

			headline_text = headline_text_data;
		}

		return;
	}
}

void
gautier_rss_win_main_headlines_frame::initialize_headline_view (GtkWidget* headlines_view,
        const int monitor_width, const int monitor_height)
{
	const double monitor_width_d = monitor_width;
	const double monitor_width_part = 1.7;

	headlines_section_width = (int) ((monitor_width_d / monitor_width_part));

	const int article_section_width = (int) ((monitor_width - headlines_section_width));

	std::cout << "Optimal article section width: " << article_section_width << "\n";
	std::cout << "NOTE - Monitor Height: " << monitor_height << "\n";

	gtk_notebook_set_scrollable (GTK_NOTEBOOK (headlines_view), true);

	return;
}

void
gautier_rss_win_main_headlines_frame::add_headline_page (GtkWidget* headlines_view, const std::string feed_name,
        const int position, void (*headline_view_select_row) (GtkTreeSelection*, gpointer))
{
	GtkWidget* scroll_win = gtk_scrolled_window_new (nullptr, nullptr);

	gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (scroll_win), GTK_POLICY_ALWAYS, GTK_POLICY_ALWAYS);

	GtkWidget* headlines_list_view = gtk_tree_view_new();

	initialize_headlines_list_view (scroll_win, headlines_list_view, headline_view_select_row);

	gint tab_i = -1;

	if (position < 0) {
		tab_i = gtk_notebook_append_page (GTK_NOTEBOOK (headlines_view), scroll_win, gtk_label_new (feed_name.data()));
	} else {
		tab_i =	gtk_notebook_insert_page (GTK_NOTEBOOK (headlines_view), scroll_win, gtk_label_new (feed_name.data()),
		                                  position);
	}

	gtk_widget_show_all (scroll_win);

	if (tab_i > -1) {
		GtkWidget* tab = gtk_notebook_get_nth_page (GTK_NOTEBOOK (headlines_view), tab_i);

		if (tab) {
			gtk_widget_set_sensitive (tab, false);
		}
	}

	return;
}

void
gautier_rss_win_main_headlines_frame::show_headlines (GtkWidget* headlines_view, const std::string& feed_name,
        const gautier_rss_data_read::headline_range_type& range,
        const gautier_rss_data_read::headlines_list_type& headlines,
        const bool prepend)
{
#ifdef DEBUG
	std::cout << __func__ << " \t\t" << feed_name << " index from \t" << range.first << " to " <<
	          range.second << ", list contains " << headlines.size() << " articles\n";
#endif
	/*
		Tab Contents (in this case a scroll window containing a list box)

		Tab > Scroll Window > List Box > individual labels (headlines)
	*/
	const size_t headline_count = headlines.size();

	const int64_t range_end = (int64_t) (headline_count);

	if (range_end > -1) {
		GtkWidget* headlines_list_view = nullptr;

		/*
			Tab (contents)
		*/
		{
			GtkWidget* tab = nullptr;

			gint tab_n = -1;

			const gint page_count = gtk_notebook_get_n_pages (GTK_NOTEBOOK (headlines_view));

			if (page_count > 0) {
				for (gint tab_i = 0; tab_i < page_count; tab_i++) {
					tab = gtk_notebook_get_nth_page (GTK_NOTEBOOK (headlines_view), tab_i);

					const std::string tab_label = gtk_notebook_get_tab_label_text (GTK_NOTEBOOK (headlines_view), tab);

					if (feed_name == tab_label) {
						tab_n = tab_i;

						break;
					}
				}
			}

			/*
				Headlines List Widget
			*/
			if (tab_n > -1 && tab) {
				GtkScrolledWindow* scroll_win = GTK_SCROLLED_WINDOW (tab);

				gautier_rss_ui_app::get_scroll_content_as_list_view (scroll_win, &headlines_list_view);
			}
		}

		/*
			Populate list box.
		*/
		if (headlines_list_view != nullptr) {
			GtkTreeModel* list_model = gtk_tree_view_get_model (GTK_TREE_VIEW (headlines_list_view));
			GtkListStore* list_store = GTK_LIST_STORE (list_model);

			GtkTreeIter iter;

			GtkTreeIter start_iter;

			gboolean start_iter_is_valid = gtk_tree_model_get_iter_first (list_model, &start_iter);

			for (int64_t i = range.first; i <= range.second && i < range_end; i++) {
				const size_t headline_i = (size_t) (i);

				std::string headline = headlines.at (headline_i);

				gchar* headline_text_data = headline.data();

				/*
					Adds a new row in the Tree Model.
				*/
				if (prepend && range.first > -1 && start_iter_is_valid) {
					gtk_list_store_insert_before (list_store, &iter, &start_iter);
				} else {
					gtk_list_store_append (list_store, &iter);
				}

				/*
					Links specific data to column positions in the row.
				*/
				gtk_list_store_set (list_store, &iter,
				                    col_pos_headline_text, headline_text_data,
				                    col_pos_stop);

				if (prepend && range.first > -1) {
					start_iter_is_valid = gtk_tree_model_get_iter_first (list_model, &start_iter);
				}
			}
		}
	}

	return;
}

void
gautier_rss_win_main_headlines_frame::get_selected_headline_text (GtkTreeSelection* headline_row,
        std::string& headline_text)
{
	if (headline_row) {
		get_row_data (headline_row, headline_text);
	}

	return;
}

void
gautier_rss_win_main_headlines_frame::select_headline_row (GtkWidget* headlines_view,
        const std::string feed_name, const std::string headline_text)
{
	GtkWidget* headlines_list_view = nullptr;

	/*
		Tab (contents)
	*/
	if (feed_name.empty() == false) {
		GtkWidget* tab = nullptr;

		const gint page_count = gtk_notebook_get_n_pages (GTK_NOTEBOOK (headlines_view));

		for (gint tab_i = 0; tab_i < page_count; tab_i++) {
			GtkWidget* notebook_tab = gtk_notebook_get_nth_page (GTK_NOTEBOOK (headlines_view), tab_i);

			const std::string tab_label = gtk_notebook_get_tab_label_text (GTK_NOTEBOOK (headlines_view), notebook_tab);

			if (feed_name == tab_label) {
				tab = notebook_tab;

				break;
			}
		}

		/*
			Headlines List Widget
		*/
		if (tab) {
			GtkScrolledWindow* scroll_win = GTK_SCROLLED_WINDOW (tab);

			gautier_rss_ui_app::get_scroll_content_as_list_view (scroll_win, &headlines_list_view);
		}
	}


	/*
		Select row.
	*/
	if (headlines_list_view != nullptr) {
		GtkTreeSelection* rss_tree_selection_manager = gtk_tree_view_get_selection (GTK_TREE_VIEW (
		            headlines_list_view));

		GtkTreeModel* tree_model = gtk_tree_view_get_model (GTK_TREE_VIEW (headlines_list_view));

		GtkTreeIter tree_iterator;

		gboolean iter_is_valid = gtk_tree_model_get_iter_first (tree_model, &tree_iterator);
		bool selected = false;

		if (headline_text.empty()) {
			if (iter_is_valid) {
				gtk_tree_selection_select_iter (rss_tree_selection_manager, &tree_iterator);
			}

			selected = iter_is_valid;
		} else {
			while (iter_is_valid) {
				gchar* data;

				gtk_tree_model_get (tree_model, &tree_iterator, col_pos_headline_text, &data, -1);

				const std::string row_headline = data;

				if (headline_text == row_headline) {
					gtk_tree_selection_select_iter (rss_tree_selection_manager, &tree_iterator);

					selected = true;

					break;
				} else {
					iter_is_valid = gtk_tree_model_iter_next (tree_model, &tree_iterator);
				}
			}
		}

		if (selected == false) {
			gtk_tree_selection_unselect_all (rss_tree_selection_manager);

			gtk_widget_show_all (headlines_list_view);
		}
	}

	return;
}

void
gautier_rss_win_main_headlines_frame::clear_row_selection (GtkWidget* headlines_view,
        const std::string feed_name)
{
	GtkWidget* headlines_list_view = nullptr;

	/*
		Tab (contents)
	*/
	if (feed_name.empty() == false) {
		GtkWidget* tab = nullptr;

		const gint page_count = gtk_notebook_get_n_pages (GTK_NOTEBOOK (headlines_view));

		for (gint tab_i = 0; tab_i < page_count; tab_i++) {
			GtkWidget* notebook_tab = gtk_notebook_get_nth_page (GTK_NOTEBOOK (headlines_view), tab_i);

			const std::string tab_label = gtk_notebook_get_tab_label_text (GTK_NOTEBOOK (headlines_view), notebook_tab);

			if (feed_name == tab_label) {
				tab = notebook_tab;

				break;
			}
		}

		/*
			Headlines List Widget
		*/
		if (tab) {
			GtkScrolledWindow* scroll_win = GTK_SCROLLED_WINDOW (tab);

			gautier_rss_ui_app::get_scroll_content_as_list_view (scroll_win, &headlines_list_view);
		}
	}


	/*
		Clear row selection.
	*/
	if (headlines_list_view != nullptr) {
		GtkTreeSelection* rss_tree_selection_manager = gtk_tree_view_get_selection (GTK_TREE_VIEW (
		            headlines_list_view));

		gtk_tree_selection_unselect_all (rss_tree_selection_manager);

		gtk_widget_show_all (headlines_list_view);
	}

	return;
}

void
gautier_rss_win_main_headlines_frame::set_headlines_title (GtkWidget* headlines_view, const gint tab_number,
        const std::string title)
{
	if (title.empty() == false && tab_number > -1) {
		GtkWidget* headlines_list_view = nullptr;

		/*
			Tab (contents)
		*/
		{
			GtkWidget* tab = gtk_notebook_get_nth_page (GTK_NOTEBOOK (headlines_view), tab_number);

			/*
				Headlines List Widget
			*/
			if (tab) {
				GtkScrolledWindow* scroll_win = GTK_SCROLLED_WINDOW (tab);

				gautier_rss_ui_app::get_scroll_content_as_list_view (scroll_win, &headlines_list_view);
			}
		}

		/*
			Select row.
		*/
		if (headlines_list_view != nullptr) {
			const gint visible_column_number = 0;

			GtkTreeViewColumn* headline_column = gtk_tree_view_get_column (GTK_TREE_VIEW (headlines_list_view),
			                                     visible_column_number);

			if (headline_column) {
				gtk_tree_view_column_set_title (headline_column, title.data());
			}
		}
	}

	return;
}

int
gautier_rss_win_main_headlines_frame::get_default_headlines_view_content_width()
{
	return headlines_section_width;
}

bool
gautier_rss_win_main_headlines_frame::is_tab_selected (GtkWidget* headlines_view, const std::string tab_name)
{
	bool selected = false;

	if (tab_name.empty() == false) {
		const std::string selected_tab_name = get_selected_tab_name (headlines_view);

		selected = (selected_tab_name == tab_name);
	}

	return selected;
}

gint
gautier_rss_win_main_headlines_frame::get_selected_tab_numb (GtkWidget* headlines_view)
{
	const gint tab_i = gtk_notebook_get_current_page (GTK_NOTEBOOK (headlines_view));

	return tab_i;
}

std::string
gautier_rss_win_main_headlines_frame::get_selected_tab_name (GtkWidget* headlines_view)
{
	std::string tab_name;

	const gint tab_i = get_selected_tab_numb (headlines_view);

	if (tab_i > -1) {
		GtkWidget* tab = gtk_notebook_get_nth_page (GTK_NOTEBOOK (headlines_view), tab_i);

		if (tab) {
			tab_name = gtk_notebook_get_tab_label_text (GTK_NOTEBOOK (headlines_view), tab);
		}
	}

	return tab_name;
}
