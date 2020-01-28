# name of your application
APPLICATION = ee250_lab11

# If no BOARD is found in the environment, use this default:
BOARD ?= native

# This has to be the absolute path to the RIOT base directory:
RIOTBASE ?= $(CURDIR)/../..

# Comment this out to disable code in RIOT that does safety checking
# which is not needed in a production environment but helps in the
# development process:
DEVELHELP ?= 1

USEMODULE += xtimer
FEATURES_REQUIRED = periph_gpio
FEATURES_OPTIONAL = periph_gpio_irq

# Include packages that pull up and auto-init the link layer.
# NOTE: 6LoWPAN will be included if IEEE802.15.4 devices are present
USEMODULE += gnrc_netdev_default
USEMODULE += auto_init_gnrc_netif
USEMODULE += gnrc_ipv6_router_default

# Only require periph_adc if building for the OpenMote
ifneq (,$(filter openmote-b,$(BOARD)))
  FEATURES_REQUIRED = periph_adc
  CFLAGS += -DOPENMOTE_BUILD 
endif

# Only require periph_adc if building for the OpenMote
ifneq (,$(filter openmote-cc2538,$(BOARD)))
  FEATURES_REQUIRED = periph_adc
  CFLAGS += -DOPENMOTE_BUILD 
endif

USEMODULE += gnrc_udp
# Additional networking modules that can be dropped if not needed
# Use minimal standard PRNG
USEMODULE += prng_minstd

# Change this to 0 show compiler invocation lines by default:
QUIET ?= 1

include $(RIOTBASE)/Makefile.include

# Set a custom channel if needed
ifneq (,$(filter cc110x,$(USEMODULE)))          # radio is cc110x sub-GHz
  DEFAULT_CHANNEL ?= 0
  CFLAGS += -DCC110X_DEFAULT_CHANNEL=$(DEFAULT_CHANNEL)
else
  ifneq (,$(filter at86rf212b,$(USEMODULE)))    # radio is IEEE 802.15.4 sub-GHz
    DEFAULT_CHANNEL ?= 5
    CFLAGS += -DIEEE802154_DEFAULT_SUBGHZ_CHANNEL=$(DEFAULT_CHANNEL)
  else                                          # radio is IEEE 802.15.4 2.4 GHz
    DEFAULT_CHANNEL ?= 26
    CFLAGS += -DIEEE802154_DEFAULT_CHANNEL=$(DEFAULT_CHANNEL)
  endif
endif
