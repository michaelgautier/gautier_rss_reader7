/*
Copyright (C) 2019 Michael Gautier

This source code is free software; you can redistribute it and/or modify it under the terms of the GNU Lesser General Public License as published by the Free Software Foundation; either version 2.1 of the License, or (at your option) any later version.

This source code is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public License along with this library. If not, see <http://www.gnu.org/licenses/>.

Author: Michael Gautier <michaelgautier.wordpress.com>
*/

#include <cctype>

#include "libxml/parser.h"
#include "libxml/tree.h"

#include "rss_lib/rss_parse/feed_parse.hpp"

static void
lower_case (char* raw_text, std::string& new_text);

static void
parse_rss_feed (xmlNode* parent_xml_node, std::vector<gautier_rss_data_read::rss_article>& feed_lines,
                gautier_rss_data_read::rss_article* previous_article);

static void
get_xml_attr_value (xmlNode* xml_node, std::string attr_name, std::string& value);

/*
	RSS FEED READER

	CENTRAL function of the entire program.

	Receives RSS data in XML format. Converts this XML to a flat format. The result is easier to reproduce on screen.

	The following function contents is taken directly from the LibXml2 website.

	Example code was indexed at:		libxml/examples/index.html#tree1.c
*/
void
gautier_rss_data_parse::get_feed_lines (std::string& feed_data,
                                        std::vector<gautier_rss_data_read::rss_article>& feed_lines)
{
	xmlDoc* doc = NULL;
	xmlNode* root_element = NULL;

	/*
	 * this initialize the library and check potential ABI mismatches
	 * between the version it was compiled for and the actual shared
	 * library used.
	 */
	LIBXML_TEST_VERSION

	int xml_parse_options = (XML_PARSE_RECOVER |
	                         XML_PARSE_NOERROR |
	                         XML_PARSE_NOWARNING |
	                         XML_PARSE_NOBLANKS |
	                         XML_PARSE_NOCDATA);

	/*parse the file and get the DOM */
	int feed_data_size = feed_data.size();

	doc = xmlParseMemory (feed_data.data(), feed_data_size);

	if (doc) {
		/*Get the root element node */
		root_element = xmlDocGetRootElement (doc);

		parse_rss_feed (root_element, feed_lines, nullptr);

		/*free the document */
		xmlFreeDoc (doc);
	}

	/*
	 *Free the global variables that may
	 *have been allocated by the parser.
	 */
	xmlCleanupParser();

	return;
}

/*
	FILE I/O	output -> file

		Uses C File I/O		Simplifies code without being obtuse.

	Overwrites the file if it exists or makes a new file.

	Unless the operating system prevents it, there will be a file.
*/
void
gautier_rss_data_parse::save_feed_data_to_file (std::string file_name, std::string ext, std::string& file_data)
{
	std::string data_file_name = file_name + ext;

	FILE* data_file = fopen (data_file_name.data(), "w");

	if (data_file) {
		char* str = file_data.data();

		fputs (str, data_file);

		fflush (data_file);

		fclose (data_file);
	}

	return;
}

/*
	FILE I/O	file -> C++ string

		Uses C File I/O		Simplifies code without being obtuse.

	If the requested file does not exist, the file_data variable remains an empty string.

	The appropriate application level error check is none-technical. Check for an empty string.
*/
void
gautier_rss_data_parse::get_feed_data_from_file (std::string file_name, std::string ext, std::string& file_data)
{
	std::string data_file_name = file_name + ext;

	FILE* data_file = fopen (data_file_name.data(), "r");

	if (data_file) {
		while (true) {
			int c = fgetc (data_file);

			if (c == EOF) {
				break;
			}

			char data = (char)c;

			file_data.push_back (data);
		}

		fclose (data_file);
	}

	return;
}

/*

	PRIMARY CODE

		Parses an RSS or ATOM feed.

		Parse conducted based on rough approximation of an RSS/ATOM schema.

		The parse does not validate RSS/ATOM schema and takes a simpler approach
			based on the element sequence expected in RSS/ATOM documents.
*/
void
parse_rss_feed (xmlNode* parent_xml_node, std::vector<gautier_rss_data_read::rss_article>& feed_lines,
                gautier_rss_data_read::rss_article* previous_article)
{
	xmlNode* xml_node = NULL;

	gautier_rss_data_read::rss_article* article = previous_article;

	for (xml_node = parent_xml_node; xml_node; xml_node = xml_node->next) {
		if (xml_node->type != XML_ELEMENT_NODE) {
			continue;
		}

		std::string node_name;

		lower_case ((char*)xml_node->name, node_name);

		if (node_name == "item" || node_name == "entry") {
			feed_lines.emplace_back (gautier_rss_data_read::rss_article());

			article = &feed_lines.back();
		}

		if (article && node_name == "title") {
			std::string text = (char*)xmlNodeGetContent (xml_node);

			article->headline = text;
		} else if (article && node_name == "link") {
			std::string text = (char*)xmlNodeGetContent (xml_node);

			//Some feed formats use an href attribute for the link
			if (text.empty() && xml_node->properties) {
				get_xml_attr_value (xml_node, "href", text);
			}

			article->url = text;
		} else if (article && (node_name == "pubdate" ||
		                       node_name == "published" ||
		                       node_name == "updated" ||
		                       node_name == "date")) {
			std::string text = (char*)xmlNodeGetContent (xml_node);

			article->article_date = text;
		} else if (article && (node_name == "description" ||
		                       node_name == "summary"
		                      )) {
			std::string text = (char*)xmlNodeGetContent (xml_node);

			article->article_summary = text;
		} else if (article && (node_name == "content" ||
		                       node_name == "encoded"
		                      )) {
			std::string text = (char*)xmlNodeGetContent (xml_node);

			article->article_text = article->article_text + text;
		}

		parse_rss_feed (xml_node->children, feed_lines, article);
	}

	return;
}

/*
	Find an xml attribute in an element based on the attribute's name.
	Returns the value of that xml attribute if found.
*/
void
get_xml_attr_value (xmlNode* xml_node, std::string attr_name, std::string& value)
{
	if (xml_node->properties) {
		xmlAttr* attr = NULL;

		for (attr = xml_node->properties; attr; attr = attr->next) {
			std::string name = (char*)attr->name;

			if (name == attr_name) {
				value = (char*)xmlNodeGetContent (attr->children);

				break;
			}
		}
	}

	return;
}

/*
	LOWER CASE all characters.

	Expects a null terminated string.

	Converts all characters to lower case.
*/
void
lower_case (char* raw_text, std::string& new_text)
{
	while (*raw_text != '\0') {
		char character = *raw_text++;

		new_text.push_back (std::tolower (character));
	}

	return;
}
