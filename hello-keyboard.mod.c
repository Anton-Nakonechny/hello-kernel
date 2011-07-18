#include <linux/module.h>
#include <linux/vermagic.h>
#include <linux/compiler.h>

MODULE_INFO(vermagic, VERMAGIC_STRING);

struct module __this_module
__attribute__((section(".gnu.linkonce.this_module"))) = {
 .name = KBUILD_MODNAME,
 .init = init_module,
#ifdef CONFIG_MODULE_UNLOAD
 .exit = cleanup_module,
#endif
 .arch = MODULE_ARCH_INIT,
};

static const struct modversion_info ____versions[]
__used
__attribute__((section("__versions"))) = {
	{ 0x1e574c72, "module_layout" },
	{ 0xa90c928a, "param_ops_int" },
	{ 0xfaef0ed, "__tasklet_schedule" },
	{ 0x3c2c5af5, "sprintf" },
	{ 0x6128b5fc, "__printk_ratelimit" },
	{ 0x859c6dc7, "request_threaded_irq" },
	{ 0x9545af6d, "tasklet_init" },
	{ 0x1a57fb19, "create_proc_entry" },
	{ 0xb50360b7, "proc_mkdir" },
	{ 0xb3decf43, "kmem_cache_alloc" },
	{ 0xd391e6e0, "malloc_sizes" },
	{ 0x37a0cba, "kfree" },
	{ 0xf20dabd8, "free_irq" },
	{ 0x50eedeb8, "printk" },
	{ 0x82072614, "tasklet_kill" },
	{ 0xc7a40c01, "remove_proc_entry" },
};

static const char __module_depends[]
__used
__attribute__((section(".modinfo"))) =
"depends=";

