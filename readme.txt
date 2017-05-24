
                                 ______________
                    __,.,---'''''              '''''---..._
                 ,-'             ,:::::,,:::::,           '`-.
                |             ''::     ::     ::''           |
                |            '':::.....::....,:::''           |
                |'-.._        ````````````````````       __,,-|
                 '-.._''`---.....______________.....---''__,,-
                      ''`---.....______________.....---''

              ___ _ _    ___     _        ___
             | _ |_) |_ / __|___(_)_ _   / __| ___ _ ___ _____ _ _ 
             | _ \ |  _| (__/ _ \ | ' \  \__ \/ -_) '_\ V / -_) '_|
             |___/_|\__|\___\___/_|_||_| |___/\___|_|  \_/\___|_|

--------------------------------------------------------------------------------
             Author     : Max Lee
             Student ID : 719577
             Email      : max@mirrorstairstudio.com
             Date       : 24/MAY/17

             Multithreaded Bitcoin Server based on CS Project 2
             Written in blood and tears, not from this project </3
             
             mirrorstairstudio.com                  mallocsizeof.me
--------------------------------------------------------------------------------

Overview:

Server process is purely responsible for handling connections and spawning
threads for each client

Server process ___ client 1 thread
               \__ client 2 thread
               \__ client 2 thread
               ...

Client thread is purely responsible for reading client commands and spawning
threads.

Client thread  ___ PING 1 thread
               \__ PING 2 thread
               \__ SOLN 1 thread
               \__ WORK 1 thread ...

For work commands, it spawns work btches to solve the work.
Work is evenly distributed for each btches.
Btch obviously is my typo for 'Batch' :P

WORK thread    ___ btch 1 thread
               \__ btch 2 thread
               \__ btch 1 thread
               \__ btch 1 thread
               ...
For each client, works are queued.
For each client, at most CONCURRENT_WORK_COUNT number of works can be processed
at a given time.


For protection purposes:
    Server can handle only CLIENT_COUNT number of clients at a time.
    Server can queue work for only GLOBAL_WORK_LIMIT number of works at a time.
    These are in driver.h

    If maximum number of clients are reached, server disconnects new ones.

    Client can spawn at most CLIENT_THREAD_COUNT number of command threads at
    a given time.
    Server can at most concurrently work on GLOBAL_WORK_LIMIT works at a 
    given time.
    These are in handler.h

    If maximum number of commands reached, the commands are just queued.

--------------------------------------------------------------------------------

Commands:
PING - ping server
WORK - provide work for the server, returns SOLN if solution found
SOLN - checks whether the SOLN is valid

Response:
PONG - response to PING
OKAY - When stuff are OKAY. May have 40 character reason
ERRO - When error occurs. May have 40 character reason
SOLN - Solution found for WORK.

Extra Commands:
PONG - plz
OKAY - dont send
ERRO - such stuff
SLEP - Spawns a work that just sleeps for 3 seconds.

--------------------------------------------------------------------------------

Features:
- Multi threaded work processing
- Concurrent background work processing
- Concat separated messages (like PI then NG\r\n will be joined to PING\r\n)
- Segfault-less* server
and more!
