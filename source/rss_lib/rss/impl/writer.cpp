/*
Copyright (C) 2019 Michael Gautier

This source code is free software; you can redistribute it and/or modify it under the terms of the GNU Lesser General Public License as published by the Free Software Foundation; either version 2.1 of the License, or (at your option) any later version.

This source code is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public License along with this library. If not, see <http://www.gnu.org/licenses/>.

Author: Michael Gautier <michaelgautier.wordpress.com>
*/

#include "rss_lib/db/db.hpp"

#include "rss_lib/rss_download/feed_download.hpp"

#include "rss_lib/rss_parse/feed_parse.hpp"

#include "rss_lib/rss/rss_article.hpp"
#include "rss_lib/rss/rss_feed.hpp"
#include "rss_lib/rss/rss_reader.hpp"
#include "rss_lib/rss/rss_writer.hpp"

void
gautier_rss_data_write::initialize_db (std::string db_file_name)
{
	namespace ns_db = gautier_rss_database;

	sqlite3* db = NULL;
	ns_db::open_db (db_file_name, &db);

	std::vector<std::string> sql_texts = {
		"CREATE TABLE feeds (feed_name TEXT, feed_url TEXT, last_retrieved TEXT, retrieve_limit_hrs TEXT, retention_days TEXT);",
		"CREATE TABLE feeds_articles (feed_name TEXT, headline_text TEXT, article_summary TEXT, article_text TEXT, article_date TEXT, article_url TEXT);"
	};

	ns_db::sql_parameter_list_type params;

	for (std::string sql_text : sql_texts) {
		ns_db::process_sql (&db, sql_text, params);
	}

	ns_db::close_db (&db);

	return;
}

void
gautier_rss_data_write::set_feed_name (std::string db_file_name,
                                       std::string feed_name,
                                       std::string feed_url,
                                       std::string retrieve_limit_hrs,
                                       std::string retention_days)
{
	namespace ns_db = gautier_rss_database;

	std::string sql_text =
	    "INSERT INTO feeds (feed_name, feed_url, last_retrieved, retrieve_limit_hrs, retention_days)\
		SELECT @feed_name, @feed_url, @feed_date, @retrieve_limit_hrs, @retention_days\
		WHERE 0 = (\
			SELECT COUNT(*) FROM feeds WHERE feed_name = @feed_name\
		)";

	ns_db::sql_parameter_list_type params = {
		feed_name,
		feed_url,
		"10/10/2019"
	};

	sqlite3* db = NULL;
	ns_db::open_db (db_file_name, &db);

	ns_db::sql_rowset_type rows;

	ns_db::process_sql (&db, sql_text, params, rows);

	ns_db::close_db (&db);

	return;
}

void
gautier_rss_data_write::set_feed_headline (std::string db_file_name,
        gautier_rss_data_read::rss_article& article)
{
	namespace ns_db = gautier_rss_database;

	std::string sql_text =
	    "INSERT INTO feeds_articles (feed_name, headline_text, article_summary, article_text, article_date, article_url)\
		SELECT @feed_name, @headline_text, @article_summary, @article_text, @article_date, @feed_url\
		WHERE 0 = (\
			SELECT COUNT(*) FROM feeds_articles WHERE feed_name = @feed_name AND headline_text = @headline_text\
		)";

	ns_db::sql_parameter_list_type params = {
		article.feed_name,
		article.headline,
		article.article_summary,
		article.article_text,
		article.article_date,
		article.url
	};

	sqlite3* db = NULL;
	ns_db::open_db (db_file_name, &db);

	ns_db::sql_rowset_type rows;

	ns_db::process_sql (&db, sql_text, params, rows);

	ns_db::close_db (&db);

	return;
}


//Higher level routines
void
gautier_rss_data_write::update_rss_feeds (std::string db_file_name)
{
	namespace ns_parse = gautier_rss_data_parse;

	namespace ns_data = gautier_rss_data_read;

	std::vector<ns_data::rss_feed> rss_feeds;

	ns_data::get_feed_names (db_file_name, rss_feeds);

	for (ns_data::rss_feed feed : rss_feeds) {
		std::string feed_name = feed.feed_name;
		std::string feed_url = feed.feed_url;

		std::string feed_data;

		ns_data::download_rss_feed (feed_url, feed_data);

		std::vector<ns_data::rss_article> feed_lines;

		ns_parse::get_feed_lines (feed_data, feed_lines);

		for (ns_data::rss_article article : feed_lines) {
			article.feed_name = feed_name;

			set_feed_headline (db_file_name, article);
		}
	}

	return;
}

void
gautier_rss_data_write::update_rss_db_from_rss_xml (std::string db_file_name,
        std::string feed_name,
        std::string feed_url,
        std::string retrieve_limit_hrs,
        std::string retention_days)
{
	namespace ns_parse = gautier_rss_data_parse;

	namespace ns_data = gautier_rss_data_read;

	set_feed_name (db_file_name, feed_name, feed_url, retrieve_limit_hrs, retention_days);

	std::string feed_data;

	ns_parse::get_feed_data_from_file (feed_name, ".xml", feed_data);

	std::vector<ns_data::rss_article> feed_lines;

	ns_parse::get_feed_lines (feed_data, feed_lines);

	for (ns_data::rss_article article : feed_lines) {
		article.feed_name = feed_name;

		set_feed_headline (db_file_name, article);
	}

	return;
}

void
gautier_rss_data_write::update_rss_xml_from_network (std::string db_file_name,
        std::string feed_name,
        std::string feed_url,
        std::string retrieve_limit_hrs,
        std::string retention_days)
{
	namespace ns_parse = gautier_rss_data_parse;

	namespace ns_data = gautier_rss_data_read;

	set_feed_name (db_file_name, feed_name, feed_url, retrieve_limit_hrs, retention_days);

	std::string feed_data;

	ns_data::download_rss_feed (feed_url, feed_data);

	ns_parse::save_feed_data_to_file (feed_name, ".xml", feed_data);

	return;
}

void
gautier_rss_data_write::update_rss_xml_db_from_network (std::string db_file_name,
        std::string feed_name,
        std::string feed_url,
        std::string retrieve_limit_hrs,
        std::string retention_days)
{
	namespace ns_parse = gautier_rss_data_parse;

	namespace ns_data = gautier_rss_data_read;

	set_feed_name (db_file_name, feed_name, feed_url, retrieve_limit_hrs, retention_days);

	std::string feed_data;

	ns_data::download_rss_feed (feed_url, feed_data);

	ns_parse::save_feed_data_to_file (feed_name, ".xml", feed_data);

	std::vector<ns_data::rss_article> feed_lines;

	ns_parse::get_feed_lines (feed_data, feed_lines);

	for (ns_data::rss_article article : feed_lines) {
		article.feed_name = feed_name;

		set_feed_headline (db_file_name, article);
	}

	return;
}

void
gautier_rss_data_write::update_rss_db_from_network (std::string db_file_name,
        std::string feed_name,
        std::string feed_url,
        std::string retrieve_limit_hrs,
        std::string retention_days)
{
	namespace ns_parse = gautier_rss_data_parse;

	namespace ns_data = gautier_rss_data_read;

	set_feed_name (db_file_name, feed_name, feed_url, retrieve_limit_hrs, retention_days);

	std::string feed_data;

	ns_data::download_rss_feed (feed_url, feed_data);

	std::vector<ns_data::rss_article> feed_lines;

	ns_parse::get_feed_lines (feed_data, feed_lines);

	for (ns_data::rss_article article : feed_lines) {
		article.feed_name = feed_name;

		set_feed_headline (db_file_name, article);
	}

	return;
}
