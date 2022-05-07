# What is this?
This is an experiment / prototype of a synchronization protocol via ethernet. This suffers from severe race conditions with more than 2 nodes so it was abandoned.

`ethernet_sync.c` contains a first simples version of the protocol where all processes wait for other to be ready to start.

`ethernet_sync2.c` contains a more complex version where all processes can synchronize at any point similar to a `pthread_barrier`.

Resources used:
- https://gist.github.com/austinmarton/1922600
- https://www.opensourceforu.com/2015/03/a-guide-to-using-raw-sockets/
- https://stackoverflow.com/questions/1779715/how-to-get-mac-address-of-your-machine-using-a-c-program
- https://man7.org/linux/man-pages/man3/getifaddrs.3.html