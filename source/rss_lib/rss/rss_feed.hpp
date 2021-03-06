/*
Copyright (C) 2020 Michael Gautier

This source code is free software; you can redistribute it and/or modify it under the terms of the GNU Lesser General Public License as published by the Free Software Foundation; either version 2.1 of the License, or (at your option) any later version.

This source code is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public License along with this library. If not, see <http://www.gnu.org/licenses/>.

Author: Michael Gautier <michaelgautier.wordpress.com>
*/

#ifndef michael_gautier_rss_data_rss_feed_source_h
#define michael_gautier_rss_data_rss_feed_source_h

#include <algorithm>
#include <cstdint>
#include <map>
#include <string>
#include <vector>

namespace gautier_rss_data_read {
	struct rss_feed;

	using feed_by_name_type = std::map<std::string, rss_feed>;

	struct rss_feed
	{
		public:
			/*
				Primary fields
			*/
			std::string feed_name;
			std::string feed_url;
			std::string last_retrieved;
			std::string retrieve_limit_hrs;
			std::string retention_days;

			/*SQLite uses 64-bit signed integer keys*/
			int64_t article_count = 0;

			int64_t row_id = -1;

			/*
				Set operations

				last_index is used for application caching
				and indexing strategies.
			*/
			int64_t last_index = -1;
	};

	bool
	check_feed_changed (rss_feed& feed_old, rss_feed& feed_new);

	//Determine best time (in elapsed seconds) to refresh a feed.
	bool
	is_feed_still_fresh (const std::string db_file_name, const std::string feed_name,
	                     const bool time_message_stdout);

	void
	copy_feed (rss_feed* feed_in, rss_feed* feed_out);

	bool
	contains_feed (feed_by_name_type feeds, const std::string feed_name);
}
#endif
