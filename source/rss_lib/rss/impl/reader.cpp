/*
Copyright (C) 2019 Michael Gautier

This source code is free software; you can redistribute it and/or modify it under the terms of the GNU Lesser General Public License as published by the Free Software Foundation; either version 2.1 of the License, or (at your option) any later version.

This source code is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public License along with this library. If not, see <http://www.gnu.org/licenses/>.

Author: Michael Gautier <michaelgautier.wordpress.com>
*/
#include <iostream>
#include <ctime>
#include <iomanip>
#include <sstream>

#include "rss_lib/rss/rss_reader.hpp"
#include "rss_lib/db/db.hpp"

static void
create_feed_from_sql_row (gautier_rss_database::sql_row_type& row, gautier_rss_data_read::rss_feed& feed);

static std::string
get_current_date_time_utc();

void
gautier_rss_data_read::get_feed (std::string db_file_name, std::string feed_name, rss_feed& feed)
{
	namespace ns_db = gautier_rss_database;

	sqlite3* db = NULL;
	ns_db::open_db (db_file_name, &db);

	ns_db::sql_rowset_type rows;
	std::string sql_text =
	    "SELECT feed_name, feed_url, last_retrieved, retrieve_limit_hrs, retention_days FROM feeds WHERE feed_name = @feed_name;";

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
gautier_rss_data_read::get_feed_names (std::string db_file_name, std::vector <rss_feed>& feed_names)
{
	namespace ns_db = gautier_rss_database;

	sqlite3* db = NULL;
	ns_db::open_db (db_file_name, &db);

	ns_db::sql_rowset_type rows;
	std::string sql_text =
	    "SELECT feed_name, feed_url, last_retrieved, retrieve_limit_hrs, retention_days FROM feeds;";

	ns_db::sql_parameter_list_type params;

	ns_db::process_sql (&db, sql_text, params, rows);

	for (ns_db::sql_row_type row : rows) {
		rss_feed feed;

		create_feed_from_sql_row (row, feed);

		feed_names.emplace_back (feed);
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
gautier_rss_data_read::get_time_difference_in_seconds (std::string date1, std::string date2)
{
	//See SQLite documentation: SQL As Understood By SQLite - Date And Time Functions

	std::string db_file_name = ":memory:";
	int seconds = 0;

	namespace ns_db = gautier_rss_database;

	sqlite3* db = NULL;
	ns_db::open_db (db_file_name, &db);

	ns_db::sql_rowset_type rows;
	std::string sql_text = "select abs(strftime('%s', @date1) - strftime('%s', @date2)) as result;";

	std::string datetime1 = date1;
	std::string datetime2 = date2;

	if (date1.empty()) {
		datetime1 = get_current_date_time_utc();
	}

	if (date2.empty()) {
		datetime2 = get_current_date_time_utc();
	}

	ns_db::sql_parameter_list_type params = {
		datetime1,
		datetime2
	};

	ns_db::process_sql (&db, sql_text, params, rows);

	for (ns_db::sql_row_type row : rows) {
		for (ns_db::sql_row_type::value_type field : row) {
			if (field.first == "result") {
				std::string result = field.second;

				seconds = std::stoi (result);
			}
		}
	}

	ns_db::close_db (&db);

	return seconds;
}

/*checks if the feed can be retrieved based on expiration date. */
bool
gautier_rss_data_read::is_feed_stale (std::string db_file_name, std::string feed_name)
{
	bool is_feed_stale = false;

	rss_feed feed;

	get_feed (db_file_name, feed_name, feed);

	if (feed.feed_name.empty() == false) {
		std::string current_date_time_utc = get_current_date_time_utc();

		int seconds_elapsed = get_time_difference_in_seconds ("", feed.last_retrieved);

		int retrieve_limit_hrs = std::stoi (feed.retrieve_limit_hrs);

		int minutes_elapsed = seconds_elapsed / 60;
		int hours_elapsed = minutes_elapsed / 60;

		std::cout << "Feed " << feed.feed_name << "(" << feed.feed_url << ")" << "\n";
		std::cout << "Retrieve Limit (hrs) " << retrieve_limit_hrs << "\n";
		std::cout << "Current date/time " << current_date_time_utc << "\n";
		std::cout << "Last retrieved " << feed.last_retrieved << "\n";
		std::cout << "Elapsed hours " << hours_elapsed << "\n";
		std::cout << "Elapsed minutes " << minutes_elapsed << "\n";
		std::cout << "Elapsed seconds " << seconds_elapsed << "\n";

		is_feed_stale = (hours_elapsed >= retrieve_limit_hrs);
	}

	return is_feed_stale;
}

static std::string
get_current_date_time_utc()
{
	/*
		Gets the current date and time.
		Provides the date/time in a format understood by many
		systems such as SQLite.
	*/
	std::string datetime;
	{
		std::time_t result = std::time (nullptr);

		std::stringstream strout;

		strout << std::put_time (std::gmtime (&result), "%F %T") << std::ends;

		datetime = strout.str();
	}

	return datetime;
}
