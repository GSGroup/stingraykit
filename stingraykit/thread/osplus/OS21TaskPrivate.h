#ifndef STINGRAYKIT_THREAD_OSPLUS_OS21TASKPRIVATE_H
#define STINGRAYKIT_THREAD_OSPLUS_OS21TASKPRIVATE_H

// Copyright (c) 2011 - 2017, GS Group, https://github.com/GSGroup
// Permission to use, copy, modify, and/or distribute this software for any purpose with or without fee is hereby granted,
// provided that the above copyright notice and this permission notice appear in all copies.
// THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS.
// IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS,
// WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.


#include <os21/task.h>
#include <os21/typedefs.h>
#include <os21/gdbsupport.h>


/* --------------------------------------------------------------------- */
/* st-toolset/ST40R5.1.0/sh-superh-elf/src/os21/src/os21/include/_list.h */

/*
 * An OS21 list node consists of:
 *
 *      A pointer to the object owning this node.
 *	A pointer to some attribute which can be used in ordering a list.
 *	A pointer to the next node (NULL if none).
 */
typedef struct os21_list_node_s os21_list_node_t;
struct _OS21_GDB_STRUCT(os21_list_node_s)
{
  void             * _OS21_GDB_FIELD(0, OS21_LIST_NODE, datap);
  void             * _OS21_GDB_FIELD(0, OS21_LIST_NODE, attrp);
  os21_list_node_t * _OS21_GDB_FIELD(0, OS21_LIST_NODE, nextp);
};

typedef struct os21_list_s os21_list_t;

/*
 * Definitions of function pointers, as used with os21_list_t.
 */
typedef void   (*OS21_LIST_FUNC) (os21_list_t * lp, os21_list_node_t * lnp);
typedef void   (*OS21_RQ_FUNC)   (os21_list_t * lp);

/*
 * An OS21 list is a linked list of os21_list_node_t. It consists of:
 *
 *	A pointer to the first node on the list.
 *	A pointer to the last node on the list.
 *	A pointer to a function used to enqueue a node on the list.
 *	A pointer to a requeue callback, which can be used as required.
 *	A flag to indicate if we are a priority ordered list.
 *
 */
struct _OS21_GDB_STRUCT(os21_list_s)
{
  os21_list_node_t * _OS21_GDB_FIELD(0, OS21_LIST, frontp);
  os21_list_node_t * _OS21_GDB_FIELD(0, OS21_LIST, backp);

  OS21_LIST_FUNC nq_func;
  OS21_RQ_FUNC   rq_callback;

  int            isPri;
};

/*
 * An empty list.
 */
#define _OS21_LIST_EMPTY        {NULL, NULL, NULL, NULL, NULL}

/* --------------------------------------------------------------------- */
/* st-toolset/ST40R5.1.0/sh-superh-elf/src/os21/src/os21/include/_task.h */

#define OS21_MAX_TASK_NAME (15)

#define OS21_MAX_PRIORITY  (OS21_MAX_USER_PRIORITY)
#define OS21_MIN_PRIORITY  (OS21_MIN_USER_PRIORITY)

_OS21_GDB_BEGIN_EXPORT
typedef enum
{
  task_state_active      = 0,	/* Task is running or able to run */
  task_state_terminated  = 1,	/* Task has completed. */

/*task_state_suspending  = 2,   Now obsolete! */

  task_state_suspended   = 3,	/* Task has been suspended and is on the suspend Q */
  task_state_deleted     = 4

} task_state_t;
_OS21_GDB_END_EXPORT

#if ! defined CONF_NO_TIME_ACCOUNTING

typedef struct task_time_s         task_time_t;

struct task_time_s
{
  osclock_t start;		/* A start time. This is a state placeholder */
  osclock_t elapsed;		/* The actual time counter */
  osclock_t preempt;		/* Time spent on CPU whilst being preempted to front */
};

#endif

struct _OS21_GDB_STRUCT(task_s)
{
  unsigned int *    _OS21_GDB_FIELD(0, OS21_TASK, stackp);	/* Current stack pointer */
  task_state_t      _OS21_GDB_FIELD(0, OS21_TASK, state);	/* Current state */
  os21_list_t *     runq_obj;
  os21_list_node_t  runq_node;

#if ! defined CONF_NO_TIME_ACCOUNTING

  int               preemptedToFront;	/* 1 if this task was preempted to the front, 0 otherwise */
  task_time_t       time;		/* Used for time logging */

#endif

  void *            td_handle;          /* Per task handle for toolset interface */
  int               priority;		/* Effective priority of this task */
  int               user_priority;	/* User programmed priority of this task */

  os21_list_t *     sync_obj;
  os21_list_node_t  sync_node;
  unsigned int      event_mask;         /* Event flag wait mask, when waiting, Flag state when woken */
  int               event_all;          /* Flag - conjunctive or disjunctive wait */
  int               lock_count;         /* Count which is used to lock this task */
  int               suspend_count;

  os21_list_t *     time_obj;
  os21_list_node_t  time_node;
  osclock_t         timeout;		/* Task timeout if on timer queue */
  int               timeout_status;	/* Did this task time out... */
  void *            datap;		/* Per task user data */

  int               immortal_count;
  os21_list_t       pri_mut_owned;	/* LIFO queue of priority mutexes this task owns */
  os21_list_t       exit_handlers;	/* List of exit handlers for this task. */
  os21_list_t       private_data;	/* List of private data. */

  int               kill_when_mortal;
  int               kill_status;
  task_kill_flags_t kill_flags;
  int               exit_in_progress;

  char *            stack_basep;	/* Pointer to base of stack */
  size_t            stack_size;		/* size of stack in bytes */

  task_stack_fill_pattern_t pattern;	/* Stack fill pattern */
  int               stack_filled;       /* Was the stack filled at creation? */

  void             (*funcp)(void *);	/* Starting function pointer */

  char *           stack_memp;		/* Stack we allocated originally */
  partition_t *    pp;			/* Partition this task was allocated from */
  partition_t *  spp;			/* Partition this task's stack is allocated from */

  int              exit_value;          /* Value passed to task_exit() */

  os21_list_node_t task_list_node;	/* List node for putting in the task list */
  unsigned int     _OS21_GDB_FIELD(0, OS21_TASK, task_number);
  char             _OS21_GDB_FIELD(0, OS21_TASK, task_name) [OS21_MAX_TASK_NAME + 1];

  unsigned int   profile_count;         /* Profile info */
};

/*----------------------------------------------------------------------------*/

osclock_t _os21_TaskElapsedTime(task_t * arg)
{ return ((struct task_s *)arg)->time.elapsed; }

unsigned int _os21_TaskNumber(task_t * arg)
{ return ((struct task_s *)arg)->task_number; }

/*----------------------------------------------------------------------------*/

#define STACKP(t) (*(unsigned int *)(t))
#define CONTEXT(t) ((struct context_s *)STACKP(t))

void _os21_TaskPrivatePrint(task_t * arg = NULL) /* TODO: remove */
{
	task_t * tid = arg ? arg : task_id();
	struct task_s * taskp = (struct task_s *)tid;

	fprintf(stderr,
			"stackp           =%p   \n"
			"state            =%d   \n"
			"time.start       =%lld \n"
			"time.elapsed     =%lld \n"
			"stack_basep      =%p   \n"
			"stack_size       =%ul  \n"
			"task_number      =%d   \n"
			"task_name        =%s   \n"
			, taskp->stackp
			, taskp->state
			, taskp->time.start
			, taskp->time.elapsed
			, taskp->stack_basep
			, taskp->stack_size
			, taskp->task_number
			, taskp->task_name
			);
}


#endif
