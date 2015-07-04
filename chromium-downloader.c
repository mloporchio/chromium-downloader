/*
  chromium-downloader version 1.0a.

  This is a simple utility that allows you to download the latest Chromium build for your platform.
  It runs on OS X and Linux and relies on libcurl (please ensure you have it installed on your system!).

  Chromium daily builds can be found at "http://commondatastorage.googleapis.com/chromium-browser-continuous"
  but may be difficult to retrieve.

  How it works:
    - The program checks which build is the latest one, by querying the website commondatastorage.googleapis.com.
    - It builds the correct URL for downloading the Chromium zip file.
    - It downloads the zip file and saves it to your current working directory.
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <curl/curl.h>

#define BASE_URL "http://commondatastorage.googleapis.com/chromium-browser-continuous"
#define ROW_LENGTH 70
#ifdef __APPLE__
  #define DEFAULT_FILENAME "chrome-mac.zip"
  #define PLATFORM "Mac"
#endif
#ifdef __linux__
  #define DEFAULT_FILENAME "chrome-linux.zip"
  #define PLATFORM "Linux"
#endif

// The following one represents a buffer that will contain libcurl output.
typedef struct {
  char *s;
  size_t l;
} buffer;

// The following one initializes a new buffer.
void newBuffer(buffer *b) {
  b->l = 0; b->s = (char *) malloc(sizeof(char));
  if (b->s == NULL) {
    fprintf(stderr, "ERROR: malloc() failed!\n");
    exit(EXIT_FAILURE);
  }
  else b->s[0] = '\0';
  return;
}

// The following function is used as a callback function for libcurl.
// It allows us to write curl_easy_perform output to a string instead of printing it to stdout.
size_t writeBuf(void *ptr, size_t size, size_t nmemb, buffer *str) {
  size_t new_len = str->l + (size * nmemb);
  str->s = realloc(str->s, new_len + 1);
  if (str->s == NULL) {
    fprintf(stderr, "ERROR: realloc() failed!\n");
    exit(EXIT_FAILURE);
  }
  memcpy(str->s+str->l, ptr, size * nmemb);
  str->s[new_len] = '\0';
  str->l = new_len;
  return (size * nmemb);
}

// The procedure performs a request to the website "http://commondatastorage.googleapis.com/chromium-browser-continuous"
// in order to obtain a string containing Chromium latest version number.
char *getLatestVersion() {
  char FULL_URL[512]; buffer chromiumVersion;
  CURL *curl = curl_easy_init();
  if (curl) {
    newBuffer(&chromiumVersion);
    snprintf(FULL_URL, sizeof(FULL_URL), "%s/%s/%s", (char *) BASE_URL, (char *) PLATFORM, (char *) "LAST_CHANGE");
    //printf("%s\n", FULL_URL);
    curl_easy_setopt(curl, CURLOPT_URL, FULL_URL);
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, writeBuf);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &chromiumVersion);
    CURLcode res = curl_easy_perform(curl);
    curl_easy_cleanup(curl);
    if (res != CURLE_OK) fprintf(stderr, "Error: curl_easy_perform failed with code %s.\n", curl_easy_strerror(res));
    else return (char *) chromiumVersion.s;
  }
  return NULL;
}

// Callback function for libcurl that displays the current download progress percentage.
int displayProgress(void* ptr, double download_size, double downloaded, double upload_size, double uploaded) {
    if (download_size != 0) {
      double ratio = downloaded / download_size;
      int i = 0, bar_width = ROW_LENGTH, bar_fill = round(ratio * bar_width);
      printf("Downloading Chromium... Total progress: %.01lf%%", 100 * ratio);
      printf("\r");
      fflush(stdout);
    }
    return 0;
}

// This function downloads the zip file containing Chromium latest build using libcurl.
// A progress meter is also displayed to keep track of the download status.
int download(char *version) {
  char FILENAME[FILENAME_MAX] = DEFAULT_FILENAME;
  char DOWNLOAD_URL[512];
  snprintf(DOWNLOAD_URL, sizeof(DOWNLOAD_URL), "%s/%s/%s/%s", (char *) BASE_URL, (char *) PLATFORM, version, FILENAME);
  CURL *curl; CURLcode res; FILE *fp;
  curl = curl_easy_init();
  if (curl) {
      fp = fopen(FILENAME, "wb");
      curl_easy_setopt(curl, CURLOPT_URL, DOWNLOAD_URL);
      curl_easy_setopt(curl, CURLOPT_NOPROGRESS, 0);
      curl_easy_setopt(curl, CURLOPT_PROGRESSFUNCTION, displayProgress);
      curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, NULL);
      curl_easy_setopt(curl, CURLOPT_WRITEDATA, fp);
      res = curl_easy_perform(curl);
      curl_easy_cleanup(curl);
      fclose(fp);
      if (res != CURLE_OK) return 0;
      return 1;
  }
  return 0;
}

// Prints a separator row made of '*' to stdout. This row contains ROW_LENGTH characters.
void printRow() {
  int i = 0;
  while (i < ROW_LENGTH) {printf("*"); i++;}; printf("\n");
}

int main(int argc, char const *argv[]) {
  int result_code; char LATEST_VER[20]; char ROW[256];
  printf("chromium-downloader (version 1.0a)\n");
  printRow();
  snprintf(LATEST_VER, sizeof(LATEST_VER), "%s", (char *) getLatestVersion());
  snprintf(ROW, sizeof(ROW), "%s: %s", (char *) "Chromium latest version for your platform is", LATEST_VER);
  printf("%s\n", ROW);
  printRow();
  result_code = download(LATEST_VER);
  if (result_code) printf("Chromium version %s has been successfully downloaded.\n", LATEST_VER);
  else fprintf(stderr, "Error: Chromium download failed.\n");
  return 0;
}
