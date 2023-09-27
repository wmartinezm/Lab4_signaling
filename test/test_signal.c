#include <stdio.h>
#include <zephyr.h>
#include <arch/cpu.h>
#include <unity.h>
#include "signaling.h"

void setUp(void) {}

void tearDown(void) {}

#define STACKSIZE 2000

K_THREAD_STACK_DEFINE(coop_stack, STACKSIZE);


void test_request(void)
{
    struct k_thread coop_thread;
    struct k_sem response;
    struct k_sem request;
    struct signal_data data = {};

    k_sem_init(&request, 0, 1);
    k_sem_init(&response, 0, 1);
    k_thread_create(&coop_thread,
                    coop_stack,
                    STACKSIZE,
                    (k_thread_entry_t) signal_handle_calculation,
                    &request,
                    &response,
                    &data,
                    K_PRIO_COOP(7),
                    0,
                    K_NO_WAIT);
    for (int counter = 46; counter < 55; counter++) {
        data.input = counter;
        int result = signal_request_calculate(&request, &response, &data);
        TEST_ASSERT_EQUAL_INT(0, result);
        TEST_ASSERT_EQUAL_INT(counter+5, data.output);
	}
    k_thread_abort(&coop_thread);
}

void test_noone_home(void)
{
    struct k_thread coop_thread;
    struct k_sem response;
    struct k_sem request;
    struct signal_data data = {42, 42};

    k_sem_init(&request, 0, 1);
    k_sem_init(&response, 0, 1);

    k_thread_create(&coop_thread,
                    coop_stack,
                    STACKSIZE,
                    (k_thread_entry_t) signal_request_calculate,
                    &request,
                    &response,
                    &data,
                    K_PRIO_COOP(7),
                    0,
                    K_NO_WAIT);
    k_sleep(K_MSEC(1000));
    TEST_ASSERT_EQUAL_INT(1, k_sem_count_get(&request));
    TEST_ASSERT_EQUAL_INT(0, k_sem_count_get(&response));
    TEST_ASSERT_EQUAL_INT(42, data.input);
    TEST_ASSERT_EQUAL_INT(42, data.output);
    k_thread_abort(&coop_thread);
}

void test_noop(void)
{
    struct k_thread coop_thread;
    struct k_sem response;
    struct k_sem request;
    struct signal_data data = {42, 42};

    k_sem_init(&request, 0, 1);
    k_sem_init(&response, 0, 1);
    k_thread_create(&coop_thread,
                    coop_stack,
                    STACKSIZE,
                    (k_thread_entry_t) signal_handle_calculation,
                    &request,
                    &response,
                    &data,
                    K_PRIO_COOP(7),
                    0,
                    K_NO_WAIT);
    k_sleep(K_MSEC(1000));
    TEST_ASSERT_EQUAL_INT(42, data.input);
    TEST_ASSERT_EQUAL_INT(42, data.output);
    k_thread_abort(&coop_thread);
}


void test_out_of_order(void)
{
    struct k_thread coop_thread;
    struct k_sem response;
    struct k_sem request;
    struct signal_data data = {42, 42};

    k_sem_init(&request, 0, 1);
    k_sem_init(&response, 0, 1);
    k_thread_create(&coop_thread,
                    coop_stack,
                    STACKSIZE,
                    (k_thread_entry_t) signal_handle_calculation,
                    &request,
                    &response,
                    &data,
                    K_PRIO_COOP(7),
                    0,
                    K_NO_WAIT);
    k_sem_give(&response);
    k_sleep(K_MSEC(1000));
    TEST_ASSERT_EQUAL_INT(-EBUSY, k_sem_take(&request, K_NO_WAIT));
    TEST_ASSERT_EQUAL_INT(0, k_sem_take(&response, K_NO_WAIT));
    TEST_ASSERT_EQUAL_INT(42, data.input);
    TEST_ASSERT_EQUAL_INT(42, data.output);
    k_thread_abort(&coop_thread);
}

void signal_handle_calculation(struct k_sem *request,
                               struct k_sem *response,
                               struct signal_data *data)
{
    printf("+ Waiting for request\n");
    k_sem_take(request, K_FOREVER);
    printf("+ Handling calculation\n");
    data->output = data->input + 5;
    k_sem_give(response);
    printf("+ Done with calculation\n");
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

int main (void)
{
    UNITY_BEGIN();
    RUN_TEST(test_request);
    //RUN_TEST(test_noop);
    //RUN_TEST(test_out_of_order);
    //RUN_TEST(test_noone_home);
    return UNITY_END();
}