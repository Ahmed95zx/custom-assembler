CC = gcc
CFLAGS = -g -ansi -pedantic -Wall -Iinclude
LDFLAGS = -lm

SRC = \
	src/Assembler.c \
	src/PreAssembler.c \
	src/MacroFunctions.c \
	src/FirstPass.c \
	src/LabelFunctions.c \
	src/SecondPass.c \
	src/InstructionsFunctions.c \
	src/DirectivesFunctions.c \
	src/LineProcessFunctions.c \
	src/ValidationFunctions.c \
	src/FilesFunctions.c

TARGET = Assembler

all: $(TARGET)

$(TARGET):
	$(CC) $(CFLAGS) $(SRC) $(LDFLAGS) -o $(TARGET)

clean:
	rm -f $(TARGET)
