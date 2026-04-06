# a logger.o file is needed (obtained by ggc -c logger.c, compiler by another team in charge of its development)
# If linking -lrt or -pthread is needed than add the option at the end of the lines $(CC) $(CFLAGS) -o $@ $^

# --- Compiler and Flags ---
CC = gcc
CFLAGS = -Wall -Wextra -pthread
LDFLAGS = -lrt -pthread

# --- Source and Object Files ---

# Pre-compiled object files that are provided, not built.
PRECOMPILED_OBJS = logger.o

# Common source files to be compiled.
COMMON_SRCS = mylogger.c structures.c
COMMON_OBJS = $(COMMON_SRCS:.c=.o)

# Server-specific files.
SRCS_SERVER = server.c
OBJS_SERVER = $(SRCS_SERVER:.c=.o)
TARGET_SERVER = server

# Client-specific files.
SRCS_CLIENT = client.c
OBJS_CLIENT = $(SRCS_CLIENT:.c=.o)
TARGET_CLIENT = client

# Inspector-specific files.
SRCS_INSPECTOR = inspector.c
OBJS_INSPECTOR = $(SRCS_INSPECTOR:.c=.o)
TARGET_INSPECTOR = inspector

# A list of all object files that are generated from source.
GENERATED_OBJS = $(COMMON_OBJS) $(OBJS_SERVER) $(OBJS_CLIENT) $(OBJS_INSPECTOR)
# A list of all executables.
TARGETS = $(TARGET_SERVER) $(TARGET_CLIENT) $(TARGET_INSPECTOR)

# --- Main Targets ---

.PHONY: all clean

# The 'all' target is the default. It builds all executables.
all: $(TARGETS)


# --- Linking Rules ---
# These rules link the object files to create the final executables.
# Note the addition of $(PRECOMPILED_OBJS) to the linking command.

$(TARGET_SERVER): $(OBJS_SERVER) $(COMMON_OBJS)
	$(CC) $(CFLAGS) -o $@ $^ $(PRECOMPILED_OBJS) $(LDFLAGS) 

$(TARGET_CLIENT): $(OBJS_CLIENT) $(COMMON_OBJS)
	$(CC) $(CFLAGS) -o $@ $^ $(PRECOMPILED_OBJS) $(LDFLAGS) 

$(TARGET_INSPECTOR): $(OBJS_INSPECTOR) $(COMMON_OBJS)
	$(CC) $(CFLAGS) -o $@ $^ $(PRECOMPILED_OBJS) $(LDFLAGS)


# --- Compilation Rule ---
# Generic rule to compile any .c file into a .o file.
%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@


# --- Cleanup Rule ---
# The 'clean' target removes only the files generated during the build.
# It specifically DOES NOT delete the pre-compiled logger.o or log files.
clean:
	rm -f $(GENERATED_OBJS) $(TARGETS)
