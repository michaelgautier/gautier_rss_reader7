/*
Copyright (C) 2020 Michael Gautier

This source code is free software; you can redistribute it and/or modify it under the terms of the GNU Lesser General Public License as published by the Free Software Foundation; either version 2.1 of the License, or (at your option) any later version.

This source code is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public License along with this library. If not, see <http://www.gnu.org/licenses/>.

Author: Michael Gautier <michaelgautier.wordpress.com>
*/
#include <iostream>

#include "rss_lib/rss/rss_feed.hpp"
#include "rss_lib/rss/rss_util.hpp"
#include "rss_lib/rss/rss_reader.hpp"

/*checks if the feed can be retrieved based on expiration date. */
bool
gautier_rss_data_read::is_feed_still_fresh (std::string db_file_name, std::string feed_name,
        bool time_message_stdout)
{
	bool fresh = true;

	rss_feed feed;

	get_feed (db_file_name, feed_name, feed);

	if (feed.feed_name.empty() == false) {
		std::string current_date_time_utc = gautier_rss_util::get_current_date_time_utc();

		int seconds_elapsed = gautier_rss_util::get_time_difference_in_seconds ("", feed.last_retrieved);

		int retrieve_limit_hrs = std::stoi (feed.retrieve_limit_hrs);
		int retrieve_limit_min = retrieve_limit_hrs * 60;
		int retrieve_limit_sec = retrieve_limit_min * 60;

		int minutes_elapsed = seconds_elapsed / 60;
		int hours_elapsed = minutes_elapsed / 60;

		if (time_message_stdout) {
			std::cout << "Feed " << feed.feed_name << " (" << feed.feed_url << ")" << "\n";
			std::cout << "\tCurrent date/time (UTC):\t" << current_date_time_utc << "\n";
			std::cout << "\tLast retrieved (UTC):\t\t" << feed.last_retrieved << "\n";
			std::cout << "\tRetrieve Limits: \t(in hrs) " << retrieve_limit_hrs << " \t(in minutes) " << retrieve_limit_min
			          << " \t(in seconds) " << retrieve_limit_sec << "\n";
			std::cout << "\tElapsed time: \t\t(in hrs) " << hours_elapsed << "\t(in minutes) " << minutes_elapsed <<
			          "\t\t(in seconds) " << seconds_elapsed << "\n";
		}

		fresh = (seconds_elapsed <= retrieve_limit_sec);
	}

	return fresh;
}

bool
gautier_rss_data_read::check_feed_changed (rss_feed& feed_old, rss_feed& feed_new)
{
	std::string old_feed_name = feed_old.feed_name;
	std::string old_last_retrieved = feed_old.last_retrieved;
	int old_article_count = feed_old.article_count;

	std::string new_feed_name = feed_new.feed_name;
	std::string new_last_retrieved = feed_new.last_retrieved;
	int new_article_count = feed_new.article_count;

	bool match_found_name = old_feed_name == new_feed_name;
	bool match_not_found_last_retrieved = old_last_retrieved != new_last_retrieved;
	bool increased_article_count = new_article_count > old_article_count;

	return match_found_name && match_not_found_last_retrieved && increased_article_count;
}
