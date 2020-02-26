/*
Copyright (C) 2020 Michael Gautier

This source code is free software; you can redistribute it and/or modify it under the terms of the GNU Lesser General Public License as published by the Free Software Foundation; either version 2.1 of the License, or (at your option) any later version.

This source code is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public License along with this library. If not, see <http://www.gnu.org/licenses/>.

Author: Michael Gautier <michaelgautier.wordpress.com>
*/

#ifndef michael_gautier_rss_data_rss_feed_source_h
#define michael_gautier_rss_data_rss_feed_source_h

#include <string>
#include <cstdint>

namespace gautier_rss_data_read {
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
			int64_t article_count;

			int64_t row_id;

			/*
				Set operations

				last_index is used for application caching
				and indexing strategies.
			*/
			int64_t last_index;

			/*
				Provides diff between what is likely loaded
				from the database on start-up versus what is
				downloaded from the network. This helps the
				application access the start of the range of
				new entries regardless of sort order.
			*/
			int64_t revised_index_start;
			int64_t revised_index_end;
	};

	bool
	check_feed_changed (rss_feed& feed_old, rss_feed& feed_new);

	//Determine best time (in elapsed seconds) to refresh a feed.
	bool
	is_feed_still_fresh (std::string db_file_name, std::string feed_name, bool time_message_stdout);

}
#endif
