#This is For Test mini-xml in Linux or All linux environment

TA=mxml-string.o mxml-node.o mxml-file.o\
	 mxml-get.o mxml-attr.o mxml-index.o mxml-private.o\
	 mxml-search.o mxml-set.o mxml-entity.o\

all: $(TA)
	ar -rc libminixml.a $(TA)

.PHONY: clean

clean:
	rm *.o
