#include "fifo.h"

void fifo_worker_handler(struct k_msgq *requests, struct k_msgq *results, int id){
    msg data;
    while(1){
        // read data from queue
         k_msgq_get(requests, &data, K_FOREVER);

         data.output = data.input + 5;
         data.handled_by = id;
        // put data on buffer until succesful
         k_msgq_put(results, &data, K_FOREVER);
    }
}