#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>


int compare(const void *a, const void *b) {
    return (*(int *)a - *(int *)b);
}


typedef struct {
    int *local_array;
    int local_n;
    int target;
    int my_rank;
    int *result;
    pthread_mutex_t *mutex;
} ThreadData;


void *binary_search(void *arg) {
    ThreadData *data = (ThreadData *)arg;
    int low = 0, high = data->local_n - 1;

    while (low <= high) {
        int mid = (low + high) / 2;
        if (data->local_array[mid] == data->target) {
            pthread_mutex_lock(data->mutex);
            if (*data->result == -1) { // Update result only if not already found
                *data->result = data->my_rank * data->local_n + mid;
            }
            pthread_mutex_unlock(data->mutex);
            return NULL;
        } else if (data->local_array[mid] < data->target) {
            low = mid + 1;
        } else {
            high = mid - 1;
        }
    }
    return NULL;
}

int main(int argc, char *argv[]) {
    if (argc != 2) {
        fprintf(stderr, "Usage: %s <number_of_threads>\n", argv[0]);
        return 1;
    }

    int n, target, result = -1, thread_count;
    thread_count = strtol(argv[1], NULL, 10);

    if (thread_count <= 0) {
        fprintf(stderr, "Error: Number of threads must be a positive integer.\n");
        return 1;
    }

    printf("Enter the size of the array: ");
    fflush(stdout);
    scanf("%d", &n);

    if (n % thread_count != 0) {
        fprintf(stderr, "Error: Number of elements must be divisible by the number of threads.\n");
        return 1;
    }

    int *array = malloc(n * sizeof(int));
    printf("Enter %d elements of the array:\n", n);
    for (int i = 0; i < n; i++) {
        scanf("%d", &array[i]);
    }

    qsort(array, n, sizeof(int), compare);

    printf("Enter the target value to search for: ");
    scanf("%d", &target);

    pthread_t *thread_handles = malloc(thread_count * sizeof(pthread_t));
    ThreadData *thread_data = malloc(thread_count * sizeof(ThreadData));
    pthread_mutex_t mutex;
    pthread_mutex_init(&mutex, NULL);

    int local_n = n / thread_count;

    for (int i = 0; i < thread_count; i++) {
        thread_data[i].local_array = array + i * local_n;
        thread_data[i].local_n = local_n;
        thread_data[i].target = target;
        thread_data[i].my_rank = i;
        thread_data[i].result = &result;
        thread_data[i].mutex = &mutex;
        pthread_create(&thread_handles[i], NULL, binary_search, &thread_data[i]);
    }

        for (int i = 0; i < thread_count; i++) {
        pthread_join(thread_handles[i], NULL);
    }

        if (result != -1) {
        printf("Target found at index: %d\n", result);
    } else {
        printf("Target not found.\n");
    }

    
    free(array);
    free(thread_handles);
    free(thread_data);
    pthread_mutex_destroy(&mutex);

    return 0;
}
