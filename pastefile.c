#include <X11/Xlib.h>
#include <profapi.h>
#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>

Display *display;
Window window;
Atom clipboard, targets, incr_type, output_atom, png_target;
int event_base, error_base;

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

	bool is_image = false;
	for (int i = 0; i < data_count; i++) {
		if (target_atoms[i] == png_target) is_image = true;
	}

	XFree(target_atoms);

	// Get clipboard data
	if (is_image) {
		unsigned char *data = get_clipboard(png_target, &data_count);
		if (data == NULL) return;

		// Write to file
		FILE *file = fopen("/tmp/image.png", "w");
		if (file == NULL) {
			prof_cons_show("Could not open file for writing");
			return;
		}
		fwrite(data, data_count, 1, file);
		fclose(file);

		prof_send_line("/sendfile /tmp/image.png");
	} else {
		prof_cons_show("TODO text");
	}
}

void prof_init(const char *version, const char *status, const char *account_name, const char *fulljid)
{
	display = XOpenDisplay(NULL);

	clipboard = XInternAtom(display, "CLIPBOARD", False);
	targets = XInternAtom(display, "TARGETS", False);
	incr_type = XInternAtom(display, "INCR", False);
	output_atom = XInternAtom(display, "CLIPBOARD_OUTPUT", False);
	png_target = XInternAtom(display, "image/png", False);

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
	XCloseDisplay(display);
}

void prof_on_start(void) {}
void prof_on_shutdown(void) {}

