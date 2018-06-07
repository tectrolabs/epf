CC=gcc
CPPFLAGS= -O2 -lssl -lcrypto -ldl -lrt -lpthread -Wall -Wextra -lstdc++  

PREFIX = $(DESTDIR)/usr/local
BINDIR = $(PREFIX)/bin


EPF = epf
RUNEPF = run-epf.sh

all: $(EPF)

$(EPF): epf.cpp
	$(CC) epf.cpp Configuration.cpp Property.cpp HttpClient.cpp HttpResponse.cpp SHA256.cpp XorCryptor.cpp  RSACryptor.cpp CryptoToken.cpp BinHexConverter.cpp -o $(EPF) $(CPPFLAGS)

clean:
	rm -f *.o ; rm $(EPF)

install:
	install $(EPF) $(BINDIR)/$(EPF)
	cp $(RUNEPF) $(BINDIR)/$(RUNEPF)
	chmod a+x $(BINDIR)/$(RUNEPF)

uninstall:
	rm $(BINDIR)/$(EPF)
	rm $(BINDIR)/$(RUNEPF)
