/*
Copyright (C) 2020 Michael Gautier

This source code is free software; you can redistribute it and/or modify it under the terms of the GNU Lesser General Public License as published by the Free Software Foundation; either version 2.1 of the License, or (at your option) any later version.

This source code is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public License along with this library. If not, see <http://www.gnu.org/licenses/>.

Author: Michael Gautier <michaelgautier.wordpress.com>
*/

#include <iostream>

#include "rss_ui/application.hpp"
#include "rss_ui/app_win/headlines_frame.hpp"

static int
headlines_section_width = 0;

/*
	Headline Model
*/
static int
col_pos_feed_name = 0;

static int
col_pos_headline_text = 1;

static int
col_pos_article_summary = 2;

static int
col_pos_article_text = 3;

static int
col_pos_article_date = 4;

static int
col_pos_article_url = 5;

static int
col_pos_stop = -1;

static void
initialize_headlines_list_view (GtkWidget* scroll_container, GtkWidget* headlines_list_view,
                                void (*headline_view_select_row) (GtkTreeSelection*, gpointer));

static void
get_row_data (GtkTreeSelection* tree_selection, std::string& feed_name, std::string& headline_text,
              std::string& article_summary, std::string& article_text, std::string& article_date, std::string& article_url);

void
gautier_rss_win_main_headlines_frame::initialize_headline_view (GtkWidget* headlines_view,
        const int monitor_width, const int monitor_height)
{
	const double monitor_width_d = monitor_width;
	const double monitor_width_part = 1.7;

	headlines_section_width = static_cast<int> ((monitor_width_d / monitor_width_part));

	const int article_section_width = static_cast<int> ((monitor_width - headlines_section_width));

	std::cout << "Optimal article section width: " << article_section_width << "\n";
	std::cout << "NOTE - Monitor Height: " << monitor_height << "\n";

	gtk_notebook_set_scrollable (GTK_NOTEBOOK (headlines_view), true);

	return;
}

