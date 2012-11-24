#include<stdio.h>
#include<stdlib.h>

struct flow{
    int flow_ended;
    int bandwidth;
    long int data_pending;
    long int allocated_speed;
    double end_time;
    double total_time;
};

void get_flow_data(int current_interval, int number_of_flows, struct flow* flows);
void print_flow(int current_interval, int number_of_flows, struct flow* flows);
void get_allocated(int current_interval, int number_of_flows, struct flow* flows, long int total_bandwidth);
double earliest_end(int current_interval, int number_of_flows, struct flow* flows);
void reset_flow(int number_of_flows, struct flow* flows, int flow_number, double time_remaining);
void mark_end(int current_interval, int number_of_flows, struct flow* flows, double earliest_ending, double time_remaining);
void update_flow_info(int current_interval, int number_of_flows, struct flow* flows, double earliest_ending);

int flow_pending(int current_interval, int number_of_flows, struct flow* flows){
    int j;

    for(j=current_interval; j < (current_interval + number_of_flows); j++){
        if((flows[j].flow_ended) == 0) return 1;
    }
    return 0;
}

int main(){

    int number_of_flows, i, j, intervals, current_interval, sum, flow_remaining;
    double interval_time = 0.001;
    double time_remaining, earliest_ending;
    long int total_bandwidth;
    struct flow* flows;

    int k;

    printf("enter the number of flows: ");
    scanf("%d", &number_of_flows);

    printf("enter the total bandwidth: ");
    scanf("%ld", &total_bandwidth);
    total_bandwidth = total_bandwidth * 1000000; //Mbps. change it as required

    printf("enter number of time intervals: ");
    scanf("%d", &intervals);

    flows = malloc(((number_of_flows * intervals) +1 )* sizeof(struct flow)); // +1 is for extra buffer space when forward copying the bandwidth and other data

    printf("enter the bandwidth per flow: \n");
    for(j=0; j<number_of_flows; j++){
        printf("flow %d\t", j);
        scanf("%d", &flows[j].bandwidth);
    }

    //for interval x=1,2,3..
    for(i=0; i<intervals; i++){

        current_interval = i * number_of_flows;
        //printf("current interval: %d\n", current_interval);
        //read data pending
        get_flow_data(current_interval, number_of_flows, flows);
        //print_flow(0, number_of_flows, flows);

        time_remaining = interval_time;



        //iterate over flows for that interval
        do{
            //calculate actual allocated bandwidth based on ratio
            get_allocated(current_interval, number_of_flows, flows, total_bandwidth);
            //find earliest pending
            earliest_ending = earliest_end(current_interval, number_of_flows, flows);

            //printf("time remains: %lf\n", time_remaining);
            if(earliest_ending > time_remaining){
                mark_end(current_interval, number_of_flows, flows, time_remaining, time_remaining);
            } else {
                mark_end(current_interval, number_of_flows, flows, earliest_ending, time_remaining);
            }

            //update_flow_info(0, number_of_flows, flows, earliest_ending);
            //boolean check
            flow_remaining = flow_pending(current_interval, number_of_flows, flows);
            time_remaining = time_remaining - earliest_ending;


            //if(flow_remaining){
            //
            //}

        }while((time_remaining > 0) && flow_remaining);
        printf("after interval 1, flow status:\t ");
        for(k=current_interval; k < (current_interval + number_of_flows); k++){
            printf("%d\t", flows[k].flow_ended);
        }

    }

    print_flow(current_interval, number_of_flows, flows);

    free(flows);
    return 0;
}

void get_flow_data(int current_interval, int number_of_flows, struct flow *flows){
    int j;
    long int current_data;

    printf("enter the amount of data per flow for interval %d: \n", current_interval);
    for(j=current_interval; j < (current_interval + number_of_flows); j++){
        printf("flow %d\t", j);
        scanf("%ld", &current_data);
        if(current_data != 0l ){
            //printf("cd: %ld", current_data);
            current_data = current_data * 1000; //kbps. change it as required
             flows[j].data_pending += current_data;
             flows[j].flow_ended = 0;
        }
    }
}

