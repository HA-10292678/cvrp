#include "main.h"

int main(int argc, char **argv)
{
    int i, j, lower, upper, ncitiesGen, nTours, its = 1;
    int *arr, *limits;
    int **distMatGen;
    
    read_cvrp(argv[1]);
    
    ncitiesGen = ncities;
    arr = malloc(sizeof(int) * (ncitiesGen + 1));
    limits = malloc(sizeof(int) * (ncitiesGen));
    
    for(i = 0; i < ncitiesGen; ++ i)
        arr[i] = i;
    
    distMatGen = compute_distances();
    
    nTours = initSol(arr, limits, distMatGen, ncitiesGen);
    for(i = 0; i < ncitiesGen; ++ i)
        printf("%d, ", arr[i]);
    
    printf("\n");
    /* return 0; */
            
    /*
    Print of the initial solutions
    */
    printf("INITIAL SOLUTION: \n");
    printSol(arr, limits, nTours, distMatGen);
    

    for(i = 0; i < its; ++ i){
        nTours = disturb(arr, limits, nTours, ncitiesGen, distMatGen);
        printSol(arr, limits, nTours, distMatGen);
        /*boxTSP(arr, lower, upper, distMatGen);
        distMat = distMatGen;*/
    }
    
    return 0;
    
    printf("\nLongitud del recorrido: %d\n\nRecorrido: ", compute_length(arr));
    for(i = 0; i < ncitiesGen; ++ i)
        printf("%d ", arr[i]);
    printf("%d\n", arr[0]);
    printf("\n");

    return 1;
}