void
gautier_rss_win_main_headlines_frame::add_headline_page (GtkWidget* headlines_view, const std::string feed_name,
        const int position, void (*headline_view_select_row) (GtkTreeSelection*, gpointer))
{
	GtkWidget* scroll_win = gtk_scrolled_window_new (NULL, NULL);

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
gautier_rss_win_main_headlines_frame::show_headlines (GtkWidget* headlines_view, std::string feed_name,
        const int64_t headline_index_start, const int64_t headline_index_end,
        std::vector<gautier_rss_data_read::rss_article>& headlines, const bool prepend)
{
	std::cout << __func__ << " \t\t" << feed_name << " index from \t" << headline_index_start << " to " <<
	          headline_index_end << ", list contains " << headlines.size() << " articles\n";

	/*
		Tab Contents (in this case a scroll window containing a list box)

		Tab > Scroll Window > List Box > individual labels (headlines)
	*/
	const int64_t headlines_count_new = static_cast<int64_t> (headlines.size());

	if (headlines_count_new > -1) {
		GtkWidget* headlines_list_view = NULL;

		/*
			Tab (contents)
		*/
		{
			GtkWidget* tab = NULL;

			const gint tab_i = get_tab_contents_container_by_feed_name (GTK_NOTEBOOK (headlines_view), feed_name, &tab);

			/*
				Headlines List Widget
			*/
			if (tab_i > -1 && tab) {
				GtkScrolledWindow* scroll_win = GTK_SCROLLED_WINDOW (tab);

				gautier_rss_ui_app::get_scroll_content_as_list_view (scroll_win, &headlines_list_view);
			}
		}

		/*
			Populate list box.
		*/
		if (headlines_list_view != NULL) {
			GtkTreeModel* list_model = gtk_tree_view_get_model (GTK_TREE_VIEW (headlines_list_view));
			GtkListStore* list_store = GTK_LIST_STORE (list_model);

			if (headline_index_start == 0 && prepend == false) {
				std::cout << __func__ << " " << feed_name << " LINE: " << __LINE__ << ", clear list\n";

				gtk_list_store_clear (list_store);
			}

			GtkTreeIter iter;

			GtkTreeIter start_iter;

			gboolean start_iter_is_valid = gtk_tree_model_get_iter_first (list_model, &start_iter);

			for (int64_t i = headline_index_start; i <= headline_index_end && i < headlines_count_new; i++) {
				const size_t headline_i = static_cast<size_t> (i);

				gautier_rss_data_read::rss_article rss_data = headlines.at (headline_i);

				gchar* feed_name_data = feed_name.data();
				gchar* headline_text_data = rss_data.headline.data();
				gchar* article_summary_data = rss_data.article_summary.data();
				gchar* article_text_data = rss_data.article_text.data();
				gchar* article_date_data = rss_data.article_date.data();
				gchar* article_url_data = rss_data.url.data();

				/*
					Adds a new row in the Tree Model.
				*/
				if (prepend && headline_index_start > -1 && start_iter_is_valid) {
					gtk_list_store_insert_before (list_store, &iter, &start_iter);
				} else {
					gtk_list_store_append (list_store, &iter);
				}

				/*
					Links specific data to column positions in the row.
				*/
				gtk_list_store_set (list_store, &iter,
				                    col_pos_feed_name, feed_name_data,
				                    col_pos_headline_text, headline_text_data,
				                    col_pos_article_summary, article_summary_data,
				                    col_pos_article_text, article_text_data,
				                    col_pos_article_date, article_date_data,
				                    col_pos_article_url, article_url_data,
				                    col_pos_stop);

				if (prepend && headline_index_start > -1) {
					start_iter_is_valid = gtk_tree_model_get_iter_first (list_model, &start_iter);
				}
			}
		}
	}

	return;
}

void
gautier_rss_win_main_headlines_frame::select_headline (gautier_rss_data_read::rss_article& rss_data,
        GtkTreeSelection* headline_row)
{
	if (headline_row) {
		std::string feed_name;
		std::string headline_text;
		std::string article_summary;
		std::string article_text;
		std::string article_date;
		std::string article_url;

		get_row_data (headline_row, feed_name, headline_text, article_summary, article_text, article_date, article_url);

		rss_data.headline = headline_text;
		rss_data.article_summary = article_summary;
		rss_data.article_text = article_text;
		rss_data.article_date = article_date;
		rss_data.url = article_url;
	}

	return;
}

void
gautier_rss_win_main_headlines_frame::select_headline_row (GtkWidget* headlines_view,
        const std::string feed_name, const int64_t headline_row_index)
{
	if (headline_row_index > -1) {
		GtkWidget* headlines_list_view = NULL;

		/*
			Tab (contents)
		*/
		{
			GtkWidget* tab = NULL;

			const gint tab_i = get_tab_contents_container_by_feed_name (GTK_NOTEBOOK (headlines_view), feed_name, &tab);

			/*
				Headlines List Widget
			*/
			if (tab_i > -1 && tab) {
				GtkScrolledWindow* scroll_win = GTK_SCROLLED_WINDOW (tab);

				gautier_rss_ui_app::get_scroll_content_as_list_view (scroll_win, &headlines_list_view);
			}
		}

		/*
			Select row.
		*/
		if (headlines_list_view != NULL) {
			const bool data_entry_in_cell_is_enabled = false;

			GtkTreeViewColumn* headline_column = gtk_tree_view_get_column (GTK_TREE_VIEW (headlines_list_view),
			                                     col_pos_headline_text);

			std::string row_path_query = std::to_string (headline_row_index);

			GtkTreePath* row_path = gtk_tree_path_new_from_string (row_path_query.data());

			gtk_tree_view_set_cursor (GTK_TREE_VIEW (headlines_list_view), row_path, headline_column,
			                          data_entry_in_cell_is_enabled);
		}
	}

	return;
}

int
gautier_rss_win_main_headlines_frame::get_default_headlines_view_content_width()
{
	return headlines_section_width;
}

static void
initialize_headlines_list_view (GtkWidget* scroll_container, GtkWidget* headlines_list_view,
                                void (*headline_view_select_row) (GtkTreeSelection*, gpointer))
{
	gtk_widget_set_size_request (headlines_list_view, headlines_section_width, -1);

	/*
		Column: Feed Name
	*/
	GtkCellRenderer* column_renderer_headline_text = gtk_cell_renderer_text_new();
	GtkTreeViewColumn* column_headline_text = gtk_tree_view_column_new_with_attributes ("Headlines",
	        column_renderer_headline_text, "text", col_pos_headline_text, NULL);
	gtk_tree_view_append_column (GTK_TREE_VIEW (headlines_list_view), column_headline_text);

	/*
		Tree Model to describe the columns
	*/
	GtkListStore* list_store = gtk_list_store_new (6 /*6 columns*/,
	                           G_TYPE_STRING,/*feed name*/
	                           G_TYPE_STRING,/*headline*/
	                           G_TYPE_STRING,/*article summary*/
	                           G_TYPE_STRING,/*article text*/
	                           G_TYPE_STRING,/*article date*/
	                           G_TYPE_STRING /*article url*/);

	gtk_tree_view_set_model (GTK_TREE_VIEW (headlines_list_view), GTK_TREE_MODEL (list_store));

	gtk_container_add (GTK_CONTAINER (scroll_container), headlines_list_view);

	if (headline_view_select_row) {
		GtkTreeSelection* rss_headline_selection_manager = gtk_tree_view_get_selection (GTK_TREE_VIEW (
		            headlines_list_view));
		gtk_tree_selection_set_mode (rss_headline_selection_manager, GTK_SELECTION_SINGLE);

		g_signal_connect (rss_headline_selection_manager, "changed", G_CALLBACK (headline_view_select_row), NULL);
	}

	return;
}

static void
get_row_data (GtkTreeSelection* tree_selection, std::string& feed_name, std::string& headline_text,
              std::string& article_summary, std::string& article_text, std::string& article_date, std::string& article_url)
{
	/*
		GTK documentation says this will not work if the selection mode is GTK_SELECTION_MULTIPLE.
	*/
	GtkTreeModel* tree_model;
	GtkTreeIter tree_iterator;

	bool row_selected = gtk_tree_selection_get_selected (tree_selection, &tree_model, &tree_iterator);

	if (row_selected) {
		gchar* feed_name_data;
		gchar* headline_text_data;
		gchar* article_summary_data;
		gchar* article_text_data;
		gchar* article_date_data;
		gchar* article_url_data;

		gtk_tree_model_get (tree_model, &tree_iterator, col_pos_feed_name, &feed_name_data, -1);
		gtk_tree_model_get (tree_model, &tree_iterator, col_pos_headline_text, &headline_text_data, -1);
		gtk_tree_model_get (tree_model, &tree_iterator, col_pos_article_summary, &article_summary_data, -1);
		gtk_tree_model_get (tree_model, &tree_iterator, col_pos_article_text, &article_text_data, -1);
		gtk_tree_model_get (tree_model, &tree_iterator, col_pos_article_date, &article_date_data, -1);
		gtk_tree_model_get (tree_model, &tree_iterator, col_pos_article_url, &article_url_data, -1);

		feed_name = feed_name_data;
		headline_text = headline_text_data;
		article_summary = article_summary_data;
		article_text = article_text_data;
		article_date = article_date_data;
		article_url = article_url_data;
	}

	return;
}

int
gautier_rss_win_main_headlines_frame::get_tab_contents_container_by_feed_name (GtkNotebook* headlines_view,
        const std::string feed_name, GtkWidget** notebook_tab)
{
	int tab_n = -1;

	const gint page_count = gtk_notebook_get_n_pages (GTK_NOTEBOOK (headlines_view));

	for (gint tab_i = 0; tab_i < page_count; tab_i++) {
		GtkWidget* tab = gtk_notebook_get_nth_page (GTK_NOTEBOOK (headlines_view), tab_i);

		const std::string tab_label = gtk_notebook_get_tab_label_text (GTK_NOTEBOOK (headlines_view), tab);

		if (feed_name == tab_label) {
			tab_n = tab_i;

			*notebook_tab = tab;
			break;
		}
	}

	return tab_n;
}
