/*
Copyright (C) 2020 Michael Gautier

This source code is free software; you can redistribute it and/or modify it under the terms of the GNU Lesser General Public License as published by the Free Software Foundation; either version 2.1 of the License, or (at your option) any later version.

This source code is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public License along with this library. If not, see <http://www.gnu.org/licenses/>.

Author: Michael Gautier <michaelgautier.wordpress.com>
*/
#include <ctime>
#include <iomanip>
#include <sstream>

#include "rss_lib/db/db.hpp"
#include "rss_lib/rss/rss_util.hpp"

int_fast32_t
gautier_rss_util::get_time_difference_in_seconds (std::string date1, std::string date2)
{
	//See SQLite documentation: SQL As Understood By SQLite - Date And Time Functions

	std::string db_file_name = ":memory:";
	int_fast32_t seconds = 0;

	namespace ns_db = gautier_rss_database;

	sqlite3* db = NULL;
	ns_db::open_db (db_file_name, &db);

	ns_db::sql_rowset_type rows;
	std::string sql_text = "select abs(strftime('%s', @date1) - strftime('%s', @date2)) as result;";

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

				seconds = std::stoi (result);
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

		strout << std::put_time (std::gmtime (&result), "%F %T") << std::ends;

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

		strout << std::put_time (std::localtime (&result), "%F %T") << std::ends;

		datetime = strout.str();
	}

	return datetime;
}
