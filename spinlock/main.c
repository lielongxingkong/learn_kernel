#include <linux/module.h>
#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/kthread.h>
#include <linux/delay.h>
#include <linux/spinlock_types.h>
#include <linux/spinlock.h>

MODULE_LICENSE("GPL");
MODULE_AUTHOR("zhaozhenlong");
MODULE_DESCRIPTION("It's a test module.");

//static DEFINE_SPINLOCK(threads_lock);
static spinlock_t threads_lock;

static void threads_spinlock_init(void){
	spin_lock_init(&threads_lock);
}

struct our_data{
	int cnt1;
	int cnt2;
};

static struct our_data mydata;

#define MAX_THREADS	10
struct task_struct *threads[MAX_THREADS];
static int thread_do(void *data){
	printk("run...\n");
	while(!kthread_should_stop()){
		spin_lock(&threads_lock);
		mydata.cnt1++;
		mydata.cnt2 += 10;
		spin_unlock(&threads_lock);
		msleep(10);
	}
	return 0;
}

static int create_threads(void){
	int i;
	for(i = 0; i < MAX_THREADS; i++){
		struct task_struct *thread;
		thread = kthread_run(thread_do, NULL, "thread-%d", i);
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
	printk("init mydata: cnt1:%d, cnt2:%d\n", mydata.cnt1, mydata.cnt2);
	threads_spinlock_init();
	if(create_threads())
		goto err;
	return 0;
err:
	cleanup_threads();
	return -1;
}

static __exit void mexit(void){
	printk("call %s\n", __FUNCTION__);
	printk("exit mydata: cnt1:%d, cnt2:%d\n", mydata.cnt1, mydata.cnt2);
	cleanup_threads();
}
module_init(minit)
module_exit(mexit)
