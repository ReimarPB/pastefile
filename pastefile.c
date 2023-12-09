#include <X11/Xlib.h>
#include <profapi.h>
#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>

Display *display;
Window window;
Atom clipboard, targets, incr_type, output_atom;
int event_base, error_base;

void pastefile(char **args)
{
	// Request selection
	XConvertSelection(display, clipboard, targets, output_atom, window, CurrentTime);
	XFlush(display);

	// Wait for selection event
	XEvent event;
	do
		XNextEvent(display, &event);
	while (event.type != SelectionNotify);

	if (event.xselection.property == None) {
		prof_cons_show("No clipboard data found");
		return;
	}

	// Get list of targets

	Atom type;
	int format;
	unsigned long target_count, target_data_size;
	unsigned char *target_data;

	// Get size of data
	XGetWindowProperty(display, window, output_atom, 0, 0, False, AnyPropertyType, &type, &format, &target_count, &target_data_size, &target_data);
	XFree(target_data);

	if (type == incr_type) {
		prof_cons_show("INCR type not implemented");
		return;
	}

	// Get actual data
	XGetWindowProperty(display, window, output_atom, 0, target_data_size, False, AnyPropertyType, &type, &format, &target_count, &target_data_size, &target_data);
	XDeleteProperty(display, window, output_atom);

	Atom *target_atoms = (Atom *)target_data;

	for (int i = 0; i < target_count; i++) {
		char *target_name = XGetAtomName(display, target_atoms[i]);

		char *msg = malloc(69);
		snprintf(msg, 69, "Target: %s", target_name);
		prof_cons_show(msg);
		free(msg);

		XFree(target_name);
	}

	XFree(target_data);
}

void prof_init(const char *version, const char *status, const char *account_name, const char *fulljid)
{
	display = XOpenDisplay(NULL);

	clipboard = XInternAtom(display, "CLIPBOARD", False);
	targets = XInternAtom(display, "TARGETS", False);
	incr_type = XInternAtom(display, "INCR", False);
	output_atom = XInternAtom(display, "XCLIP_OUT", False);

	// Create dummy window for receiving events
	window = XCreateSimpleWindow(display, DefaultRootWindow(display), 0, 0, 1, 1, 0, 0, 0);
	XSelectInput(display, window, PropertyChangeMask);

	char *synopsis[] = { "/pastefile", NULL };
	char *description = "Uploads the file currently stored on the clipboard";
	char *arguments[][2] = { {NULL, NULL} };
	char *examples[] = { NULL };
	prof_register_command("/pastefile", 0, 0, synopsis, description, arguments, examples, pastefile);

	prof_cons_show("Loaded pastefile plugin");
}

void prof_on_unload(void)
{
	XCloseDisplay(display);
}

void prof_on_start(void) {}
void prof_on_shutdown(void) {}

