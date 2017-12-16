CFLAGS ?= -Wall
LIBS ?= -lm
RM ?= rm -f
MKDIR ?= mkdir -p
INSTALL_DATA ?= install -m 644
INSTALL_BINARY ?= install -m 555

all: build

build:
	$(CC) $(CFLAGS) -o virtual_input virtual_input.c $(LIBS)

install:
	$(MKDIR) /opt/virtual_input
	$(INSTALL_BINARY) virtual_input /opt/virtual_input/virtual_input
	$(INSTALL_DATA) virtual_input.service /etc/systemd/system/virtual_input.service
	systemctl enable virtual_input

uninstall:
	systemctl disable virtual_input
	$(RM) /etc/systemd/system/virtual_input.service
	$(RM) /opt/virtual_input/virtual_input

clean:
	$(RM) virtual_input
