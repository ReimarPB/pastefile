/*
 * Pastefile profanity plugin
 *
 * Run /pastefile to paste what is currently in your clipboard
 * and upload it as a file.
 *
 * - if it is an image, the png of that is uploaded
 * - if copied from a file manager, the file you copied will be uploaded
 * - if your clipboard just holds text, it will be written to a file and uploaded
 */

#include <X11/Xlib.h>
#include <profapi.h>
#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>

Display *display;
Window window;
Atom clipboard, targets, incr_type, output_atom, png_target, string_target, file_transfer_target, plain_text_target;

enum clipboard_type {
	CLIPBOARD_UNKNOWN,
	CLIPBOARD_TEXT,
	CLIPBOARD_IMAGE,
	CLIPBOARD_FILE,
};

void console_print_error(char *error)
{
	char *system_error = strerror(errno);

	char *full_error = malloc(strlen(error) + strlen(system_error) + 3);
	sprintf(full_error, "%s: %s", error, system_error);

	prof_cons_show(full_error);
	prof_log_error(full_error);

	free(full_error);
}

char *run_command_get_first_line(char *command)
{
	FILE *cmd = popen(command, "r");
	if (cmd == NULL) {
		console_print_error("Failed to run command");
		return NULL;
	}

	char *data = malloc(128);
	fgets(data, 128, cmd);
	data[strcspn(data, "\n")] = '\0'; // Remove trailing newline

	int status = pclose(cmd);

	if (status > 0) {
		prof_cons_show("Failed to run command");
		return NULL;
	}

	if (status == -1) {
		console_print_error("Failed to run command");
		return NULL;
	}

	return data;
}

char *detect_file_extension(char *file_path)
{
	// Run command to get mime type
	char *mime_type = run_command_get_first_line("file --mime --brief /tmp/file 2>&1");
	if (mime_type == NULL) return NULL;

	// Remove everything after ;
	char *separator = strchr(mime_type, ';');
	if (separator != NULL) mime_type[separator - mime_type] = '\0';

	// Run command to find extension by mime type
	char *grep_cmd_str = malloc(164);
	snprintf(grep_cmd_str, 164, "grep /etc/mime.types -e '^%s\\s' --color=never --max-count=1 | awk '{print $2}' 2>&1", mime_type);
	char *extension = run_command_get_first_line(grep_cmd_str);

	// Fallback to .txt
	if (extension != NULL && strlen(extension) == 0) {
		free(extension);
		extension = strdup("txt");
	}

	free(mime_type);
	free(grep_cmd_str);

	return extension;
}

unsigned char *get_clipboard(Atom target, unsigned long *data_count)
{
	unsigned long data_size;

	// Request selection
	XConvertSelection(display, clipboard, target, output_atom, window, CurrentTime);
	XFlush(display);

	// Wait for selection event
	XEvent event;
	do
		XNextEvent(display, &event);
	while (event.type != SelectionNotify);

	if (event.xselection.property == None) {
		prof_cons_show("No clipboard data found");
		return NULL;
	}

	Atom type;
	int format;
	unsigned char *data;

	// Get size of data
	XGetWindowProperty(display, window, output_atom, 0, 0, False, AnyPropertyType, &type, &format, data_count, &data_size, &data);
	XFree(data);

	// TODO implement
	if (type == incr_type) {
		prof_cons_show("INCR type not implemented");
		return NULL;
	}

	// Get actual data
	XGetWindowProperty(display, window, output_atom, 0, data_size, False, AnyPropertyType, &type, &format, data_count, &data_size, &data);
	XDeleteProperty(display, window, output_atom);

	if (data == NULL) {
		prof_cons_show("Clipboard empty");
	}

	return data;
}

void pastefile(char **args)
{
	unsigned long data_count;

	// Get list of targets
	Atom *target_atoms = (Atom *)get_clipboard(targets, &data_count);
	if (target_atoms == NULL) return;

	enum clipboard_type clipboard_type = CLIPBOARD_UNKNOWN;
	for (int i = 0; i < data_count; i++) {
		if (target_atoms[i] == string_target) clipboard_type = CLIPBOARD_TEXT;
		else if (target_atoms[i] == png_target) clipboard_type = CLIPBOARD_IMAGE;
		else if (target_atoms[i] == file_transfer_target) clipboard_type = CLIPBOARD_FILE;
	}

	XFree(target_atoms);

	if (clipboard_type == CLIPBOARD_UNKNOWN) {
		prof_cons_show("Clipboard data is in unknown format");
		return;
	}

	char *file_path;

	// Handle file copied from file manager
	if (clipboard_type == CLIPBOARD_FILE) {
		// Get path to copied file
		file_path = (char *)get_clipboard(plain_text_target, &data_count);
		if (file_path == NULL) return;

	// Handle text/image, write to temporary file and upload that
	} else {
		bool is_image = clipboard_type == CLIPBOARD_IMAGE;

		// Get clipboard data
		file_path = strdup(is_image ? "/tmp/image.png" : "/tmp/file");

		unsigned char *data = get_clipboard(is_image ? png_target : string_target, &data_count);
		if (data == NULL) return;

		// Write to file
		FILE *file = fopen(file_path, "w");
		if (file == NULL) {
			console_print_error("Could not open temporary file for writing");
			return;
		}
		fwrite(data, data_count, 1, file);
		fclose(file);

		// Auto-detect file type if text
		if (!is_image) {
			// Get extension
			char *extension = detect_file_extension(file_path);
			if (extension == NULL) return;

			// Get new file name
			char *new_file_path = malloc(strlen(file_path) + strlen(extension) + 2);
			sprintf(new_file_path, "%s.%s", file_path, extension);

			// Rename file and replace file_path if successful
			if (rename(file_path, new_file_path) != -1) {
				free(file_path);
				file_path = new_file_path;
			} else {
				free(new_file_path);
			}
		}
	}

	// Run command to upload file
	char *command = malloc(strlen("/sendfile ") + strlen(file_path) + 1);
	sprintf(command, "/sendfile %s", file_path);
	prof_send_line(command);

	free(file_path);
	free(command);
}

void prof_init(const char *version, const char *status, const char *account_name, const char *fulljid)
{
	display = XOpenDisplay(NULL);

	// Get atoms
	clipboard = XInternAtom(display, "CLIPBOARD", False);
	targets = XInternAtom(display, "TARGETS", False);
	incr_type = XInternAtom(display, "INCR", False);
	output_atom = XInternAtom(display, "CLIPBOARD_OUTPUT", False);
	png_target = XInternAtom(display, "image/png", False);
	string_target = XInternAtom(display, "UTF8_STRING", False);
	file_transfer_target = XInternAtom(display, "application/vnd.portal.filetransfer", False);
	plain_text_target = XInternAtom(display, "text/plain", False);

	// Create dummy window for receiving events
	window = XCreateSimpleWindow(display, DefaultRootWindow(display), 0, 0, 1, 1, 0, 0, 0);
	XSelectInput(display, window, PropertyChangeMask);

	// Register /pastefile command
	char *synopsis[] = { "/pastefile", NULL };
	char *description = "Uploads the file currently stored on the clipboard";
	char *arguments[][2] = { {NULL, NULL} };
	char *examples[] = { NULL };
	prof_register_command("/pastefile", 0, 0, synopsis, description, arguments, examples, pastefile);
}

void prof_on_unload(void)
{
	XDestroyWindow(display, window);
	XCloseDisplay(display);
}

