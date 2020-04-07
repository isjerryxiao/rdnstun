#include <time.h>
#include <pico_stack.h>
#include <pico_frame.h>
#include <pico_ipv4.h>
#include <pico_icmp4.h>
#include <pico_dev_tun.h>
#include <pico_socket.h>
#include <stdint.h>


#define IPADDR  "172.20.51.113"
#define NETMASK "255.0.0.0"    // whatever
#define LISTENRANGE 5
#define TUNNAME "dn42-rdns"

static struct pico_ip4 ipaddr;

static int ip_distance(uint32_t addr) {
    for (unsigned dis=0; dis < LISTENRANGE; dis++) {
        if ((ipaddr.addr - dis * (2<<23)) == addr) {
            return dis;
        }
    }

    #ifdef DEBUG
    char s_addr[30], s_ipaddr[30];
    pico_ipv4_to_string(s_addr, addr);
    pico_ipv4_to_string(s_ipaddr, ipaddr.addr);
    printf("bad %s %s\n", s_addr, s_ipaddr);
    #endif

    return -1;
}

int cb_ping(struct pico_frame *f) {

    struct pico_ipv4_hdr *hdr = (struct pico_ipv4_hdr *) f->net_hdr;

    uint32_t * dst = &(hdr->dst.addr);
    uint32_t * src = &(hdr->src.addr);

    #ifdef DEBUG
    char s_dst[30], s_src[30];
    pico_ipv4_to_string(s_dst, *dst);
    pico_ipv4_to_string(s_src, *src);
    printf("icmp src=%s dst=%s\n", s_src, s_dst);
    #endif

    int ip_dis = ip_distance(*dst);
    if (ip_dis < 0) {
        pico_frame_discard(f);
        return 0;
    }
    else if (hdr->ttl < ip_dis+1) {
        f->use_src_addr = ipaddr.addr - (hdr->ttl - 1) * (2<<23);
        hdr->ttl = 1;
        pico_icmp4_ttl_expired(f);
        return 0;
    }
    f->use_src_addr = *dst;
    hdr->ttl = 64u;
    return 1;
}


int main(void){
    struct pico_ip4 netmask;
    struct pico_device* dev;

    /* initialise the stack. Super important if you don't want ugly stuff like
     * segfaults and such! */
    pico_stack_init();

    /* create the tap device */
    dev = pico_tun_create(TUNNAME);
    if (!dev)
        return -1;

    /* assign the IP address to the tap interface */
    pico_string_to_ipv4(IPADDR, &ipaddr.addr);
    pico_string_to_ipv4(NETMASK, &netmask.addr);

    for (uint32_t lr = 0; lr < LISTENRANGE; lr++) {
        pico_ipv4_link_add(dev, ipaddr, netmask);
        ipaddr.addr += 2<<23;
    }
    ipaddr.addr -= 2<<23;

    setup_icmp4_in_listener(cb_ping);
    setup_transport_in_listener(cb_ping);

    while (1)
    {
        usleep(1000);
        pico_stack_tick();
    }

    return 0;
}
