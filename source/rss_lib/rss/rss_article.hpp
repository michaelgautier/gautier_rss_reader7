/*
Copyright (C) 2019 Michael Gautier

This source code is free software; you can redistribute it and/or modify it under the terms of the GNU Lesser General Public License as published by the Free Software Foundation; either version 2.1 of the License, or (at your option) any later version.

This source code is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public License along with this library. If not, see <http://www.gnu.org/licenses/>.

Author: Michael Gautier <michaelgautier.wordpress.com>
*/

#ifndef michael_gautier_rss_data_read_article_h
#define michael_gautier_rss_data_read_article_h

#include <string>

namespace gautier_rss_data_read {
	struct rss_article
	{
		public:
			std::string feed_name;
			std::string headline;
			std::string article_date;
			std::string article_summary;
			std::string article_text;
			std::string url;
	};

	void
	clear_feed_data_all (rss_article& feed_data);

	void
	clear_feed_data_keep_name (rss_article& feed_data);

	bool
	validate_feed_info_missing (std::string feed_name, std::string feed_url);
}
#endif
