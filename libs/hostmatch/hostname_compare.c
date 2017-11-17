#include "hostname_compare.h"

#include <string.h>
#include "rawstr.h"
#include "qcloud_iot_export_log.h"

// https://github.com/bagder/curl/blob/master/lib/hostcheck.c

int hostmatch(const char *hostname, const char *pattern) {
	Log_d("pattern: %s, hostname: %s", pattern, hostname);
	const char *pattern_label_end, *pattern_wildcard, *hostname_label_end;
	int wildcard_enabled;
	size_t prefixlen, suffixlen;
	size_t patternLen, hostnameLen;

	/* normalize pattern and hostname by stripping off trailing dots */
//	size_t len = strlen(hostname);
//	if (hostname[len - 1] == '.')
//		hostname[len - 1] = 0;
//	len = strlen(pattern);
//	if (pattern[len - 1] == '.')
//		pattern[len - 1] = 0;

	pattern_wildcard = strchr(pattern, '*');
	if (pattern_wildcard == NULL) {
		patternLen = strlen(pattern);
		hostnameLen = strlen(hostname);
		if (patternLen <= hostnameLen) {
			return Curl_raw_equal(pattern, (hostname + hostnameLen - patternLen)) ?
				   CURL_HOST_MATCH : CURL_HOST_NOMATCH;
		} else {
			return CURL_HOST_NOMATCH;
		}
	}


	/* We require at least 2 dots in pattern to avoid too wide wildcard
	 match. */
	wildcard_enabled = 1;
	pattern_label_end = strchr(pattern, '.');
	if (pattern_label_end == NULL || strchr(pattern_label_end + 1, '.') == NULL
			|| pattern_wildcard > pattern_label_end
			|| Curl_raw_nequal(pattern, "xn--", 4)) {
		wildcard_enabled = 0;
	}
	if (!wildcard_enabled)
		return Curl_raw_equal(pattern, hostname) ?
		CURL_HOST_MATCH :
													CURL_HOST_NOMATCH;

	hostname_label_end = strchr(hostname, '.');
	if (hostname_label_end == NULL
			|| !Curl_raw_equal(pattern_label_end, hostname_label_end))
		return CURL_HOST_NOMATCH;

	/* The wildcard must match at least one character, so the left-most
	 label of the hostname is at least as large as the left-most label
	 of the pattern. */
	if (hostname_label_end - hostname < pattern_label_end - pattern)
		return CURL_HOST_NOMATCH;

	prefixlen = pattern_wildcard - pattern;
	suffixlen = pattern_label_end - (pattern_wildcard + 1);
	return Curl_raw_nequal(pattern, hostname, prefixlen)
			&& Curl_raw_nequal(pattern_wildcard + 1,
					hostname_label_end - suffixlen, suffixlen) ?
			CURL_HOST_MATCH : CURL_HOST_NOMATCH;
}

