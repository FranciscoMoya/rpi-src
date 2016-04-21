SUBDIRS=reactor ejemplos

.PHONY=subdirs dist

all: subdirs

subdirs:
	for i in $(SUBDIRS) ; do make -C $$i ; done

dist:
	tar -czvf taller-rpi-c-`date -I`.tgz \
		`find . -name \*.c \
		     -o -name \*.h \
		     -o -name makefile \
		     -o -name \*.md \
		     -o -name AUTHORS \
		     -o -name THANKS \
                     -o -name COPYING`

clean:
	for i in $(SUBDIRS) ; do make -C $$i clean ; done
	$(RM) *~ *.tgz
