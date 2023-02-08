#include <types.h>
#include <kern/errno.h>
#include <kern/unistd.h>
#include <kern/wait.h>
#include <lib.h>
#include <syscall.h>
#include <current.h>
#include <proc.h>
#include <thread.h>
#include <addrspace.h>
#include <copyinout.h>
#include "opt-A1.h"
#include <clock.h>
#include <mips/trapframe.h>
#include <array.h>
#include <synch.h>

  /* this implementation of sys__exit does not do anything with the exit code */
  /* this needs to be fixed to get exit() and waitpid() working properly */

void sys__exit(int exitcode) {

  struct addrspace *as;
  struct proc *p = curproc;
  /* for now, just include this to keep the compiler from complaining about
     an unused variable */
  (void)exitcode;

  DEBUG(DB_SYSCALL,"Syscall: _exit(%d)\n",exitcode);

  KASSERT(curproc->p_addrspace != NULL);
  as_deactivate();
  /*
   * clear p_addrspace before calling as_destroy. Otherwise if
   * as_destroy sleeps (which is quite possible) when we
   * come back we'll be calling as_activate on a
   * half-destroyed address space. This tends to be
   * messily fatal.
   */
  as = curproc_setas(NULL);
  as_destroy(as);

#if OPT_A1
  //iterate through the array of children
  for (unsigned int i = 0; i < array_num(p->p_children); i++) {
    //copy the child process
    struct proc *temp_child = (struct proc *)array_get(p->p_children, i);

    //remove the child process from the parent's array
    array_remove(p->p_children, i);

    //check the p_exitstatus of it
    spinlock_acquire(&(temp_child->p_lock));
    if (temp_child->p_exitstatus == 1)
    {
      //destroy the child process
      spinlock_release(&(temp_child->p_lock));
      proc_destroy(temp_child);
    } 
    else {
      //set the p_parent to NULL
      temp_child->p_parent = NULL;
      spinlock_release(&(temp_child->p_lock));
    }
  }
#endif

  /* detach this thread from its process */
  /* note: curproc cannot be used after this call */
  proc_remthread(curthread);

#if OPT_A1
  //Remove the proc_destroy() function
  //Replace it with the following code

  spinlock_acquire(&p->p_lock);

  if (p->p_parent == NULL || p->p_parent->p_exitstatus == 1) {
    spinlock_release(&p->p_lock);
    proc_destroy(p);
  } else {
    p->p_exitstatus = 1;
    p->p_exitcode = (int) exitcode;
    spinlock_release(&p->p_lock);
  }

#else
  /* if this is the last user process in the system, proc_destroy()
     will wake up the kernel menu thread */

  //proc_destroy(p);

#endif
  
  thread_exit();
  /* thread_exit() does not return, so we should never get here */
  panic("return from thread_exit in sys_exit\n");
}


/* stub handler for getpid() system call                */
int
sys_getpid(pid_t *retval)
{
  /* for now, this is just a stub that always returns a PID of 1 */
  /* you need to fix this to make it work properly */
#if OPT_A1
  *retval = curproc->p_pid;
#else
  *retval = 1;
#endif
  return(0);
}


/* stub handler for waitpid() system call                */
#if OPT_A1
int
sys_fork(pid_t * retval, struct trapframe *tf)
{
  struct proc *p = curproc;
  struct proc *child_proc = proc_create_runprogram("child");
  if (child_proc == NULL) {
    return ENOMEM;
  }

  //Set the parent pointer of child process
  child_proc->p_parent = p;

  //Add the pointer of child process to the array of parent's p_children
  int re = array_add(p->p_children, (void *)child_proc, NULL);
  if (re != 0) {
    proc_destroy(child_proc);
    return EMPROC;
  }

  //copy address space of current process to child process
  re = as_copy(curproc_getas(), &(child_proc->p_addrspace));
  if (re != 0) {
    proc_destroy(child_proc);
    return EMPROC;
  }

  //copy trapframe of current process to child process
  struct trapframe *child_tf = kmalloc(sizeof(struct trapframe));
  memcpy(child_tf, tf, sizeof(struct trapframe)); 
  if (child_tf == NULL) {
    proc_destroy(child_proc);
    return ENOMEM;
  }

  //create thread for child process
  re = thread_fork("child thread", child_proc, (void *)&enter_forked_process, child_tf, 0);
  if (re != 0) {
    proc_destroy(child_proc);
    return ENOMEM;
  }

  *retval = child_proc->p_pid;

  clocksleep(1);
  
  return(0);
}
#endif


/* stub handler for waitpid() system call                */
int
sys_waitpid(pid_t pid,
	    userptr_t status,
	    int options,
	    pid_t *retval)
{
  int exitstatus;
  int result;

  /* this is just a stub implementation that always reports an
     exit status of 0, regardless of the actual exit status of
     the specified process.   
     In fact, this will return 0 even if the specified process
     is still running, and even if it never existed in the first place.

     Fix this!
  */

  if (options != 0) {
    return(EINVAL);
  }
  /* for now, just pretend the exitstatus is 0 */

#if OPT_A1
  struct proc *p = curproc;
  struct proc *temp_child = NULL;

  //iterate through the array of children
  for(unsigned int i = 0; i < array_num(p->p_children); i++) {
    //copy the child process
    struct proc *cur_child = (struct proc *)array_get(p->p_children, i);

    //check the pid of it
    if(cur_child->p_pid == pid) {
      temp_child = cur_child;
      array_remove(p->p_children, i);
      break;
    }
  }

  if (temp_child == NULL) {
    *retval = -1;
    return ESRCH;
  }

  // Use busy polling to wait for the child process to exit
  spinlock_acquire (&temp_child ->p_lock);
  while (!temp_child ->p_exitstatus) {
    spinlock_release (&temp_child ->p_lock);
    clocksleep (1);
    spinlock_acquire (&temp_child ->p_lock);
  }
  spinlock_release (&temp_child ->p_lock);

  //Extract the exit code of the child process and destroy it
  exitstatus = (int)temp_child->p_exitcode;
  proc_destroy(temp_child);

  //Pass the exit code to the _MKWAIT_EXIT and assign it to exitstatus
  exitstatus = _MKWAIT_EXIT(exitstatus);
#else
  exitstatus = 0;
#endif

  result = copyout((void *)&exitstatus,status,sizeof(int));
  if (result) {
    return(result);
  }
  *retval = pid;
  return(0);
}

