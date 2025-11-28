CC = gcc
CFLAGS = -Wall -Wextra -std=c99 -I/usr/local/include
LIBDIR = -L/usr/local/lib/ipsec -L/usr/local/lib
RPATH = -Wl,-rpath,/usr/local/lib/ipsec -Wl,-rpath,/usr/local/lib
LIBS = -lstrongswan -lvici

vici_client: vici_client.c
	$(CC) $(CFLAGS) -o vici_client vici_client.c $(LIBDIR) $(RPATH) $(LIBS)

clean:
	rm -f vici_client

.PHONY: clean