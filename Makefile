all:
	$(CC) -shared -o pastefile.so -fPIC pastefile.c -lprofanity -lX11

install: all
	cp pastefile.so $(HOME)/.local/share/profanity/plugins/pastefile.so

