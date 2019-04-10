ifeq (${PREFIX},)
	PREFIX=/usr
endif
sharedir = $(DESTDIR)$(PREFIX)/share
libexecdir = $(DESTDIR)$(PREFIX)/bin
builddir=build

all: ibus-engine-lunispim

ibus-engine-lunispim:
	mkdir -p $(builddir)
	(cd $(builddir); cmake -DCMAKE_INSTALL_PREFIX=$(PREFIX)  -DDESTDIR=$(DESTDIR) .. && make)
	@echo ':)'

install:
	make -C $(builddir) install

uninstall:
	rm $(sharedir)/ibus/component/lunispim.xml
	rm -R $(libexecdir)/ibus-lunispim

clean:
	if  [ -e $(builddir) ]; then rm -R $(builddir); fi
