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
#include "opt-A3.h"
#include <vm.h>
#include <vfs.h>
#include <test.h>
#include <kern/fcntl.h>

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
  // Fix the misundertanding in the A1
  unsigned int num_children = array_num(p->p_children);
  for (unsigned int i = 0; i < num_children; i++) {
    //copy the child process
    struct proc *temp_child = (struct proc *)array_get(p->p_children, 0);

    //remove the child process from the parent's array
    array_remove(p->p_children, 0);

    // Case it is NULL
    if (temp_child == NULL) {
      continue;
    }

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

  proc_destroy(p);

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


/* stub handler for fork() system call                */
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

#if OPT_A1
  struct proc *p = curproc;
  struct proc *temp_child = NULL;

  //iterate through the array of children
  // Fix the misundertanding in the A1
  unsigned int num_children = array_num(p->p_children);
  for(unsigned int i = 0; i < num_children; i++) {
    //copy the child process
    struct proc *cur_child = (struct proc *)array_get(p->p_children, 0);

    //Skip the child process if it is NULL
    if (cur_child == NULL) {
      continue;
    }

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
  // Fix the wrong place in A1
  if (options != 0) {
    return(EINVAL);
  }
  /* for now, just pretend the exitstatus is 0 */

  result = copyout((void *)&exitstatus,status,sizeof(int));
  if (result) {
    return(result);
  }
  *retval = pid;
  return(0);
}


# if OPT_A3
/* stub handler for execv() system call                */
int sys_execv(char *progname, char **argv){
  // Directly copy from runprogram with slight modification
  struct addrspace *as;
	struct vnode *v;
	vaddr_t entrypoint, stackptr;
	int result;

	/* Open the file. */
	result = vfs_open(progname, O_RDONLY, 0, &v);
	if (result) {
		return result;
	}

  /* Copy the arguments into kernel space. */
  char ** temp_argv = (char **) args_alloc();
  unsigned long argc = argcopy_in(temp_argv, argv);
  
  // Record the old address space
  struct addrspace *old_as = curproc_getas();

	/* Create a new address space. */
	as = as_create();
	if (as ==NULL) {
		vfs_close(v);
		return ENOMEM;
	}

	/* Switch to it and activate it. */
	curproc_setas(as);
	as_activate();

  /* Destroy the old address space. */
  as_destroy(old_as);

	/* Load the executable. */
	result = load_elf(v, &entrypoint);
	if (result) {
		/* p_addrspace will go away when curproc is destroyed */
		vfs_close(v);
		return result;
	}

	/* Done with the file now. */
	vfs_close(v);

	/* Define the user stack in the address space */
	result = as_define_stack(as, &stackptr);
	if (result) {
		/* p_addrspace will go away when curproc is destroyed */
		return result;
	}

  // End with NULL for argv
	char **new_argv = kmalloc(sizeof(vaddr_t *) * (argc + 1));

	// Copy the arguments into the stack
	for (unsigned long i = 0; i < argc; i++) {
		new_argv[i] = (char *)argcopy_out(&stackptr, temp_argv[i]);
	}
	// End with NULL for argv
	new_argv[argc] = NULL;

	// Round the stack pointer down to the nearest multiple of 4 as required by MIPS
	stackptr -= stackptr % sizeof(vaddr_t);
	stackptr -= sizeof(vaddr_t) * (argc + 1);

	// Copy the argv into the stack
	copyout(new_argv, (userptr_t)stackptr, sizeof(vaddr_t *) * (argc + 1));

  // Free the memory
	kfree(new_argv);
  args_free((void **)temp_argv);

	enter_new_process(argc, (userptr_t) stackptr, stackptr, entrypoint);

	/* enter_new_process does not return. */
	panic("enter_new_process returned\n");
	return EINVAL;
}
# endif

# if OPT_A3
// Use char ** to avoid tidious type casting in previous trial
char **args_alloc(void){
  //max number of arguments is 16, max number of characters is 128
  char **argv = (char **) kmalloc(sizeof(char *) * (16 + 1));
  for (unsigned int i = 0; i < 16; i++) {
    argv[i] = (char *) kmalloc(sizeof(char) * (128 + 1));
  }
  argv[16] = NULL;

  return argv;
}
# endif

# if OPT_A3
void args_free(void **args){
  // Free as maximum of 16 arguments
  for (unsigned int i = 0; i < 16; i++) {
    kfree(args[i]);
  }
  kfree(args);
  return;
}
# endif

# if OPT_A3
unsigned long argcopy_in(char **kern_argv, char **user_argv){
  unsigned long argc = 0;

  // Try as upper bound of 16
  for (int i = 0; i < 16; i++) {
    argc++;
    if (user_argv[i] == NULL) {
      break;
    }
    copyinstr((userptr_t)user_argv[i], kern_argv[i], strlen(user_argv[i]) + 1, NULL);
  }

  return argc;
}
# endif
