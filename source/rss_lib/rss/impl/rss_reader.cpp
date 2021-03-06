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

#include "rss_lib/rss/rss_reader.hpp"
#include "rss_lib/db/db.hpp"

namespace {
	void
	create_feed_from_sql_row (gautier_rss_database::sql_row_type& row, gautier_rss_data_read::rss_feed& feed);

	void
	create_article_from_sql_row (gautier_rss_database::sql_row_type& row,
	                             gautier_rss_data_read::rss_article& article);

	void
	create_headline_text_from_sql_row (gautier_rss_database::sql_row_type& row, std::string& headline_text);

	void
	create_feed_from_sql_row (gautier_rss_database::sql_row_type& row, gautier_rss_data_read::rss_feed& feed)
	{
		for (auto [name, value] : row) {
			if (name == "feed_name") {
				feed.feed_name = value;
			} else if (name == "feed_url") {
				feed.feed_url = value;
			} else if (name == "last_retrieved") {
				feed.last_retrieved = value;
			} else if (name == "retrieve_limit_hrs") {
				feed.retrieve_limit_hrs = value;
			} else if (name == "retention_days") {
				feed.retention_days = value;
			} else if (name == "article_count") {
				feed.article_count = std::stoll (value);
			}
		}

		return;
	}

	void
	create_article_from_sql_row (gautier_rss_database::sql_row_type& row,
	                             gautier_rss_data_read::rss_article& article)
	{
		for (auto [name, value] : row) {
			if (name == "feed_name") {
				article.feed_name = value;
			} else if (name == "headline_text") {
				article.headline = value;
			} else if (name == "article_summary") {
				article.article_summary = value;
			} else if (name == "article_text") {
				article.article_text = value;
			} else if (name == "article_date") {
				article.article_date = value;
			} else if (name == "article_url") {
				article.url = value;
			}
		}

		return;
	}

	void
	create_headline_text_from_sql_row (gautier_rss_database::sql_row_type& row, std::string& headline_text)
	{
		for (auto [name, value] : row) {
			if (name == "headline_text") {
				headline_text = value;

				break;
			}
		}

		return;
	}
}

