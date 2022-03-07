1. Compile server file by using "g++ server.cpp -o server -lpthread".
2. Compile client file by using "g++ client.cpp -o client -lpthread".

Run the server by executing "./server".
Run the clients (MAX_5) by executing "./client".

Type and send "stats" from a client to get the list
of all connected nodes with their status.

Type and send "connect <FD_OF_THE_CLIENT_YOU_WANT_TO_CONNECT_TO>".

Type and send "goodbye" from a paired client to end
the pairing. Both clients still remain connected to the server.

Type and send "close" to disconnect a client from
the server.