void print_flow(int current_interval, int number_of_flows, struct flow *flows){
    int j;

    printf("the amount of data per flow: \n");
    for(j=current_interval; j < (current_interval + number_of_flows); j++){
        printf("flow %d\t", j+1);
        printf("%ld\n", flows[j].data_pending);
    }
}

void get_allocated(int current_interval, int number_of_flows, struct flow* flows, long int total_bandwidth){
    int j, sum = 0;
    for(j=current_interval; j < (current_interval + number_of_flows); j++){
        if((flows[j].flow_ended) == 0){
            sum = sum + (flows[j].bandwidth);
        }
    }
    printf("total bandwidth is: %ld \n", total_bandwidth);
    printf("sum is: %d \n", sum);
    printf("allocated bandwidth: \n");

    for(j=current_interval;j < (current_interval + number_of_flows); j++){
        if((flows[j].flow_ended) == 0){
            flows[j].allocated_speed = ((flows[j].bandwidth)*total_bandwidth)/sum;
        }

    }

    for(j=current_interval; j < (current_interval + number_of_flows); j++){
        if((flows[j].flow_ended) == 0){
            printf("flow %d: %ld\n",j ,flows[j].allocated_speed);
        }

    }
}

double earliest_end(int current_interval, int number_of_flows, struct flow* flows){

    int j;
    double end_time = 999999999.0, temp;
    for(j=current_interval; j < (current_interval + number_of_flows); j++){
        //print_flow_as(0, number_of_flows, flows);
        if((flows[j].flow_ended) == 0){
            printf("data pending flow %d: %ld\n", j, flows[j].data_pending);
            temp = (double) (flows[j].data_pending) / (flows[j].allocated_speed);
            flows[j].end_time = temp;
            //flows[j].total_time = (double) (flows[j].total_time) + temp;
            //printf("temp:%lf\n", temp);
            if(temp < end_time) end_time = temp;
        }
    }

    //printf("end time: %lf\n", end_time);
    return end_time;
}

void reset_flow(int number_of_flows, struct flow* flows, int flow_number, double time_remaining){
    int j=flow_number;
    //printf("flow number %d", flow_number);
    //printf("next flow number %d", flow_number+number_of_flows);
    flows[j+number_of_flows].bandwidth = flows[j].bandwidth;
    flows[j+number_of_flows].flow_ended = flows[j].flow_ended;
    //flow is empty, return
    if(flows[j].flow_ended == 1) return;
    //transfer current data to next interval
    double temp;
    long int temp2;

    temp = (double) time_remaining * ((double)(flows[j].allocated_speed));
    temp2 = (long int) temp;
    printf("temp: %lf temp2: %ld\n", temp, temp2);
    flows[j].data_pending = (flows[j].data_pending) - temp;
    flows[j+number_of_flows].data_pending = flows[j].data_pending;
    flows[j+number_of_flows].flow_ended = 0;
}

void mark_end(int current_interval, int number_of_flows, struct flow* flows, double earliest_ending, double time_remaining){
    int j;
    if(earliest_ending == time_remaining){
        for(j=current_interval; j < (current_interval + number_of_flows); j++){
            reset_flow(number_of_flows, flows, j, time_remaining);
            /*if(((flows[j].end_time) == earliest_ending) && time_remaining){
                printf("flow %d ended", j);
                flows[j].flow_ended = 1;
            }*/
        }
    } else {
        update_flow_info(current_interval, number_of_flows, flows, earliest_ending);
    }

}

void update_flow_info(int current_interval, int number_of_flows, struct flow* flows, double earliest_ending){
    int j;
    double temp;
    long int temp2;
    for(j=current_interval; j < (current_interval + number_of_flows); j++){
        if((flows[j].flow_ended) == 0){
            temp = (double) earliest_ending * ((double)(flows[j].allocated_speed));
            temp2 = (long int) temp;
            printf("temp: %lf temp2: %ld\n", temp, temp2);
            flows[j].data_pending = (flows[j].data_pending) - temp;
            flows[j].total_time = (double) (flows[j].total_time) + earliest_ending;
            if(flows[j].end_time == earliest_ending){
                printf("flow %d ends at time %lf\n", j, earliest_ending);
                flows[j].data_pending = 0;
                flows[j].flow_ended = 1;
            }
        }
    }
}
