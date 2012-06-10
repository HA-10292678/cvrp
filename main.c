#include "main.h"

int main(int argc, char **argv){
    int i, lower, upper, ncitiesGen;
    int *arr;
    int **distMatGen;
    
    read_instance(argv[1]);
    
    ncitiesGen = ncities;
    arr = malloc(sizeof(int) * (ncitiesGen + 1));
    
    for(i = 0; i < ncitiesGen; ++ i)
        arr[i] = i;
    
    lower = 10;
    upper = 20;
    
    distMatGen = compute_distances();

    boxTSP(arr, lower, upper, distMatGen);
    distMat = distMatGen;
    
    printf("\nLongitud del recorrido: %d\n\nRecorrido: ", compute_length(arr));
    for(i = 0; i < ncitiesGen; ++ i)
        printf("%d ", arr[i]);
    printf("%d\n", arr[0]);
    printf("\n");

    return 1;
}

void boxTSP(int *arr, int lower, int upper, int **distMatGen){
    int *best;
    int i, j;
    
    ncities = upper - lower + 1;
    best = malloc(sizeof(int) * (ncities + 1));

    /* Code taken from compute_distances (With light modifications) */
    if((distMat = malloc(sizeof( int) * ncities * ncities +
                sizeof( int *) * ncities )) == NULL){
        printf("Out of memory, exit.");
        exit(1);
    }
    
    for ( i = 0 ; i < ncities ; i++ ) {
        distMat[i] = (int *)(distMat + ncities) + i*ncities;
        for ( j = 0  ; j < ncities ; j++ ) {
        distMat[i][j] = distMatGen[arr[i + lower]][arr[j + lower]];
        }
    }
    /*End of code taken from compute_distances*/
    
    tsp(best);
    
    for(i = 0; i < ncities; ++i)
        best[i] = arr[best[i] + lower];
    
    for(i = 0; i < ncities; ++i)
        arr[i + lower] = best[i];
    
    free(distMat);
    free(best);
}

void tsp(int *best){
    int *vc, *vn, *tmp;
    int t, max = 5000, i;
    double T = 1500000, prob, ran;
    clock_t ticks;

    nn_ls = MIN (ncities - 1, 40);
    nnMat = compute_NNLists();

    
    vc = generate_random_vector();
    vc[ncities] = vc[0];
    tmp = malloc(sizeof(int) * (ncities + 1));
    vn = malloc(sizeof(int) * (ncities + 1));

    for(i = 0; i <= ncities; ++i){
        best[i] = vc[i];
    }

    for(t = 0; t < max; ++t){
        copy_tour(vc, tmp);
        
        dlb = calloc(ncities, sizeof(int));
        three_opt_first(vc);
        
        copy_tour(vc, vn);
        if(is_same(tmp, vc)){
            random_neighbour(vn);
        }else{
            copy_tour(tmp, vc);
        }

        prob = calculate_prob(vc, vn, T);
        /*printf("%f\n", prob);*/
        ticks = clock();
        seed = (int) ticks;
        ran = ran01(&seed);
        /*printf("%f ? %f\n",ran, prob);*/
        if(ran < prob){
            for(i = 0; i <= ncities; ++ i){
                vc[i] = vn[i];
            }
        }
        
        /*printf("%d, %d\n", compute_length(vc), compute_length(best));*/
        if(compute_length(vc) < compute_length(best)){
            for(i = 0; i <= ncities; ++i)
                best[i] = vc[i];
        }
        
        free(dlb);
    }

    free(vn);
    free(vc);
    
    return;
}

void random_neighbour(int *t){
    int r0, r1, i;
    clock_t ticks;
    
    ticks = clock();
    
    /*Se le suma 1 porque, si se llega con 0 ticks, se quiere que la semilla no
      sea 0, ya que esto no generará ningún número
      */
    seed = (int)ticks + 1;
    
    r0 = random_number(&seed) % ncities;
    r1 = random_number(&seed) % ncities;
    int tmp = t[r0];
    t[r0] = t[r1];
    t[r1] = tmp;
    if (0 == r0) {
        t[ncities] = t[r0];
    } else if (0 == r1) {
        t[ncities] = t[r1];
    }
    
    return;
}

double calculate_prob(int *vc, int *vn, double T){
    return 1 / (1 + exp((compute_length(vc) - compute_length(vn)) / T));
}

int is_same(int *t0, int *t1){
    int i;
    for(i = 0; i <= ncities; ++ i)
        if(t0[i] != t1[i])
            return 0;
    return 1;    
}

void copy_tour(int *t0, int *t1){
    int i;
    for(i = 0; i <= ncities; ++i)
        t1[i] = t0[i];
}