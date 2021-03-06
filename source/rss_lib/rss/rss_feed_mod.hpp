/*
Copyright (C) 2020 Michael Gautier

This source code is free software; you can redistribute it and/or modify it under the terms of the GNU Lesser General Public License as published by the Free Software Foundation; either version 2.1 of the License, or (at your option) any later version.

This source code is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public License along with this library. If not, see <http://www.gnu.org/licenses/>.

Author: Michael Gautier <michaelgautier.wordpress.com>
*/

#ifndef michael_gautier_rss_data_read_feed_mod_h
#define michael_gautier_rss_data_read_feed_mod_h

#include <string>
#include <cstdint>

namespace gautier_rss_data_read {
	enum class
	rss_feed_mod_status
	{
		none,
		remove,
		change,
		insert
	};

	struct rss_feed_mod
	{
		public:
			std::string feed_name;
			rss_feed_mod_status status = rss_feed_mod_status::none;
			int64_t row_id = -1;
	};
}
#endif
