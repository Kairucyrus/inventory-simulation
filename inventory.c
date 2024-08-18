#include <stdio.h>
#include "lcgrand.h"
#include <math.h>
#include <stdlib.h>

//global variables
int amount, bigs, initial_inv_level, inv_level, next_event_type, num_events,
num_months, num_values_demand, smalls;

float area_holding, area_shortage, holding_cost, incremental_cost, maxlag,
mean_interdemand, minlag, prob_distrib_demand[26], setup_cost,
shortage_cost, sim_time, time_last_event, time_next_event[5],
total_ordering_cost;

//file pointers
FILE *infile, *outfile;
void initialize();
void timing();
void demand();
void order_arrival();
void evaluate();
void report();
void update_time_avg_stats();
float expon(float mean);
int random_integer(float prob_distrib []);
float uniform(float a, float b);

void initialize(){
    sim_time = 0.0;
    inv_level = initial_inv_level;
    time_last_event = 0.0;
    total_ordering_cost = 0.0;
    area_holding = 0.0;
    area_shortage = 0.0;

    time_next_event[1] = 1.0e+30;
    time_next_event[2] = sim_time + expon(mean_interdemand);
    time_next_event[3] = num_months;
    time_next_event[4] = 0.0;

}

void demand(void){
    inv_level -= random_integer(prob_distrib_demand);
    time_next_event[2] = sim_time + expon(mean_interdemand);
}

void order_arrival(void){
    inv_level += amount;
    time_next_event[1] = 1.0e+30;

}

void evaluate(void){
    if (inv_level < smalls){
        amount = bigs - inv_level;
        total_ordering_cost += setup_cost + incremental_cost*amount;
        time_next_event[1] = sim_time + uniform(minlag, maxlag);

    }
    time_next_event[4] = sim_time + 1.0;
}

void report(void){
    float avg_holding_cost, avg_ordering_cost, avg_shortage_cost;
    avg_holding_cost = holding_cost * area_holding / num_months;
    avg_ordering_cost = total_ordering_cost / num_months;
    avg_shortage_cost = shortage_cost * area_shortage / num_months;
    fprintf(outfile, "\n\n(%3d, %3d)%15.2f%15.2f%15.2f%15.2f", 
    smalls, bigs, avg_ordering_cost + avg_shortage_cost + avg_holding_cost,
    avg_ordering_cost, avg_holding_cost, avg_shortage_cost);
}

void update_time_avg_stats(void){
    float time_since_last_event;
    time_since_last_event = sim_time - time_last_event;
    time_last_event = sim_time;

    if (inv_level < 0)
        area_shortage -= inv_level * time_since_last_event;
    else if (inv_level > 0)
        area_holding += inv_level * time_since_last_event;
}

int random_integer(float prob_distrib[])
{
    int i;
    float u;
    u = lcgrand(1);
    for (i = 1; u >= prob_distrib[i]; ++i)
        ;
    return i;
}

float uniform(float a, float b)
{
    return a + lcgrand(1) * (b-a);
}
float expon(float mean)
{
   return -mean * log(lcgrand(1));
}




int main(){
    int i, num_policies;
    infile = fopen("inv.in", "r");
    outfile = fopen("inv.out", "w");

    num_events = 4;

    fscanf(infile, "%d %d %d %d %f %f %f %f %f %f %f", &initial_inv_level, &num_months, 
    &num_policies, &num_values_demand, &mean_interdemand, &setup_cost, &incremental_cost, 
    &holding_cost, &shortage_cost, &minlag, &maxlag);

    for (i=1; i<num_values_demand; ++i)
        fscanf(infile, "%f", &prob_distrib_demand[i]);
    
    fprintf(outfile, "Single-product inventory system\n\n");
    fprintf(outfile, "Initial inventory level%24d items\n\n", initial_inv_level);
    fprintf(outfile, "Number of demand sizes%25d\n\n", num_values_demand);
    fprintf(outfile, "Distribution function of demand sizes ");
    for (i = 1; i <= num_values_demand; ++i)
        fprintf(outfile, "%8.3f", prob_distrib_demand[i]);
    fprintf(outfile, "\n\nMean interdemand time%26.2f\n\n", mean_interdemand);
    fprintf(outfile, "Delivery lag range%29.2f to%10.2f months\n\n", minlag,
    maxlag);
    fprintf(outfile, "Length of the simulation%23d months\n\n", num_months);
    fprintf(outfile, "K =%6.1f i =%6.1f h =%6.1f pi =%6.1f\n\n", setup_cost, incremental_cost, holding_cost, shortage_cost);
    fprintf(outfile, "Number of policies%29d\n\n", num_policies);
    fprintf(outfile, "                  Average         Average");
    fprintf(outfile, "           Average       Average");
    fprintf(outfile, " Policy       total cost      ordering cost");
    fprintf(outfile, " holding cost     shortage cost");


    for (i=1; i<= num_policies; ++i) {
        fscanf(infile, "%d %d", &smalls, &bigs);
        initialize();

        do {
            timing();
            update_time_avg_stats();
            switch (next_event_type){
                case 1:
                    order_arrival();
                    break;
                
                case 2:
                    demand();
                    break;
                case 4:
                    evaluate();
                    break;
                case 3:
                    report();
                    break;

            
            }

        }while (next_event_type != 3);
    }
    fclose(infile);
    fclose(outfile);
    return 0;



}
