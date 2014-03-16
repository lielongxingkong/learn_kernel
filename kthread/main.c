#include <linux/module.h>
#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/kthread.h>

MODULE_LICENSE("GPL");
MODULE_AUTHOR("zhaozhenlong");
MODULE_DESCRIPTION("It's a test module.");

#define MAX_THREADS	10
struct task_struct *threads[MAX_THREADS];
static int thread_do(void *data){
	printk("run...\n");
	return 0;
}

static int create_threads(void){
	int i;
	for(i = 0; i < MAX_THREADS; i++){
		struct task_struct *thread;
		thread = kthread_create(thread_do, NULL, "thread-%d", i);
		if(IS_ERR(thread))
			return -1;
		threads[i] = thread;
	}
	return 0;
}

static int cleanup_threads(void){
	int i;
	for(i = 0; i < MAX_THREADS; i++){
		kthread_stop(threads[i]);	
	}
	return 0;
}

static __init int minit(void){
	printk("call %s\n", __FUNCTION__);
	if(create_threads())
		goto err;
	return 0;
err:
	cleanup_threads();
	return -1;
}

static __exit void mexit(void){
	printk("call %s\n", __FUNCTION__);
	cleanup_threads();
}
module_init(minit)
module_exit(mexit)
