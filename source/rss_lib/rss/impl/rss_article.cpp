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
#include <iostream>
#include <string>
#include <string_view>

#include "rss_lib/rss/rss_article.hpp"

/*
	Some RSS/Atom feeds only provide headline and url. Barebones info.
	Validate and clean-up feed data in case something else is missing.
	If critical information is missing such as url, skip the entry.
	Output to [standard out] will highlight the presence of missing entries.
	You will typically see a large number of errors when the feed is newly added
	or the feed data format has changed in a way that eludes data element identification.
*/
bool
gautier_rss_data_read::finalize_rss_article (const std::string& feed_name, const std::string& process_date,
        rss_article& article)
{
	const bool name_url_missing = validate_feed_info_missing (feed_name, article.url);

	bool article_good = (name_url_missing == false);

	if (article_good) {
		/*
			Some feeds do not supply a publication date for the feed entry.
			Provide the download date when that is the case.
		*/
		if (article.article_date.empty()) {
			article.article_date = process_date;
		}

		const std::string a_head = article.headline;

		if (a_head.empty()) {
			article_good = false;
		}

		if (article_good) {
			const std::string a_text = article.article_text;
			const std::string a_summ = article.article_summary;

			const bool a_text_filled = (a_text.empty() == false);
			const bool a_summ_filled = (a_summ.empty() == false);

			const bool a_text_html = indicates_html (a_text);
			const bool a_summ_html = indicates_html (a_summ);

			/*
				Ordinarily, the most information exists in the article_text field.
				The RSS description field should have the most information
					and would often have a detailed format.
				That is not always the case, but this is an attempt to identify
				which field has a 'more complete' summary.

				There are 2 summary fields. A short summary, and a medium length summary.
				The goal is to identify the medium length summary first and check if it is in HTML format.

				A headline (an RSS title) could also be in HTML format. That is less useful, but a possibility.
			*/
			std::string plain_text;
			std::string html_text;

			if (a_text_filled && a_text_html) {
				html_text = a_text;
			} else if (a_summ_filled && a_summ_html) {
				html_text = a_summ;
			}

			if (a_text_filled && a_text_html == false) {
				plain_text = a_text;
			} else if (a_summ_filled && a_summ_html == false) {
				plain_text = a_summ;
			}

			/*
				In some cases:
					- Both field have 'no' HTML content.
					- Both fields are blank.
					- Both fields are filled but only one can be plain_text.
			*/
			if (plain_text.empty()) {
				/*
					Assuming that both fields are eligible and are not HTML formatted,
					Use the lesser of the fields for the plain_text;
					If the fields are the same size, then choose a default.

					If the assumptions are invalid, fallback to the headline as the summary.
					The headline could be HTML encoded, but any HTML would be minimal in most cases
						compared to the same in a field that typically has more content.
				*/
				if (a_text_filled && a_summ_filled && a_text_html == false && a_summ_html == false) {
					if (a_text.size() > a_summ.size()) {
						plain_text = a_summ;
					} else {
						plain_text = a_text;
					}
				} else {
					plain_text = a_head;
				}
			}

			if (html_text.empty()) {
				/*
					Similar to the conditions for plain_text selection but reversed.
					The fallback objective is to insert text with faux HTML encoding.
				*/
				if (a_text_filled || a_summ_filled) {
					if (a_text.size() > a_summ.size()) {
						html_text = "<div>" + std::string (a_text) + "</div>";
					} else {
						html_text = "<div>" + std::string (a_summ) + "</div>";
					}
				} else {
					html_text = "<div>" + std::string (a_head) + "</div>";
				}
			}

			article.feed_name = feed_name;
			article.article_summary = plain_text;
			article.article_text = html_text;
		}
	}

	/*
		Primarily triggered if url or headline is missing. All other missing info pieces are non-critical.
	*/
	if (article_good == false) {
		std::cout << "ERROR ***************** MISSING RSS ENTRY " << feed_name << " headline [" << article.headline <<
		          "\n";
	}

	return article_good;
}

bool
gautier_rss_data_read::validate_feed_info_missing (std::string_view feed_name, std::string_view feed_url)
{
	bool is_missing = false;

	if (feed_name.empty() || feed_url.empty()) {
		is_missing = true;
	}

	return is_missing;
}

bool
gautier_rss_data_read::indicates_html (std::string_view text)
{
	bool probably_has_html = false;

	const size_t t1_pos = text.find ("</");
	const size_t t2_pos = text.find ("&lt;/");
	const size_t t3_pos = text.find ("/>");
	const size_t t4_pos = text.find ("/&gt;");

	const size_t start = 0;

	if (t1_pos >= start || t2_pos >= start || t3_pos >= start || t4_pos >= start) {
		probably_has_html = true;
	}

	return probably_has_html;
}
