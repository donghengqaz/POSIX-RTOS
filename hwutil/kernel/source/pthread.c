/*
 * File         : mutex.c
 * This file is part of POSIX-RTOS
 * COPYRIGHT (C) 2015 - 2016, DongHeng
 * Change Logs:
 * DATA             Author          Note
 * 2015-11-26       DongHeng        create
 */

#include "pthread.h"
#include "sched.h"
#include "debug.h"
#include "stdlib.h"
#include "string.h"

/*@{*/
#define USR_INIT_TYPE_DEFAULT       PTHREAD_TYPE_USER
#define USR_INIT_PRIO_DEFAULT       16
#define USR_INIT_TICK_DEFAULT       1
#define USR_INIT_STK_ADDR_DEFAULT   NULL
#define USR_INIT_STK_SIZE_DEFAULT   512

#define PTHREAD_STACK_SIZE_MIN      256

/*@}*/

/*@{*/

/*
 * pthread_setname_np - the function will name the thread
 *
 * @param thread the handle of the thread;
 * @param name the name of the thread;
 *
 * @return the result
 */
int pthread_setname_np(pthread_t thread, const char *name) {
	os_pthread_t *pthread = (os_pthread_t *) thread;

	memcpy(pthread->name, name, STDOBJ_NAME_MAX - 1);
	pthread->name[STDOBJ_NAME_MAX - 1] = '\0';

	return 0;
}

/*
 * pthread_attr_setstacksize - the function will set the stack address point and
 *                             the stack size of the thread
 *
 * @param attr the thread attribute point
 * @param stack_addr the stack address point
 * @param stack_size the stack size
 *
 * @return the result
 */
int pthread_attr_setstack (pthread_attr_t *RESTRICT attr,
		void *stack_addr,
		size_t stack_size)
{
	if (stack_size < PTHREAD_STACK_SIZE_MIN)
	return -EINVAL;

	if (!PHYS_ADDR_ALIGN(stack_addr))
	return -EINVAL;

	attr->stk_addr = stack_addr;
	attr->stk_size = stack_size;

	return 0;
}

/*
 * pthread_attr_setstacksize - the function will set the stack size of
 *                             the thread
 *
 * @param attr the thread attribute point
 * @param stack_size the stack size
 *
 * @return the result
 */
int pthread_attr_setstacksize(pthread_attr_t *RESTRICT attr,
		size_t stack_size)
{
	if (stack_size <= PTHREAD_STACK_SIZE_MIN)
	return -EINVAL;

	attr->stk_addr = NULL;
	attr->stk_size = stack_size;

	return 0;
}

/*
 * pthread_attr_setschedparam - the function will set the paramer of the thread
 *                              of the scheduler
 *
 * @param attr the thread attribute point
 * @param param the paramer point of the thread
 *
 * @return the result
 */
int pthread_attr_setschedparam(pthread_attr_t *RESTRICT attr,
		const struct sched_param *restrict param)
{

	if (param->type != PTHREAD_TYPE_USER
			&& param->type != PTHREAD_TYPE_IDLE
			&& param->type != PTHREAD_TYPE_KERNEL)
	return -EINVAL;

	if (param->init_prio > PTHREAD_PRIORITY_MAX)
	return -EINVAL;
	if (param->init_ticks == 0)
	return -EINVAL;

	attr->type = param->type;
	attr->init_prio = param->init_prio;
	attr->init_ticks = param->init_ticks;

	return 0;
}

/*
 * __pthread_attr_check - the function will check if the thread attribute
 *                        is legal
 *
 * @param pthread_attr the thread attribute point
 *
 * @return the result
 */
bool __pthread_attr_check(const pthread_attr_t *RESTRICT pthread_attr)
{
	if (NULL == pthread_attr)
	return true;

	if (pthread_attr->stk_addr && !PHYS_ADDR_ALIGN(pthread_attr->stk_addr))
	return false;
	if (pthread_attr->stk_size < PTHREAD_STACK_SIZE_MIN)
	return false;
	if (pthread_attr->init_prio > PTHREAD_PRIORITY_MAX)
	return false;
	if (pthread_attr->init_ticks == 0)
	return false;

	if (pthread_attr->type != PTHREAD_TYPE_USER
			&& pthread_attr->type != PTHREAD_TYPE_IDLE
			&& pthread_attr->type != PTHREAD_TYPE_KERNEL)
	return false;

	return true;
}

