#include <omp.h>
#include "knn.h"

char knn(int n_groups, Group * groups, int k, Point to_evaluate) {
    char * labels = (char *) malloc(sizeof(char) * k);
    float * distances = (float *) malloc(sizeof(float) * k);
    char *all_labels;
    float *all_distances;
    int array_size;

    int i, j, x, y;

    #pragma omp parallel
    {
        int nthreads = omp_get_num_threads();
        int ithread = omp_get_thread_num();
        char * local_labels = (char *) malloc(sizeof(char) * k);
        float * local_distances = (float *) malloc(sizeof(float) * k);

        #pragma omp single
        {
            all_labels = (char *) malloc(sizeof(char) * k * nthreads);
            all_distances = (float *) malloc(sizeof(float) * k * nthreads);
            array_size = k * nthreads;
        }

        for (i = 0; i < k; i++) {
            local_labels[i] = -1;
            local_distances[i] = -1;
        }

        #pragma omp for
        for (i = 0; i < n_groups; i++) {
            Group g = groups[i];

            for (j = 0; j < g.length; j++) {
                float d = euclidean_distance_no_sqrt(to_evaluate, g.points[j]);

                for (x = 0; x < k; x++) {
                    if (d < local_distances[x] || local_distances[x] == -1) {
                        for (y = k - 1; y > x; y--) {
                            local_distances[y] = local_distances[y - 1];
                            local_labels[y] = local_labels[y - 1];
                        }

                        local_distances[x] = d;
                        local_labels[x] = g.label;

                        break;
                    }
                }
            }
        }

        for(i = 0; i < k; i++) {
            all_labels[ithread * k + i] = local_labels[i];
            all_distances[ithread * k + i] = local_distances[i];
        }

        free(local_labels);
        free(local_distances);
    }

    label_distance_wrapper *data = (label_distance_wrapper *) malloc(sizeof(label_distance_wrapper) * array_size);
    for(i = 0; i < array_size; i++) {
        data[i].label = all_labels[i];
        data[i].distance = all_distances[i];
    }

    qsort(data, array_size, sizeof(label_distance_wrapper), compare_struct);

    int valid_label_count = 0;
    for (i = 0; i < array_size && valid_label_count < k; i++) {
        if (data[i].distance != -1) {
            labels[valid_label_count++] = data[i].label;
        }
    }

    qsort(labels, k, sizeof(char), compare_for_sort);

    char most_frequent = labels[0];
    int most_frequent_count = 1;
    int current_frequency = 1;

    for (i = 1; i < k; i++) {
        if (labels[i] != labels[i - 1]) {
            if (current_frequency > most_frequent_count) {
                most_frequent = labels[i - 1];
                most_frequent_count = current_frequency;
            }

            current_frequency = 1;
        } else {
            current_frequency++;
        }

        if (i == k - 1 && current_frequency > most_frequent_count) {
            most_frequent = labels[i - 1];
            most_frequent_count = current_frequency;
        }
    }

    free(labels);
    free(distances);
    free(all_labels);
    free(all_distances);
    free(data);

    return most_frequent;
}

int main() {
    int n_groups = parse_number_of_groups();
    
    Group * groups = (Group *) malloc(sizeof(Group) * n_groups);

    for (int i = 0; i < n_groups; i++) {
        groups[i] = parse_next_group();
    }

    int k = parse_k();

    Point to_evaluate = parse_point();

    printf("%c", knn(n_groups, groups, k, to_evaluate));
}
