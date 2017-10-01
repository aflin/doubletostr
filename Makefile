CFLAGS=-O2 -g -Wall -Wextra -pedantic -std=c99
CC=gcc

all: dtos doubletostr.o test

test: doubletostr_test

doubletostr.o: doubletostr.c
	$(CC) $(CFLAGS) -c doubletostr.c -o doubletostr.o	

doubletostr_test: dtos
	@for j in "0=+01PPPPPPPPPPP" "DBL_MAX=+ooWoooooooooo" "INF=+ooXicV\icV\id" "-INF=*00G6<IC6<IC6;" "-DBL_MAX=*00H0000000000" "NAN=+00XicV\\icV\\id" "DBL_MIN=+01TPPPPPPPPPP" "-DBL_MIN=*onKOOOOOOOOOO" "-1=*OOKOOOOOOOOOO" "10.2345=+PSUShRQaTkdgb" ; do\
		i=`echo $$j| grep -o -E "^[^=]+"`;\
		v=`echo $$j| grep -o -E "[^=]+$$"`;\
		o=`./dtos $$i| grep -o -E "^[^ ]+"`;\
		if [ "$$o" != "$$v" ]; \
			then echo "./dtos $$i failed: got $$o, expected $$v while doing ./dtos $$i" 1>&2;\
			exit 1;\
		fi;\
	done;
	@echo "dtos test successful"	
	@rm -f stod; ln -s dtos stod

dtos: doubletostr.c
	$(CC) $(CFLAGS) -DDBLTEST -lm doubletostr.c -o dtos


clean:
	rm -f *.o
	rm -f dtos
	
