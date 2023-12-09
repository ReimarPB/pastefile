all:
	$(CC) -shared -o pastefile.so -fPIC pastefile.c -lprofanity -lX11
	cp pastefile.so /home/reimar/pastefile.so

