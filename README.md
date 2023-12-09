# pastefile

A plugin for the [Profanity XMPP client](https://profanity-im.github.io/) that allows you to paste images and text from your clipboard and upload them as files.

When running `/pastefile`, the plugin will upload what you have in your clipboard and send it in the current chat window:

- If you have copied an image, that image will be uploaded as a PNG
- If you have copied a file from a file manager, that file will be uploaded
- If you have copied some text, it will be written to a file with an auto-generated file extension (usually .txt) and uploaded

The plugin is created for Linux and X11. It has not been tested with other Unix systems or Xwayland.

To install the plugin, run `make install` and inside profanity, run `/plugins load pastefile.so`.

