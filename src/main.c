#include "opt_common.h"

int main(int argc, char *argv[]) {
    // parse options from file and commandline
    if (parse_opt_all(argc, argv) != 0) return 1;
    // setup sighandler
    
    return 0;
}