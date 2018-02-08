* The lectionary will always wake at 1 minute past the start of the next hour, however long away that may be.

* Debug output enable/suppression flag:

The flag can be entered manually:

1) Connect power so that lectionary resets and runs configuration web server.
2) Navigate to http://lectionary.local and apply settings.
3) Wait until the lectionary reboots again, and add the querystring parameter ?debug=1 (or true) to the url of the confirmation page in the address bar, then press enter.
4) The lectionary will reboot again, but with debug output enabled.
5) Any values for the debug parameter other than 1 or true will turn off debug output.

