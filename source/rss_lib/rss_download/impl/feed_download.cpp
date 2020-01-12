/*
Copyright (C) 2020 Michael Gautier

This source code is free software; you can redistribute it and/or modify it under the terms of the GNU Lesser General Public License as published by the Free Software Foundation; either version 2.1 of the License, or (at your option) any later version.

This source code is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public License along with this library. If not, see <http://www.gnu.org/licenses/>.

Author: Michael Gautier <michaelgautier.wordpress.com>
*/

#include <cstring>
#include <cstdlib>

#include <iostream>

#include <curl/curl.h>

#include "rss_lib/rss_download/feed_download.hpp"

static const long
http_version = CURL_HTTP_VERSION_2TLS;

static const long
http_response_min_good = 200;

static const long
http_response_max_good = 399;

void
gautier_rss_data_read::initialize_network()
{
	/*Startup*/
	curl_global_init (CURL_GLOBAL_ALL);

	return;
}
/*
	99% of this code comes from cURL Documentation almost verbatim.

	It is essentially C code that use curl directly to download rss data in xml format.

	   https://ec.haxx.se/libcurlexamples.html
	   https://ec.haxx.se/libcurl-http-requests.html
*/
struct MemoryStruct
{
	char* memory;
	size_t size;
};

static size_t
WriteMemoryCallback (void* contents, size_t size, size_t nmemb, void* userp)
{
	size_t realsize = size * nmemb;

	struct MemoryStruct* mem = (struct MemoryStruct*)userp;

	mem->memory = (char*)realloc (mem->memory, mem->size + realsize + 1);

	/*May indicate out of memory.*/
	bool is_bad_memory_allocation = (mem->memory == NULL);

	if (is_bad_memory_allocation) {
		std::cout << __FILE__ << " in " << __func__ << " at line: " << __LINE__ << "\n";
		std::cout << "Bad memory allocation. Possibly out of memory.\n";

		realsize = 0;
	} else {
		memcpy (& (mem->memory[mem->size]), contents, realsize);
		mem->size += realsize;
		mem->memory[mem->size] = 0;
	}

	return realsize;
}

long
gautier_rss_data_read::download_rss_feed (std::string feed_url, std::string& headlines)
{
	long response_code = 0L;

	/*HTTP Request*/
	CURL* curl_client = curl_easy_init();

	if (curl_client) {
		struct MemoryStruct chunk;
		chunk.memory = (char*)malloc (1);
		chunk.size = 0;

		/*
			CURL setup.
		*/
		curl_easy_setopt (curl_client, CURLOPT_VERBOSE, 1L);//1 = true; show everything
		curl_easy_setopt (curl_client, CURLOPT_NOPROGRESS, 1L);//1 = true; except don't show progress text

		/*Connection durations*/
		curl_easy_setopt (curl_client, CURLOPT_DNS_CACHE_TIMEOUT, 120L);//2 minutes to cache DNS
		curl_easy_setopt (curl_client, CURLOPT_TCP_KEEPALIVE, 1L);//1 = true; enable Keep Alive

		/*Protocol configuration*/
		curl_easy_setopt (curl_client, CURLOPT_HTTP_VERSION, http_version);//Request HTTP2 connections with TLS
		curl_easy_setopt (curl_client, CURLOPT_IPRESOLVE, CURL_IPRESOLVE_V4);//Prefer IPv4
		curl_easy_setopt (curl_client, CURLOPT_HTTPGET, 1L);//1 = true; prefer GET requests

		/*URL and response handling*/
		curl_easy_setopt (curl_client, CURLOPT_MAXREDIRS,
		                  4L);//Up to 4 redirects; even 1 redirect hints at possibly wrong url
		curl_easy_setopt (curl_client, CURLOPT_URL, feed_url.data());//Connect to this URL

		/*
			CURL DOC: Send all data to this function.

			Register the callback function that will be used to assemble data from http.
		*/
		curl_easy_setopt (curl_client, CURLOPT_WRITEFUNCTION, WriteMemoryCallback);

		/*
			CURL DOC: Pass the 'chunk' struct to the callback function.

			Associate the chunk reference to the callback function.
		*/
		curl_easy_setopt (curl_client, CURLOPT_WRITEDATA, (void*)&chunk);

		/*
			Some servers reject requests lacking a proper user-agent.

			The following user agent comes from these sources:
				Firefox Developer Tools
				Mozilla website
			This is the most acceptable generic version for now. Standards on user agents change over time.
		*/
		curl_easy_setopt (curl_client, CURLOPT_USERAGENT,
		                  "Mozilla/5.0 (X11; Linux x86_64; rv:10.0) Gecko/20100101 Firefox/10.0");

		/*
			HTTP header.
		*/
		struct curl_slist* curl_http_header = NULL;
		curl_http_header = curl_slist_append (curl_http_header, "Accept: application/xhtml+xml, application/xml");
		curl_http_header = curl_slist_append (curl_http_header, "Cache-Control: max-age=0");
		curl_http_header = curl_slist_append (curl_http_header, "Robots: off");

		curl_easy_setopt (curl_client, CURLOPT_HTTPHEADER, curl_http_header);

		/*
			DOWNLOAD the web page in xml format.
		*/
		CURLcode curl_response = curl_easy_perform (curl_client);

		/*
			Verify response back is usable.
		*/
		if (curl_response == CURLE_OK) {
			std::string response_data (chunk.memory);

			headlines = response_data;
		}

		curl_response = curl_easy_getinfo (curl_client, CURLINFO_RESPONSE_CODE, &response_code);

		if (curl_response != CURLE_OK) {
			response_code = -1;
		}

		/*
			Clean up
		*/
		curl_slist_free_all (curl_http_header);
		curl_easy_cleanup (curl_client);

		free (chunk.memory);
	}

	curl_client = NULL;

	return response_code;
}

bool
gautier_rss_data_read::is_network_response_ok (long response_code)
{
	bool good = (response_code >= http_response_min_good && response_code <= http_response_max_good);

	return good;
}

void
gautier_rss_data_read::de_initialize_network()
{
	/*
		Clean up following startup
	*/
	curl_global_cleanup();

	return;
}
