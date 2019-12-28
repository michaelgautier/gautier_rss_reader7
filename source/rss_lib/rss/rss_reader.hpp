/*
Copyright (C) 2019 Michael Gautier

This source code is free software; you can redistribute it and/or modify it under the terms of the GNU Lesser General Public License as published by the Free Software Foundation; either version 2.1 of the License, or (at your option) any later version.

This source code is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public License along with this library. If not, see <http://www.gnu.org/licenses/>.

Author: Michael Gautier <michaelgautier.wordpress.com>
*/

#ifndef michael_gautier_rss_data_read_api_h
#define michael_gautier_rss_data_read_api_h

#include <vector>
#include <string>

#include "rss_article.hpp"
#include "rss_feed.hpp"

namespace gautier_rss_data_read {
	void
	get_feed (std::string db_file_name, std::string feed_name, rss_feed& feed);

	void
	get_feed_by_row_id (std::string db_file_name, std::string row_id, rss_feed& feed);

	void
	get_feed_names (std::string db_file_name, std::vector < rss_feed >& feed_names);

	void
	get_feed_headlines (std::string db_file_name, std::string feed_name, std::vector < std::string >& headlines);

	void
	get_feed_article_summary (std::string db_file_name, std::string feed_name, std::string headline,
	                          rss_article& article);

	std::string
	get_row_id (std::string db_file_name, std::string feed_url);

	std::string
	get_current_date_time_utc();

	//See SQLite documentation: SQL As Understood By SQLite - Date And Time Functions
	int
	get_time_difference_in_seconds (std::string date1, std::string date2);

	//checks if the feed can be retrieved based on expiration date.
	bool
	is_feed_stale (std::string db_file_name, std::string feed_name);
}
#endif
