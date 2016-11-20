/*
 *	chromium-downloader version 1.1a.
 *
 *	This is a simple utility that allows you to download 
 *	the latest Chromium build for your platform.
 *
 *	It runs on macOS and Linux and depends on libcurl 
 *	(please ensure you have it installed on your system).
 *
 *	This utility may be useful if you wish to update your web browser 
 *	to the latest nightly build. Chromium daily builds can be found at
 *	"http://commondatastorage.googleapis.com/chromium-browser-continuous"
 *	but may be difficult to retrieve.
 *
 *	The program works as follows:
 *
 *	- 	It checks which build is the latest one, by querying the website 
 *		commondatastorage.googleapis.com.
 *	- 	It builds the correct URL for downloading the Chromium zip file.
 *	-	It downloads the zip file and saves it to your current working directory.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <curl/curl.h>

#define BASE_URL "http://commondatastorage.googleapis.com/chromium-browser-continuous"
#define ROW_LENGTH 70
#define BUFSIZE 1024
#ifdef __APPLE__
  #define DEFAULT_FILENAME "chrome-mac.zip"
  #define PLATFORM "Mac"
#endif
#ifdef __linux__
  #define DEFAULT_FILENAME "chrome-linux.zip"
  #define PLATFORM "Linux"
#endif

/*
 *	The following buffer will be used to store information
 *	while performing requests using libcurl.
 */
typedef struct {
	char *s;
	size_t l;
} buffer_t;

/*
 *	This function creates a new buffer and returns a pointer to it.
 *	The function returns NULL if memory allocation fails.
 */
buffer_t *newBuffer() {
	buffer_t *buf = malloc(sizeof(buffer_t));
	if (buf) {
		buf -> l = 0;
		buf -> s = calloc(1, sizeof(char));
		if (!(buf -> s)) {
			free(buf);
			return NULL;
		}
		return buf;
	}
	return NULL;
}

/*
 *	This callback function will be passed to 'curl_easy_setopt' in order
 *	to write curl output to a variable.
 */
size_t write_f(void *ptr, size_t size, size_t nmemb, buffer_t *buf) {
	if (!ptr || !buf) return 0;
	size_t buf_len = buf -> l, new_len = buf_len + (size * nmemb);
	buf -> s = realloc(buf -> s, new_len + 1);
	if (!(buf -> s)) return 0;
	memcpy((buf -> s) + buf_len, ptr, size * nmemb);
	buf -> s[new_len] = '\0';
	buf -> l = new_len;
	return (size * nmemb);
}

/*
 *	The procedure performs a request to the website 
 *	"http://commondatastorage.googleapis.com/chromium-browser-continuous"
 *	to obtain a string containing Chromium latest version number.
 */
char *getLatestVersion(CURLcode *reply_code) {
	CURL *curl = NULL;
	char request_url[BUFSIZE], *reply_msg = NULL;
	if ((curl = curl_easy_init())) {
		buffer_t *reply_buf = newBuffer();
		if (!reply_buf) {
			curl_easy_cleanup(curl);
			curl_global_cleanup();
			return NULL;
		}
		snprintf(request_url, sizeof(request_url), "%s/%s/%s", 
		(char *) BASE_URL, (char *) PLATFORM, (char *) "LAST_CHANGE");
		curl_easy_setopt(curl, CURLOPT_URL, request_url);
		curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_f);
		curl_easy_setopt(curl, CURLOPT_WRITEDATA, reply_buf);
		*reply_code = curl_easy_perform(curl);
		if (*reply_code == CURLE_OK) reply_msg = strdup(reply_buf -> s);
		if (reply_buf -> s) free(reply_buf -> s);
		free(reply_buf);
		curl_easy_cleanup(curl);
		curl_global_cleanup();
		return reply_msg;
	}
	return NULL;
}

/*
 *	Callback function for libcurl that displays the current download 
 *	progress percentage.
 */
int display_progress(void *ptr, double download_size, double downloaded,
double upload_size, double uploaded) {
	if (download_size != 0) {
		double ratio = downloaded / download_size;
		int i = 0, bar_width = ROW_LENGTH, bar_fill = round(ratio * bar_width);
		fprintf(stdout, "Downloading Chromium... Total progress: %.01lf%%",
		100 * ratio);
		fprintf(stdout, "\r");
		fflush(stdout);
	}
	return 0;
}

/*
 *	This function downloads the zip file containing Chromium latest build
 *	using libcurl. A progress meter is also displayed to keep track of 
 *	the download status.
 */
int download(char *version, CURLcode *reply_code) {
  	char download_url[BUFSIZE];
	CURL *curl = NULL;
	FILE *fp;
	if ((curl = curl_easy_init())) {
		if (!(fp = fopen(DEFAULT_FILENAME, "wb"))) {
			curl_easy_cleanup(curl);
			curl_global_cleanup();
			return 1;
		}
		snprintf(download_url, sizeof(download_url), "%s/%s/%s/%s", 
		(char *) BASE_URL, (char *) PLATFORM, version, DEFAULT_FILENAME);
		curl_easy_setopt(curl, CURLOPT_URL, download_url);
		curl_easy_setopt(curl, CURLOPT_NOPROGRESS, 0);
		curl_easy_setopt(curl, CURLOPT_PROGRESSFUNCTION, display_progress);
		curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, NULL);
		curl_easy_setopt(curl, CURLOPT_WRITEDATA, fp);
      		*reply_code = curl_easy_perform(curl);
		fclose(fp);
		curl_easy_cleanup(curl);
		curl_global_cleanup();
		return 0;	
	}
	return 1;
}

/*
 *	Prints a separator row made of '*' to stdout. 
 *	The row contains ROW_LENGTH characters.
 */
void print_row() {
	for (int i = 0; i < ROW_LENGTH; i++) fprintf(stdout, "*");
	fprintf(stdout, "\n");
}

int main(int argc, char **argv) {
	char *version = NULL;
	CURLcode version_code, download_code;
	fprintf(stdout, "chromium-downloader (version 1.1a)\n");
	print_row();
	if (!(version = getLatestVersion(&version_code))) {
		fprintf(stdout, "Error: version request initialization failed.\n");
		return EXIT_FAILURE;	
	}
	if (version_code == CURLE_OK) {
		fprintf(stdout, "Chromium latest version for your platform is: %s\n",
		version);
		print_row();
	}
	else {
		fprintf(stderr, "Could not retrieve Chromium version number: %s\n", 
		curl_easy_strerror(version_code));
		return EXIT_FAILURE;
	}
	if (download(version, &download_code)) {
		fprintf(stderr, "Error: download initialization failed.\n");
		return EXIT_FAILURE;
	}
	if (download_code == CURLE_OK) {
		fprintf(stdout, "Chromium version %s has been successfully downloaded.\n",
		version);
	}
	else {
		fprintf(stderr, "Server returned an error code while downloading: %s\n", 
		curl_easy_strerror(download_code));
		return EXIT_FAILURE;
	}
	return EXIT_SUCCESS;
}
