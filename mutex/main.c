#include <linux/module.h>
#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/kthread.h>
#include <linux/delay.h>
#include <linux/slab.h>
#include <linux/mutex.h>

MODULE_LICENSE("GPL");
MODULE_AUTHOR("zhaozhenlong");
MODULE_DESCRIPTION("It's a test module.");

#define MAX_THREADS	10
static unsigned long reader_bitmap;

static void set_reader_number(int reader){

	reader_bitmap = 0;

	while(reader){
		reader_bitmap |= (1 << --reader);
	}
}

struct our_data{
	int cnt1;
	int cnt2;
};

static struct our_data mydata;

struct task_struct *threads[MAX_THREADS];
DEFINE_MUTEX(mut);

static void reader_do(void){
	return;
}

static void writer_do(void){
	mutex_lock(&mut);
	mydata.cnt1++;
	mydata.cnt2 += 10;
	mutex_unlock(&mut);
}

static int thread_do(void *data){
	long i = (long)data;
	int reader = (reader_bitmap & (1 << i));

	printk("run..., %ld %s\n", i, reader? "reader" : "writer");
	while(!kthread_should_stop()){
		if(reader)
			reader_do();
		else 
			writer_do();
		msleep(10);
	}
	return 0;
}

static int create_threads(void){
	int i;
	for(i = 0; i < MAX_THREADS; i++){
		struct task_struct *thread;
		thread = kthread_run(thread_do, (void *)(long)i, "thread-%d", i);
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
	set_reader_number(0);
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
