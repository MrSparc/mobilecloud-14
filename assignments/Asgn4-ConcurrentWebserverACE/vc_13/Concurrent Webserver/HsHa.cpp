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

#define POOL_SIZE 5

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

			// Process the message.
			process_message(mb);
		}

		return 0;
	}
private:
	void process_message(ACE_Message_Block *mb)
	{
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
		ACE_OS::sleep(3);
		ACE_DEBUG((LM_DEBUG,
			ACE_TEXT("(%t) Finished processing message %s\n"),
			buf));
	}
};



//Equivalent to Echo_Task
class Manager : public ACE_Task < ACE_MT_SYNCH >
{
public:
	enum {MAX_TIMEOUT = 5 };

	Manager() : shutdown_(0)
	{
		ACE_TRACE(ACE_TEXT("Manager::Manager"));
	}

	int svc(void)
	{
		ACE_TRACE(ACE_TEXT("Manager::svc"));

		ACE_DEBUG((LM_INFO, ACE_TEXT("(%t) Manager started\n")));

		// Create pool.
		Echo_Task pool;
		pool.activate(THR_NEW_LWP | THR_JOINABLE, POOL_SIZE);

		while (!done())
		{
			ACE_Message_Block *mb = NULL;
			ACE_Time_Value tv((long)MAX_TIMEOUT);
			//tv += ACE_OS::time(0);

			// Get a message request.
			if (this->getq(mb, &tv) < 0)
			{
				pool.msg_queue()->deactivate();
				pool.wait();
			}

			// Ask the worker pool to do the job.
			pool.putq(mb);
		}

		return 0;
	}

private:
	int done(void);

	int shutdown_;
};

int Manager::done(void)
{
	return (shutdown_ == 1);
}


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

	void echo_task(Echo_Task *&et){
		echo_task_ = et;
	}; 

protected:

	virtual int handle_input(ACE_HANDLE)
	{
		ACE_DEBUG((LM_DEBUG,
				"(%t) Echo_Svc_Handler::handle_input\n"));
	
		/// TODO:  Reads the client data [ACE_SOCK_Stream] until the end of a line is reached 
		/// (i.e., the symbols "\n", "\r", or "\r\n" are read). 
		char buf[ACE_DEFAULT_MAX_SOCKET_BUFSIZ];
		ssize_t recv_cnt;

		recv_cnt = this->peer().recv(buf, sizeof(buf));

		ACE_Message_Block *mb = 0;
		ACE_NEW_RETURN(mb, ACE_Message_Block(recv_cnt), -1);

		ACE_OS::memcpy(mb->wr_ptr(), buf, recv_cnt);

		put(mb);
		//Echo_Task::put(mb);

		return 0;
	}
	
private:
	Echo_Task * echo_task_;
};

class Echo_Acceptor : public  ACE_Acceptor <Echo_Svc_Handler, ACE_SOCK_ACCEPTOR > {
public:
	void echo_task(Echo_Task *&et){
		echo_task_ = et;
	}

	virtual int make_svc_handler(Echo_Svc_Handler *&sh)
	{
		ACE_TRACE("Echo_Acceptor::make_svc_handler");

		if (sh == 0)
			ACE_NEW_RETURN(sh,
			Echo_Svc_Handler,
			-1);
		
		/// Assign to each new handler the same thread pool
		sh->echo_task(echo_task_);
		

		// Set the reactor of the newly created <SVC_HANDLER> to the same
		// reactor that this <ACE_Acceptor> is using.
		sh->reactor(this->reactor());
		return 0;
	}

private :
	Echo_Task *echo_task_;

};


int ACE_TMAIN(int argc, ACE_TCHAR *argv[])
{
	ACE_OS::printf("Usage: %s [port-number]\n", argv[0]);
	u_short port = argc < 2 ? ACE_DEFAULT_SERVER_PORT : ACE_OS::atoi(argv[1]);
	ACE_INET_Addr addr(port);

	ACE_Reactor *reactor = ACE_Reactor::instance();

	//Manager tp;
	Echo_Task * echo_task;
	ACE_NEW_RETURN(echo_task,Echo_Task,-1);

	echo_task->activate(THR_NEW_LWP | THR_JOINABLE, POOL_SIZE);

	Echo_Acceptor acceptor;

	acceptor.echo_task(echo_task);
	
	acceptor.open(addr, ACE_Reactor::instance());

	ACE_OS::printf("listening at port %d\n", port);

	reactor->run_reactor_event_loop();

	ACE_Thread_Manager::instance()->wait();

	return 0;
	
}