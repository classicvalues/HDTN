#include <arpa/inet.h>
#include <string.h>

#include "egress.h"

using namespace hdtn;

hegr_entry *hegr_manager::_entry(int offset) {
    return (hegr_entry *)(((uint8_t *)_entries) + (offset * HEGR_ENTRY_SZ));
}

hegr_manager::~hegr_manager() {
    int shutdown_status;
    for (int i = 0; i < HEGR_ENTRY_COUNT; ++i) {

// JCF, debuging
//        hegr_entry * pEntry = _entry(i);
//        std::cout << "In hegr_manager::~hegr_manager, i = " << i << " , *entry = " << pEntry << std::endl << std::flush;

        _entry(i)->shutdown();

// JCF, the following line seg faults.  It looks like the destructor is not implemented.  Could be related to linked list also.
// JCF, commented out so code development could continue.

//        delete (_entry(i));
    }
    free(_entries);
}
void hegr_manager::init() {
    _entries = malloc(HEGR_ENTRY_SZ * HEGR_ENTRY_COUNT);
    for (int i = 0; i < HEGR_ENTRY_COUNT; ++i) {
        hegr_entry *tmp = new (_entry(i)) hegr_entry;
        tmp->label(i);
    }
    //socket for cut-through mode straight to egress
    zmqCutThroughCtx = new zmq::context_t;
    zmqCutThroughSock = new zmq::socket_t(*zmqCutThroughCtx, zmq::socket_type::pull);
    zmqCutThroughSock->connect(cutThroughAddress);
    //socket for sending bundles to storage
    zmqReleaseCtx = new zmq::context_t;
    zmqReleaseSock = new zmq::socket_t(*zmqReleaseCtx, zmq::socket_type::pull);
    zmqReleaseSock->bind(ReleaseAddress);
}

int hegr_manager::add(int fec, uint64_t flags, const char *dst, int port) {
    struct sockaddr_in saddr;
    saddr.sin_port = htons((uint16_t)port);
    saddr.sin_family = AF_INET;
    int conversion_status;
    conversion_status = inet_pton(AF_INET, dst, &(saddr.sin_addr));
    if (conversion_status != 1) {
        printf("Failure to convert IP address from text to binary");
        return 0;
    }
    if (flags & HEGR_FLAG_STCPv1) {
        hegr_stcp_entry *tcp = new (_entry(fec)) hegr_stcp_entry;
        tcp->init(&saddr, flags);
        tcp->disable();
        return 1;
    } else if (flags & HEGR_FLAG_UDP) {
        hegr_udp_entry *udp = new (_entry(fec)) hegr_udp_entry;
        udp->init(&saddr, flags);
        udp->disable();
        return 1;
    } else {
        return -HDTN_MSGTYPE_ENOTIMPL;
    }

    return 0;
}

void hegr_manager::down(int fec) {
    _entry(fec)->disable();
}

void hegr_manager::up(int fec) {
    _entry(fec)->enable();
}

/** Leaving function for now. Need to know if these sockets will be removed throughout running the code.
int hegr_manager::remove(int fec) {
    int shutdown_status;
    shutdown_status = _entry(fec)->shutdown();
    delete _entry(fec);
    return 0;
}
**/
int hegr_manager::forward(int fec, char *msg, int sz) {
    return _entry(fec)->forward((char **)(&msg), &sz, 1);
}

hegr_entry::hegr_entry() {
    _flags = 0;
    //_next = NULL;
}

// JCF -- Missing destructor, added below
hegr_entry::~hegr_entry() {
}

void hegr_entry::init(sockaddr_in *inaddr, uint64_t flags) {
}

bool hegr_entry::available() {
    return (_flags & HEGR_FLAG_ACTIVE) && (_flags & HEGR_FLAG_UP);
}

int hegr_entry::disable() {
    return -1;
}

void hegr_entry::rate(uint64_t rate) {
    //_rate = rate;
}

void hegr_entry::label(uint64_t label) {
    _label = label;
}

void hegr_entry::name(char *n) {
    //strncpy(_name, n, HEGR_NAME_SZ);
}

int hegr_entry::enable() {
    return -1;
}

void hegr_entry::update(uint64_t delta) {
    return;
}

int hegr_entry::forward(char **msg, int *sz, int count) {
    return 0;
}

void hegr_entry::shutdown() {
}
