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
gautier_rss_data_read::is_feed_still_fresh (const std::string db_file_name, const std::string feed_name,
        const bool time_message_stdout)
{
	bool fresh = true;

	rss_feed feed;

	get_feed (db_file_name, feed_name, feed);

	if (feed.feed_name.empty() == false) {
		std::string current_date_time_utc = gautier_rss_util::get_current_date_time_utc();

		const int_fast32_t seconds_elapsed = gautier_rss_util::get_time_difference_in_seconds ("", feed.last_retrieved);

		const int_fast32_t retrieve_limit_hrs = std::stoi (feed.retrieve_limit_hrs);
		const int_fast32_t retrieve_limit_min = retrieve_limit_hrs * 60;
		const int_fast32_t retrieve_limit_sec = retrieve_limit_min * 60;

		const int_fast32_t minutes_elapsed = seconds_elapsed / 60;
		const int_fast32_t hours_elapsed = minutes_elapsed / 60;

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
	const std::string old_feed_name = feed_old.feed_name;
	const std::string old_last_retrieved = feed_old.last_retrieved;
	const int64_t old_article_count = feed_old.article_count;

	const std::string new_feed_name = feed_new.feed_name;
	const std::string new_last_retrieved = feed_new.last_retrieved;
	const int64_t new_article_count = feed_new.article_count;

	const bool named_the_same = (old_feed_name == new_feed_name);
	const bool last_retrieved_differs = (old_last_retrieved != new_last_retrieved);
	const bool article_count_differs = (new_article_count != old_article_count);

	return (named_the_same && (last_retrieved_differs || article_count_differs));
}

void
gautier_rss_data_read::copy_feed (rss_feed* feed_in, rss_feed* feed_out)
{
	if (feed_in && feed_out) {
		feed_out->row_id = feed_in->row_id;
		feed_out->feed_name = feed_in->feed_name;
		feed_out->feed_url = feed_in->feed_url;
		feed_out->last_retrieved = feed_in->last_retrieved;
		feed_out->retrieve_limit_hrs = feed_in->retrieve_limit_hrs;
		feed_out->retention_days = feed_in->retention_days;
		feed_out->article_count = feed_in->article_count;
		feed_out->last_index = feed_in->last_index;
	}

	return;
}
