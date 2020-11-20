
#include "rmp_server.hpp"
#include <iostream>
#include <assert.h>
#include <string.h>

int passed = 0, failed = 0;

bool test(bool test, bool set)
{
    return test && set;
}

rmp::packet *create_new_packet()
{
    rmp::packet *p = new rmp::packet;
    p->error = 0;
    p->size = 0;
    p->npages = 0;
    p->offset = 0;
    return p;
}

rmp::packet *get_alloc_packet(u32 npages, u32 page_size)
{
    rmp::packet *p = create_new_packet();
    p->action = 0;
    p->npages = npages;
    p->size = page_size;
    p->hndl = -1;
    return p;
}

rmp::packet *get_read_packet(rmp::handle hndl, u32 offset)
{
    rmp::packet *p = create_new_packet();
    p->action = 1;
    p->hndl = hndl;
    p->offset = offset;
    return p;
}

rmp::packet *get_write_packet(rmp::handle hndl, u32 offset, char *data)
{
    rmp::packet *p = create_new_packet();
    p->action = 2;
    p->hndl = hndl;
    p->offset = offset;
    strncpy(p->data, data, 4096);
    return p;
}

rmp::packet *get_free_packet(rmp::handle hndl)
{
    rmp::packet *p = create_new_packet();
    p->action = 3;
    p->hndl = hndl;
    return p;
}

// test alloc
bool test_alloc_basic()
{
    bool valid;
    rmp::Server *s = new rmp::Server(rmp::config{});
    rmp::packet *alloc_req = get_alloc_packet(10, 4096);

    s->handle(alloc_req);

    valid = test(alloc_req->error == 0, alloc_req->hndl == 0);

    delete s, alloc_req;
    return valid;
}

// test alloc
bool test_alloc_100pages()
{
    bool valid;
    rmp::Server *s = new rmp::Server(rmp::config{});
    rmp::packet *alloc_req = get_alloc_packet(100, 4096);

    s->handle(alloc_req);

    valid = test(alloc_req->error == 0, alloc_req->hndl == 0);

    delete s, alloc_req;
    return valid;
}

// test alloc
bool test_alloc_a_lot_of_pages()
{
    bool valid;
    rmp::Server *s = new rmp::Server(rmp::config{});
    rmp::packet *alloc_req = get_alloc_packet(10000000000000000000000000, 4096);

    s->handle(alloc_req);

    // std::cout << alloc_req->error << std::endl;
    valid = test(alloc_req->error == ALLOC_FAILED, true);

    delete s, alloc_req;
    return valid;
}

// test read
bool test_read()
{
    bool valid;
    rmp::Server *s = new rmp::Server(rmp::config{});
    rmp::packet *alloc_req = new rmp::packet;
    alloc_req->action = 0;
    alloc_req->npages = 10;
    alloc_req->size = 4096;
    alloc_req->error = 0;

    s->handle(alloc_req);

    valid = test(alloc_req->hndl == 0, true);

    char *msg = "Hello World!";
    rmp::packet *write_req = new rmp::packet;
    write_req->action = 2;
    write_req->offset = 0;
    write_req->size = 4096;
    write_req->error = 0;
    write_req->hndl = alloc_req->hndl;
    strncpy(write_req->data, msg, strlen(msg));

    s->handle(write_req);

    rmp::packet *read_req = new rmp::packet;
    read_req->action = 1;
    read_req->offset = 0;
    read_req->size = 4096;
    read_req->error = 0;
    read_req->hndl = alloc_req->hndl;

    s->handle(read_req);

    valid = test(strcmp(read_req->data, msg) == 0, valid);
    // std::cout << read_req->data << std::endl;

    delete s, alloc_req, write_req, read_req;
    return valid;
}

// test read
bool test_read_wrong_offset()
{
    bool valid;
    rmp::Server *s = new rmp::Server(rmp::config{});
    rmp::packet *alloc_req = new rmp::packet;
    alloc_req->action = 0;
    alloc_req->npages = 10;
    alloc_req->size = 4096;
    alloc_req->error = 0;

    s->handle(alloc_req);

    assert(alloc_req->hndl == 0);
    valid = test(alloc_req->hndl == 0, true);

    char *msg = "Hello World!";
    rmp::packet *write_req = new rmp::packet;
    write_req->action = 2;
    write_req->offset = 0;
    write_req->size = 4096;
    write_req->error = 0;
    write_req->hndl = alloc_req->hndl;
    strncpy(write_req->data, msg, strlen(msg));

    s->handle(write_req);

    rmp::packet *read_req = new rmp::packet;
    read_req->action = 1;
    read_req->offset = 11;
    read_req->size = 4096;
    read_req->error = 0;
    read_req->hndl = alloc_req->hndl;

    s->handle(read_req);

    valid = test(read_req->error == READ_FAILED, valid);
    delete s, alloc_req, write_req, read_req;
    return valid;
}

// test read
bool test_read_wrong_handle()
{
    bool valid;
    rmp::Server *s = new rmp::Server(rmp::config{});
    rmp::packet *read_req = new rmp::packet;
    read_req->action = 1;
    read_req->offset = 0;
    read_req->size = 4096;
    read_req->error = 0;
    read_req->hndl = 0;

    s->handle(read_req);

    valid = test(read_req->error == INVALID_HANDLE, true);

    delete s, read_req;
    return valid;
}

