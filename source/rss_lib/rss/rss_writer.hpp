/*
Copyright (C) 2019 Michael Gautier

This source code is free software; you can redistribute it and/or modify it under the terms of the GNU Lesser General Public License as published by the Free Software Foundation; either version 2.1 of the License, or (at your option) any later version.

This source code is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public License along with this library. If not, see <http://www.gnu.org/licenses/>.

Author: Michael Gautier <michaelgautier.wordpress.com>
*/

#ifndef michael_gautier_rss_data_write_api_h
#define michael_gautier_rss_data_write_api_h

#include <string>

#include "rss_article.hpp"

namespace gautier_rss_data_write {
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
	initialize_db (std::string db_file_name);

	/*
		RSS FEED CONFIGURATION

		Creates an RSS feed configuration.
	*/
	void
	set_feed_config (std::string db_file_name,
	                 std::string feed_name,
	                 std::string feed_url,
	                 std::string retrieve_limit_hrs,
	                 std::string retention_days);

	/*
		RSS FEED CONFIGURATION

		Updates an RSS feed configuration.
	*/
	void
	update_feed_config (std::string db_file_name,
	                    std::string row_id,
	                    std::string feed_name,
	                    std::string feed_url,
	                    std::string retrieve_limit_hrs,
	                    std::string retention_days);

	/*
		RSS FEED CONFIGURATION - PART III

		Fixes links to the feed configuration.
	*/
	void
	update_feed_config_related (std::string db_file_name, std::string feed_name_old, std::string feed_name_new);

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
	delete_feed (std::string db_file_name,
	             std::string feed_url);

	/*
		RSS HEADLINE/ARTICLE

		Stores an individual line from an RSS feed.
	*/
	void
	set_feed_headline (std::string db_file_name,
	                   gautier_rss_data_read::rss_article&
	                   article);

	/*
		Primary RSS function.

		Automates the RSS feed retrieval and database update process.
		Visits each feed url stored in the database.
		Retrieves the data for the feed.
		Stores the feed data in the database.
	*/
	void
	update_rss_feeds (std::string db_file_name);

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
	update_rss_db_from_rss_xml (std::string db_file_name,
	                            std::string feed_name,
	                            std::string feed_url,
	                            std::string retrieve_limit_hrs,
	                            std::string retention_days);

	/*
		RSS XML (HTTP) to RSS XML (LOCAL FILE)

		An alternative way to process RSS information.
		1.)	Gets RSS XML data from a web server.
		2.)	Stores the XML data to a file.

		Often used during development of RSS to get RSS data samples
		for inspection and analysis. Used to calibrate parsing functions.
	*/
	void
	update_rss_xml_from_network (std::string db_file_name,
	                             std::string feed_name,
	                             std::string feed_url,
	                             std::string retrieve_limit_hrs,
	                             std::string retention_days);

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
	void
	update_rss_xml_db_from_network (std::string db_file_name,
	                                std::string feed_name,
	                                std::string feed_url,
	                                std::string retrieve_limit_hrs,
	                                std::string retention_days);

	/*
		Foundation RSS function.

		Downloads a single RSS feed.
		Visits the feed url.
		Retrieves the data for the feed.
		Stores the feed data in the database.
	*/
	void
	update_rss_db_from_network (std::string db_file_name,
	                            std::string feed_name,
	                            std::string feed_url,
	                            std::string retrieve_limit_hrs,
	                            std::string retention_days);
}
#endif
