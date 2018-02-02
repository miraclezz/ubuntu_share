#include <linux/netfilter.h>
#include <linux.init.h>
#include <linux/module.h>
#include <linux/netfilter_ipv4.h>
#include <linux/ip.h>
#include <linux/inet.h>

unsigned int my_hookfn(unsigned int hooknum,
		struct sk_buff *skb,
		const struct net_device *in,
		const struct net_device *out,
		int (*okfn)(struct sk_buf *))
{

}
