/*
Copyright (C) 2019 Michael Gautier

This source code is free software; you can redistribute it and/or modify it under the terms of the GNU Lesser General Public License as published by the Free Software Foundation; either version 2.1 of the License, or (at your option) any later version.

This source code is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public License along with this library. If not, see <http://www.gnu.org/licenses/>.

Author: Michael Gautier <michaelgautier.wordpress.com>
*/

#include "rss_lib/rss/rss_reader.hpp"
#include "rss_lib/db/db.hpp"

static void
create_feed_from_sql_row (gautier_rss_database::sql_row_type& row, gautier_rss_data_read::rss_feed& feed);

void
gautier_rss_data_read::get_feed (std::string db_file_name, std::string feed_name, rss_feed& feed)
{
	namespace ns_db = gautier_rss_database;

	sqlite3* db = NULL;
	ns_db::open_db (db_file_name, &db);

	ns_db::sql_rowset_type rows;
	std::string sql_text =
	    "SELECT f.feed_name, f.feed_url, \
		f.last_retrieved, f.retrieve_limit_hrs, f.retention_days, \
		COUNT(*) AS article_count \
		FROM feeds AS f \
		LEFT OUTER JOIN feeds_articles AS fa ON f.feed_name = fa.feed_name \
		WHERE f.feed_name = @feed_name \
		GROUP BY f.feed_name, f.feed_url, f.last_retrieved, f.retrieve_limit_hrs, f.retention_days;";

	ns_db::sql_parameter_list_type params = {
		feed_name
	};

	ns_db::process_sql (&db, sql_text, params, rows);

	for (ns_db::sql_row_type row : rows) {
		rss_feed feed_info;

		create_feed_from_sql_row (row, feed_info);

		feed = feed_info;
	}

	ns_db::close_db (&db);

	return;
}

void
gautier_rss_data_read::get_feed_by_row_id (std::string db_file_name, std::string row_id, rss_feed& feed)
{
	namespace ns_db = gautier_rss_database;

	sqlite3* db = NULL;
	ns_db::open_db (db_file_name, &db);

	ns_db::sql_rowset_type rows;
	std::string sql_text =
	    "SELECT f.feed_name, f.feed_url, \
		f.last_retrieved, f.retrieve_limit_hrs, f.retention_days, \
		COUNT(*) AS article_count \
		FROM feeds AS f \
		LEFT OUTER JOIN feeds_articles AS fa ON f.feed_name = fa.feed_name \
		WHERE f.rowid = @row_id \
		GROUP BY f.feed_name, f.feed_url, f.last_retrieved, f.retrieve_limit_hrs, f.retention_days;";

	ns_db::sql_parameter_list_type params = {
		row_id
	};

	ns_db::process_sql (&db, sql_text, params, rows);

	for (ns_db::sql_row_type row : rows) {
		rss_feed feed_info;

		create_feed_from_sql_row (row, feed_info);

		feed = feed_info;
	}

	ns_db::close_db (&db);

	return;
}

void
gautier_rss_data_read::get_feeds (std::string db_file_name, std::vector <rss_feed>& feeds)
{
	namespace ns_db = gautier_rss_database;

	sqlite3* db = NULL;
	ns_db::open_db (db_file_name, &db);

	ns_db::sql_rowset_type rows;
	std::string sql_text =
	    "SELECT f.feed_name, f.feed_url, \
		f.last_retrieved, f.retrieve_limit_hrs, f.retention_days, \
		COUNT(*) AS article_count \
		FROM feeds AS f \
		LEFT OUTER JOIN feeds_articles AS fa ON f.feed_name = fa.feed_name \
		GROUP BY f.feed_name, f.feed_url, f.last_retrieved, f.retrieve_limit_hrs, f.retention_days;";

	ns_db::sql_parameter_list_type params;

	ns_db::process_sql (&db, sql_text, params, rows);

	for (ns_db::sql_row_type row : rows) {
		rss_feed feed;

		create_feed_from_sql_row (row, feed);

		feeds.emplace_back (feed);
	}

	ns_db::close_db (&db);

	return;
}

static void
create_feed_from_sql_row (gautier_rss_database::sql_row_type& row, gautier_rss_data_read::rss_feed& feed)
{
	for (gautier_rss_database::sql_row_type::value_type field : row) {
		if (field.first == "feed_name") {
			feed.feed_name = field.second;
		} else if (field.first == "feed_url") {
			feed.feed_url = field.second;
		} else if (field.first == "last_retrieved") {
			feed.last_retrieved = field.second;
		} else if (field.first == "retrieve_limit_hrs") {
			feed.retrieve_limit_hrs = field.second;
		} else if (field.first == "retention_days") {
			feed.retention_days = field.second;
		} else if (field.first == "article_count") {
			feed.article_count = std::stoi (field.second);
		}
	}

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
	std::string sql_text =
	    "SELECT headline_text FROM feeds_articles WHERE feed_name = @feed_name;";

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
	    "SELECT article_summary, article_text, article_date, article_url FROM feeds_articles WHERE feed_name = @feed_name AND headline_text = @headline_text";

	ns_db::sql_parameter_list_type params = {
		feed_name,
		headline
	};

	ns_db::process_sql (&db, sql_text, params, rows);

	article.feed_name = feed_name;
	article.headline = headline;

	for (ns_db::sql_row_type row : rows) {
		for (ns_db::sql_row_type::value_type field : row) {
			if (field.first == "article_summary") {
				article.article_summary = field.second;
			} else if (field.first == "article_text") {
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

int
gautier_rss_data_read::get_feed_headline_count (std::string db_file_name, std::string feed_name)
{
	int size = 0;

	namespace ns_db = gautier_rss_database;

	sqlite3* db = NULL;
	ns_db::open_db (db_file_name, &db);

	ns_db::sql_rowset_type rows;
	std::string sql_text = "SELECT count(*) as article_count FROM feeds_articles WHERE feed_name = @feed_name";

	ns_db::sql_parameter_list_type params = {
		feed_name
	};

	ns_db::process_sql (&db, sql_text, params, rows);

	for (ns_db::sql_row_type row : rows) {
		for (ns_db::sql_row_type::value_type field : row) {
			if (field.first == "article_count") {
				size = std::stoi (field.second);
			}
		}
	}

	ns_db::close_db (&db);

	return size;
}

std::string
gautier_rss_data_read::get_row_id (std::string db_file_name, std::string feed_url)
{
	std::string row_id;

	namespace ns_db = gautier_rss_database;

	sqlite3* db = NULL;
	ns_db::open_db (db_file_name, &db);

	ns_db::sql_rowset_type rows;
	std::string sql_text = "SELECT rowid FROM feeds WHERE feed_url = @feed_url";

	ns_db::sql_parameter_list_type params = {
		feed_url
	};

	ns_db::process_sql (&db, sql_text, params, rows);

	for (ns_db::sql_row_type row : rows) {
		for (ns_db::sql_row_type::value_type field : row) {
			if (field.first == "rowid") {
				row_id = field.second;

				break;
			}
		}
	}

	ns_db::close_db (&db);

	return row_id;
}

