.PHONY: clean tar
CFLAGS = -std=gnu17 -Wpedantic -Wall -O0 -pipe -fno-plt -fPIC
ifeq ($(shell uname -s),Darwin)
	LDFLAGS =
else
	LDFLAGS = -lrt -Wl,-O1,--sort-common,--as-needed,-z,relro,-z,now
endif
SUBMISSION_FILES=README.md rr.c
UID=005412538

rr: rr.o

tar:
	@tar -cf ${UID}-lab2-submission.tar ${SUBMISSION_FILES}

clean:
	@rm -rf rr.o rr __pycache__
