To compile and run: 

````
gcc curltest.c -c
gcc socket_server.c -c 
gcc curltest.o socket_server.o -lcurl -ljson-c -o main
./main
````
