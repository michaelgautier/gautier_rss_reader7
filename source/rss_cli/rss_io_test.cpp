/*
Copyright (C) 2019 Michael Gautier

This source code is free software; you can redistribute it and/or modify it under the terms of the GNU Lesser General Public License as published by the Free Software Foundation; either version 2.1 of the License, or (at your option) any later version.

This source code is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public License along with this library. If not, see <http://www.gnu.org/licenses/>.

Author: Michael Gautier <michaelgautier.wordpress.com>
*/

#include <iostream>

#include "rss_lib/rss/rss_feed.hpp"
#include "rss_lib/rss/rss_reader.hpp"
#include "rss_lib/rss/rss_writer.hpp"
#include "rss_lib/rss_parse/feed_parse.hpp"
#include "rss_lib/rss_download/feed_download.hpp"

#include "external/argtable/argtable3.h"

static bool
feed_expire_time_enabled = true;

static void
flatten_rss_xml_to_text (std::string feed_name);

static int
cleanup_argtable (void** argtable, int exit_code);

int
main (int argc, char** argv)
{
	/*
		RSS I/O TEST PROGRAM

		NOTE:

		When you have something as involved as xml parsing and conversion to another format, you need to validate it
		without the distraction of a larger program. This test program accomplishes this for the core parsing module.

		DESCRIPTION:

		An XML file has RSS feed data inside. This program pulls the data into the program's memory.

		The program then translates the XML to friendlier programming format.

		The flattened out data is applied to an output file used to verify the final quality of the XML parse.

		Run the program.	New .TXT file should match the essential contents of the original .XML file.

		USAGE:

		The primary option is option #3 described in the next section. The other options test core I/O functionality
		used by option #3. Over time, it makes sense to evolve this functionality to improve I/O processing.

		---------------------

		PRIMARY PURPOSE:	TEST core RSS Feed data processing functions.

		Four secondary operation modes:

			1)	File in and out:	Read the contents of a file and write those contents out to another file.

			2)	RSS Feed Download:	Request document at a given network address and save the contents to a file.

			3)	Flatten RSS XML:	Parse a file in RSS XML format into a file that is easier for people to read.
	*/
	struct arg_lit* cli_op_file_in_and_out;	//1
	struct arg_lit* cli_op_rss_download;	//2
	struct arg_lit* cli_op_flat_rss;	//3

	struct arg_str* cli_rss_feed_name;	//RSS Feed Name
	struct arg_str* cli_rss_feed_url;	//RSS Feed URL

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
		cli_op_file_in_and_out  = arg_litn /*1*/ (NULL, "fifo", 0, 1, "Duplicate a file to test basic file I/O"),
		cli_op_rss_download   	= arg_litn /*2*/ (NULL, "get-rss", 0, 1, "Download rss feed to an offline rss xml file"),
		cli_op_flat_rss    	= arg_litn /*3*/ (NULL, "flat-rss-file", 0, 1, "Convert an RSS XML file to flat data file"),

		/*Additional options*/
		cli_rss_feed_name	= arg_strn (NULL, "rss-feedname", "<string>", 0, 1, "Name of the RSS Feed"),
		cli_rss_feed_url	= arg_strn (NULL, "rss-url", "<string>", 0, 1, "Network address containing latest feed content for rss-feedname"),

		/*
			TABLE END
		*/
		cli_end     = arg_end (128),
	};

	std::string cli_progname = "RSS I/O TESTER";

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

	namespace ns_parse = gautier_rss_data_parse;

	std::string feed_name;
	std::string feed_url;

	if (cli_rss_feed_name->count > 0) {
		feed_name = *cli_rss_feed_name->sval;
	}

	if (cli_rss_feed_url->count > 0) {
		feed_url = *cli_rss_feed_url->sval;
	}

	/*
		1)	File in and out
	*/

	if (cli_op_file_in_and_out->count > 0) {
		/*
			Read the contents of a file and write those contents out to another file.
		*/
		if (feed_name.empty()) {
			std::cout << "ERROR: Need value --rss-feedname=\"value\" for --xmldb\n";

			return cleanup_argtable (argtable, exit_code);
		} else {
			std::cout << "Expect an input file with the name: \"" << feed_name << "\"" << ".xml\"\n";

			std::string file_data;

			ns_parse::get_feed_data_from_file (feed_name, ".xml", file_data);

			std::string feed_dup = feed_name + " TEST -- DUPLICATE";

			ns_parse::save_feed_data_to_file (feed_dup, ".xml", file_data);

			if (verbose) {
				std::cout << "Check for file: \"" << feed_dup << "\"\n";
			}
		}

		if (verbose) {
			std::cout << "XML File -> Flat file process finished.\n";
		}
	}

	/*
		2)	RSS Feed Download
	*/

	else if (cli_op_rss_download->count > 0) {
		/*
			Request document at a given network address and save the contents to a file.
		*/
		bool http_feed_info_good =
		    (ns_read::validate_feed_info_missing (feed_name, feed_url) == false);

		if (http_feed_info_good) {
			std::cout << "Save RSS feed to offline XML file: \"" << feed_name << "\" \"" << feed_url << "\"\n";

			std::string headlines;

			std::string db_file_name = "rss_test.db";

			gautier_rss_data_write::initialize_db (db_file_name);
			bool is_feed_still_fresh = gautier_rss_data_read::is_feed_still_fresh (db_file_name, feed_name,
			                           feed_expire_time_enabled);

			if (is_feed_still_fresh == false) {
				ns_read::download_rss_feed (feed_url, headlines);

				if (headlines.empty() == false) {
					ns_parse::save_feed_data_to_file (feed_name, ".xml", headlines);
				} else {
					std::cout << "empty result\n";

					return cleanup_argtable (argtable, exit_code);
				}

				if (verbose) {
					std::cout << "Downloaded data, file contents size: " << headlines.size() << "\n";
				}
			} else {
				std::cout << "Too early to download from remote third-party.\n";
			}

			gautier_rss_data_write::de_initialize_db (db_file_name);
		} else {
			std::cout << "get-rss: Missing either feed name [rss-feedname] or feed url [rss-url]\n";
			return cleanup_argtable (argtable, exit_code);
		}

		if (verbose) {
			std::cout << "HTTP/XML Feed -> XML File process finished.\n";
		}
	}

	/*
		3)	Flatten RSS XML
	*/

	else if (cli_op_flat_rss->count > 0) {
		/*
			Parse a file in RSS XML format into a file that is easier for people to read.
		*/
		std::cout << "Transform XML file into human comfortable flat file format: \"" << feed_name << "\"\n";

		if (feed_name.empty()) {
			std::cout << "ERROR: Need value --rss-feedname=\"value\" for --xmldb\n";

			return cleanup_argtable (argtable, exit_code);
		} else {

			flatten_rss_xml_to_text (feed_name);
		}

		if (verbose) {
			std::cout << "RSS XML File -> RSS Flat File process finished.";
		}
	}

	/*
		***	else-error	****

		ERROR

		Program requires explicit input. Default behvior not applicable for test program.
	*/

	exit_code = 0;

	return cleanup_argtable (argtable, exit_code);
}

