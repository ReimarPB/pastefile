# Profanity plugins

This is a collection of plugins I've made for the [Profanity XMPP client](https://profanity-im.github.io/). Click on the plugin names below for more information about each plugin.

- [pastefile](#pastefile) - Allows pasting files such as images from your clipboard
- [substitute](#substitute) - Adds sed-like substitutions to edit the last message writing `s/.../.../`

Note that the plugins are created for Linux and X11, so they might not all work on other systems.

# Installation

1. Clone the git repo and run `make` to build all plugins or `make <name>` to build a single plugin.
2. Run `make install` to install the plugins you've built.
3. Within profanity, write `/plugins` to see a list of plugins and `/plugins load <name>` to load each.

# Plugins

## pastefile

When running `/pastefile`, the plugin will upload what you have in your clipboard and send it in the current chat window:

- If you have copied an image, that image will be uploaded as a PNG
- If you have copied a file from a file manager, that file will be uploaded
- If you have copied some text, it will be written to a file with an auto-generated file extension (usually .txt) and uploaded

## substitute

This plugin allows you to edit your previous message using syntax from the `sed` unix command.

After having sent a message, you can type e.g. `s/old/new/` to replace the first instance of the word "old" in your message with the word "new". To replace all instances, add a `g` at the end.

For more information about the syntax, see [GNU's manual](https://www.gnu.org/software/sed/manual/html_node/The-_0022s_0022-Command.html) on the `s` command from `sed`.