/*
 * __pthread_exit_entry - the function is the entry of the thread exiting
 *
 * @param arg the return data of the thread
 */
void __pthread_exit_entry(void *arg) {
	phys_reg_t temp;
	os_pthread_t *thread;

	temp = hw_interrupt_suspend();

	thread = get_current_thread();

	thread->ret = arg;

	sched_delete_thread(thread);

	hw_interrupt_recover(temp);
}

/*
 * __pthread_exit_entry - the function is the entry of the thread exiting
 *
 * @param arg the return data of the thread
 */
void __pthread_int_exit_entry(void *arg) {
	phys_reg_t temp;
	os_pthread_t *thread;
	extern void hw_context_switch_to(phys_reg_t sp);

	temp = hw_interrupt_suspend();

	thread = get_current_thread();

	thread->status = PTHREAD_STATE_READY;

	hw_context_switch_to((phys_reg_t) &thread->sp);

	hw_interrupt_recover(temp);
}

/*
 * __pthread_create - the function will create a physics thread  
 *
 * @param pthread_attr the thread attribute point
 * @param start_routine the usr thread entry
 * @param arg the user point
 *
 * @return the thread handle
 */
os_pthread_t* __pthread_create(const pthread_attr_t *RESTRICT pthread_attr,
		void *(*start_routine)(void*),
		void *RESTRICT arg)
{
	os_pthread_t *pthread;
	pthread_attr_t thread_attr_init;
	extern char* pthread_hw_stack_init(void* entry(void *),
			void *paramter,
			char *stack,
			void exit(void *));

	if (!pthread_attr)
	{
		thread_attr_init.type = USR_INIT_TYPE_DEFAULT;
		thread_attr_init.init_prio = USR_INIT_PRIO_DEFAULT;
		thread_attr_init.init_ticks = USR_INIT_TICK_DEFAULT;
		thread_attr_init.stk_addr = USR_INIT_STK_ADDR_DEFAULT;
		thread_attr_init.stk_size = USR_INIT_STK_SIZE_DEFAULT;
	}
	else
	memcpy(&thread_attr_init, pthread_attr, sizeof(pthread_attr_t));

	if (!(pthread = calloc(sizeof(os_pthread_t))))
	return NULL;

	if (!thread_attr_init.stk_addr)
	{
		if (!(pthread->stk_addr = malloc(thread_attr_init.stk_size)))
		goto free_thread;
	}
	else
	pthread->stk_addr = thread_attr_init.stk_addr;

	list_init(&pthread->list);
	list_init(&pthread->sigevent_list);
	list_init(&pthread->sig_list);

	pthread->start_routine = start_routine;
	pthread->arg = arg;

	pthread->stk_size = thread_attr_init.stk_size;

	pthread->init_prio = thread_attr_init.init_prio;
	pthread->cur_prio = pthread->init_prio;
	pthread->prio_mask = 1 << pthread->cur_prio;

	pthread->init_ticks = thread_attr_init.init_ticks;
	pthread->cur_ticks = pthread->init_ticks;

	pthread->status = PTHREAD_STATE_INIT;
	pthread->type = thread_attr_init.type;

	pthread->sp = pthread_hw_stack_init(pthread->start_routine,
			pthread->arg,
			(char *)((size_t)pthread->stk_addr + pthread->stk_size - 4),
			__pthread_exit_entry);

	return pthread;

	free_thread:
	free(pthread);

	return 0;
}

/*
 * pthread_create - the function will create a thread and set its state initing
 *
 * @param pthread the handle of the pthread
 * @param pthread_attr the thread atrribute point
 * @param start_routine the usr thread entry
 * @param arg the user point
 *
 * @return the result
 */
int pthread_create (pthread_t *RESTRICT pthread,
		const pthread_attr_t *RESTRICT pthread_attr,
		void *(*start_routine)(void*),
		void *RESTRICT arg)
{
	os_pthread_t *pthread_new;

	if (__pthread_attr_check(pthread_attr) == false)
	return -EPERM;

	if (!(pthread_new = __pthread_create(pthread_attr, start_routine, arg)))
	return -EAGAIN;

	sched_insert_thread(pthread_new);

	*pthread = (pthread_t)pthread_new;

	return 0;
}

/*
 * __pthread_int_init - the function will init the stack for thread soft interrupt
 *
 * @param pthread the handle of the pthread
 * @param start_routine the usr thread entry
 * @param arg the user point
 *
 * @return the result
 */