void read_cvrp(char *filename){

    FILE *cvrp_file = fopen(filename, "r");
    if ( cvrp_file == NULL ) {
        fprintf(stderr,"No instance file specified, abort\n");
        exit(1);
    }
    printf("\nreading cvrp-file %s ... \n\n", filename);

    /* number of customers, vehicle capacity, maximum route time, drop time */
    fscanf(cvrp_file, "%d %d %d %d", &ncities, &mcap, &mst, &dt);
    /* printf("ncities: %d, mcap: %d, mst: %d, dt: %d\n", ncities, mcap, mst, dt); */
    ncities ++;

    if( (xc = malloc(sizeof(double) * ncities)) == NULL )
	exit(EXIT_FAILURE);
    if( (yc = malloc(sizeof(double) * ncities)) == NULL )
	exit(EXIT_FAILURE);

    /* depot x-coordinate, depot y-coordinate */
    fscanf(cvrp_file, "%d %d", &xc[0], &yc[0]);

    if( (caps = malloc(sizeof(int) * ncities)) == NULL )
	exit(EXIT_FAILURE);
   
    caps = malloc(sizeof(int) * (ncities));
    /* for each customer in turn: x-coordinate, y-coordinate, quantity */
    int i;
    for (i = 1 ; i < ncities ; i++ ) {
        fscanf(cvrp_file,"%lf %lf %d", &xc[i], &yc[i], &caps[i] );
    }

    distance = round_distance;
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
    int r0, r1;
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

int initSol(int *arr, int *limits, int **dist, int ncities){
    int i, j, k, numPairs = ((ncities - 1) * ((ncities - 1) - 1) / 2), ammTour = 0, start;
    int foundMatch = 0;
    int isNotAvailable[ncities], isInTour[ncities];
    int tourTime[numPairs], ammCitiesTour[numPairs], tourCap[numPairs], *at;
    int **savings, tours[numPairs][ncities];
    city_pair **pairs = malloc(numPairs * sizeof(city_pair *));
    city_pair *pair;
   
    memset(tourTime, 0, numPairs * sizeof(int));
    memset(ammCitiesTour, 0, numPairs * sizeof(int));
    memset(tourCap, 0, numPairs * sizeof(int));
    memset(isNotAvailable, 0, ncities * sizeof(int));
    memset(isInTour, 0, ncities * sizeof(int));
    /*Memory allocation for savings*/
    if((savings = malloc(sizeof( int) * ncities * ncities +
                sizeof( int *) * ncities )) == NULL){
        printf("Out of memory, exit.");
        exit(1);
    }
    
    for ( i = 1; i < ncities ; ++ i) {
        savings[i] = (int *)(savings + ncities) + i*ncities;
        for ( j = i  ; j < ncities ; ++ j) {
            savings[i][j] = (2 * dist[0][i] + 2 * dist[0][j]) - (dist[0][i] + dist[i][j] + dist[j][0]);
        }
    }
    
    k = 0;
    for(i = 1; i < ncities; ++ i){
        for(j = i + 1; j < ncities; ++ j){
            if((pair = (city_pair *)malloc(sizeof(city_pair))) == NULL){
                printf("Out of memory, exit.");
                exit(1);
            }
            pair->city0 = i;
            pair->city1 = j;
            pairs[k] = pair;
            ++ k;
        }
    }
    
    sortPairs(pairs, savings, 0, numPairs);

    /*
     * Parallel version of the Clark and Wright algorithm
     */
    for(i = 0; i < numPairs; ++i){
        /* printf("%d - %d\n", pairs[i]->city0, pairs[i]->city1); */
        if(isNotAvailable[pairs[i]->city0] || isNotAvailable[pairs[i]->city1])
            continue;
        
        foundMatch = 0;
        for(j = 0; j < ammTour; ++ j){
            at = tours[j];
            /*
             * If the pair represents the corners of the tour, do not proceed
             */
            if((pairs[i]->city0 == at[0] && pairs[i]->city1 == at[ammCitiesTour[j] - 1])
                ||
                (pairs[i]->city1 == at[0] && pairs[i]->city0 == at[ammCitiesTour[j] - 1])){
                foundMatch = 1;
                break;
            }
            
            /*
             * Either city is already part of a tour, but cannot be joined with the current tour
             */ 
            if((isInTour[pairs[i]->city0] && (pairs[i]->city0 != at[0]) && (pairs[i]->city0 != at[ammCitiesTour[j] - 1]))
                || 
                (isInTour[pairs[i]->city1] && (pairs[i]->city1 != at[0]) && (pairs[i]->city1 != at[ammCitiesTour[j] - 1])))
                continue;
            
            if(pairs[i]->city0 == at[0]){
                if(((tourTime[j] - dist[0][at[0]]) + 
                    (dist[0][pairs[i]->city1] + dist[pairs[i]->city1][at[0]] + dt) < mst)
                    &&
                    (tourCap[j] + caps[pairs[i]->city1] < mcap)){
                        for(k = ammCitiesTour[j]; k >= 0; -- k)
                            at[k + 1] = at[k];
                        at[0] = pairs[i]->city1;
                        isNotAvailable[at[1]] = 1;
                        tourTime[j] = (tourTime[j] - dist[0][at[1]])
                                    + (dist[0][at[0]] + dist[at[0]][at[1]]
                                       + dt);
                        tourCap[j] = tourCap[j] + caps[pairs[i]->city1];
                        ++ ammCitiesTour[j];
                        foundMatch = 1;
                        break;
                }
            }else if(pairs[i]->city0 == at[ammCitiesTour[j] - 1]){
                if(((tourTime[j] - dist[0][at[ammCitiesTour[j] - 1]]) + 
                    (dist[0][pairs[i]->city1] + dist[pairs[i]->city1][at[ammCitiesTour[j] - 1]] 
                     + dt) < mst)
                    &&
                    (tourCap[j] + caps[pairs[i]->city1] < mcap)){
                        at[ammCitiesTour[j]] = pairs[i]->city1;
                        isNotAvailable[at[ammCitiesTour[j] - 1]] = 1;
                        tourTime[j] = (tourTime[j] - dist[0][at[ammCitiesTour[j] - 1]])
                                    + (dist[0][pairs[i]->city1] + dist[pairs[i]->city1][at[ammCitiesTour[j] - 1]]
                                       + dt);
                        tourCap[j] = tourCap[j] + caps[pairs[i]->city1];
                        ++ ammCitiesTour[j];
                        foundMatch = 1;
                        break;
                }
            }else if(pairs[i]->city1 == at[0]){
                if(((tourTime[j] - dist[0][at[0]]) + 
                    (dist[0][pairs[i]->city0] + dist[pairs[i]->city0][at[0]] + dt) < mst)
                    &&
                    (tourCap[j] + caps[pairs[i]->city0] < mcap)){
                        for(k = ammCitiesTour[j]; k >= 0; -- k)
                            at[k + 1] = at[k];
                        at[0] = pairs[i]->city0;
                        isNotAvailable[at[1]] = 1;
                        tourTime[j] = (tourTime[j] - dist[0][at[1]])
                                    + (dist[0][at[0]] + dist[at[0]][at[1]]
                                       + dt);
                        tourCap[j] = tourCap[j] + caps[pairs[i]->city0];
                        ++ ammCitiesTour[j];
                        foundMatch = 1;
                        break;
                }            
            }else if(pairs[i]->city1 == at[ammCitiesTour[j] - 1]){
                if(((tourTime[j] - dist[0][at[ammCitiesTour[j] - 1]]) + 
                    (dist[0][pairs[i]->city0] + dist[pairs[i]->city0][at[ammCitiesTour[j] - 1]] 
                     + dt) < mst)
                    &&
                    (tourCap[j] + caps[pairs[i]->city0] < mcap)){
                        at[ammCitiesTour[j]] = pairs[i]->city0;
                        isNotAvailable[at[ammCitiesTour[j] - 1]] = 1;
                        tourTime[j] = (tourTime[j] - dist[0][at[ammCitiesTour[j] - 1]])
                                    + (dist[0][pairs[i]->city0] + dist[pairs[i]->city0][at[ammCitiesTour[j] - 1]]
                                       + dt);
                        tourCap[j] = tourCap[j] + caps[pairs[i]->city0];
                        ++ ammCitiesTour[j];
                        foundMatch = 1;
                        break;
                }            
            }
        }
        
        if(!foundMatch && !isInTour[pairs[i]->city0] && !isInTour[pairs[i]->city1]){
            if((dist[0][pairs[i]->city0] + dist[pairs[i]->city0][pairs[i]->city1] + dist[0][pairs[i]->city1]
                + 2 * dt < mst)
                &&
                caps[pairs[i]->city0] + caps[pairs[i]->city1] < mcap){
                tours[ammTour][0] = pairs[i]->city0;
                tours[ammTour][1] = pairs[i]->city1;
                tourTime[ammTour] = dist[0][pairs[i]->city0] + dist[pairs[i]->city0][pairs[i]->city1] + dist[0][pairs[i]->city1]
                                    + 2 * dt;
                tourCap[ammTour] = caps[pairs[i]->city0] + caps[pairs[i]->city1];
                ammCitiesTour[ammTour] = 2;
                ++ ammTour;
            }
        }
        
        /*
         * Both cities are necessarily now part of a tour
         */
        isInTour[pairs[i]->city0] = 1;
        isInTour[pairs[i]->city1] = 1;
        /*
        int p0, p1;
        printf("amMTour: %d\n", ammTour);
        for(p0 = 0; p0 < ammTour; ++ p0){
            printf("Tour %d: ", p0);
            for(p1 = 0; p1 < ammCitiesTour[p0]; ++ p1){
                printf("%d, ", tours[p0][p1]);
            }
            printf("\n\n");
        }
        printf("---------------------------\n");*/
    }
    
    /*
     * Now we write each tour and set the limits
     */
    for(i = 0; i < ammTour; ++ i){
        start = (i == 0 ? 0 : limits[i - 1]);
        for (j = 0; j < ammCitiesTour[i]; ++j){
            arr[start + j] = tours[i][j];
        }
        limits[i] = start + ammCitiesTour[i];
    }
    
    return ammTour;
}

int disturb(int *arr, int *limits, int ammTours, int ncities, int **dist){
    int i, startT, endT, tmp0 = -1, tmp1, pos0, ranPos;
    long seed;
    int ammT = 0, totCap, totTime;
    
    /* Perform the sequential swap between tours */
    for(i = 0; i < ammTours; ++ i){
        printf("Changing tour: %d / %d\n", i, ammTours);
        startT = (i == 0 ? 0 : limits[i - 1]);
        endT = limits[i];
        seed = (long)(clock());
        ranPos = (int) (ran01(&seed) * (endT - startT));
        if(i == 0)
            pos0 = ranPos;
        
        if(i == (ammTours - 1))
            arr[pos0] = arr[startT + ranPos];

        printf("\tSwapping position: %d\n", ranPos);
        printf("\tValue: %d, for: %d\n", arr[startT + ranPos], tmp0);

        tmp1 = arr[startT + ranPos];
        arr[startT + ranPos] = tmp0;
        tmp0 = tmp1;
    }
    
    /* Recalculate the limits of the tours */
    totCap = caps[arr[0]];
    totTime = 2 * dist[0][arr[0]] + dt;
    for(i = 0; i < ncities - 1; ++ i){
        if(((totTime - dist[0][arr[i]]) + (dist[arr[i]][arr[i + 1]] + dist[arr[i + 1]][0] + dt) < mst)
            &&
            totCap + caps[arr[i + 1]] < mcap){
            totTime = (totTime - dist[0][arr[i]]) + (dist[arr[i]][arr[i + 1]] + dist[arr[i + 1]][0] + dt);
            totCap += caps[arr[i + 1]];
        }else{
            limits[ammT] = i;
            ++ ammT;
            totCap = caps[arr[i + 1]];
            totTime = 2 * dist[0][arr[i + 1]] + dt;
        }
    }
    
    if(limits[ammT - 1] != ncities){
        limits[ammT] = ncities;
        ++ ammT;
    }
    
    return ammT;
}
void sortPairs(city_pair **arr, int **s, int start, int end){
    if(start == end - 1)
        return;
    
    sortPairs(arr, s, start, (start + end) / 2);
    sortPairs(arr, s, (start + end) / 2, end);
    
    int i = start, j = (end + start) / 2, k = 0;
    city_pair **tmp = malloc((end - start + 1) * sizeof(city_pair *));
    while((i < ((end + start) / 2)) || (j < end)){
        if(i >= (end + start) / 2){
            tmp[k] = arr[j];
            ++ j;
        }else if(j >= end){
            tmp[k] = arr[i];
            ++ i;
        }else{
            if(s[arr[i]->city0][arr[i]->city1] > s[arr[j]->city0][arr[j]->city1]){
                tmp[k] = arr[i];
                ++ i;
            }else{
                tmp[k] = arr[j];
                ++ j;
            }
        }
        ++ k;
    }
        
    for(k = 0; k < end - start; ++ k)
        arr[start + k] = tmp[k];
    
    free(tmp);
    return;
}

void printSol(int *arr, int *limits, int nTours, int **dist){
    int servTime, capacity, i, j;
    for(i = 0; i < nTours; ++ i){
        printf("Tour %d: ", i);
        for(j = (i == 0 ? 0 : limits[i - 1]); j < limits[i]; ++j){
            printf("%d ", arr[j]);
        }
        printf("\n");
        servTime = 0;
        capacity = 0;
        for(j = (i == 0 ? 0 : limits[i - 1]); j < limits[i] - 1; ++ j){
            servTime += dist[arr[j]][arr[j + 1]] + dt;
            capacity += caps[arr[j]];
        }
        
        int k = (i == 0 ? 0 : limits[i - 1]),
            j = arr[k],
            l = limits[i],
            m = arr[l],
            n = dist[0][j],
            o = dist[0][m];
        /*printf("dist[0][0]: %d\n", dist[0][0]);
        printf("n: %d\n", n);
        printf("o: %d\n", o);
        printf("dt: %d\n", dt);*/
        servTime += n
                    + o
                    + dt;
        capacity += caps[arr[limits[i]]];
        printf("Service time: %d / %d\n", servTime, mst);
        printf("Capacity: %d / %d", capacity, mcap);
        printf("\n\n");
    }     
}