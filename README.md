


# RMP: Remote Memory Pool
### A Research project under [Prof. Kyle Hale](https://www.halek.co/):
* [Nithin Rao](https://www.halek.co/authors/nithin-rao/) 
* [Nanda Velugoti](https://www.halek.co/authors/nanda-velugoti/)

## To Run the test program, 

```
cd server
make
./bin/server 127.0.0.1 6768
```

```
cd client
make
./bin/client 127.0.0.1 6768
```

## Abstract

Memory management is one of the key components ofany software system
environment. For most of the cases local memory (localphysical RAM) to allocate, read, write
and free memory regions in a program. In these casesthe amount of memory available to it is
limited for a program. Remote Memory Pool (RMP) isdesigned to address this memory
limitation, by providing a way for programs to allocatememory remotely. We do this by
implementing RMP as a user-level library that allowsusers to set up a server program that
servers the remote memory and a client program canuse that remote memory from the server.
Not only that RMP does this exposing a truly minimalAPI, just three to be exact, for the client
program to allocate remote memory. Also the readsand writes to this remote memory does not
involve invocation of any external API or methods.Reading and writing is done just like a
variable access in C. To achieve this we use somelinux kernel mechanisms such as userfaultfd
to resolve reads and writes under the hood. This meansthat users can start using RMP without
any major changes to their existing program code.

## Introduction

In the evolution of distributed systems, managingmemory gets even more difficult. Many
of the implementations of remote memory expose APIsfor reading and writing data to memory
remotely, which makes it very hard for the developersto integrate such implementations into
their existing programs to use remote memory. Evenfor developers to write new programs with
these remote memory implementations need to use APIslike read/write to do memory
operations.

Remote Memory Pool (RMP) is implemented as a userspace library that exposes basic
initialization, allocation and freeing APIs for usersto manage remote memory. All the reads and
writes to this is remote memory is done as usual meaningusing variables.

In normal use cases of memory allocation and access,the memory is allocated in the
local physical RAM of the machine. With our implementationof RMP (Remote Memory Pool)
users can allocate memory, with page granularity,on a remote server’s physical RAM and
access (Read/Write) in real-time on demand. The objectivehere is to provide the user with a
minimal API that looks like a generic memory allocationAPI, such as malloc, while taking care
of sending and receiving the data back and forth withoutthe user having to change his code
significantly.

What exactly are we trying to accomplish? Below arethe examples illustrating how memory is
allocated, accessed and freed locally (normal) andremotely (RMP).


```
Accessing Local Memory
```
```
Accessing Remote Memory (RMP)
```
## Userfaultfd

The userfaultfd mechanism is designed to allow a threadin a multithreaded program to
perform user-space paging for the other threads inthe process. When a page fault occurs for
one of the regions registered to the userfaultfd object,the faulting thread is put to sleep and an
event is generated that can be read via the userfaultfdfile descriptor. The fault-handling thread
reads events from this file descriptor and servicesthem using operations described in
ioctl_userfaultfd(2). When servicing the page faultevents, the fault-handling thread can trigger
a wake-up for the sleeping thread.

It is possible for the faulting threads and the fault-handlingthreads to run in the context
of processes. In this case, these threads may belongto differ‐ ent programs, and the program
that executes the faulting threads will not necessarilycooperate with those that handle the page


faults. In such non-cooperative mode, the process that monitors userfaultfd and handles page
faults needs to be aware of the changes in the memorylayout of the faulting process to avoid
mem‐ ory corruption.

After the userfaultfd object is created with userfaultfd(),the application must enable it
using the UFFDIO_API ioctl(2) operation. This operationallows a handshake between the
kernel and user space to determine the API versionand supported features. This operation
must be performed before any of the other ioctl(2)operations de‐ scribed below (or those
operations fail with the EINVAL error).

After a successful UFFDIO_API operation, the applicationthen registers memory
address ranges using UFFDIO_REGISTER ioctl(2) operation. After successful completion of a
UFFDIO_REGISTER operation, a page fault occurringin the requested memory range, and
satisfying the defined at the registration time, willbe forwarded by the kernel to the user-space
application. The application can then use the UFFDIO_COPYor UFFDIO_ZEROPAGE ioctl(
operations to resolve the page fault.


## Problems with userfaultfd

**There are two basic problems:**

1. A pagefault (userfault) won't occur for subsequentreads/writes once it is
    resolved/handled.
2. In a pagefault due to a write, we won't get the data(part of faulting instruction) that is
    being written in the userfault handler.

## Our Solution: Off-by-one invalidation

**Off-by-one Invalidation:** If a pagefault occurs, thenthe faulting address and operation
(read/write) is saved, so that it can be invalidatedwhen the next pagefault occurs.

### Constraints:

1. At Most two pages of memory is required.
2. The memory of the entity which is resolving pageswill be behind by one memory
    address in terms of consistency. Since the name Off-by-one.

## RMP Components

RMP as library has a

**Server Component**
● Exposes user API to setup RMP server: rmp_server_init()
● Serves pages to clients by sending a page on readand receiving a page on write.
**Client Component**
● Registers with userfaultfd manager to listen to pagefaults
● Exposes user API: rmp_init(), rmp_alloc(n_pages),rmp_free(addr)
● Communicates with the server by sending a page onwrite and receiving a page on read.
**Userfaultfd Manager**
● Handles user page faults on the registered region.
● A fault handler thread gets an userfault event withfault address and operation type
(R/W)


## Under the hood of RMP

**Process of allocation, access and freeing of remotememory:**

1. Client requests to allocate n pages on a remote server.
2. Server allocates the n pages in memory and returnsa mapped handle.
3. Client stores the handle by mapping it to the localaddress.
4. Client then registers the local region with UFFD Manager
5. UFFD Manager then registers the region as a UFFD regionand starts a fault handler
    thread for that region.
6. A read/write to this region will cause a page faultthat will be caught by the appropriate
    fault handler thread, which then is forwarded to theRMP Client to resolve the fault.
7. This read/write will be resolved by requesting theserver by sending the appropriate
    handle that was stored in step 3 and the calculatedoffset of the page within the region.
    The offset is calculated by,
       Offset =(Faulting Address - Region Start Address) / Page Size
8. After repeating step 7 for all the accesses to allthe remote memory regions, the client
    then can send a free request to Server, which serverfrees the region in it’s memory after
    which the client does the same.


## Using RMP: Server/Client API and Example

## Programs

**Server API:**
● void rmp_server_init(server_conn_config);
**Client API:**
● void rmp_init(server_conn_config);
● void* rmp_alloc(long n_pages);
● void rmp_free(void* addr);

```
Client.c Server.c
```
## Testing and Evaluation

### Testing Setup:

```
Server 4 Core, 8GB RAM, Gigabit Ethernet
```
```
Client 4 Core, 1GB RAM, Gigabit Ethernet
```

### Test Suites:

```
● Program to allocate 1000, 10000, 100000 sized arraysand fill them up with numbers
and calculate the sum.
    ○ Sequential Read Write Execution
    ○ Random Read Write Execution
● Insertion Sort Algorithm
```
### Results

**1. Sequential Access**

```
Operation Type Mode Time (seconds)
```
```
Sequential Read Local 2.59893e-
```
```
Sequential Read Remote 2.3117e-
```
```
Sequential Write Local 3.22393e-
```
```
Sequential Write Remote 6.4034e-
```
**Observation:** Sequential remote accesses are slowerin the order of 10^4 microseconds

**2. Random Access**

```
Operation Type Mode Time (seconds)
```
```
Random Read Local 0.0774958e-
```
```
Random Read Remote 0.
```
```
Random Write Local 0.0857879e-
```
```
Random Write Remote 0.
```
**Observation:** Random remote accesses are slower inthe order of 10^9 microseconds

3. **Insertion Sort (** 100000 Elements)

```
Local Execution Time 10.7446 Seconds
```
```
Remote Execution Time 199.1882 Seconds
```

**Observation:** Remote accesses are slower in the orderof 20x compared to local accesses

### Graphs

**Graphs for Insertion Sort with RMP:** 1000 to 1M elements

```
Linear view Logarithmic view
```
## RMP Advantages and Disadvantages

```
Advantages Disadvantages
```
```
Clients can access more memory than it’s
own local memory (given that remote
machines can satisfy the client’s
requirement).
```
```
Not as fast as local memory. Speed of the
memory accesses is dependent on the
network communication type (RDMA, TCP/IP,
etc).
```
```
Minimal development or integration or porting
effort required.
```
```
Scaling and load balancing is not taken care
of in the current implementation.
```
```
Extensible, meaning that this implementation
can be extended to support many more
features. (see next section).
```
```
Currently every read/write will result in a
network communication (which is costly). This
can be solved by implementing a client side
cache.
```
```
This can be extended to implement
distributed remote memory.
```
```
Handling consistency and recovery in a
scenario with unexpected crashes is really
difficult as a lot state information and logging
is required
```

## Possible Improvements and Extensions

```
● Use upcoming linux kernel patches for userfaultfdthat resolves the problems that we
faced earlier instead of using the Off-by-one invalidationsolution proposed before.
● Use RDMA instead of TCP/IP (expected performance improvements)
    ○ Hardware (RoCE or Infiniband)
    ○ Software (SoftRoCE)
● Extend it to work with NDP (Near Data Processing)units.
● Support multiple servers and clients (distributedremote memory)
● A distributed memory with
● Discovery mechanism for available RMP servers
● Nodes in the network act as both servers and clients.
```