void
gautier_rss_data_read::get_feed (const std::string db_file_name, const std::string feed_name, rss_feed& feed)
{
	namespace ns_db = gautier_rss_database;

	sqlite3* db = nullptr;
	ns_db::open_db (db_file_name, &db);

	ns_db::sql_rowset_type rows;
	const std::string sql_text =
	    "SELECT \
			f.feed_name, f.feed_url, \
			f.last_retrieved, f.retrieve_limit_hrs, f.retention_days, \
			COUNT(fa.rowid) AS article_count \
		FROM feeds AS f LEFT OUTER JOIN \
			feeds_articles AS fa ON f.feed_name = fa.feed_name \
		WHERE f.feed_name = @feed_name \
		GROUP BY f.feed_name, f.feed_url, \
			f.last_retrieved, f.retrieve_limit_hrs, f.retention_days;";

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
gautier_rss_data_read::get_feed_by_row_id (const std::string db_file_name, const int64_t row_id, rss_feed& feed)
{
	namespace ns_db = gautier_rss_database;

	sqlite3* db = nullptr;
	ns_db::open_db (db_file_name, &db);

	ns_db::sql_rowset_type rows;
	const std::string sql_text =
	    "SELECT \
			f.feed_name, f.feed_url, \
			f.last_retrieved, f.retrieve_limit_hrs, f.retention_days, \
			COUNT(fa.rowid) AS article_count \
		FROM feeds AS f LEFT OUTER JOIN \
			feeds_articles AS fa ON f.feed_name = fa.feed_name \
		WHERE f.rowid = @row_id \
		GROUP BY f.feed_name, f.feed_url, \
			f.last_retrieved, f.retrieve_limit_hrs, f.retention_days;";

	ns_db::sql_parameter_list_type params = {
		std::to_string (row_id)
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
gautier_rss_data_read::get_feeds (const std::string db_file_name, feeds_list_type& feeds)
{
	namespace ns_db = gautier_rss_database;

	sqlite3* db = nullptr;
	ns_db::open_db (db_file_name, &db);

	ns_db::sql_rowset_type rows;
	const std::string sql_text =
	    "SELECT \
			f.feed_name, f.feed_url, \
			f.last_retrieved, f.retrieve_limit_hrs, f.retention_days, \
			COUNT(fa.rowid) AS article_count \
		FROM feeds AS f LEFT OUTER JOIN \
			feeds_articles AS fa ON f.feed_name = fa.feed_name \
		GROUP BY f.feed_name, f.feed_url, \
			f.last_retrieved, f.retrieve_limit_hrs, f.retention_days \
		ORDER BY f.feed_name, f.feed_url;";

	ns_db::sql_parameter_list_type params;

	ns_db::process_sql (&db, sql_text, params, rows);

	feeds.reserve (rows.size());

	for (ns_db::sql_row_type row : rows) {
		rss_feed feed;

		create_feed_from_sql_row (row, feed);

		feeds.emplace_back (feed);
	}

	ns_db::close_db (&db);

	return;
}

void
gautier_rss_data_read::get_feed_articles (const std::string db_file_name, const std::string feed_name,
        articles_list_type& headlines, const bool descending)
{
	namespace ns_db = gautier_rss_database;

	sqlite3* db = nullptr;
	ns_db::open_db (db_file_name, &db);

	std::string sort_direction = "ASC";

	if (descending) {
		sort_direction = "DESC";
	}

	ns_db::sql_rowset_type rows;
	const std::string sql_text =
	    "SELECT \
			feed_name, headline_text, \
			article_summary, article_text, \
			article_date, article_url \
		FROM feeds_articles \
		WHERE feed_name = @feed_name \
		ORDER BY rowid " + sort_direction + ";";

	ns_db::sql_parameter_list_type params = {
		feed_name
	};

	ns_db::process_sql (&db, sql_text, params, rows);

	headlines.reserve (rows.size());

	for (ns_db::sql_row_type row : rows) {
		rss_article article;

		create_article_from_sql_row (row, article);

		if (article.feed_name.empty() == false && article.feed_name == feed_name) {
			headlines.emplace_back (article);
		}
	}

	ns_db::close_db (&db);

	return;
}

void
gautier_rss_data_read::get_feed_headlines (const std::string db_file_name, const std::string feed_name,
        headlines_list_type& headlines, const bool descending)
{
	namespace ns_db = gautier_rss_database;

	sqlite3* db = nullptr;
	ns_db::open_db (db_file_name, &db);

	std::string sort_direction = "ASC";

	if (descending) {
		sort_direction = "DESC";
	}

	ns_db::sql_rowset_type rows;
	const std::string sql_text =
	    "SELECT \
			headline_text \
		FROM feeds_articles \
		WHERE feed_name = @feed_name \
		ORDER BY rowid " + sort_direction + ";";

	ns_db::sql_parameter_list_type params = {
		feed_name
	};

	ns_db::process_sql (&db, sql_text, params, rows);

	headlines.reserve (rows.size());

	for (ns_db::sql_row_type row : rows) {
		std::string headline_text;

		create_headline_text_from_sql_row (row, headline_text);

		if (headline_text.empty() == false) {
			headlines.emplace_back (headline_text);
		}
	}

	ns_db::close_db (&db);

	return;
}

void
gautier_rss_data_read::get_feed_articles_after_row_id (const std::string db_file_name,
        const std::string feed_name,
        articles_list_type& headlines, const bool descending, const int64_t row_id)
{
	namespace ns_db = gautier_rss_database;

	sqlite3* db = nullptr;
	ns_db::open_db (db_file_name, &db);

	std::string sort_direction = "ASC";

	if (descending) {
		sort_direction = "DESC";
	}

	ns_db::sql_rowset_type rows;
	const std::string sql_text =
	    "SELECT \
			feed_name, headline_text, \
			article_summary, article_text, \
			article_date, article_url \
		FROM feeds_articles \
		WHERE feed_name = @feed_name and rowid > @rowid \
		ORDER BY rowid " + sort_direction + ";";

	ns_db::sql_parameter_list_type params = {
		feed_name,
		std::to_string (row_id)
	};

	ns_db::process_sql (&db, sql_text, params, rows);

	headlines.reserve (rows.size());

	for (ns_db::sql_row_type row : rows) {
		rss_article article;

		create_article_from_sql_row (row, article);

		if (article.feed_name.empty() == false && article.feed_name == feed_name) {
			headlines.emplace_back (article);
		}
	}

	ns_db::close_db (&db);

	return;
}

void
gautier_rss_data_read::get_feed_headlines_after_row_id (const std::string db_file_name,
        const std::string feed_name,
        headlines_list_type& headlines, const bool descending, const int64_t row_id)
{
	namespace ns_db = gautier_rss_database;

	sqlite3* db = nullptr;
	ns_db::open_db (db_file_name, &db);

	std::string sort_direction = "ASC";

	if (descending) {
		sort_direction = "DESC";
	}

	ns_db::sql_rowset_type rows;
	const std::string sql_text =
	    "SELECT \
			headline_text \
		FROM feeds_articles \
		WHERE feed_name = @feed_name and rowid > @rowid \
		ORDER BY rowid " + sort_direction + ";";

	ns_db::sql_parameter_list_type params = {
		feed_name,
		std::to_string (row_id)
	};

	ns_db::process_sql (&db, sql_text, params, rows);

	headlines.reserve (rows.size());

	for (ns_db::sql_row_type row : rows) {
		std::string headline_text;

		create_headline_text_from_sql_row (row, headline_text);

		if (headline_text.empty() == false) {
			headlines.emplace_back (headline_text);
		}
	}

	ns_db::close_db (&db);

	return;
}

void
gautier_rss_data_read::get_feed_article_summary (const std::string db_file_name, const std::string feed_name,
        const std::string headline, rss_article& article)
{
	namespace ns_db = gautier_rss_database;

	sqlite3* db = nullptr;
	ns_db::open_db (db_file_name, &db);

	ns_db::sql_rowset_type rows;
	const std::string sql_text =
	    "SELECT \
			article_summary, article_text, \
			article_date, article_url \
		FROM feeds_articles \
		WHERE feed_name = @feed_name \
			AND headline_text = @headline_text";

	ns_db::sql_parameter_list_type params = {
		feed_name,
		headline
	};

	ns_db::process_sql (&db, sql_text, params, rows);

	article.feed_name = feed_name;
	article.headline = headline;

	for (ns_db::sql_row_type row : rows) {
		for (auto [name, value] : row) {
			if (name == "article_summary") {
				article.article_summary = value;
			} else if (name == "article_text") {
				article.article_text = value;
			} else if (name == "article_date") {
				article.article_date = value;
			} else if (name == "article_url") {
				article.url = value;
			}
		}

		/*Only need first matching row.*/
		break;
	}

	ns_db::close_db (&db);

	return;
}

int64_t
gautier_rss_data_read::get_feed_headline_count (const std::string db_file_name, const std::string feed_name)
{
	int64_t size = 0;

	namespace ns_db = gautier_rss_database;

	sqlite3* db = nullptr;
	ns_db::open_db (db_file_name, &db);

	ns_db::sql_rowset_type rows;
	const std::string sql_text =
	    "SELECT \
			COUNT(*) AS article_count \
		FROM feeds_articles \
		WHERE feed_name = @feed_name";

	ns_db::sql_parameter_list_type params = {
		feed_name
	};

	ns_db::process_sql (&db, sql_text, params, rows);

	for (ns_db::sql_row_type row : rows) {
		for (auto [name, value] : row) {
			if (name == "article_count") {
				size = (int64_t) (std::stoll (value));
			}
		}
	}

	ns_db::close_db (&db);

	return size;
}

int64_t
gautier_rss_data_read::get_row_id (const std::string db_file_name, const std::string feed_url)
{
	int64_t row_id = -1;

	namespace ns_db = gautier_rss_database;

	sqlite3* db = nullptr;
	ns_db::open_db (db_file_name, &db);

	ns_db::sql_rowset_type rows;
	const std::string sql_text =
	    "SELECT \
			rowid \
		FROM feeds \
		WHERE feed_url = @feed_url";

	ns_db::sql_parameter_list_type params = {
		feed_url
	};

	ns_db::process_sql (&db, sql_text, params, rows);

	for (ns_db::sql_row_type row : rows) {
		for (auto [name, value] : row) {
			if (name == "rowid") {
				row_id = std::stoll (value);

				break;
			}
		}
	}

	ns_db::close_db (&db);

	return row_id;
}

int64_t
gautier_rss_data_read::get_article_row_id (const std::string db_file_name, const std::string article_url)
{
	int64_t row_id = -1;

	namespace ns_db = gautier_rss_database;

	sqlite3* db = nullptr;
	ns_db::open_db (db_file_name, &db);

	ns_db::sql_rowset_type rows;

	const std::string sql_text =
	    "SELECT \
			rowid \
		FROM feeds_articles \
		WHERE article_url = @article_url";

	ns_db::sql_parameter_list_type params = {
		article_url
	};

	ns_db::process_sql (&db, sql_text, params, rows);

	for (ns_db::sql_row_type row : rows) {
		for (auto [name, value] : row) {
			if (name == "rowid") {
				row_id = std::stoll (value);

				break;
			}
		}
	}

	ns_db::close_db (&db);

	return row_id;
}

int64_t
gautier_rss_data_read::get_feed_article_max_row_id (const std::string db_file_name, const std::string feed_name)
{
	int64_t row_id = -1;

	namespace ns_db = gautier_rss_database;

	sqlite3* db = nullptr;
	ns_db::open_db (db_file_name, &db);

	ns_db::sql_rowset_type rows;
	const std::string sql_text =
	    "SELECT \
			IFNULL(MAX(rowid), -1) AS rowid \
		FROM feeds_articles \
		WHERE feed_name = @feed_name";

	ns_db::sql_parameter_list_type params = {
		feed_name
	};

	ns_db::process_sql (&db, sql_text, params, rows);

	for (ns_db::sql_row_type row : rows) {
		for (auto [name, value] : row) {
			if (name == "rowid") {
				row_id = std::stoll (value);

				break;
			}
		}
	}

	ns_db::close_db (&db);

	return row_id;
}

gautier_rss_data_read::headline_range_type
gautier_rss_data_read::acquire_headline_range (const std::string& feed_name, feed_by_name_type& feeds,
        const int64_t& headline_max)
{
	rss_feed* feed = &feeds[feed_name];

	const int64_t range_end = feed->article_count;

	const int64_t index_start = (feed->last_index + 1);
	int64_t index_end = (index_start + (headline_max - 1));

	if (index_end > range_end) {
		index_end = range_end;
	}

	feed->last_index = index_end;

	return std::make_pair (index_start, index_end);
}

bool
gautier_rss_data_read::headline_range_valid (const headline_range_type& range)
{
	return range.first > -1 && range.first < range.second;
}
