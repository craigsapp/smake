

all: compile install


compile:
	(cd src && make)

install:
	(cd src && make install)

