CFLAGS  = -Wall --std=gnu11 -Ofast -march=native -mtune=native -flto -fuse-linker-plugin
LDADD   = -lreactor -ldynamic

all: plaintext plaintext_lowlevel plaintext_push

plaintext_lowlevel: plaintext_lowlevel.o setup.o
	$(CC) $(CFLAGS) -o $@ $^ $(LDADD)

plaintext_push: plaintext_push.o setup.o
	$(CC) $(CFLAGS) -o $@ $^ $(LDADD)

plaintext: plaintext.o setup.o
	$(CC) $(CFLAGS) -o $@ $^ $(LDADD)

clean:
	rm -f plaintext plaintext_lowlevel plaintext_push *.o
