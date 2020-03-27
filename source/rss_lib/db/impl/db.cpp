/*
Copyright (C) 2020 Michael Gautier

This source code is free software; you can redistribute it and/or modify it under the terms of the GNU Lesser General Public License as published by the Free Software Foundation; either version 2.1 of the License, or (at your option) any later version.

This source code is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public License along with this library. If not, see <http://www.gnu.org/licenses/>.

Author: Michael Gautier <michaelgautier.wordpress.com>
*/

#include <iostream>
#include <sstream>
#include <string>

#include <libxml/parser.h>
#include <libxml/tree.h>

#include "rss_lib/db/db.hpp"

bool
gautier_rss_database::open_db (const std::string db_file_name, sqlite3** db)
{
	bool success = false;

	const int options = (SQLITE_OPEN_FULLMUTEX | SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE);

	const int open_result = sqlite3_open_v2 (db_file_name.data(), db, options, NULL);

	if (open_result == SQLITE_OK) {
		success = true;
	}

	return success;
}

void
gautier_rss_database::close_db (sqlite3** db)
{
	if (db) {
		sqlite3_close_v2 (*db);
	}

	return;
}

int
gautier_rss_database::create_sql_row (void* generic_object, const int col_count, char** col_values,
                                      char** col_names)
{
	sql_rowset_type* rows = (sql_rowset_type*)generic_object;

	rows->emplace_back (sql_row_type());

	sql_row_type* row = & (rows->back());

	for (int i = 0; i < col_count; i++) {
		const std::string name = col_names[i];
		const std::string value = (col_values[i] ? col_values[i] : "");

		row->insert_or_assign (name, value);
	}

	return 0;
}

void
gautier_rss_database::process_sql_simple (sqlite3** db, const std::string sql_text, sql_rowset_type& rows)
{
	sql_rowset_type* tmp_rows = &rows;

	char* error_message;

	sqlite3_exec (*db, sql_text.data(), create_sql_row, (void*)tmp_rows, &error_message);

	return;
}

void
gautier_rss_database::process_sql_simple (sqlite3** db, const std::string sql_text)
{
	char* error_message;

	sqlite3_exec (*db, sql_text.data(), NULL, NULL, &error_message);

	return;
}

void
gautier_rss_database::process_sql (sqlite3** db, const std::string sql_text,
                                   sql_parameter_list_type& sql_param_values, sql_rowset_type& rows)
{
	sqlite3_stmt* sql_statement;

	sqlite3_prepare_v2 (*db,  sql_text.data(),  -1,  &sql_statement,  NULL);

	int i = 0;

	for (sql_parameter_list_type::value_type& sql_parameter : sql_param_values) {
		i++;
		sqlite3_bind_text (sql_statement, i, sql_parameter.data(), -1, SQLITE_TRANSIENT);
	}

	int sql_result = SQLITE_DONE;

	do {
		sql_result = sqlite3_step (sql_statement);

		if (sql_result == SQLITE_ROW) {
			const int col_count = sqlite3_data_count (sql_statement);

			rows.emplace_back (sql_row_type());

			sql_row_type* row = & (rows.back());

			for (int col_i = 0; col_i < col_count; col_i++) {
				std::string col_name = sqlite3_column_name (sql_statement, col_i);

				std::stringstream col_value;

				col_value << sqlite3_column_text (sql_statement, col_i);

				row->insert_or_assign (col_name, col_value.str());
			}

		}

	} while (sql_result == SQLITE_ROW);

	sqlite3_finalize (sql_statement);

	return;
}

void
gautier_rss_database::process_sql (sqlite3** db, const std::string sql_text,
                                   sql_parameter_list_type& sql_param_values)
{
	sql_rowset_type rows;

	process_sql (db, sql_text, sql_param_values, rows);

	return;
}
