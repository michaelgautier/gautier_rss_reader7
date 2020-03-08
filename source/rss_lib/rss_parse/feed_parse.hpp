/*
Copyright (C) 2020 Michael Gautier

This source code is free software; you can redistribute it and/or modify it under the terms of the GNU Lesser General Public License as published by the Free Software Foundation; either version 2.1 of the License, or (at your option) any later version.

This source code is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public License along with this library. If not, see <http://www.gnu.org/licenses/>.

Author: Michael Gautier <michaelgautier.wordpress.com>
*/

#ifndef michael_gautier_rss_data_parse_api_h
#define michael_gautier_rss_data_parse_api_h

#include <vector>
#include <string>

#include "rss_lib/rss/rss_article.hpp"

namespace gautier_rss_data_parse {
	void
	get_feed_lines (const std::string feed_data, std::vector<gautier_rss_data_read::rss_article>& feed_lines);

	void
	save_feed_data_to_file (const std::string file_name, const std::string ext, const std::string file_data);

	void
	get_feed_data_from_file (const std::string file_name, const std::string ext, std::string& file_data);
}
#endif
