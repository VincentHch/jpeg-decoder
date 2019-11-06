# Repertoires du projet
BIN_DIR = bin
SRC_DIR = src
INC_DIR = include
OBJ_DIR = obj
OBJPROF_DIR = obj-prof
IMG = images

# Options de compilation/édition des liens
CC = clang
LD = clang
INC = -I$(INC_DIR)
CFLAGS += $(INC) -Wall -std=c99 -O0 -g  -Wextra
LDFLAGS = -lm

# Listage de tous les fichiers et images
SRC_FILES = $(wildcard $(SRC_DIR)/*.c)
OBJ_FILES =	$(patsubst $(SRC_DIR)/%, $(OBJ_DIR)/%, $(patsubst %.c,%.o, $(SRC_FILES)))
IMAGES = $(wildcard $(IMG)/*.jpg) $(wildcard $(IMG)/*.jpeg)
RESULTATS = $(wildcard $(IMG)/*.ppm) $(wildcard $(IMG)/*.pgm)

# cible par défaut
TARGET = $(BIN_DIR)/jpeg2ppm

all: $(TARGET)

test: $(TARGET) $(IMAGES)

example: $(IMG)/invader_progressive.jpg

%.jpg: $(BIN_DIR)/jpeg2ppm
	@echo ""
	@echo "        Décodage de $* :"
	bin/jpeg2ppm $*.jpg
	@echo ""


%.jpeg: $(BIN_DIR)/jpeg2ppm
	@echo ""
	@echo "        Décodage de $* :"
	@echo ""


$(TARGET): $(OBJPROF_FILES) $(OBJ_FILES)
	$(LD) $(LDFLAGS) $(OBJPROF_FILES) $(OBJ_FILES) -o $(TARGET)

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.c $(INC_DIR)/%.h
	$(CC) $(CFLAGS) -c $(SRC_DIR)/$*.c -o $(OBJ_DIR)/$*.o

.PHONY: clean

clean:
	rm -f $(TARGET) $(OBJ_FILES) $(RESULTATS)

mr_propre:
	rm -f $(TARGET) $(OBJ_FILES) $(RESULTATS) $(IMAGES)
