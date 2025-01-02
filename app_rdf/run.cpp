#include "app_rdf.h"

using namespace std;

int main(int argc, char *argv[])
{
    int num_thread = 4;
    RDFWorker worker(atoi(argv[2]));
    worker.load_data(argv[1]);
    worker.run();


    int ttl_task_num = 0;
    for (int i=0; i<32; ++i)
        ttl_task_num += global_task_num[i];
    std::cout << "task num = " << ttl_task_num + 1 << std::endl;
}   