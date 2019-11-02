/*
Copyright (C) 2019 Michael Gautier

This source code is free software; you can redistribute it and/or modify it under the terms of the GNU Lesser General Public License as published by the Free Software Foundation; either version 2.1 of the License, or (at your option) any later version.

This source code is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public License along with this library. If not, see <http://www.gnu.org/licenses/>.

Author: Michael Gautier <michaelgautier.wordpress.com>
*/

#ifndef michael_gautier_rss_database_api_h
#define michael_gautier_rss_database_api_h

#include <vector>
#include <string>
#include  <unordered_map>

#include <sqlite3.h>

namespace gautier_rss_database {
	using sql_row_type = std::unordered_map < std::string, std::string >;
	using sql_rowset_type = std::vector<sql_row_type>;
	using sql_parameter_list_type = std::vector<std::string>;

	bool
	open_db (std::string& db_file_name, sqlite3** db);

	extern "C" int
	create_sql_row (void* generic_object, int col_count, char** col_values, char** col_names);

	void
	process_sql_simple (sqlite3** db, std::string& sql_text, sql_rowset_type& rows);

	void
	process_sql_simple (sqlite3** db, std::string& sql_text);

	void
	close_db (sqlite3** db);

	void
	process_sql (sqlite3** db, std::string& sql_text, sql_parameter_list_type& sql_param_values,
	             sql_rowset_type& rows);

	void
	process_sql (sqlite3** db, std::string& sql_text, sql_parameter_list_type& sql_param_values);

}
#endif
