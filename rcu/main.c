#include <linux/module.h>
#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/kthread.h>
#include <linux/delay.h>
#include <linux/spinlock_types.h>
#include <linux/spinlock.h>
#include <linux/slab.h>
#include <linux/rcupdate.h>

MODULE_LICENSE("GPL");
MODULE_AUTHOR("zhaozhenlong");
MODULE_DESCRIPTION("It's a test module.");

#define MAX_THREADS	10
static unsigned long reader_bitmap;

static void set_reader_number(int reader){
	WARN_ON((MAX_THREADS - reader) != 1);

	reader_bitmap = 0;

	while(reader){
		reader_bitmap |= (1 << --reader);
	}
}

struct our_data{
	int cnt1;
	int cnt2;
#ifdef CALL_RCU
	struct rcu_head rhead;
#endif 
};

static struct our_data mydata;
static struct our_data __rcu *pmydata = &mydata;

struct task_struct *threads[MAX_THREADS];


static void reader_do(void){
	struct our_data *data;
	rcu_read_lock();
	data = rcu_dereference(pmydata);
	printk("read cnt1: %d, cnt2: %d\n", data->cnt1, data->cnt2);
	rcu_read_unlock();
}

#ifdef CALL_RCU
static void rcu_free(struct rcu_head *head){
	struct our_data *data;
	data = container_of(head, struct our_data, rhead);
	kfree(data);
}
#endif

static void writer_do(void){
	struct our_data *data, *tmp = pmydata;
	
	data = kmalloc(sizeof(*data), GFP_KERNEL); 	
	if(!data)
		return;
	memcpy(data, pmydata, sizeof(*data));
	data->cnt1++;
	data->cnt2 += 10;
	
	rcu_assign_pointer(pmydata, data);
	if (tmp != &mydata){
#ifdef CALL_RCU
		call_rcu(&tmp->rhead, rcu_free); 
#else
		synchronize_rcu();
		kfree(tmp);
#endif 
	}
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
		msleep(100);
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
#ifdef CALL_RCU
	int call_rcu = 1;
#else
	int call_rcu = 0;
#endif
	printk("call %s\n", call_rcu? "call rcu" : "sync rcu");
	printk("init mydata: cnt1:%d, cnt2:%d\n", mydata.cnt1, mydata.cnt2);
	set_reader_number(9);
	if(create_threads())
		goto err;
	return 0;
err:
	cleanup_threads();
	return -1;
}

static __exit void mexit(void){
	printk("call %s\n", __FUNCTION__);
	printk("exit mydata: cnt1:%d, cnt2:%d\n", pmydata->cnt1, pmydata->cnt2);
	cleanup_threads();
}
module_init(minit)
module_exit(mexit)
