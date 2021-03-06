/*
Copyright (C) 2020 Michael Gautier

This source code is free software; you can redistribute it and/or modify it under the terms of the GNU Lesser General Public License as published by the Free Software Foundation; either version 2.1 of the License, or (at your option) any later version.

This source code is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public License along with this library. If not, see <http://www.gnu.org/licenses/>.

Author: Michael Gautier <michaelgautier.wordpress.com>
*/

#ifndef michael_gautier_rss_data_write_api_h
#define michael_gautier_rss_data_write_api_h

#include <cstdint>
#include <map>
#include <string>
#include <vector>

#include "rss_lib/rss/rss_article.hpp"
#include "rss_lib/rss/rss_feed.hpp"
#include "rss_lib/rss/rss_reader.hpp"

namespace gautier_rss_data_write {
	using articles_by_feed_type = std::map<std::string, gautier_rss_data_read::articles_list_type>;
	using headlines_by_feed_type = std::map<std::string, gautier_rss_data_read::headlines_list_type>;
	/*
		REQUIRED!

		Always call this function first.
		Creates a database if it does not exist.
		Manages the database schema.
		Creates any tables needed.

		This function does not keep the database open.
		All it does is initialize the database.

		You only need to call it once at the start of a program.
		Call it once near the start of a program before any other
		function that access the database (read or write).
	*/
	void
	initialize_db (const std::string db_file_name);

	/*
		RSS FEED CONFIGURATION

		Creates an RSS feed configuration.
	*/
	void
	set_feed_config (const std::string db_file_name,
	                 const std::string feed_name,
	                 const std::string feed_url,
	                 const std::string retrieve_limit_hrs,
	                 const std::string retention_days);

	/*
		RSS FEED CONFIGURATION

		Updates an RSS feed configuration.
	*/
	void
	update_feed_config (const std::string db_file_name,
	                    const int64_t row_id,
	                    const std::string feed_name,
	                    const std::string feed_url,
	                    const std::string retrieve_limit_hrs,
	                    const std::string retention_days);

	/*
		RSS FEED CONFIGURATION - PART III

		Fixes links to the feed configuration.
	*/
	void
	update_feed_config_related (const std::string db_file_name, const std::string feed_name_old,
	                            const std::string feed_name_new);

	/*
		RSS FEED REMOVAL

		Removes an RSS feed.

		The actual implementation can manage feed data removal a few ways:

		1.)	Actual deletion followed by an SQLite vacuum.
		2.)	Mark the feed as deleted but leave data intact.

		Decided to go with the first option.

		The fundamental element that must be unique in the RSS system overall
		is the feed url. Whether a website address, FTP, or direct TCP socket,
		the address must be unique. Exist only once in the entire system.

		That does not mean it is the primary key from a data management
		standpoint but that it can be used to reliably remove all related
		information.
	*/
	void
	delete_feed (const std::string db_file_name, const std::string feed_url);

	/*
		RSS HEADLINE/ARTICLE

		Stores an individual line from an RSS feed.
	*/
	void
	set_feed_headline (const std::string db_file_name, gautier_rss_data_read::rss_article& article);

	/*
		Primary RSS function.

		Automates the RSS feed retrieval and database update process.
		Visits each feed url stored in the database.
		Retrieves the data for the feed.
		Stores the feed data in the database.
	*/
	void
	update_rss_feeds (const std::string db_file_name,
	                  articles_by_feed_type& feed_data);

	/*
		Convenience RSS function, application-level.

		Best for batch and background processes. Not for use in a GUI.

		Uses the get_feed_info update_rss_feeds functions to create a before and after snapshot.
		The before and after snapshot is used to create a list containing only feeds that changed
		since the last download. That way, the application only has to deal with modifications
		instead of reprocessing all lines.

		Pass 0 to pause_interval_in_seconds unless you want a pause before download occurs.

		Includes articles downloaded for the updated feeds.
	*/
	void
	download_feeds (const std::string db_file_name, const int_fast32_t pause_interval_in_seconds,
	                std::vector<std::pair<gautier_rss_data_read::rss_feed, gautier_rss_data_read::rss_feed>>& changed_feeds,
	                articles_by_feed_type articles);

	/*
		RSS FEED Retrieve Date

		Controls how often a feed is updated.

		*** Extremely important ***
		Intended to prevent accessing a feed's website too often.

		Sets the last_retrieved date.
	*/
	void
	update_feed_retrieved (const std::string db_file_name, const std::string feed_url);

	/*
		RSS XML file to SQLite Db

		Used during the development of RSS data transfer processing.
		1.)	Stores the XML data to a file.
		2.)	Updates the database using only the XML file.

		Developing an XML parsing routine can require multiple steps
		and rounds of debugging before the routine accurately parses
		the input data. This function greatly assist that effort.
	*/
	void
	update_rss_db_from_rss_xml (const std::string db_file_name,
	                            const std::string feed_name,
	                            const std::string feed_url,
	                            const std::string retrieve_limit_hrs,
	                            const std::string retention_days);

	/*
		RSS XML (HTTP) to RSS XML (LOCAL FILE)

		An alternative way to process RSS information.
		1.)	Gets RSS XML data from a web server.
		2.)	Stores the XML data to a file.

		Often used during development of RSS to get RSS data samples
		for inspection and analysis. Used to calibrate parsing functions.
	*/
	long
	update_rss_xml_from_network (const std::string db_file_name,
	                             const std::string feed_name,
	                             const std::string feed_url,
	                             const std::string retrieve_limit_hrs,
	                             const std::string retention_days);

	/*
		RSS XML (HTTP) to RSS XML (LOCAL FILE) to SQLite Db

		Often used during the development of RSS data transfer processing.
		1.)	Gets RSS XML data from a web server.
		2.)	Stores the XML data to a file.
		3.)	Updates the database from setup #1.

		Step #2 provides diagnostic information that can be used to inspect
		the data that is downloaded. Sometimes this is needed to troubleshoot
		XML parsing for example but can have another purpose in providing
		an alternative source of offline/recovery information.
	*/
	long
	update_rss_xml_db_from_network (const std::string db_file_name,
	                                const std::string feed_name,
	                                const std::string feed_url,
	                                const std::string retrieve_limit_hrs,
	                                const std::string retention_days);

	/*
		Foundation RSS function.

		Downloads a single RSS feed.
		Visits the feed url.
		Retrieves the data for the feed.
		Stores the feed data in the database.

		Provides an article download.
	*/
	long
	update_rss_db_from_network (const std::string db_file_name,
	                            const std::string feed_name,
	                            const std::string feed_url,
	                            const std::string retrieve_limit_hrs,
	                            const std::string retention_days,
	                            gautier_rss_data_read::headlines_list_type& headlines);

	long
	update_rss_db_from_network (const std::string db_file_name,
	                            const std::string feed_name,
	                            const std::string feed_url,
	                            const std::string retrieve_limit_hrs,
	                            const std::string retention_days,
	                            gautier_rss_data_read::articles_list_type& articles);

	void
	remove_expired_articles (const std::string db_file_name);

	void
	de_initialize_db (const std::string db_file_name);
}
#endif
