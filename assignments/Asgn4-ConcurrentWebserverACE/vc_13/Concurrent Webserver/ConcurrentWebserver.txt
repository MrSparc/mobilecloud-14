// $Id$

/**
 * @file ConcurrentWebserver.cpp
 * @author Ariel Machado <arielgmachado AT gmail DOT com>
 *
 *  Programming Cloud Services for Android Handheld Systems
 *  Optional Assignment 4 - Concurrent Webserver (ACE C++)
 *
 *
 * ACE version: 6.2.7
 *
 * Tested environments:
 * OS: Linux 3.13.0-33-generic #58-Ubuntu SMP x86_64 GNU/Linux
 * g++ version 4.8.3 (Ubuntu 4.8.2-19ubuntu1)
 *
 * OS: Windows 7 (64 bits)
 * VC++ Express 2013 for Windows Desktop
 */

//When adding ACE tracing to an application one option is to add
#define ACE_NTRACE 1
// in the line above #include "ace/Log_Msg.h".
#include "ace/Log_Msg.h"

#include "ace/Svc_Handler.h"
#include "ace/Acceptor.h"
#include "ace/Reactor.h"
#include "ace/INET_Addr.h"
#include "ace/SOCK_Stream.h"
#include "ace/SOCK_Acceptor.h"
#include "ace/Task_T.h"
#include "ace/Message_Block.h"
#include "ace/OS_NS_Thread.h"


// Number of threads
#define POOL_SIZE 5

/* Stores a string version of the current thread id into buffer and
 * returns the size of this thread id in bytes.
 */
ssize_t ACE_OS_thr_id(char buffer[])
{
#if defined (ACE_WIN32)
  return ACE_OS::sprintf(buffer,
			 "Thread id: <%u>",
			 static_cast <unsigned> (ACE_Thread::self()));
#else
  ACE_hthread_t t_id;
  ACE_OS::thr_self(t_id);
  return ACE_OS::sprintf(buffer,
			 "Thread id: <%lu>",
			 (unsigned long)t_id);
#endif /* WIN32 */
}


// Forward declaration
class Echo_Svc_Handler;

/**
 * @class Echo_Task
 * @brief Echo task worker
 *
 * Create an Echo_Task that inherits from ACE_Task 
 * (configured with the ACE_MT_SYNCH traits class to obtain a synchronized request queue)
 */
class Echo_Task : public ACE_Task < ACE_MT_SYNCH >
{
public:
  Echo_Task();
  void echo_svc_handler(Echo_Svc_Handler *);
  virtual int svc(void);
  void process_message(ACE_Message_Block *);

private:
  Echo_Svc_Handler *echo_svc_handler_;
};


/**
 * @class Echo_Svc_Handler
 * @brief Service handler using TCP sockets stream (using a wrapper facade SOCK_STREAM)
 *
 * Create an Echo_Svc_Handler that inherits from ACE_Svc_Handler
 * (configured with the ACE_SOCK_STREAM class and ACE_NULL_SYNCH traits class)
 */
class Echo_Svc_Handler : public ACE_Svc_Handler < ACE_SOCK_STREAM, ACE_NULL_SYNCH >
{

public:
  void echo_task(Echo_Task *);
  virtual int handle_input(ACE_HANDLE);

private:
  Echo_Task *echo_task_;
};


/**
 * @class Echo_Acceptor
 * @brief Acceptor using TCP sockets stream
 *
 * Create an Echo_Acceptor that inherits from ACE_Acceptor (or uses a simple typedef)
 * and uses an Internet domain ''passive-mode'' stream socket to listen a designated
 * port number [ACE_Acceptor, ACE_SOCK_Acceptor, ACE_INET_Addr, etc.].
 */
class Echo_Acceptor : public  ACE_Acceptor <Echo_Svc_Handler, ACE_SOCK_ACCEPTOR > {
public:
  void echo_task(Echo_Task *);
  virtual int make_svc_handler(Echo_Svc_Handler *&);

private:
  Echo_Task *echo_task_;
};



Echo_Task::Echo_Task()
{
  ACE_DEBUG((LM_INFO,
	     ACE_TEXT("(%t) Echo_task::Constructor\n")));
}
	
void Echo_Task::echo_svc_handler( Echo_Svc_Handler *svc)
{
  echo_svc_handler_ = svc;
}

/// Implement its svc() hook method to perform the "half-sync"
int Echo_Task::svc(void)
{
  ACE_DEBUG((LM_INFO,
	     ACE_TEXT("(%t) Echo_task::svc\n")));

  while (1)
    {
      // Dequeueing messages (ACE_Message_Blocks obtained via ACE_Task::getq()) 
      // containing the client input that was put into its synchronized request queue
      ACE_Message_Block *mb = NULL;
      if (this->getq(mb) == -1)
	{
	  ACE_DEBUG((LM_INFO,
		     ACE_TEXT("(%t) Shutting down\n")));
	  break;
	}

      ACE_DEBUG((LM_INFO, 
		 ACE_TEXT("(%t) Call process_message\n")));

      process_message(mb);
    }

  return 0;
}

