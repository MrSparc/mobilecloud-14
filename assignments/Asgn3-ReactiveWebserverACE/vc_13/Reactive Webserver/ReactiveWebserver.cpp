// $Id$

/**
* @file ReactiveWebserver.cpp
* @author Ariel Machado <arielgmachado AT gmail DOT com>
*
*  Programming Cloud Services for Android Handheld Systems
*  Optional Assignment 3 - Reactive Webserver (ACE C++)
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

/**
* @class Echo_Svc_Handler
* @brief Service handler using TCP sockets stream (using a wrapper facade SOCK_STREAM)
*
* Create an Echo_Svc_Handler that inherits from ACE_Svc_Handler
* and implement its handle_input() hook method so that it echos back the client's input
* either (a) a "chunk" at a time or (b) a "line" at a time (i.e., until
* the symbols "\n", "\r", or "\r\n" are read), rather than a character at a time
* [ACE_Svc_Handler, ACE_SOCK_Stream, etc.].
*/
class Echo_Svc_Handler : public ACE_Svc_Handler < ACE_SOCK_STREAM, ACE_NULL_SYNCH >
{

public:

	/// When a client connection request arrives, the ACE_Reactor will automatically call
	/// the handle_input() method of the ACE_Acceptor. This template method automatically
	/// accepts the connection and call the Echo_Svc_Handler::open() hook method to register
	/// it with the ACE_Reactor (after after the Echo_Acceptor has established the connection
	/// and created the Echo_Svc_Handler instance).  
	int open(void *)
	{
		ACE_DEBUG((LM_DEBUG,
			"(%P|%t) client connection request\n"));

		if (reactor()->register_handler(this,
			ACE_Event_Handler::READ_MASK) == -1)
			ACE_ERROR_RETURN((LM_ERROR,
			"(%P|%t) can't register with reactor\n"),
			-1);
		return 0;
	}


protected:

	/// When data arrives from the client the ACE_Reactor will automatically call back on
	/// the Echo_Svc_Handler::handle_input() method, which you will need to write so that
	/// it echos the client's input back to the client [ACE_SOCK_Stream].
	virtual int handle_input(ACE_HANDLE)
	{
		char buf[ACE_DEFAULT_MAX_SOCKET_BUFSIZ];
		ssize_t recv_cnt, send_cnt;

		switch (recv_cnt = this->peer().recv(buf, sizeof(buf)))
		{
		case -1:
			ACE_ERROR_RETURN((LM_ERROR,
				"(%P|%t) %p bad read\n",
				"client logger"),
				-1);
		case 0:
			ACE_ERROR_RETURN((LM_ERROR,
				"(%P|%t) closing log daemon (fd = %d)\n",
				this->get_handle()),
				-1);
		default:
			buf[recv_cnt] = '\0';
			ACE_DEBUG((LM_DEBUG,
				"(%P|%t) from client: %s",
				buf));

			send_cnt = this->peer().send(buf, recv_cnt);
			if (send_cnt < 0)
				ACE_ERROR_RETURN((LM_ERROR,
				"%s\n",
				"send_n failed"),
				-1);
		}

		return 0;
	}
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
