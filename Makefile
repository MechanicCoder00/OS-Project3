CC	= gcc
CFLAGS = -lm -lrt -lpthread #-DDEBUG
TARGET1	= master
TARGET2 = bin_adder
OBJ1	= master.o
OBJ2	= bin_adder.o

ALL:	$(TARGET1) $(TARGET2)

$(TARGET1): $(OBJ1)
	$(CC) ${CFLAGS} -o $@ $(OBJ1)
	
$(TARGET2): $(OBJ2)
	$(CC) ${CFLAGS} -o $@ $(OBJ2)

$(OBJ1): master.c
	$(CC) ${CFLAGS} -c master.c

$(OBJ2): bin_adder.c
	$(CC) ${CFLAGS} -c bin_adder.c

.PHONY: clean
clean:
	/bin/rm -f *.o adder_log waits_log $(TARGET1) $(TARGET2)
