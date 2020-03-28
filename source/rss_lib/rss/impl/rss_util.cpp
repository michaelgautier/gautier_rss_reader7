/*
Copyright (C) 2020 Michael Gautier

This source code is free software; you can redistribute it and/or modify it under the terms of the GNU Lesser General Public License as published by the Free Software Foundation; either version 2.1 of the License, or (at your option) any later version.

This source code is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public License along with this library. If not, see <http://www.gnu.org/licenses/>.

Author: Michael Gautier <michaelgautier.wordpress.com>
*/

#include <cstddef>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <ctime>
#include <cuchar>
#include <cwchar>
#include <iomanip>
#include <iostream>
#include <sstream>
#include <string>

#include "rss_lib/db/db.hpp"
#include "rss_lib/rss/rss_util.hpp"

int_fast32_t
gautier_rss_util::get_time_difference_in_seconds (const std::string date1, const std::string date2)
{
	//See SQLite documentation: SQL As Understood By SQLite - Date And Time Functions

	const std::string db_file_name = ":memory:";
	int_fast32_t seconds = 0;

	namespace ns_db = gautier_rss_database;

	sqlite3* db = nullptr;
	ns_db::open_db (db_file_name, &db);

	ns_db::sql_rowset_type rows;
	const std::string sql_text = "SELECT ABS(STRFTIME('%s', @date1) - strftime('%s', @date2)) AS result;";

	std::string datetime1 = date1;
	std::string datetime2 = date2;

	if (date1.empty()) {
		datetime1 = get_current_date_time_utc();
	}

	if (date2.empty()) {
		datetime2 = get_current_date_time_utc();
	}

	ns_db::sql_parameter_list_type params = {
		datetime1,
		datetime2
	};

	ns_db::process_sql (&db, sql_text, params, rows);

	for (ns_db::sql_row_type row : rows) {
		for (ns_db::sql_row_type::value_type field : row) {
			if (field.first == "result") {
				std::string result = field.second;

				if (result.empty() == false) {
					seconds = std::stoi (result);
				}
			}
		}
	}

	ns_db::close_db (&db);

	return seconds;
}

std::string
gautier_rss_util::get_current_date_time_utc()
{
	/*
		Gets the current date and time.
		Provides the date/time in a format understood by many
		systems such as SQLite.
	*/
	std::string datetime;
	{
		std::time_t result = std::time (nullptr);

		std::stringstream strout;

		/* MinGW Windows Cross Compile does not understand the short-hand: %F %T */
		strout << std::put_time (std::gmtime (&result), "%Y-%m-%d %H:%M:%S");

		datetime = strout.str();
	}

	return datetime;
}

std::string
gautier_rss_util::get_current_date_time_local()
{
	/*
		Gets the current date and time.
		Provides the date/time in a format understood by many
		systems such as SQLite as the local time.
	*/
	std::string datetime;
	{
		std::time_t result = std::time (nullptr);

		std::stringstream strout;

		/* MinGW Windows Cross Compile does not understand the short-hand: %F %T */
		strout << std::put_time (std::localtime (&result), "%Y-%m-%d %H:%M:%S");

		datetime = strout.str();
	}

	return datetime;
}


/*
	LOWER CASE all characters.

	Expects a null terminated string.

	Converts all characters to lower case.
*/
void
gautier_rss_util::convert_chars_to_lower_case_string (const char* chars, std::string& str)
{
	const std::string detail = chars;
	const size_t char_count = detail.size();

	for (size_t char_index = 0; char_index < char_count; char_index++) {
		const char character = detail.at (char_index);

		str.push_back ((char) (std::tolower (character)));
	}

	return;
}

/*
	Chars to Numeric
*/
bool
gautier_rss_util::convert_chars_to_string (const char* chars, std::string& str)
{
	const std::string detail = chars;
	const size_t char_count = detail.size();

	for (size_t char_index = 0; char_index < char_count; char_index++) {
		const char character = detail.at (char_index);

		str.push_back ((char) (character));
	}

	return (str.empty() == false);
}

bool
gautier_rss_util::convert_chars_to_int_string (const char* chars, std::string& str)
{
	const std::string detail = chars;
	const size_t char_count = detail.size();

	for (size_t char_index = 0; char_index < char_count; char_index++) {
		const char character = detail.at (char_index);

		if (std::isdigit (character)) {
			str.push_back ((char) (character));
		}
	}

	return (str.empty() == false);
}
