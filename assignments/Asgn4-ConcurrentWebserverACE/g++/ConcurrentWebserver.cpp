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



#define POOL_SIZE 5
#define MAX_TIMEOUT 5

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


/**
 * @class Echo_Task
 * @brief 
 *
 * Create an Echo_Task that inherits from ACE_Task (configured with the ACE_MT_SYNCH traits class to obtain a synchronized request queue) 
 */
class Echo_Task : public ACE_Task < ACE_MT_SYNCH >
{
public:
  Echo_Task()
  { }

  virtual int svc(void)
  {
    ACE_TRACE(ACE_TEXT("Workers::svc"));

    ACE_DEBUG((LM_INFO, ACE_TEXT("(%t) Workers started\n")));
    while (1)
      {
	ACE_Message_Block *mb = NULL;

	if (this->getq(mb) == -1)
	  {
	    ACE_DEBUG((LM_INFO,
		       ACE_TEXT("(%t) Shutting down\n")));
	    break;
	  }


        ACE_DEBUG((LM_INFO, ACE_TEXT("(%t) Call process_message\n")));

	//First sends back to the client [ACE_SOCK_Stream] the thread id [ACE_OS::thr_self()] that is running the svc() method 
	//char threadId[256];
	//size_t threadIdSize = ACE_OS_thr_id(threadId);
	// if ( this->send(threadId, threadIdSize) < 0)
	//   {
	//     ACE_ERROR_RETURN((LM_ERROR,
	// 		      "%s\n",
	// 		      "send_n failed"),
	// 		     -1);
	//   }

	// Process the message.	
	process_message(mb);
      }

    return 0;
  }
private:
  void process_message(ACE_Message_Block *mb)
  {
    ACE_DEBUG((LM_INFO, ACE_TEXT("(%t) Workers::process_message\n")))
    ACE_TRACE("Workers::process_message");
    char buf[ACE_DEFAULT_MAX_SOCKET_BUFSIZ];
		


    size_t length = mb->length();
		
    if (length > 0)
      ACE_DEBUG((LM_DEBUG,
		 "(%t) in iteration length = %d, text = \"%*s\"\n",
		 length,
		 length - 1,
		 mb->rd_ptr()));
    ACE_OS::memcpy(buf, mb->rd_ptr(), length);
    mb->release();

    ACE_DEBUG((LM_DEBUG,
	       ACE_TEXT("(%t) Started processing message %s\n"),
	       buf));

    /// This sleep emulates a long operation 
    ACE_OS::sleep(3); 

    ACE_DEBUG((LM_DEBUG,
	       ACE_TEXT("(%t) Finished processing message %s\n"),
	       buf));
  }
};


/**
 * @class Echo_Svc_Handler
 * @brief Service handler using TCP sockets stream (using a wrapper facade SOCK_STREAM)
 *
 * Create an Echo_Svc_Handler that inherits from ACE_Svc_Handler 
 * (configured with the ACE_SOCK_STREAM class and ACE_NULL_SYNCH traits class) 
 * and implement its handle_input() hook method so that it :
 * Reads the client data [ACE_SOCK_Stream] until the end of a line is reached 
 * (i.e., the symbols "\n", "\r", or "\r\n" are read). 
 * Puts the client data into a message [ACE_Message_Block], and 
 * Calls Echo_Task::put(), which uses ACE_Task::putq() to enqueue the message for 
 * subsequent processing by a thread in the pool of threads that are running the Echo_Task::svc() hook method.   
 */
class Echo_Svc_Handler : public ACE_Svc_Handler < ACE_SOCK_STREAM, ACE_NULL_SYNCH >
{

public:

  void echo_task(Echo_Task *et){
    echo_task_ = et;
  }; 

protected:

  virtual int handle_input(ACE_HANDLE)
  {
    ACE_TRACE ("Echo_Svc_Handler::handle_input");
    ACE_DEBUG((LM_DEBUG,
	       "(%t) Echo_Svc_Handler::handle_input\n"));
	
    /// TODO:  Reads the client data [ACE_SOCK_Stream] until the end of a line is reached 
    /// (i.e., the symbols "\n", "\r", or "\r\n" are read). 
    char buf[ACE_DEFAULT_MAX_SOCKET_BUFSIZ];
    ssize_t recv_cnt;

    recv_cnt = this->peer().recv(buf, sizeof(buf));
    if (recv_cnt <=0) {
      ACE_DEBUG ((LM_DEBUG, ACE_TEXT ("(% P |% t) connection closed \n")));
      return -1;
    }

    ACE_Message_Block *mb = 0;
    ACE_NEW_RETURN(mb, 
		   ACE_Message_Block(recv_cnt),
		   -1);

    ACE_OS::memcpy(mb->wr_ptr(), buf, recv_cnt);

    
    echo_task_->putq(mb);

    return 0;
  }
	
private:
  Echo_Task * echo_task_;
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

  void echo_task(Echo_Task *et){
    echo_task_ = et;
  }
  Echo_Task * echo_task(void){
    return echo_task_;
  }

  /**
   * Bridge method for creating a Echo_Svc_Handler.
   * Is overrided to perform Echo_Svc_Handler creation and assign
   * the Echo_Task
   * linking the handler, etc.).  Returns -1 on failure, else 0.
   */
  virtual int make_svc_handler(Echo_Svc_Handler *&sh)
  {
    ACE_TRACE("Echo_Acceptor::make_svc_handler");

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

private :
  Echo_Task *echo_task_;

};





/* Program's entry point */
int ACE_TMAIN(int argc, ACE_TCHAR *argv[])
{
  ACE_OS::printf("Usage: %s [port-number]\n", argv[0]);
  u_short port = argc < 2 ? ACE_DEFAULT_SERVER_PORT : ACE_OS::atoi(argv[1]);
  ACE_INET_Addr addr(port);

  ACE_Reactor *reactor = ACE_Reactor::instance();

  Echo_Task * echo_task;
  ACE_NEW_RETURN(echo_task,Echo_Task,-1);
  echo_task->activate(THR_NEW_LWP | THR_JOINABLE, POOL_SIZE);

  Echo_Acceptor acceptor;
  acceptor.echo_task(echo_task);	
  acceptor.open(addr, ACE_Reactor::instance());

  ACE_OS::printf("listening at port %d\n", port);

  reactor->run_reactor_event_loop();

  ACE_Thread_Manager::instance()->wait();

  delete echo_task;

  return 0;	
}
