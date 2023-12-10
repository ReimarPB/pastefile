/*
 * Substitution profanity plugin
 *
 * Type s/../../ after sending a message to run the substitution
 * on the last message and edit it.
 */

#include <profapi.h>
#include <string.h>
#include <strings.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>

char *account_jid;

struct room {
	char *barejid;
	char *last_message;
	struct room *next;
};

static struct room *rooms = NULL;

// Returns a pointer to the room or null
struct room *find_room(const char *barejid)
{
	struct room *current_room = rooms;

	while (current_room != NULL) {
		if (strcmp(current_room->barejid, barejid) == 0)
			break;

		current_room = current_room->next;
	}

	return current_room;
}

void prof_on_connect(const char *account_name, const char *fulljid)
{
	account_jid = strdup(fulljid);

	// Remove resource part of JID
	char *separator = strchr(account_jid, '/');
	if (separator != NULL) account_jid[separator - account_jid] = '\0';
}

void prof_on_unload(void)
{
	free(account_jid);

	struct room *current_room = rooms;
	while (current_room != NULL) {
		struct room *next_room = current_room->next;

		free(current_room->last_message);
		free(current_room->barejid);
		free(current_room);

		current_room = next_room;
	}
}

// Save the messages being sent
void post_message_send(const char *barejid, const char *message)
{
	// Find the room this message was sent in
	struct room *current_room = find_room(barejid);

	// If it doesn't exist, create it
	if (current_room == NULL) {
		struct room *old_head = rooms;

		rooms = malloc(sizeof(struct room));
		rooms->barejid = strdup(barejid);
		rooms->next = old_head;

		current_room = rooms;
	}

	// Save the last message
	current_room->last_message = strdup(message);
}

void prof_post_chat_message_send(const char *barejid, const char *message)
{
	post_message_send(barejid, message);
}

void prof_post_priv_message_send(const char *barejid, const char *nick, const char *message)
{
	post_message_send(barejid, message);
}

void prof_post_room_message_send(const char *barejid, const char *message)
{
	post_message_send(barejid, message);
}

char *pre_message_send(const char *barejid, const char *message)
{
	// Ignore if not substitution command
	if (strncmp(message, "s/", 2) != 0) return strdup(message);

	// Find the room this message was sent in
	struct room *current_room = find_room(barejid);

	// Check if we have a last message stored for this room
	if (current_room == NULL || current_room->last_message == NULL) {
		prof_cons_show("No last message found");
		return NULL;
	}

	// Write substitution command to file
	// (passing it directly in the sed command would create problems with escaping quotes)
	FILE *sed_script = fopen("/tmp/sed_script", "w");
	if (sed_script == NULL) {
		prof_cons_show("Could not create temporary file");
		return NULL;
	}
	fwrite(message, 1, strlen(message), sed_script);
	fclose(sed_script);

	// Write old message to file
	FILE *message_file = fopen("/tmp/sed_message", "w");
	if (message_file == NULL) {
		prof_cons_show("Could not create temporary file");
		return NULL;
	}
	fwrite(current_room->last_message, 1, strlen(current_room->last_message), message_file);
	fclose(message_file);

	char *result = malloc(1024);
	bzero(result, 1024);

	// Run sed command and read result
	FILE *sed_command = popen("sed -f /tmp/sed_script /tmp/sed_message 2>&1", "r");
	fread(result, 1, 1024, sed_command);
	int status = fclose(sed_command);

	if (status > 0) {
		prof_cons_show(result);
	} else {
		char *command = malloc(strlen("/correct ") + strlen(result) + 1);
		sprintf(command, "/correct %s", result);
		prof_send_line(command);
		free(command);
	}

	free(result);

	return NULL;
}

char *prof_pre_chat_message_send(const char *barejid, const char *message)
{
	return pre_message_send(barejid, message);
}

char *prof_pre_priv_message_send(const char *barejid, const char *nick, const char *message)
{
	return pre_message_send(barejid, message);
}

char *prof_pre_room_message_send(const char *barejid, const char *message)
{
	return pre_message_send(barejid, message);
}

