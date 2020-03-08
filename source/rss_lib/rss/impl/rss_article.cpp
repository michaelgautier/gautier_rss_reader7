/*
Copyright (C) 2020 Michael Gautier

This source code is free software; you can redistribute it and/or modify it under the terms of the GNU Lesser General Public License as published by the Free Software Foundation; either version 2.1 of the License, or (at your option) any later version.

This source code is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public License along with this library. If not, see <http://www.gnu.org/licenses/>.

Author: Michael Gautier <michaelgautier.wordpress.com>
*/

#include "rss_lib/rss/rss_article.hpp"

void
gautier_rss_data_read::clear_feed_data_all (rss_article& feed_data)
{
	feed_data.feed_name = "";
	feed_data.headline = "";
	feed_data.article_date = "";
	feed_data.article_summary = "";
	feed_data.article_text = "";
	feed_data.url = "";

	return;
}

void
gautier_rss_data_read::clear_feed_data_keep_name (rss_article& feed_data)
{
	std::string feed_name = feed_data.feed_name;

	clear_feed_data_all (feed_data);

	feed_data.feed_name = feed_name;

	return;
}

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

	const int_fast32_t t1_pos = text.find ("</");
	const int_fast32_t t2_pos = text.find ("&lt;/");
	const int_fast32_t t3_pos = text.find ("/>");
	const int_fast32_t t4_pos = text.find ("/&gt;");

	if (t1_pos >= 0 || t2_pos >= 0 || t3_pos >= 0 || t4_pos >= 0) {
		probably_has_html = true;
	}

	return probably_has_html;
}