// test write
bool test_write()
{
    bool valid;
    rmp::Server *s = new rmp::Server(rmp::config{});
    rmp::packet *alloc_req = new rmp::packet;
    alloc_req->action = 0;
    alloc_req->npages = 10;
    alloc_req->size = 4096;
    alloc_req->error = 0;

    s->handle(alloc_req);

    valid = test(alloc_req->hndl == 0, true);

    char *msg = "Hello World!";
    rmp::packet *write_req = new rmp::packet;
    write_req->action = 2;
    write_req->offset = 0;
    write_req->size = 4096;
    write_req->error = 0;
    write_req->hndl = alloc_req->hndl;
    strncpy(write_req->data, msg, strlen(msg));

    s->handle(write_req);

    valid = test(write_req->error == 0, valid);

    delete s, alloc_req, write_req;
    return valid;
}

// test write
bool test_write_wrong_offset()
{
    bool valid;
    rmp::Server *s = new rmp::Server(rmp::config{});
    rmp::packet *alloc_req = new rmp::packet;
    alloc_req->action = 0;
    alloc_req->npages = 10;
    alloc_req->size = 4096;
    alloc_req->error = 0;

    s->handle(alloc_req);

    valid = test(alloc_req->hndl == 0, true);

    char *msg = "Hello World!";
    rmp::packet *write_req = new rmp::packet;
    write_req->action = 2;
    write_req->offset = 11;
    write_req->size = 4096;
    write_req->error = 0;
    write_req->hndl = alloc_req->hndl;
    strncpy(write_req->data, msg, strlen(msg));

    s->handle(write_req);

    valid = test(write_req->error == WRITE_FAILED, valid);

    delete s, alloc_req, write_req;
    return valid;
}

// test write
bool test_write_wrong_handle()
{
    bool valid;
    rmp::Server *s = new rmp::Server(rmp::config{});

    char *msg = "Hello World!";
    rmp::packet *write_req = new rmp::packet;
    write_req->action = 2;
    write_req->offset = 0;
    write_req->size = 4096;
    write_req->error = 0;
    write_req->hndl = 0;
    strncpy(write_req->data, msg, strlen(msg));

    s->handle(write_req);

    assert(write_req->error == INVALID_HANDLE);
    valid = test(write_req->error == INVALID_HANDLE, true);

    delete s, write_req;
    return valid;
}

// test free
bool test_free()
{
    bool valid;
    rmp::Server *s = new rmp::Server(rmp::config{});
    rmp::packet *alloc_req = new rmp::packet;
    alloc_req->action = 0;
    alloc_req->npages = 10;
    alloc_req->size = 4096;
    alloc_req->error = 0;

    s->handle(alloc_req);

    // std::cout << alloc_req->error << std::endl;
    valid = test(alloc_req->hndl == 0, true);

    rmp::packet *free_req = new rmp::packet;
    free_req->action = 3;
    free_req->size = 4096;
    free_req->error = 0;
    free_req->hndl = alloc_req->hndl;

    s->handle(free_req);

    // std::cout << free_req->error << std::endl;
    valid = test(free_req->error == 0, valid);

    delete s, alloc_req, free_req;
    return valid;
}

// test free
bool test_free_and_read()
{
    bool valid;
    rmp::Server *s = new rmp::Server(rmp::config{});
    rmp::packet *alloc_req = new rmp::packet;
    alloc_req->action = 0;
    alloc_req->npages = 10;
    alloc_req->size = 4096;
    alloc_req->error = 0;

    s->handle(alloc_req);

    // std::cout << alloc_req->error << std::endl;
    valid = test(alloc_req->hndl == 0, true);

    rmp::packet *free_req = new rmp::packet;
    free_req->action = 3;
    free_req->size = 4096;
    free_req->error = 0;
    free_req->hndl = alloc_req->hndl;

    s->handle(free_req);

    // std::cout << free_req->error << std::endl;
    valid = test(free_req->error == 0, valid);

    rmp::packet *read_req = new rmp::packet;
    read_req->action = 1;
    read_req->offset = 0;
    read_req->size = 4096;
    read_req->error = 0;
    read_req->hndl = alloc_req->hndl;

    s->handle(read_req);

    // std::cout << read_req->error << std::endl;
    valid = test(read_req->error == INVALID_HANDLE, valid);
    delete s, alloc_req, free_req, read_req;
    return valid;
}

bool test_free_wrong_handle()
{
    bool valid;
    rmp::Server *s = new rmp::Server(rmp::config{});
    rmp::packet *free_req = new rmp::packet;
    free_req->action = 3;
    free_req->size = 4096;
    free_req->error = 0;
    free_req->hndl = 0;

    s->handle(free_req);

    valid = test(free_req->error == INVALID_HANDLE, true);

    delete s, free_req;
    return valid;
}

void runner(bool func(void), std::string name)
{

    if (func())
    {
        std::cout << name << " PASSED         +" << ++passed << " -" << failed << std::endl;
    }
    else
    {
        std::cout << name << " FAILED         +" << passed << " -" << ++failed << std::endl;
    }
}

int main()
{
    runner(test_alloc_basic, "test_alloc_basic         ");
    runner(test_alloc_a_lot_of_pages, "test_alloc_a_lot_of_pages");
    runner(test_read, "test_read                ");
    runner(test_read_wrong_offset, "test_read_wrong_offset    ");
    runner(test_read_wrong_handle, "test_read_wrong_handle    ");
    runner(test_write, "test_write              ");
    runner(test_write_wrong_offset, "test_write_wrong_offset  ");
    runner(test_write_wrong_handle, "test_write_wrong_handle  ");
    runner(test_free, "test_free                ");
    runner(test_free, "test_free_and_read       ");
    runner(test_free_wrong_handle, "test_free_wrong_handle    ");
}
