/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   miniserver.cpp                                     :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ndahib <ndahib@student.42.fr>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/05/06 16:59:16 by ndahib            #+#    #+#             */
/*   Updated: 2024/05/06 18:16:40 by ndahib           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */


# include <iostream>
# include <arpa/inet.h>
# include <sys/socket.h>
# include <sys/select.h>
# include <sys/stat.h>
# include <cstring>
# include <unistd.h>
# include <fcntl.h>
# include <vector>
# include <map>
# include <fstream>
# include <sstream>
# include <dirent.h>
# include <csignal>

template <typename T>
void print(T body)
{
	// Just For asscociative Containers;
	for ( size_t i = 0; i < body.size(); i++ )
		std::cout << body[i];
}

std::string generateHTTPResponse(int statusCode, const std::string& statusMessage, const std::string& content) {
    std::ostringstream response;
    
    // HTTP header
    response << "HTTP/1.1 " << statusCode << " " << statusMessage << "\r\n";
    response << "Content-Type: text/plain\r\n";
    response << "Content-Length: " << content.length() << "\r\n";
    response << "Connection: close\r\n";
    response << "\r\n"; // End of headers
    
    // Content
    response << content;
    
    return response.str();
}

int set_nonblocking(int fd) {

    // Set the new flag back to the file descriptor
    if (fcntl(fd, F_SETFL, O_NONBLOCK) == -1) {
        perror("Error setting file descriptor to non-blocking : ");
        return -1;
    }

    return 0; // Success
}

int main()
{
    struct sockaddr_in serverAddr;
    
    int port = 8080;
    memset(&serverAddr, '\0', sizeof( struct sockaddr ));
	serverAddr.sin_family = AF_INET;
	serverAddr.sin_port = htons( 8080 );
	serverAddr.sin_addr.s_addr = INADDR_ANY; // tempo
    int listener = socket( PF_INET, SOCK_STREAM, 0 ); // change to AF_INET
	if ( listener == -1 )
	{
		std::cerr << " socket error\n";
		return (0);
	}
    int option_value = 1;
	if ( setsockopt(listener, SOL_SOCKET, SO_REUSEADDR, &option_value, 4 ) == -1 )
	{
		std::cerr << " setsockopt error\n";
		return (0);
	}
	set_nonblocking( listener );
	if ( bind( listener, ( struct sockaddr * )&serverAddr, sizeof(struct sockaddr) ) == -1)
	{
		std::cerr << " bind error\n";
		return (0);
	}
	if ( listen( listener, 100 ) == -1)
	{
		std::cerr << " listen error\n";
		return (0);
	}
	//FD_SET( this->listener, &multiplex.tmp_readfds );
	//multiplex.nfds = this->listener;
	std::cout << "listening ... on port " << port << std::endl;

    fd_set	readfds;
	fd_set	writefds;
	fd_set	tmp_readfds;
	fd_set	tmp_writefds;
    /// MUltiplex:
    FD_ZERO(&readfds);
	FD_ZERO(&writefds);
	FD_ZERO(&tmp_readfds);
	FD_ZERO(&tmp_writefds);
	FD_SET(listener, &tmp_readfds);
    
	int nfds = listener;

    while (true)
	{
		readfds = tmp_readfds;
		writefds = tmp_writefds;
		if (select(nfds + 1, &readfds, &writefds, NULL, NULL) == -1)
		{
			perror("select");
		}
		for (int i = 0; i <= nfds; i++)
		{
			if (FD_ISSET(i, &readfds))
			{
				std::cout << "***********************READ HANLDER *******************************" << std::endl;
				if (listener == i ){
				    int newfd = accept(listener, NULL, NULL);
                    if (newfd == -1){
                        perror("Error in Accepting new Client: ");
                        return (0);
                    }
                    set_nonblocking(newfd);
                    std::cout << "Accept :" << newfd << std::endl;
                    FD_SET(newfd, &tmp_readfds);
                    nfds = std::max(newfd, nfds);
                    // client.addClient(ClientInfo(newfd, server.get_serverInfo_idx(ServerSocket)));
				}
				else{
						char	recv_buffer[1024];
                        std::vector<char > buffer;

                        memset( recv_buffer, '\0', sizeof( recv_buffer ) );
                        buffer.clear();
                        int rec = recv(i, recv_buffer, sizeof(recv_buffer), 0);
                        if (rec <= 0)
                        {
                            if (rec == 0){
                                std::cout << "Connection Closed" << std::endl;
                            }
                            // client.removeClient(i);
                            FD_CLR(i, &tmp_readfds);
                            FD_CLR(i, &tmp_writefds);
                            close( i );
                        }
                        else{
                            ::print(buffer);
                            FD_SET(i, &tmp_writefds);
							
                        }
				}
			}
			else if (FD_ISSET( i, &writefds )) {
				std::cout << "***********************WRITE HANLDER ******************************" << std::endl;
					std::string httpResponse = generateHTTPResponse(200, "OK" ,"Hello, World!");
                    int sen = send( i, httpResponse.c_str(), httpResponse.length(), 0 );
                    if ( sen <= 0) {
                        perror( "send error: " );
			        }
                   	std::cout << "Clearing write\n";
			        FD_CLR(i, &tmp_writefds);
					FD_CLR(i, &tmp_readfds); 
					close(i);
			}
		}
	}
}