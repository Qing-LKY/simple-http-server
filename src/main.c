#include "opt_common.h"
#include "sig.h"

int main(int argc, char *argv[]) {
    // parse options from file and commandline
    if (parse_opt_all(argc, argv) != 0) return 1;
    // setup sighandler
    if(set_sighandler() != 0) return 1;
    // setup socket and binding ports
    
    return 0;
}