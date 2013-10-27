i2u :
	gcc -Wall -pedantic -D_GNU_SOURCE -std=c99 -O2 -lgd i2u.c -o i2u
#	gcc -Wall -pedantic -lgd i2u.c -o i2u

all: i2u

clean:	
