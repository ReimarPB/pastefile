# pastefile

A plugin for the [Profanity XMPP client](https://profanity-im.github.io/) that allows you to paste images and text from your clipboard and upload them as files.

If your clipboard holds an image, it will be uploaded in PNG format. Otherwise, if it holds text, the file type will automatically be detected from the text content (with .txt as fallback).

The plugin is created for Linux and X11. It has not been tested with other Unix systems or Xwayland.

To install the plugin, run `make install`.

Command usage: `/pastefile`

