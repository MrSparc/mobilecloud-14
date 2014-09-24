//// $Id$
//
///**
//* @file ConcurrentWebserver.cpp
//* @author Ariel Machado <arielgmachado AT gmail DOT com>
//*
//*  Programming Cloud Services for Android Handheld Systems
//*  Optional Assignment 4 - Concurrent Webserver (ACE C++)
//*
//*
//* ACE version: 6.2.7
//*
//* Tested environments:
//* OS: Linux 3.13.0-33-generic #58-Ubuntu SMP x86_64 GNU/Linux
//* g++ version 4.8.3 (Ubuntu 4.8.2-19ubuntu1)
//*
//* OS: Windows 7 (64 bits)
//* VC++ Express 2013 for Windows Desktop
//*/
//
//#include "ace/Svc_Handler.h"
//#include "ace/Acceptor.h"
//#include "ace/Reactor.h"
//#include "ace/INET_Addr.h"
//#include "ace/SOCK_Stream.h"
//#include "ace/SOCK_Acceptor.h"
//#include "ace/Log_Msg.h"
//#include "ace/Task_T.h"
//#include "ace/Message_Block.h"
//#include "ace/OS_NS_Thread.h"
//
//
//// Stores a string version of the current thread id into buffer and
//// returns the size of this thread id in bytes.
//ssize_t ACE_OS_thr_id(char buffer[], size_t buffer_length)
//{
//#if defined (ACE_WIN32)
//	return ACE_OS::sprintf(buffer,
//		"Thread id: <%u>",
//		static_cast <unsigned> (ACE_Thread::self()));
//#else
//	ACE_hthread_t t_id;
//	ACE_OS::thr_self(t_id);
//	return ACE_OS::sprintf(buffer,
//		"Thread id: <%lu>",
//		(unsigned long)t_id);
//#endif /* WIN32 */
//}
//
//
//
///**
// @class Echo_Task
// @brief 
//
// Create an Echo_Task that inherits from ACE_Task (configured with the ACE_MT_SYNCH traits class to obtain a synchronized request queue) 
//*/
//class Echo_Task : public ACE_Task < ACE_MT_SYNCH > {
//
//public:
//	Echo_Task(ACE_Thread_Manager *thr_mgr, int n_threads) : ACE_Task<ACE_MT_SYNCH>(thr_mgr)
//	{
//		// Create the pool of worker threads.
//		if (this->activate(THR_NEW_LWP, n_threads) == -1)
//		{
//			ACE_ERROR((LM_ERROR, "%p\n", "activate failed"));
//		}
//	}
//
//	virtual int svc(void)
//	{
//
//		int count = 1;
//
//		// Keep looping, reading a message out of the queue, until we get a
//		// message with a length == 0, which signals us to quit.
//
//		for (;; count++)
//		{
//			ACE_Message_Block *mb = 0;
//
//			ACE_DEBUG((LM_DEBUG,
//				"(%t) in iteration %d before getq ()\n",
//				count));
//
//			if (this->getq(mb) == -1)
//			{
//				ACE_ERROR((LM_ERROR,
//					"(%t) in iteration %d, got result -1, exiting\n",
//					count));
//				break;
//			}
//
//			size_t length = mb->length();
//
//			if (length > 0)
//				ACE_DEBUG((LM_DEBUG,
//				"(%t) in iteration %d, length = %d, text = \"%*s\"\n",
//				count,
//				length,
//				length - 1,
//				mb->rd_ptr()));
//
//			// We're responsible for deallocating this.
//			mb->release();
//
//			if (length == 0)
//			{
//				ACE_DEBUG((LM_DEBUG,
//					"(%t) in iteration %d, got NULL message, exiting\n",
//					count));
//
//				break;
//			}
//		}
//		
//		return 0;
//  }
//
//};
//
//
///**
//* @class Echo_Svc_Handler
//* @brief Service handler using TCP sockets stream (using a wrapper facade SOCK_STREAM)
//*
//* Create an Echo_Svc_Handler that inherits from ACE_Svc_Handler 
//* (configured with the ACE_SOCK_STREAM class and ACE_NULL_SYNCH traits class) 
//* and implement its handle_input() hook method so that it :
//* Reads the client data [ACE_SOCK_Stream] until the end of a line is reached 
//* (i.e., the symbols "\n", "\r", or "\r\n" are read). 
//* Puts the client data into a message [ACE_Message_Block], and 
//* Calls Echo_Task::put(), which uses ACE_Task::putq() to enqueue the message for 
//* subsequent processing by a thread in the pool of threads that are running the Echo_Task::svc() hook method.   
//*/
//class Echo_Svc_Handler : public ACE_Svc_Handler < ACE_SOCK_STREAM, ACE_NULL_SYNCH >
//{
//
//public:
//
//
//protected:
//
//	virtual int handle_input(ACE_HANDLE)
//	{
//	
//		
//		ACE_Message_Block *mb = new ACE_Message_Block();
//
//			/// TODO:  Reads the client data [ACE_SOCK_Stream] until the end of a line is reached 
//			/// (i.e., the symbols "\n", "\r", or "\r\n" are read). 
//		
//		
//
//		return 0;
//	}
//	
//	
//
//protected:
//
//
//	Echo_Acceptor * echo_acceptor(void)
//	{
//		return this->echo_acceptor_;
//	}
//
//	/* And since you shouldn't access a member variable directly,
//	neither should you set (mutate) it.  Although it might seem silly
//	to do it this way, you'll thank yourself for it later.  */
//	void echo_acceptor(Echo_Acceptor *_echo_acceptor)
//	{
//		this->echo_acceptor_ = _echo_acceptor;
//	}
//
//
//	Echo_Acceptor * echo_acceptor_;
//	Echo_Task *echo_task_;
//
//};
//
//
//
//
///**
//* @class Echo_Acceptor
//* @brief Acceptor using TCP sockets stream
//*
//* Create an Echo_Acceptor that inherits from ACE_Acceptor (or uses a simple typedef)
//* and uses an Internet domain ''passive-mode'' stream socket to listen a designated
//* port number [ACE_Acceptor, ACE_SOCK_Acceptor, ACE_INET_Addr, etc.].
//*/
//
//class Echo_Acceptor : public ACE_Acceptor < Echo_Svc_Handler, ACE_SOCK_ACCEPTOR > {
//	Echo_Acceptor(Echo_Task &echo_task) : echo_task_(echo_task){};
//
//
//protected:
//	Echo_Task &echo_task_;
//};
//
//
//
//
//
///* Program's entry point */
//int ACE_TMAIN(int argc, ACE_TCHAR *argv[])
//{
//	ACE_OS::printf("Usage: %s [port-number]\n", argv[0]);
//	u_short port = argc < 2 ? ACE_DEFAULT_SERVER_PORT : ACE_OS::atoi(argv[1]);
//
//	ACE_INET_Addr addr(port);
//
//
//	ACE_Reactor *reactor = ACE_Reactor::instance();
//
//	// Number of threads
//	int n_threads = 5;
//
//	Echo_Task *echo_task_pool = new Echo_Task(ACE_Thread_Manager::instance(), n_threads);
//
//
//	Echo_Acceptor acceptor(echo_task_pool);
//
//	acceptor.open(addr, reactor);
//		
//	
//
//	ACE_OS::printf("listening at port %d\n", port);
//
//	reactor->run_reactor_event_loop();
//
//
//	if (echo_task_pool->wait() == -1)
//		ACE_ERROR_RETURN((LM_ERROR, "(%t) wait() failed\n"),
//		1);
//
//	ACE_DEBUG((LM_DEBUG,
//		"(%t) destroying worker tasks and exiting...\n"));
//
//	return 0;
//};
