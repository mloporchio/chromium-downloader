# chromium-downloader
A simple utility that allows you to download the latest Chromium build for your platform.<br>
It runs on macOS and Linux and depends on libcurl (please ensure you have it installed on your system).<br>
This utility may be useful if you wish to update your web browser to the latest nightly build.<br>

To compile, just type:
  <code>gcc chromium-downloader.c -o chromium-downloader -lcurl</code><br>
To run:
  <code>./chromium-downloader</code><br>
