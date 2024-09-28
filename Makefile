CC = clang
CFLAGS = -Wall -Werror -Wextra

CLIENT = client
CLIENTSRCS = $(wildcard $(CLIENT)/*.c)
CLIENTBIN = $(CLIENT)/$(CLIENT).out

CMD = cmd
CMDSRCS = $(wildcard $(CLIENT)/$(CMD)/*.c)
CMDBIN = $(patsubst $(CLIENT)/$(CMD)/%.c, $(CLIENT)/$(CMD)/bin/%, $(CMDSRCS))

SERVER = server
SERVERSRCS = $(wildcard $(SERVER)/*.c)
SERVERBIN = $(SERVER)/$(SERVER).out

all: $(CLIENTBIN) $(SERVERBIN)

$(CLIENTBIN): $(CLIENTSRCS) $(CMDBIN)
	@$(CC) $(CFLAGS) $(CLIENTSRCS) -o $@

$(CLIENT)/$(CMD)/bin/%: $(CLIENT)/$(CMD)/%.c
	@mkdir -p $(CLIENT)/$(CMD)/bin
	@$(CC) $(CFLAGS) $< -o $@

$(SERVERBIN): $(SERVERSRCS)
	@$(CC) $(CFLAGS) $^ -o $@

clean:
	@rm -rf $(CLIENTBIN) $(CMDBIN) $(SERVERBIN)

.PHONY: all clean
