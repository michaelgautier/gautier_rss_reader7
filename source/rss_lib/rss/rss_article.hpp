/*
Copyright (C) 2020 Michael Gautier

This source code is free software; you can redistribute it and/or modify it under the terms of the GNU Lesser General Public License as published by the Free Software Foundation; either version 2.1 of the License, or (at your option) any later version.

This source code is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public License along with this library. If not, see <http://www.gnu.org/licenses/>.

Author: Michael Gautier <michaelgautier.wordpress.com>
*/

#ifndef michael_gautier_rss_data_read_article_h
#define michael_gautier_rss_data_read_article_h

#include <string>
#include <string_view>

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

	bool
	validate_feed_info_missing (std::string_view feed_name, std::string_view feed_url);

	bool
	indicates_html (std::string_view& text);
}
#endif
