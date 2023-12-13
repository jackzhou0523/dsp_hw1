#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "hmm.h"

#ifndef max_num_of_observation_seq 
#	define max_num_of_observation_seq 10000
#endif

#ifndef max_len_of_observation_seq
#	define max_len_of_observation_seq 51
#endif


typedef struct{

	double alpha[max_len_of_observation_seq][MAX_STATE] ;
	double beta[max_len_of_observation_seq][MAX_STATE] ;
	double gamma[max_len_of_observation_seq][MAX_STATE] ;
	double epsilon[max_len_of_observation_seq][MAX_STATE][MAX_STATE] ;
	double sum_gamma[max_len_of_observation_seq][MAX_STATE] ;
	double sum_gamma_ob[max_len_of_observation_seq][MAX_STATE][MAX_OBSERV] ;
	double sum_epsilon[max_len_of_observation_seq][MAX_STATE][MAX_STATE] ;
}Calculate;;

void forward_algorithm(Calculate *calculate, HMM *hmm, char *observation_seq, int len_of_observation_seq){
	//Initialization
	double alpha_initial = 0;
	for (int i = 0; i < hmm->state_num; i++){
		int observation_seq_index = (int)observation_seq[i] - (int)'A';
		alpha_initial = hmm->initial[i] * hmm->observation[observation_seq_index][i];
		calculate->alpha[0][i] = alpha_initial;
	}
	//Induction
	for (int t = 1; t < len_of_observation_seq; t++){
		int observation_seq_index = (int)observation_seq[t] - (int)'A';
		for (int j = 0; j < hmm->state_num; j++){
			calculate->alpha[t][j] = 0;
			for (int i = 0; i < hmm->state_num; i++){
				calculate->alpha[t][j] += calculate->alpha[t - 1][i] * hmm->transition[i][j] * hmm->observation[observation_seq_index][j];
			}
		}

	
	}
	return;
}

void backward_algorithm(Calculate *calculate, HMM *hmm, char *observation_seq, int len_of_observation_seq){
	//Initialization
	for (int i = 0; i < hmm->state_num; i++){
		calculate->beta[len_of_observation_seq - 1][i] = 1;
	}
	//Induction
	for (int t = len_of_observation_seq - 2; t >= 0; t--){
		int observation_seq_index = (int)observation_seq[t + 1] - (int)'A';
		for (int i = 0; i < hmm->state_num; i++){
			calculate->beta[t][i] = 0;
			for (int j = 0; j < hmm->state_num; j++){
				calculate->beta[t][i] += hmm->transition[i][j] * hmm->observation[observation_seq_index][j] * calculate->beta[t + 1][j];
			}
		}
	}
	return;
}

void calculate_gamma(Calculate *calculate, HMM *hmm, char *observation_seq, int len_of_observation_seq){
	for (int t = 0; t < len_of_observation_seq; t++){
		double sum = 0;
		int observation_seq_index = (int)observation_seq[t] - (int)'A';
		for (int i = 0; i < hmm->state_num; i++){
			sum += calculate->alpha[t][i] * calculate->beta[t][i];
		}
		for (int i = 0; i < hmm->state_num; i++){
			calculate->gamma[t][i] = calculate->alpha[t][i] * calculate->beta[t][i] / sum;
			calculate->sum_gamma[t][i] += calculate->gamma[t][i]; //accumulate gamma through all samples
			calculate->sum_gamma_ob[t][i][observation_seq_index] += calculate->gamma[t][i]; // different observation
		}
	}

	return;
}


