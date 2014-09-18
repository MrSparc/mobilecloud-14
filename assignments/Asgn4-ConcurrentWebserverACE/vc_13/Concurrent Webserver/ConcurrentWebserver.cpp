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

#include "ace/Svc_Handler.h"
#include "ace/Acceptor.h"
#include "ace/Reactor.h"
#include "ace/INET_Addr.h"
#include "ace/SOCK_Stream.h"
#include "ace/SOCK_Acceptor.h"
#include "ace/Log_Msg.h"
#include "ace/Task_T.h"
#include "ace/Message_Block.h"
#include "ace/OS_NS_Thread.h"


// Stores a string version of the current thread id into buffer and
// returns the size of this thread id in bytes.
ssize_t ACE_OS_thr_id(char buffer[], size_t buffer_length)
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
 @class Echo_Task
 @brief 

 Create an Echo_Task that inherits from ACE_Task (configured with the ACE_MT_SYNCH traits class to obtain a synchronized request queue) 
*/
class Echo_Task : public ACE_Task < ACE_MT_SYNCH > {

public:
	/// Implement its svc() hook method to perform the "half-sync" portion of the server by
	/// Dequeueing messages(ACE_Message_Blocks obtained via ACE_Task::getq()) 
	/// containing the client input that was put into its synchronized request 
	/// queue(via the Echo_Svc_Handler::put() method, which in turn calls ACE_Task::putq())
	/// First sends back to the client[ACE_SOCK_Stream] the thread id[ACE_OS::thr_self()] that is running the svc() method
	/// Then sends back the client's input that was in the message[ACE_SOCK_Stream::sendv_n()].
	virtual int svc(void){
		ACE_DEBUG((LM_DEBUG,
			"(%P|%t) Echo_Task::svc\n"));
		/// Dequeueing messages (ACE_Message_Blocks obtained via ACE_Task::getq()) 
		/// containing the client input that was put into its synchronized request queue 
		/// (via the Echo_Svc_Handler::put() method, which in turn calls ACE_Task::putq())

		for (ACE_Message_Block *msgb = 0; this->getq(msgb) != -1;){

			Echo_Svc_Handler *echo_svc_handler = reinterpret_cast<Echo_Svc_Handler*>(msgb->rd_ptr());
			
			
			/// The thread id[ACE_OS::thr_self()] that is running the svc() method
			char threadId[256];
			size_t threadIdSize = ACE_OS_thr_id(threadId, sizeof(threadId));


			/// First sends back to the client[ACE_SOCK_Stream] the thread id[ACE_OS::thr_self()] that is running the svc() method
			int error = echo_svc_handler->peer().send_n(threadId, threadIdSize);
			if (error <= 0)
				return error;

			char* sendMsg = echo_svc_handler->client_input_->data_block()->base();
			size_t sendSize = echo_svc_handler->client_input_->data_block()->size();
			error = echo_svc_handler->peer().send_n(sendMsg, sendSize);
			if (error <= 0)
				return error;
		}
	

	return 0;

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



protected:

	/// When data arrives from the client the ACE_Reactor will automatically call back on
	/// the Echo_Svc_Handler::handle_input() method, which you will need to write so that
	/// it echos the client's input back to the client [ACE_SOCK_Stream].
	virtual int handle_input(ACE_HANDLE)
	{
	
		if (m_client_input)
			ACE_Message_Block *message_block = new ACE_Message_Block();

			if (recv(message_block) == -1)
				ACE_ERROR_RETURN((LM_ERROR, ACE_TEXT("(%P|%t) handle_input failed\n")), -1);

			ACE_DEBUG((LM_DEBUG,
				"(%P|%t) from client: %s",
				buf));
			/// TODO:  Reads the client data [ACE_SOCK_Stream] until the end of a line is reached 
			/// (i.e., the symbols "\n", "\r", or "\r\n" are read). 
			/// Puts the client data into a message [ACE_Message_Block]

			
			m_echo_task->put(message_block);
		}

		return 0;
	}
	
	// Contains the message fragment(s) received from the connected client.
	ACE_Message_Block *m_client_input;

private:
	Echo_Task *m_echo_task;

};


/**
* @class Echo_Acceptor
* @brief Acceptor using TCP sockets stream
*
* Create an Echo_Acceptor that inherits from ACE_Acceptor (or uses a simple typedef)
* and uses an Internet domain ''passive-mode'' stream socket to listen a designated
* port number [ACE_Acceptor, ACE_SOCK_Acceptor, ACE_INET_Addr, etc.].
*/
typedef ACE_Acceptor<Echo_Svc_Handler, ACE_SOCK_ACCEPTOR> Echo_Acceptor;


/* Program's entry point */
int ACE_TMAIN(int argc, ACE_TCHAR *argv[])
{
	ACE_OS::printf("Usage: %s [port-number]\n", argv[0]);
	u_short port = argc < 2 ? ACE_DEFAULT_SERVER_PORT : ACE_OS::atoi(argv[1]);

	/// Creating an address object which specifies the TCP/IP port on
	/// which the server will listen for new connection requests. 
	/// (using a wrapper facade INET_Addr class that encapsulates the Internet domain address struct)
	ACE_INET_Addr addr(port);

	/// Create an ACE_Reactor (or use the singleton instance of the ACE_Reactor), register
	/// the Echo_Acceptor instance with it [ACE_Reactor::register_handler()], and use its
	/// event loop [ACE_Reactor::run_reactor_event_loop()] to wait for a connection
	/// to arrive from a client.  
	Echo_Acceptor acceptor(addr, ACE_Reactor::instance());

	ACE_OS::printf("listening at port %d\n", port);

	ACE_Reactor::instance()->run_reactor_event_loop();

	return 0;
};
