/*
Copyright (C) 2020 Michael Gautier

This source code is free software; you can redistribute it and/or modify it under the terms of the GNU Lesser General Public License as published by the Free Software Foundation; either version 2.1 of the License, or (at your option) any later version.

This source code is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public License along with this library. If not, see <http://www.gnu.org/licenses/>.

Author: Michael Gautier <michaelgautier.wordpress.com>
*/

#include "rss_lib/rss/rss_article.hpp"

bool
gautier_rss_data_read::validate_feed_info_missing (const std::string feed_name, const std::string feed_url)
{
	bool is_missing = false;

	if (feed_name.empty() || feed_url.empty()) {
		is_missing = true;
	}

	return is_missing;
}

bool
gautier_rss_data_read::indicates_html (const std::string& text)
{
	bool probably_has_html = false;

	const size_t t1_pos = text.find ("</");
	const size_t t2_pos = text.find ("&lt;/");
	const size_t t3_pos = text.find ("/>");
	const size_t t4_pos = text.find ("/&gt;");

	const size_t start = 0;

	if (t1_pos >= start || t2_pos >= start || t3_pos >= start || t4_pos >= start) {
		probably_has_html = true;
	}

	return probably_has_html;
}
