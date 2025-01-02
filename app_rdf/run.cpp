#include "app_rdf.h"

using namespace std;

int main(int argc, char *argv[])
{
    int num_thread = 4;
    RDFWorker worker(atoi(argv[2]));
    worker.load_data(argv[1]);
    worker.run();
}   