CC = gcc

BIN = 8080
DEBUG_BIN = 8080-debug

SRC_DIR = src
INCLUDE_DIR = include

#vpath %.c $(SRC_DIR)
#vpath %.h $(INCLUDE_DIR)
#vpath % .

SRCS = $(wildcard $(SRC_DIR)/*.c)
HEADERS = $(wildcard $(INCLUDE_DIR)/*.h)

DEBUG_OBJ_DIR = debug_obj
RELEASE_OBJ_DIR = release_obj
BINDIR = bin

DEBUG_OBJS = $(subst $(SRC_DIR),$(DEBUG_OBJ_DIR),$(SRCS:%.c=%.o))
RELEASE_OBJS = $(subst $(SRC_DIR),$(RELEASE_OBJ_DIR),$(SRCS:%.c=%.o))

CFLAGS = -std=gnu11 -Wall -Werror -Wextra -lpthread -I$(INCLUDE_DIR)
DEBUG_CFLAGS = -g -O0 -DVERBOSE -DDEBUG
RELEASE_CFLAGS = -O2 -DNDEBUG

all : release debug

$(DEBUG_OBJ_DIR) :
	mkdir -p $@
$(RELEASE_OBJ_DIR) :
	mkdir -p $(RELEASE_OBJ_DIR)
$(BINDIR) :
	mkdir -p $(BINDIR)

$(DEBUG_OBJ_DIR)/%.o : $(SRC_DIR)/%.c $(HEADERS) | $(DEBUG_OBJ_DIR)
	$(CC) $(CFLAGS) $(DEBUG_CFLAGS) -c $< -o $@

$(RELEASE_OBJ_DIR)/%.o : $(SRC_DIR)/%.c $(HEADERS) | $(RELEASE_OBJ_DIR)
	$(CC) $(CFLAGS) $(RELEASE_CFLAGS) -c $< -o $@

debug : $(DEBUG_OBJS) | $(BINDIR)
	$(CC) $(CFLAGS) $(DEBUG_CFLAGS) $^ -o $(BINDIR)/$(DEBUG_BIN)

release : $(RELEASE_OBJS) | $(BINDIR)
	$(CC) $(CFLAGS) $(RELEASE_CFLAGS) $^ -o $(BINDIR)/$(BIN)
clean :
	rm -rf $(BINDIR) $(RELEASE_OBJ_DIR) $(DEBUG_OBJ_DIR)

