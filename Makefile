CC = clang
CFLAGS = -Wall -Werror -Wextra

CLIENT = client
CLIENTSRCS = $(addprefix $(CLIENT)/, main.c shell.c)
CLIENTBIN = $(CLIENT)/$(CLIENT).out

SERVER = server
SERVERSRCS = $(SERVER)/main.c
SERVERBIN = $(SERVER)/$(SERVER).out

all: $(CLIENTBIN) $(SERVERBIN)

$(CLIENTBIN): $(CLIENTSRCS)
	@$(CC) $(CFLAGS) $^ -o $@

$(SERVERBIN): $(SERVERSRCS)
	@$(CC) $(CFLAGS) $^ -o $@

clean:
	@rm -f $(CLIENTBIN) $(SERVERBIN)

.PHONY: all clean
