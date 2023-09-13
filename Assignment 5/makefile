MXOBJ = node.o main.o
MSRC = ospf

all: ${MSRC}

${MSRC}: ${MXOBJ}
	g++ -o $@ ${MXOBJ}

clean:
	rm -f Makefile.bak ${MSRC} *.o *.output