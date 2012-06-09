#include "main.h"

int main(int argc, char **argv){
    int i;
    int *best;
    
    read_instance(argv[1]);
    
    distMat = compute_distances();

    best = malloc(sizeof(int) * (ncities + 1));

    tsp(best);
    
    printf("\nLongitud del recorrido: %d\n\nRecorrido: ", compute_length(best));
    for(i = 0; i < ncities; ++ i)
        printf("%d ", best[i]);
    printf("%d\n", best[0]);
    printf("\n");

    free(best);
    return 1;
}

void tsp(int *best){
    int *vc, *vn, *tmp;
    int t, max = 5000, i;
    double T = 1500000, prob, ran;
    clock_t ticks;

    nn_ls = MIN (ncities - 1, 40);
    nnMat = compute_NNLists();

    
    vc = generate_random_vector();
    tmp = malloc(sizeof(int) * (ncities + 1));
    vn = malloc(sizeof(int) * (ncities + 1));
        
    for(i = 0; i <= ncities; ++i){
/*        printf("%d ", vc[i]);*/
        best[i] = vc[i];
    }
    for(t = 0; t < max; ++t){
        copy_tour(vc, tmp);
        
        dlb = calloc(ncities, sizeof(int));
        three_opt_first(vc);
        
        if(is_same(tmp, vc)){
            random_neighbour(vc, vn);
        }else{
            copy_tour(vc, vn);
            copy_tour(tmp, vc);
        }

        prob = calculate_prob(vc, vn, T);
        /*printf("%f\n", prob);*/
        ticks = clock();
        seed = (int) ticks;
        ran = ran01(&seed);
        /*printf("%f ? %f\n",ran, prob);*/
        if(ran < prob){
            for(i = 0; i < ncities; ++ i){
                vc[i] = vn[i];
            }
        }
        
        /*printf("%d, %d\n", compute_length(vc), compute_length(best));*/
        if(compute_length(vc) < compute_length(best)){
            for(i = 0; i < ncities; ++i)
                best[i] = vc[i];
        }
        
        free(dlb);
    }
    
    free(vn);
    free(vc);
    
    return;
}

void random_neighbour(int *t0, int *t1){
    int r0, r1, i;
    clock_t ticks;
    
    ticks = clock();
    
    /*Se le suma 1 porque, si se llega con 0 ticks, se quiere que la semilla no
      sea 0, ya que esto no generará ningún número
      */
    seed = (int)ticks + 1;
    
    r0 = random_number(&seed) % ncities;
    r1 = random_number(&seed) % ncities;
    
    for(i = 0; i <= ncities; ++ i){
        if(i == r0){
            t1[i] = t0[r1];
        }else if(i == r1){
            t1[i] = t0[r0];
        }else{
            t1[i] = t0[i];
        }
    }
    
    return;
}

double calculate_prob(int *vc, int *vn, double T){
    return 1 / (1 + exp((compute_length(vc) - compute_length(vn)) / T));
}

int is_same(int *t0, int *t1){
    int i;
    for(i = 0; i < ncities; ++ i)
        if(t0[i] != t1[i])
            return 0;
    return 1;    
}

void copy_tour(int *t0, int *t1){
    int i;
    for(i = 0; i < ncities; ++i)
        t1[i] = t0[i];
}