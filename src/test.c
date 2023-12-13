#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "hmm.h"

#ifndef num_of_model
#	define num_of_model 5
#endif

#ifndef max_num_of_observation_seq
#	define max_num_of_observation_seq 2500
#endif

#ifndef max_len_of_observation_seq
#	define max_len_of_observation_seq 51
#endif

typedef struct 
{
	int index;
	double likelihood;

}Result;


void calculate_delta (HMM hmm, char *observation_seq, int len_of_observation_seq, double delta[max_len_of_observation_seq][MAX_STATE]){
	// Initialization
	int observation_seq_index = (int) observation_seq[0] - (int) 'A';
	for (int i = 0; i < hmm.state_num; i++){
		delta[0][i] = hmm.initial[i] * hmm.observation[observation_seq_index][i];
	}
	//Recursion
	for (int t = 1; t < len_of_observation_seq; t++){
		int observation_seq_index = (int) observation_seq[t] - (int) 'A';
		for (int j = 0; j < hmm.state_num; j++){
			// calculate the maximum part (delta*aij)
			double delta_aij, max_delta_aij = 0;
			for (int i = 0; i < hmm.state_num; i++){
				delta_aij = delta[t - 1][i] * hmm.transition[i][j];
				if (delta_aij > max_delta_aij)
					max_delta_aij = delta_aij;
			}
			// then times with bj(ot)
			delta[t][j] = max_delta_aij * hmm.observation[observation_seq_index][j];
		}
	}
}

double Viterbi(HMM hmm, int len_of_observation_seq, double max_likelihood, double delta[max_len_of_observation_seq][MAX_STATE]){
	for (int i = 0; i < hmm.state_num; i++)
		if (delta[len_of_observation_seq - 1][i] > max_likelihood)
			max_likelihood += delta[len_of_observation_seq - 1][i];
	return max_likelihood;
}

int main(int argc , char **argv)
{
	HMM hmm[num_of_model];
	int num_of_observation_seq = 0;
	int len_of_observation_seq;
	char observation_seq[max_num_of_observation_seq][max_len_of_observation_seq] = {{0}};
	char *model_list = argv[1];
	char *testing_fname = argv[2];
	char *result_fname = argv[3];
	
	load_models(model_list, hmm, num_of_model);


	// Read testing data
	FILE *testing_data = fopen(testing_fname, "r");
	while (fscanf(testing_data, "%s" , observation_seq[num_of_observation_seq]) != EOF){
		num_of_observation_seq++;
	}
	len_of_observation_seq = strlen(observation_seq[0]);
	fclose(testing_data);

	// Result
	FILE *dump_file = fopen(result_fname , "w");
	for (int n = 0; n < num_of_observation_seq; n++){
		Result result = {0};
		for (int m = 0; m < num_of_model; m++){
			double delta[max_len_of_observation_seq][MAX_STATE] = {{0}};
			double max_likelihood = 0;
			calculate_delta(hmm[m], observation_seq[n], len_of_observation_seq, delta);
			max_likelihood = Viterbi(hmm[m], len_of_observation_seq, max_likelihood, delta);

			if (max_likelihood > result.likelihood){
				result.index = m;
				result.likelihood = max_likelihood;
			}
		}
		fprintf(dump_file, "%s %e\n", hmm[result.index].model_name, result.likelihood);
	}
	fclose(dump_file);
	return 0;
}