int __pthread_int_init (os_pthread_t *pthread,
		void *(*start_routine)(void*),
		void *RESTRICT arg)
{
	extern char* pthread_hw_stack_init(void* entry(void *),
			void *paramter,
			char *stack,
			void exit(void *));

	pthread->int_sp = pthread_hw_stack_init(start_routine,
			arg,
			(char *)((size_t)pthread->sp - 8),
			__pthread_int_exit_entry);

	return 0;
}

/*
 * pthread_mutex_init - the function will init the mutex
 *
 * @param mutex the point of the mutex
 * @param attr  the attribute of the mutex
 *
 * @return the result
 */
int pthread_mutex_init(pthread_mutex_t *mutex, const pthread_mutexattr_t *attr)
{
	mutex->own_thread = NULL;

	list_init(&mutex->wait_list);

	return 0;
}

/*
 * __pthread_mutex_lock - the function will try to take the mutex if it is 
 *                        now owned, otherwise will suspend the current thread
 *
 * @param mutex the point of the mutex
 *
 * @return the result of taking the mutex
 */
INLINE int __pthread_mutex_lock(pthread_mutex_t *mutex) {
	phys_reg_t temp;
	os_pthread_t *thread;
	int ret;

	/* check if the os is running */
	if ((thread = PTHREAD_POINT(get_current_thread())) == NULL) {
		return 0;
	}

	/* suspend the hardware interrupt for atomic operation */
	temp = hw_interrupt_suspend();

	/* if the mutex is unlocked */
	if (NULL == mutex->own_thread) {
		/* the current thread owns the mutex */
		mutex->own_thread = thread;

		ret = 0;
	} else /* if the mutex is locked */
	{
		/* if the thread already has the lock */
		if (thread == mutex->own_thread) {
			ret = -EDEADLK;
		} else {
			/* priority inversion */
			if (thread->cur_prio > (mutex->own_thread)->cur_prio) {
				/* change the priority of the thread owned the mutex, then wakeup it */
				/*
				 thread_ctrl(mutex->own_thread,
				 THREAD_START | THREAD_CHANGE_PRIORITY,
				 thread->cur_priority);
				 */
			}
			/* set current thread wait state */
			sched_set_thread_suspend(thread);

			/* insert the current thread to the wait list */
			list_insert_tail(&mutex->wait_list, &thread->list);

			sched_switch_thread();

			ret = -EAGAIN;
		}
	}

	hw_interrupt_recover(temp);

	return ret;
}

/*
 * pthread_mutex_lock - the function will try to take the mutex if it is 
 *                      now owned, otherwise will suspend the current thread
 *                      for the times of lock recursion times
 *
 * @param mutex the point of the mutex
 *
 * @return the result of taking the mutex
 */
int pthread_mutex_lock(pthread_mutex_t *mutex) {
	int i;
	int ret;

	for (i = MUTEX_RECURSIVE_MAX; i > 0; i--) {
		if ((ret = __pthread_mutex_lock(mutex)) != -EAGAIN)
			break;
	}

	return ret;
}

/*
 * the function will try to release the mutex if it is now owned, otherwise will 
 * suspend the current thread
 *
 * @param mutex the point of the mutex
 *
 * @return the result of releasing the mutex
 */
int pthread_mutex_unlock(pthread_mutex_t *mutex) {
	int ret;
	phys_reg_t temp;
	os_pthread_t *thread;

	/* check if the os is running */
	if ((thread = PTHREAD_POINT(get_current_thread())) == NULL)
		return 0;

	/* suspend the hardware interrupt for atomic operation */
	temp = hw_interrupt_suspend();

	/* check if current thread owns it */
	if (thread == mutex->own_thread) {
		os_pthread_t *thread_wait;

		mutex->own_thread = NULL;

		/* recover the prioity current thread */
		if (thread->cur_prio != thread->init_prio) {
			/*
			 thread_ctrl(thread,
			 THREAD_START | THREAD_CHANGE_PRIORITY,
			 thread->init_priority);
			 */
		}

		/* wakeup one thread in the wait queue */
		LIST_FOR_EACH_HEAD_NEXT(thread_wait,
				&mutex->wait_list,
				os_pthread_t,
				list) {
			sched_set_thread_ready(thread_wait);
			sched_switch_thread();
			break;
		}

		ret = 0;
	} else {
		ret = -EINVAL;
	}

	hw_interrupt_recover(temp);

	return ret;
}

/*@}*/
