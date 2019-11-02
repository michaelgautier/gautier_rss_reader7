/*
Copyright (C) 2019 Michael Gautier

This source code is free software; you can redistribute it and/or modify it under the terms of the GNU Lesser General Public License as published by the Free Software Foundation; either version 2.1 of the License, or (at your option) any later version.

This source code is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public License along with this library. If not, see <http://www.gnu.org/licenses/>.

Author: Michael Gautier <michaelgautier.wordpress.com>
*/

#ifndef michael_gautier_rss_download_h
#define michael_gautier_rss_download_h

#include <string>

namespace gautier_rss_data_read {
	/*
		Merely a wrapper for curl.

		Gathers html/xml content into a string variable.

		The string data can then be parsed.
	*/
	void
	download_rss_feed (std::string feed_url, std::string& headlines);
}
#endif
