/*
Copyright (C) 2019 Michael Gautier

This source code is free software; you can redistribute it and/or modify it under the terms of the GNU Lesser General Public License as published by the Free Software Foundation; either version 2.1 of the License, or (at your option) any later version.

This source code is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public License along with this library. If not, see <http://www.gnu.org/licenses/>.

Author: Michael Gautier <michaelgautier.wordpress.com>
*/

#include "rss_lib/rss/rss_reader.hpp"
#include "rss_lib/db/db.hpp"

void
gautier_rss_data_read::get_feed_names (std::string db_file_name, std::vector <rss_feed>& feed_names)
{
	namespace ns_db = gautier_rss_database;

	sqlite3* db = NULL;
	ns_db::open_db (db_file_name, &db);

	ns_db::sql_rowset_type rows;
	std::string sql_text = "SELECT feed_name, feed_url, last_retrieved FROM feeds;";

	ns_db::sql_parameter_list_type params;

	ns_db::process_sql (&db, sql_text, params, rows);

	for (ns_db::sql_row_type row : rows) {
		feed_names.emplace_back (rss_feed());
		rss_feed* feed = &feed_names.back();

		for (ns_db::sql_row_type::value_type field : row) {
			if (field.first == "feed_name") {
				feed->feed_name = field.second;
			} else if (field.first == "feed_url") {
				feed->feed_url = field.second;
			} else if (field.first == "last_retrieved") {
				feed->last_retrieved = field.second;
			}
		}
	}

	ns_db::close_db (&db);

	return;
}

void
gautier_rss_data_read::get_feed_headlines (std::string db_file_name, std::string feed_name,
        std::vector <std::string>& headlines)
{
	namespace ns_db = gautier_rss_database;

	sqlite3* db = NULL;
	ns_db::open_db (db_file_name, &db);

	ns_db::sql_rowset_type rows;
	std::string sql_text = "SELECT headline_text FROM feeds_articles WHERE feed_name = @feed_name";

	ns_db::sql_parameter_list_type params = {
		feed_name
	};

	ns_db::process_sql (&db, sql_text, params, rows);

	for (ns_db::sql_row_type row : rows) {
		for (ns_db::sql_row_type::value_type field : row) {
			if (field.first == "headline_text") {
				headlines.emplace_back (field.second);
			}
		}
	}

	ns_db::close_db (&db);

	return;
}

void
gautier_rss_data_read::get_feed_article_summary (std::string db_file_name, std::string feed_name,
        std::string headline, rss_article& article)
{
	namespace ns_db = gautier_rss_database;

	sqlite3* db = NULL;
	ns_db::open_db (db_file_name, &db);

	ns_db::sql_rowset_type rows;
	std::string sql_text =
	    "SELECT article_text, article_date, article_url FROM feeds_articles WHERE feed_name = @feed_name AND headline_text = @headline_text";

	ns_db::sql_parameter_list_type params = {
		feed_name,
		headline
	};

	ns_db::process_sql (&db, sql_text, params, rows);

	article.feed_name = feed_name;
	article.headline = headline;

	for (ns_db::sql_row_type row : rows) {
		for (ns_db::sql_row_type::value_type field : row) {
			if (field.first == "article_text") {
				article.article_text = field.second;
			} else if (field.first == "article_date") {
				article.article_date = field.second;
			} else if (field.first == "article_url") {
				article.url = field.second;
			}
		}

		/*Only need first matching row.*/
		break;
	}

	ns_db::close_db (&db);

	return;
}

/*checks if the feed can be retrieved based on expiration date. */
bool
gautier_rss_data_read::is_feed_stale (std::string db_file_name, std::string feed_name)
{
	namespace ns_db = gautier_rss_database;

	sqlite3* db = NULL;
	ns_db::open_db (db_file_name, &db);

	ns_db::sql_rowset_type rows;
	std::string sql_text = "SELECT feed_name, feed_url, last_retrieved FROM feeds;";

	ns_db::sql_parameter_list_type params;

	ns_db::process_sql (&db, sql_text, params, rows);

	bool is_feed_stale = false;

	for (ns_db::sql_row_type row : rows) {
		for (ns_db::sql_row_type::value_type field : row) {
			if (field.first == "last_retrieved") {
				std::string last_retrieved = field.second;

				/*
					Compare the last_retrieved date to the current date

					assign result:

					is_feed_stale = (last_retrieved + 1 hour > current date and time);
				*/

				break;
			}
		}
	}

	ns_db::close_db (&db);

	return false;
}
