#include <X11/extensions/Xfixes.h>
#include <profapi.h>

void pastefile(char **args)
{
	prof_cons_show("TODO paste file");
}

void prof_init(const char *version, const char *status, const char *account_name, const char *fulljid)
{
	prof_cons_show("Loaded pastefile plugin");

	char *synopsis[] = { "/pastefile", NULL };
	char *description = "Uploads the file currently stored on the clipboard";
	char *arguments[][2] = { {NULL, NULL} };
	char *examples[] = { NULL };
	prof_register_command("/pastefile", 0, 0, synopsis, description, arguments, examples, pastefile);
}

void prof_on_start(void) {}
void prof_on_shutdown(void) {}
void prof_on_unload(void) {}