void calculate_epsilon(Calculate *calculate, HMM *hmm, char *observation_seq, int len_of_observation_seq){
	for (int t = 0; t < len_of_observation_seq - 1; t++){
		double sum = 0;
		int observation_seq_index = (int)observation_seq[t + 1] - (int)'A';
		for (int i = 0; i < hmm->state_num; i++){
			for (int j = 0; j < hmm->state_num; j++){
				sum += calculate->alpha[t][i] * hmm->transition[i][j] * hmm->observation[observation_seq_index][j] * calculate->beta[t + 1][j];
			}
		}
		for (int i = 0; i < hmm->state_num; i++){
			for (int j = 0; j < hmm->state_num; j++){
				calculate->epsilon[t][i][j] = calculate->alpha[t][i] * hmm->transition[i][j] * hmm->observation[observation_seq_index][j] * calculate->beta[t + 1][j] / sum;
				calculate->sum_epsilon[t][i][j] += calculate->epsilon[t][i][j]; //accumulate epsilon through all samples 
			}
		}
	}

	return;
}

void new_initial(Calculate *calculate, HMM *hmm, int len_of_observation_seq, int num_of_observation_seq){	
	for (int i = 0; i < hmm->state_num; i++){
		hmm->initial[i] = calculate->sum_gamma[0][i] / num_of_observation_seq;
	}
	return;
}

void new_transition(Calculate *calculate, HMM *hmm, int len_of_observation_seq, int num_of_observation_seq){	
	for (int i = 0; i < hmm->state_num; i++){
		for (int j = 0; j < hmm->state_num; j++){
			double upper = 0, lower = 0;
			for (int t = 0; t < len_of_observation_seq - 1; t++){
				upper += calculate->sum_epsilon[t][i][j];
				lower += calculate->sum_gamma[t][i];
			}
			hmm->transition[i][j] = upper / lower;
		}
	}
	return;
}

void new_observation(Calculate *calculate, HMM *hmm, int len_of_observation_seq, int num_of_observation_seq){	
	for (int k = 0; k < hmm->observ_num; k++){
		for (int i = 0; i < hmm->state_num; i++){
			double upper = 0, lower = 0;
			for (int t = 0; t < len_of_observation_seq; t++){
				upper += calculate->sum_gamma_ob[t][i][k];
				lower += calculate->sum_gamma[t][i];
			}
			hmm->observation[k][i] = upper / lower;
		}
	}
	return;
}


void trainHMM(Calculate *calculate, HMM *hmm , int num_of_observation_seq , int len_of_observation_seq, char observation_seq[max_num_of_observation_seq][max_len_of_observation_seq], int iteration){
	for (int i = 0 ; i < iteration ; i++){
		for (int n = 0 ; n < num_of_observation_seq ; n++){
			forward_algorithm(calculate, hmm, observation_seq[n], len_of_observation_seq);
			backward_algorithm(calculate, hmm, observation_seq[n], len_of_observation_seq);
			calculate_gamma(calculate, hmm, observation_seq[n], len_of_observation_seq);
			calculate_epsilon(calculate, hmm, observation_seq[n], len_of_observation_seq);
		}
		new_initial(calculate, hmm, len_of_observation_seq, num_of_observation_seq);
		new_transition(calculate, hmm, len_of_observation_seq, num_of_observation_seq);
		new_observation(calculate, hmm, len_of_observation_seq, num_of_observation_seq);
	}
	return;
}

int main(int argc, char **argv)
{
	HMM hmm;
	Calculate calculate;

	// Get some parameters
	int iteration = atoi(argv[1]);
	char* initModel = argv[2];
	char *training_fname = argv[3];
	char *output_fname = argv[4];
	int num_of_observation_seq = 0;
	int len_of_observation_seq;
	char observation_seq[max_num_of_observation_seq][max_len_of_observation_seq] = {{0}};

	FILE *training_data = fopen(training_fname, "r");
	while (fscanf(training_data, "%s", observation_seq[num_of_observation_seq]) != EOF)
		num_of_observation_seq++;
	len_of_observation_seq = strlen(observation_seq[0]);
	fclose(training_data);

	//Initialization & Training
	loadHMM(&hmm, initModel);
	trainHMM(&calculate, &hmm, num_of_observation_seq, len_of_observation_seq, observation_seq, iteration);
	
	// Output
	FILE *dump_file = fopen(output_fname, "w");
	dumpHMM(dump_file, &hmm);
	fclose(dump_file);
	return 0;
}


