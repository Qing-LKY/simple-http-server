#include "opt.h"
#include "sig.h"
#include "sock.h"
#include "worker.h"

int main(int argc, char *argv[]) {
    // parse options from file and commandline
    if (parse_opt_all(argc, argv) != 0) return 1;
    // setup sighandler
    if(set_sighandler() != 0) return 1;
    // setup socket and binding ports
    if (init_socket() != 0) return 1;
    // init workers
    if (init_workers() != 0) return 1;
    main_loop();
    return 0;
}