APPLICATION = Pet_feeder

BOARD ?= nucleo-f401re

RIOTBASE ?= $(CURDIR)/RIOT

USEMODULE += periph_gpio
USEMODULE += xtimer
USEMODULE += periph_gpio_irq
USEMODULE += shell_commands
USEMODULE += netdev_default
USEMODULE += auto_init_gnrc_netif
USEMODULE += gnrc_ipv6_default
USEMODULE += emcute
USEMODULE += stdio_ethos
USEMODULE += gnrc_uhcpc

CFLAGS += -DCONFIG_GNRC_NETIF_IPV6_ADDRS_NUMOF=5
IPV6_PREFIX ?= 2000:2::/64
STATIC_ROUTES ?= 1
UPLINK ?= ethos
ETHOS_BAUDRATE ?= 115200
CFLAGS += -DETHOS_BAUDRATE=$(ETHOS_BAUDRATE)
TAP ?= tap0
host-tools:
	$(Q)env -u CC -u CFLAGS $(MAKE) -C $(RIOTTOOLS)
TERMDEPS += host-tools
TERMPROG ?= sudo sh $(RIOTTOOLS)/ethos/start_network.sh
TERMFLAGS ?= $(FLAGS_EXTRAS) $(PORT) $(TAP) $(IPV6_PREFIX) $(ETHOS_BAUDRATE)

FEATURES_REQUIRED = periph_gpio
FEATURES_REQUIRED = periph_adc

include $(RIOTBASE)/Makefile.include
