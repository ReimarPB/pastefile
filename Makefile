all: pastefile substitute

pastefile:
	mkdir -p build
	$(CC) -shared -o build/pastefile.so -fPIC src/pastefile.c -lprofanity -lX11

substitute:
	mkdir -p build
	$(CC) -shared -o build/substitute.so -fPIC src/substitute.c -lprofanity

install:
	mkdir -p $(HOME)/.local/share/profanity/plugins/
	cp build/* $(HOME)/.local/share/profanity/plugins/

clean:
	rm -r build/

