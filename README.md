# Linux Socket Chat
A simple chat application with client-server architecture, written in C++ using linux socket programming and multithreading

### Overview
- One server, multiple clients
- Clients can connect to the server and either send connection request to other free clients or wait and accept a connection request
- One mutex mapped to each client
- While attempting to connect with another client, the connecting client acquires its own mutex and then the other client's to ensure that
concurrency issues are handled well while connecting
- Paired clients release the acquired mutex on disconnecting

### Setup and use locally
1. Clone the repo and change directory
```
cd linux-socket-chat
```
2. Compile the `server.cpp` and `client.cpp` files
```
g++ server.cpp -o server -lpthread
g++ client.cpp -o client -lpthread
```
4. Run the server
```
./server
```
5. Run the clients
```
./client
```

### Commands
<table>
  <tr>
    <td> <h4>Command</h4> </td>
    <td> <h4>Use</h4> </td>
  </tr>
   <tr>
    <td> menu </td>
    <td> show list of supported commands </td>
  </tr>
  <tr>
    <td> stats </td>
    <td> show list of clients with their FDs and status { FREE, BUSY } </td>
  </tr>
  <tr>
    <td> connect *CLIENT_FD* </td>
    <td> pair with (connect to) client identified by CLIENT_FD </td>
  </tr>
  <tr>
    <td> goodbye </td>
    <td> unpair with a connected client </td>
  </tr>
  <tr>
    <td> close </td>
    <td> disconnect from server </td>
  </tr>
  </table>
