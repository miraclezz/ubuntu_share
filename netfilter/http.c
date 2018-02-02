#include <linux/netfilter.h>
#include <linux/init.h>
#include <linux/module.h>
#include <linux/netfilter_ipv4.h>
#include <linux/ip.h>
#include <linux/inet.h>

unsigned int my_hookfn(unsigned int hooknum,
						struct sk_buff *skb,
						const struct net_device *in,
						const struct net_device *out,
						int (*okfn)(struct sk_buff *))
{
	struct iphdr *iph;  
    iph = ip_hdr(skb);  
  
    /* log the original src IP */  
    printk(KERN_INFO"src IP %pI4\n", &iph->daddr);  
  
    /* modify the packet's src IP */  
    iph->saddr = in_aton("192.168.10.222");  
	
	printk(KERN_INFO"src IP %pI4\n", &iph->daddr);  
  
    return NF_ACCEPT;  			
}

/* A netfilter instance to use */  
static struct nf_hook_ops nfho = {  
    .hook = my_hookfn,  
    .pf = PF_INET,  
    .hooknum = NF_INET_FORWARD,  
    .priority = NF_IP_PRI_FIRST,  
    .owner = THIS_MODULE,  
};  
  
static int __init http_init(void)  
{  
    if (nf_register_hook(&nfho)) {  
        printk(KERN_ERR"nf_register_hook() failed\n");  
        return -1;  
    }  
    return 0;  
}  
  
static void __exit http_exit(void)  
{  
    nf_unregister_hook(&nfho);  
}  
  
module_init(http_init);  
module_exit(http_exit);  
MODULE_AUTHOR("flyking");  
MODULE_LICENSE("GPL"); 
