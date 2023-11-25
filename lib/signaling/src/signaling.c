#include <stdio.h>
#include "signaling.h"

void signal_handle_calculation(struct k_sem *request,
                               struct k_sem *response,
                               struct signal_data *data)
{
    while(1){
    printf("+ Waiting for request\n");
    k_sem_take(request, K_FOREVER);
    printf("+ Handling calculation\n");
    data->output = data->input + 5;
    k_sem_give(response);
    printf("+ Done with calculation\n");
    }
}

int signal_request_calculate(struct k_sem *request,
                             struct k_sem *response,
                             struct signal_data *data)
{
    printf("- Handoff to worker\n");
    k_sem_give(request);
    printf("- Waiting for results\n");
    if (!k_sem_take(response, K_MSEC(1000)))
    {
         printf("- Result ready\n");
        return 0;
    }
    
    else{
         printf("- Result failed\n");
        return 1;
    }
}