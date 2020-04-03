/*
Copyright (C) 2020 Michael Gautier

This source code is free software; you can redistribute it and/or modify it under the terms of the GNU Lesser General Public License as published by the Free Software Foundation; either version 2.1 of the License, or (at your option) any later version.

This source code is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public License along with this library. If not, see <http://www.gnu.org/licenses/>.

Author: Michael Gautier <michaelgautier.wordpress.com>
*/

#ifndef michael_gautier_rss_data_read_api_h
#define michael_gautier_rss_data_read_api_h

#include <cstdint>
#include <string>
#include <vector>

#include "rss_article.hpp"
#include "rss_feed.hpp"

namespace gautier_rss_data_read {
	using feeds_list_type = std::vector<rss_feed>;
	using articles_list_type = std::vector<gautier_rss_data_read::rss_article>;
	using headlines_list_type = std::vector<std::string>;

	using headline_range_type = std::pair<int64_t, int64_t>;

	headline_range_type
	acquire_headline_range (const std::string& feed_name, feed_by_name_type& feeds, const int64_t& headline_max);

	int64_t
	get_article_row_id (const std::string db_file_name, const std::string article_url);

	int64_t
	get_feed_article_max_row_id (const std::string db_file_name, const std::string feed_name);

	void
	get_feed_articles_after_row_id (const std::string db_file_name, const std::string feed_name,
	                                articles_list_type& headlines, const bool descending, const int64_t row_id);

	void
	get_feed_articles (const std::string db_file_name, const std::string feed_name, articles_list_type& headlines,
	                   const bool descending);

	void
	get_feed_article_summary (const std::string db_file_name, const std::string feed_name,
	                          const std::string headline, rss_article& article);

	void
	get_feed_by_row_id (const std::string db_file_name, const int64_t row_id, rss_feed& feed);

	void
	get_feed (const std::string db_file_name, const std::string feed_name, rss_feed& feed);

	int64_t
	get_feed_headline_count (const std::string db_file_name, const std::string feed_name);

	void
	get_feed_headlines_after_row_id (const std::string db_file_name, const std::string feed_name,
	                                 headlines_list_type& headlines, const bool descending, const int64_t row_id);

	void
	get_feed_headlines_by_range (const std::string db_file_name, const std::string feed_name,
	                             headlines_list_type& headlines, const bool descending, const headline_range_type range);

	void
	get_feed_headlines (const std::string db_file_name, const std::string feed_name, headlines_list_type& headlines,
	                    const bool descending);

	void
	get_feeds (const std::string db_file_name, feeds_list_type& feeds);

	int64_t
	get_row_id (const std::string db_file_name, const std::string feed_url);

	bool
	headline_range_valid (const headline_range_type& range);

}
#endif
