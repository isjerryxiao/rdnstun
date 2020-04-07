all: rdnstun

DEBUG ?= 0
CFLAGS += -O3
ifneq ($(DEBUG),0)
      CFLAGS += -O0 -DDEBUG
endif

rdnstun.o: rdnstun.c build/lib/libpicotcp.a
	$(CC) $(CFLAGS) -c -o rdnstun.o -Ibuild/include rdnstun.c
rdnstun: rdnstun.o build/lib/libpicotcp.a

build/lib/libpicotcp.a:
	+$(MAKE) -C ../picotcp DEBUG=0 PERF=1 TCP=0 UDP=1 ETH=1 IPV6=0 NAT=0 MCAST=0 TUN=1

clean:
	$(MAKE) -C ../picotcp clean
	rm -f rdnstun.o rdnstun
