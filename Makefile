# set of dependencies used to control compilation
CC = gcc
CFLAGS = -Wall -Werror -g
SUPPORTING_FILES = 

.PHONY: all
all: pagerank searchPagerank scaledFootrule

pagerank: pagerank.c $(SUPPORTING_FILES)
	$(CC) $(CFLAGS) -o pagerank pagerank.c $(SUPPORTING_FILES) -lm
	cp pagerank ex1
	cp pagerank ex2
	cp pagerank ex3

searchPagerank: searchPagerank.c $(SUPPORTING_FILES)
	$(CC) $(CFLAGS) -o searchPagerank searchPagerank.c $(SUPPORTING_FILES) -lm
	cp searchPagerank ex1
	cp searchPagerank ex2
	cp searchPagerank ex3

scaledFootrule: scaledFootrule.c $(SUPPORTING_FILES)
	$(CC) $(CFLAGS) -o scaledFootrule scaledFootrule.c $(SUPPORTING_FILES) -lm
	cp scaledFootrule ex1
	cp scaledFootrule ex2
	cp scaledFootrule ex3

.PHONY: clean
clean:
	rm -f pagerank searchPagerank scaledFootrule
	rm -f ex?/pagerank ex?/searchPagerank ex?/scaledFootrule

