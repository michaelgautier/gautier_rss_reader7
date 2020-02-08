/*
Copyright (C) 2020 Michael Gautier

This source code is free software; you can redistribute it and/or modify it under the terms of the GNU Lesser General Public License as published by the Free Software Foundation; either version 2.1 of the License, or (at your option) any later version.

This source code is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public License along with this library. If not, see <http://www.gnu.org/licenses/>.

Author: Michael Gautier <michaelgautier.wordpress.com>
*/

#include "rss_ui/app_win/article_header.hpp"

void
gautier_rss_win_main_article_header:: initialize_article_header_bar (GtkWidget* header_bar)
{
	gtk_header_bar_set_title (GTK_HEADER_BAR (header_bar), "Feed Name");
	gtk_header_bar_set_has_subtitle (GTK_HEADER_BAR (header_bar), false);

	return;
}
