/*
Copyright (C) 2019 Michael Gautier

This source code is free software; you can redistribute it and/or modify it under the terms of the GNU Lesser General Public License as published by the Free Software Foundation; either version 2.1 of the License, or (at your option) any later version.

This source code is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public License along with this library. If not, see <http://www.gnu.org/licenses/>.

Author: Michael Gautier <michaelgautier.wordpress.com>
*/

#include <iostream>

#include "rss_lib/rss/rss_writer.hpp"
#include "rss_lib/rss/rss_reader.hpp"

#include "external/argtable/argtable3.h"

static int
cleanup_argtable (void** argtable, int exit_code);

int
main (int argc, char** argv)
{
	/*
		PRIMARY RSS PROGRAM

		---------------------

		NOTE:

		The program can be ran with no options. When ran without options, it functions as a typical RSS download program.

		Earlier versions showed all the feeds on screen. That feature was good for judging perceived performance but was
		of minimal utility otherwise. Now, the program executes logic, updates data files/database and exits.

		DESCRIPTION:

		Program has various modes to process RSS feed data. Most modes are useful for diagnostic purposes.

		When the RSS database file has one or more RSS feed/url entries, the program should be ran without options
		specified. That will cause the program to automatically update the RSS database.

		The best way to get started with an empty database is to apply option #4. After option #4 has been invoked
		for one or more RSS feed/url entries, the program can be ran without options.

		New RSS feed sources can be added using option #4.

		---------------------

		PRIMARY PURPOSE:	DOWNLOAD RSS feeds to an SQLite database.

		Four secondary operation modes:

			1)	Apply RSS feed data from an offline RSS feed XML file to the database.

			2)	Download RSS feed from an HTTP source and save to an XML file.

			3)	Combine operations 1 and 2.

			4)	Download RSS feed from an HTTP source and save to the database.

		A single primary operation mode:

			5)	Execute normal RSS feed update functions. PRIMARY OPERATION.

		The 4 secondary modes are developed progressively as reference sequences for the 5th mode.
	*/
	struct arg_lit* cli_op_xmldb;		//1
	struct arg_lit* cli_op_http_xml_only;	//2
	struct arg_lit* cli_op_http_xmldb;	//3
	struct arg_lit* cli_op_http_db;		//4
	struct arg_lit* cli_op_self_update;	//5

	struct arg_str* cli_alt_db_name;	//Alternative Database Name
	struct arg_str* cli_rss_feed_name;	//RSS Feed Name
	struct arg_str* cli_rss_feed_url;	//RSS Feed URL

	/*
		Date time values
	*/
	struct arg_lit* cli_op_datetime_sec;	//datetime seconds option
	struct arg_str* cli_datetime1;		//datetime value 1
	struct arg_str* cli_datetime2;		//datetime value 2

	/*
		Service access limits
	*/
	struct arg_lit* cli_check_time_limit;

	/*
		Linux command-line program minimum options.
	*/
	struct arg_lit* cli_help;
	struct arg_lit* cli_version;
	struct arg_lit* cli_verbose;
	struct arg_end* cli_end;

	/*
		Command-line options table. Go to argtable.org for details.
	*/
	void* argtable[] = {
		/*
			TABLE BEGIN

			Common Linux command-line options
		*/
		cli_help		= arg_litn (NULL, "help", 0, 1, "display this help and exit"),
		cli_version 		= arg_litn (NULL, "version", 0, 1, "display version info and exit"),
		cli_verbose    		= arg_litn (NULL, "verbose", 0, 1, "verbose output"),

		/*Primary options*/
		cli_op_xmldb    	= arg_litn /*1*/ (NULL, "xmldb", 0, 1, "Use generated offline rss xml to load rss database"),
		cli_op_http_xml_only   	= arg_litn /*2*/ (NULL, "http-xml-only", 0, 1, "Download rss feed to an offline rss xml file"),
		cli_op_http_xmldb    	= arg_litn /*3*/ (NULL, "http-xmldb", 0, 1, "Combines xmldb + http-xml-only"),
		cli_op_http_db    	= arg_litn /*4*/ (NULL, "http-db-only", 0, 1, "Download rss feed directly to rss database"),
		cli_op_self_update    	= arg_litn /*5*/ (NULL, "self-update", 0, 1, "[DEFAULT] Download updated content for feeds in the database"),

		/*Additional options*/
		cli_alt_db_name		= arg_strn (NULL, "alt-dbname", "<string>", 0, 1, "Useful for testing, i.e. rss_test.db"),
		cli_rss_feed_name	= arg_strn (NULL, "rss-feedname", "<string>", 0, 1, "Name of the RSS Feed"),
		cli_rss_feed_url	= arg_strn (NULL, "rss-url", "<string>", 0, 1, "Network address containing latest feed content for rss-feedname"),

		cli_op_datetime_sec 	= arg_litn /*6*/ (NULL, "datetime-sec", 0, 1, "Use SQLite to obtain the seconds that passed between two date time values"),
		cli_check_time_limit	= arg_litn /*7*/ (NULL, "check-timelimit", 0, 1, "true/false to see a diagnostic output of the time limit"),
		cli_datetime1		= arg_strn (NULL, "datetime1", "<string>", 0, 1, "datetime-1"),
		cli_datetime2		= arg_strn (NULL, "datetime2", "<string>", 0, 1, "datetime-2"),

		/*
			TABLE END
		*/
		cli_end     = arg_end (128),
	};

	std::string cli_progname = "RSS Downloader";

	std::cout << cli_progname << "\n";

	/*
		ERROR REPORTING

		Assume something went wrong. Prove by the end of execution all is good.

		Some error reporting logic maintained verbatim from argtable3 examples on argtable.org
	*/
	int exit_code = 1;

	int cli_parse_errors = arg_parse (argc, argv, argtable);

	if (arg_nullcheck (argtable) != 0) {
		std::cout << "line: " << __LINE__ << ". CLI options parse table contains one or more NULL entries.\n";

		return cleanup_argtable (argtable, exit_code);
	}

	if (cli_help->count > 0) {
		printf ("Usage: %s", cli_progname.data());

		arg_print_syntax (stdout, argtable, "\n");
		arg_print_glossary (stdout, argtable, "  %-25s %s\n");

		exit_code = 0;

		return cleanup_argtable (argtable, exit_code);
	}

	/* If the parser returned any errors then display them and exit */
	if (cli_parse_errors > 0) {
		/* Display the error details contained in the arg_end struct.*/
		arg_print_errors (stdout, cli_end, cli_progname.data());

		printf ("Try '%s --help' for more information.\n", cli_progname.data());

		return cleanup_argtable (argtable, exit_code);
	}

	bool verbose = cli_verbose->count > 0;

	namespace ns_read = gautier_rss_data_read;

	namespace ns_write = gautier_rss_data_write;

	std::string db_file_name = "rss.db";

	if (cli_alt_db_name->count > 0) {
		std::string alt_db_file_name = *cli_alt_db_name->sval;

		if (alt_db_file_name.empty()) {
			std::cout << "Alternate db name was suggested. However, the value supplied is blank or not recognized.\n";
			std::cout << "Need value --alt-dbname=\"value\"\n";

			return cleanup_argtable (argtable, exit_code);
		}
	}

	if (db_file_name.empty() == false) {
		std::cout << "Open database: " << db_file_name << "\n";

		ns_write::initialize_db (db_file_name);
	}

	std::string feed_name;
	std::string feed_url;

	if (cli_rss_feed_name->count > 0) {
		feed_name = *cli_rss_feed_name->sval;
	}

	if (cli_rss_feed_url->count > 0) {
		feed_url = *cli_rss_feed_url->sval;
	}

	std::string feed_retrieve_limit_hrs = "2";
	std::string feed_retention_days = "-1";

	std::string check_feed_retrieve_limit_text;

	bool http_scenario = false;
	bool http_feed_info_good = false;

	if (cli_check_time_limit->count > 0) {
		if (feed_name.empty() == false) {
			bool is_feed_still_fresh = ns_read::is_feed_stale (db_file_name, feed_name);

			std::cout << "is feed network retrieval permitted: " << is_feed_still_fresh << "\n";
		} else {
			std::cout << "feed name missing see --rss-feedname\n";
			exit_code = 1;
		}
	}

	/*
		Automatic RSS update process.

		Begin by checking for scenario #5.
		When the database has one or more feed name/feed url pairs, it can be used
		in conjunction with application logic to automatically retrieve rss feed data
		into the database.

		The other scenarios assist with testing and inspecting RSS data to refine program logic.

		5)	Process known feed sources listed in database
	*/

	/*
		Normal RSS update process.
	*/
	if (cli_op_self_update->count > 0) {
		std::cout << "Get latest RSS feed data and update database\n";

		ns_write::update_rss_feeds (db_file_name);

		if (verbose) {
			std::cout << "RSS feed update process finished.\n";
		}
	}

	/*
		1)	Copy from XML file to database
	*/

	else if (cli_op_xmldb->count > 0) {
		/*
			Primary goal is to validate XML parsing and core database insert.
		*/
		if (feed_name.empty()) {
			std::cout << "ERROR: Need value --rss-feedname=\"value\" for --xmldb\n";

			return cleanup_argtable (argtable, exit_code);
		} else {
			std::cout << "Read offline XML file into database: \"" << feed_name << "\" \"" << feed_url << "\"\n";

			ns_write::update_rss_db_from_rss_xml (db_file_name, feed_name, feed_url, feed_retrieve_limit_hrs,
			                                      feed_retention_days);
		}

		if (verbose) {
			std::cout << "XML File -> Database process finished.\n";
		}
	}

	/*
		2)	Download from HTTP/XML to file only
	*/

	else if (cli_op_http_xml_only->count > 0) {
		http_scenario = true;

		http_feed_info_good =
		    (ns_read::validate_feed_info_missing (feed_name, feed_url) == false);

		if (http_feed_info_good) {
			std::cout << "Save RSS feed to offline XML file: \"" << feed_name << "\" \"" << feed_url << "\"\n";

			ns_write::update_rss_xml_from_network (db_file_name, feed_name, feed_url, feed_retrieve_limit_hrs,
			                                       feed_retention_days);

			if (verbose) {
				std::cout << "HTTP/XML Feed -> XML File process finished.\n";
			}
		}
	}

	/*
		3)	Download from HTTP/XML to file and then database
	*/

	else if (cli_op_http_xmldb->count > 0) {
		http_scenario = true;

		http_feed_info_good =
		    (ns_read::validate_feed_info_missing (feed_name, feed_url) == false);

		if (http_feed_info_good) {
			std::cout << "Save RSS feed to offline XML file and database: \"" << feed_name << "\" \"" << feed_url << "\"\n";

			ns_write::update_rss_xml_db_from_network (db_file_name, feed_name, feed_url, feed_retrieve_limit_hrs,
			        feed_retention_days);

			if (verbose) {
				std::cout << "HTTP/XML Feed -> [XML File, Database] process finished.\n";
			}
		}
	}

	/*
		4)	Download from HTTP/XML to database only
	*/

	else if (cli_op_http_db->count > 0) {
		http_scenario = true;

		http_feed_info_good =
		    (ns_read::validate_feed_info_missing (feed_name, feed_url) == false);

		if (http_feed_info_good) {
			std::cout << "Save RSS feed to database: \"" << feed_name << "\" \"" << feed_url << "\"\n";

			ns_write::update_rss_db_from_network (db_file_name, feed_name, feed_url, feed_retrieve_limit_hrs,
			                                      feed_retention_days);

			if (verbose) {
				std::cout << "HTTP/XML Feed -> Database process finished.\n";
			}
		}
	}

	/*
		Get Seconds from two dates times.
	*/

	else if (cli_op_datetime_sec->count > 0) {
		std::string datetime1 = *cli_datetime1->sval;
		std::string datetime2 = *cli_datetime2->sval;

		int seconds = ns_read::get_time_difference_in_seconds (datetime1, datetime2);

		std::cout << "elapsed seconds: " << seconds << "\n";
	}

	/*
		Automatic RSS update process.

		5)	Process known feed sources listed in database
	*/

	else {
		std::cout << "Get latest RSS feed data and update database\n";

		ns_write::update_rss_feeds (db_file_name);

		if (verbose) {
			std::cout << "RSS feed update process finished.\n";
		}
	}

	if (http_scenario && http_feed_info_good == false) {
		std::cout << "ERROR: Need value --rss-feedname=\"value\" for all --http-xxx options\n";
		std::cout << "ERROR: Need value --rss-url=\"value\" for all --http-xxx options\n";

		return cleanup_argtable (argtable, exit_code);
	}

	exit_code = 0;

	return cleanup_argtable (argtable, exit_code);
}

int
cleanup_argtable (void** argtable, int exit_code)
{
	if (exit_code != 0) {
		std::cout << "Exiting program with one or more errors. Exit code: " << exit_code << "\n";
	} else {
		std::cout << "done.\n";
	}

	/* deallocate each non-null entry in argtable[] */
	arg_freetable (argtable, sizeof (argtable) / sizeof (argtable[0]));

	return exit_code;
}