/// Process the message (sends back the thread_id and original message)
void Echo_Task::process_message(ACE_Message_Block *mb)
{
  ACE_DEBUG((LM_INFO,
	     ACE_TEXT("(%t) Echo_Task::process_message\n")));

  char buf[ACE_DEFAULT_MAX_SOCKET_BUFSIZ];

  size_t length = mb->length();

  ACE_DEBUG((LM_INFO,
	     "(%t) Message Length %d\n", length));

  ACE_OS::memcpy(buf, mb->rd_ptr(), length);
  mb->release();

  buf[length] = 0;

  ACE_DEBUG((LM_DEBUG,
	     ACE_TEXT("(%t) Started processing message: %s\n"),
	     buf));

  // Sends back to client the thread-id
  char tid[256];
  ssize_t tid_length = ACE_OS_thr_id(tid);
  echo_svc_handler_->peer().send(tid, tid_length);

  // Sends back to client the original data
  echo_svc_handler_->peer().send(buf, length);

  ACE_OS::sleep(3); /// This sleep emulates a long operation

  ACE_DEBUG((LM_DEBUG,
	     ACE_TEXT("(%t) Finished processing message: %s\n"),
	     buf));
}
	       

/// Setter method in order service handler use the thread pool
void Echo_Svc_Handler::echo_task(Echo_Task *et){
  echo_task_ = et;

  // The thread pool also holds a reference this service handler 
  // in order to send data back to the peer
  echo_task_->echo_svc_handler(this); 
}



//  Implement its handle_input() hook method to perform the "Half-Async" 
int Echo_Svc_Handler::handle_input(ACE_HANDLE)
{
  ACE_DEBUG((LM_DEBUG,
	     "(%t) Echo_Svc_Handler::handle_input\n"));


  // Reads the client data [ACE_SOCK_Stream] until the end of a line is reached
  char buf[ACE_DEFAULT_MAX_SOCKET_BUFSIZ];
  ssize_t recv_cnt;

  recv_cnt = this->peer().recv(buf, sizeof(buf));
  if (recv_cnt <= 0) {
    ACE_DEBUG((LM_DEBUG, ACE_TEXT("(%t) connection closed \n")));
    return -1;
  }

  // Puts the client data into a message [ACE_Message_Block]
  ACE_Message_Block *mb = 0;
  ACE_NEW_RETURN(mb,
		 ACE_Message_Block(recv_cnt),
		 -1);

  mb->copy(buf, recv_cnt);

  // Calls Echo_Task::put(), which uses ACE_Task::putq() to enqueue the message 
  // for subsequent processing by a thread in the pool of threads that are running
  // the Echo_Task::svc() hook method.  
  echo_task_->putq(mb);

  return 0;
}



	
// Echo_Task setter
void Echo_Acceptor::echo_task(Echo_Task *et)
{
  echo_task_ = et;
}
	
/**
 * Bridge method used to create the new service handler object [Echo_Svc_Handler].
 * Customized to perform Echo_Svc_Handler creation and set a pointer to the Echo_Task
 * Returns -1 on failure, else 0.
 */
int Echo_Acceptor::make_svc_handler(Echo_Svc_Handler *&sh)
{
  ACE_DEBUG((LM_DEBUG,
	     "(%t) Echo_Acceptor::make_svc_handler\n"));

  if (sh == 0)
    ACE_NEW_RETURN(sh,
		   Echo_Svc_Handler,
		   -1);

  /// Assign to each new handler the same thread pool
  sh->echo_task(this->echo_task_);

  // Set the reactor of the newly created <SVC_HANDLER> to the same
  // reactor that this <ACE_Acceptor> is using.
  sh->reactor(this->reactor());
  return 0;
}



/* Program's entry point */
int ACE_TMAIN(int argc, ACE_TCHAR *argv[])
{
  ACE_OS::printf("Usage: %s [port-number]\n", argv[0]);
  u_short port = argc < 2 ? ACE_DEFAULT_SERVER_PORT : ACE_OS::atoi(argv[1]);
  ACE_INET_Addr addr(port);

  ACE_DEBUG((LM_DEBUG,
	     "(%t) Program's entry point\n"));

  ACE_OS::printf("listening at port %d\n", port);

  // Implement a main() function that:

  //1. Creates an Echo_Task instance and have it spawn a pool of N threads
  // (where N > 1) within itself (Echo_Task::activate()).
  Echo_Task echo_task;
  Echo_Task *ptask = &echo_task;
  //Create POOL_SIZE kernel-level threads and allow the new threads to be joined with.
  echo_task.activate(THR_NEW_LWP | THR_JOINABLE, POOL_SIZE);  

  ///2. Creates an Echo_Acceptor instance and associate it with the Echo_Task.
  Echo_Acceptor acceptor;
  acceptor.echo_task(ptask); // Using a setter method defined in Echo_Acceptor

  //3. Creates an  ACE_Reactor(or use the singleton instance of the ACE_Reactor)
  ACE_Reactor *reactor = ACE_Reactor::instance();
	
  //4. Registers the Echo_Acceptor instance with the reactor
  acceptor.open(addr, reactor);

  //5. Run the reactor's event loop [ACE_Reactor::run_reactor_event_loop()] 
  // to wait for connections/data to arrive from a client. 
  reactor->run_reactor_event_loop();

  // Wait for all threads to exit
  ACE_Thread_Manager::instance()->wait();

  return 0;
}
