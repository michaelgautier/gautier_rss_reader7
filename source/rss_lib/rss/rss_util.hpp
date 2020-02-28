/*
Copyright (C) 2020 Michael Gautier

This source code is free software; you can redistribute it and/or modify it under the terms of the GNU Lesser General Public License as published by the Free Software Foundation; either version 2.1 of the License, or (at your option) any later version.

This source code is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public License along with this library. If not, see <http://www.gnu.org/licenses/>.

Author: Michael Gautier <michaelgautier.wordpress.com>
*/

#ifndef michael_gautier_rss_util_source_h
#define michael_gautier_rss_util_source_h

#include <string>
#include <cstdint>
#include <charconv>

namespace gautier_rss_util {
	std::string
	get_current_date_time_utc();

	std::string
	get_current_date_time_local();

	//See SQLite documentation: SQL As Understood By SQLite - Date And Time Functions
	int_fast32_t
	get_time_difference_in_seconds (std::string date1, std::string date2);

	void
	convert_chars_to_lower_case_string (char* chars, std::string& str);

	bool
	convert_chars_to_string (const char* chars, std::string& str);

	bool
	convert_chars_to_int_string (const char* chars, std::string& str);

	template<typename T> T
	convert_chars_to_number (const char* chars, T default_error_value)
	{
		const std::string detail = chars;
		const std::string::size_type char_count = detail.size();

		T n = default_error_value;

		std::from_chars_result res = std::from_chars (detail.data(), detail.data() + char_count, n);

		if (res.ec == std::errc::invalid_argument || res.ec == std::errc::result_out_of_range) {
			n = default_error_value;
		}

		return n;
	}

	//template int64_t convert_chars_to_number<int64_t>
	//(const char* chars, int64_t default_error_value);
}
#endif
