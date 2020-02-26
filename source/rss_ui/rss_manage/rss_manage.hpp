/*
Copyright (C) 2020 Michael Gautier

This source code is free software; you can redistribute it and/or modify it under the terms of the GNU Lesser General Public License as published by the Free Software Foundation; either version 2.1 of the License, or (at your option) any later version.

This source code is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public License along with this library. If not, see <http://www.gnu.org/licenses/>.

Author: Michael Gautier <michaelgautier.wordpress.com>
*/

#ifndef michael_gautier_rss_win_rss_manage_h
#define michael_gautier_rss_win_rss_manage_h

#include <gtk/gtk.h>

#include <queue>
#include <unordered_map>

#include "rss_lib/rss/rss_feed.hpp"
#include "rss_lib/rss/rss_feed_mod.hpp"

namespace gautier_rss_win_rss_manage {
	using feed_mod_cb_type = void (std::queue<gautier_rss_data_read::rss_feed_mod>&,
	                               std::unordered_map<std::string, gautier_rss_data_read::rss_feed>&);

	void
	set_feed_model (std::unordered_map<std::string, gautier_rss_data_read::rss_feed> feed_model);

	void
	set_modification_callback (feed_mod_cb_type* feed_mod_callback);

	void
	show_dialog (GtkApplication* app, GtkWindow* parent, int window_width, int window_height);
}
#endif
