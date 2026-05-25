#include "../include/config.h"
#include "../include/db.h"
#include "../include/rooms.h"
#include "../include/server.h"
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>

static void handle_signal(int sig) {
    (void)sig;
    fprintf(stdout, "\n[main] Shutting down...\n");
    server_stop();
}

int main(void) {
    fprintf(stdout, "╔══════════════════════════════╗\n");
    fprintf(stdout, "║       SocketRoom Server      ║\n");
    fprintf(stdout, "╚══════════════════════════════╝\n");
    signal(SIGINT, handle_signal);
    signal(SIGTERM, handle_signal);
    if (db_open(SR_DB_PATH) != 0) {
        fprintf(stderr, "[main] Failed to open database. Exiting.\n");
        return EXIT_FAILURE;
    }
    rooms_seed();
    server_run();
    db_close();
    fprintf(stdout, "[main] Clean exit.\n");
    return EXIT_SUCCESS;
}
