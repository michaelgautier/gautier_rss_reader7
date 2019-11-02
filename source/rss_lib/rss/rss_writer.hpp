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
	void
	initialize_db (std::string db_file_name);

	void
	set_feed_name (std::string db_file_name,
	               std::string feed_name,
	               std::string feed_url);

	void
	set_feed_headline (std::string db_file_name,
	                   gautier_rss_data_read::rss_article&
	                   article);

	void
	update_rss_feeds (std::string db_file_name);

	void
	update_rss_db_from_rss_xml (std::string db_file_name, std::string feed_name, std::string feed_url);

	void
	update_rss_xml_from_network (std::string db_file_name, std::string feed_name, std::string feed_url);

	void
	update_rss_xml_db_from_network (std::string db_file_name, std::string feed_name, std::string feed_url);

	void
	update_rss_db_from_network (std::string db_file_name, std::string feed_name, std::string feed_url);
}
#endif
