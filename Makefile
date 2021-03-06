#=============================================================================
#     FileName: Makefile
#         Desc: custom make file for the project
#       Author: Max Lee
#        Email: hoso1312@gmail.com
#     HomePage: mallocsizeof.me
#      Version: 0.0.1
#   LastChange: 2017-05-23 04:53:39
#=============================================================================
CC = gcc
CFLAGS = -Wall -Wextra -std=gnu99 -O3#-g

LIB = -lm -pthread
HDR = driver.h netsock.h handler.h handler_helper.h threads.h sha256.h logger.h
SRC = driver.c netsock.c handler.c handler_helper.c threads.c sha256.c logger.c work_handler.c
OBJ = $(SRC:.c=.o)

EXE = server

## Top level target is executable.
$(EXE):	$(OBJ) Makefile
	$(CC) $(CFLAGS) -o $(EXE) $(OBJ) $(LIB)

clean:
	rm $(OBJ) 

clobber: clean
	rm $(EXE) 

test:
	@echo "Success, all tests passed"

$(OBJ): $(HDR)

#=============================================================================

deftest: run_test_client def_clean

test_crypto: crypto/sha256.h crypto/sha256.c crypto/sha256_test.c
	$(CC) $(CFLAGS) -o sha256_test crypto/sha256.c crypto/sha256_test.c

test_uint256: uint256.h uint256_test.c
	$(CC) $(CFLAGS) -o uint256_test uint256_test.c

run_test_client: test_crypto test_uint256
	./sha256_test
	./uint256_test

def_clean:
	rm -rf ./sha256_test ./uint256_test *.o

.PHONY = run_test_client clean