void
flatten_rss_xml_to_text (std::string feed_name)
{
	namespace ns_read = gautier_rss_data_read;

	namespace ns_parse = gautier_rss_data_parse;

	std::string file_name = feed_name;
	std::string file_data;

	ns_parse::get_feed_data_from_file (file_name, ".xml", file_data);
	{
		std::vector<ns_read::rss_article> feed_lines;

		ns_parse::get_feed_lines (file_data, feed_lines);

		std::string rss_parse_results;

		for (ns_read::rss_article a : feed_lines) {
			std::string feed_name = file_name;
			std::string headline = a.headline;
			std::string article_date = a.article_date;
			std::string article_text = a.article_text;
			std::string url = a.url;

			rss_parse_results = rss_parse_results + feed_name + "\n";
			rss_parse_results = rss_parse_results + "\t" + article_date + "\n";
			rss_parse_results = rss_parse_results + "\t" + url + "\n";
			rss_parse_results = rss_parse_results + "\t\t" + headline + "\n";
			rss_parse_results = rss_parse_results + "\t\t\t" + article_text + "\n";
			rss_parse_results = rss_parse_results + "\n";
		}

		file_data = rss_parse_results;
	}

	file_name = feed_name + " TEST -- RSS flat";

	ns_parse::save_feed_data_to_file (file_name, ".txt", file_data);

	return;
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
