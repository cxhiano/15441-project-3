--------------------------------
15441 Computer Network Project 3
--------------------------------

src/
    namerserver/
        globals.h - Declaration of global variables in name server
        graph.[c|h] - Definition of graph objects and implementation of shortest path algorithm
        main.c - Main routine of dns server
        strategy.[c|h] - Implementation of load balancing strategies
    utils/
        list.[c|h] - General purpose linked list
        helpers.[c|h] - Helper functions in dns message serialization
        log.[c|h] - Helper functions for logging
        message.[c|h] - Definition of DNS message and implementation of DNS message serialization
        mydns.[c|h] - Implementation of DNS client
        net.[c|h] - Helper functions for manipulating C socket address struct.
    test/
        main.c - Test routines
    proxy/
        io.[c|h] - Functions dealing with the underlying socket io.
        log.[c|h] - Functions that logs output to file.
        message.[c|h] - Abstraction of http messages.
        proxy.[c|h] - Implementation of the bit rate adaption proxy.
        server.[c|h] - Underlying select engine for connections.
        transaction.[c|h] - Abstraction of http requests and responses.
        main.c - Executable proxy.


