# dsp_hw1

Digital Speech Processing HW1 Implementing Discrete Hidden Markov Model

## Files

* `data/train_seq_0X.txt`
  Training data (10K observation sequences).

* `data/test_lbl.txt`
  Testing labels.

* `data/test_seq.txt`
  Testing data (2.5K observation sequences).

* `inc/hmm.h`
  .

* `model_init.txt`
  Initial model parameters.

* `modellist.txt`
  Paths to model files.

* `src/test_hmm.c`
  A showcase of the usage of hmm.h.

## Input and Output of Your Program

### Training

#### Input
1. Number of iterations
2. Initial model (* `model_init.txt`)
3. Observation sequences (* `train_seq_01~05.txt`)

#### Output
Five files of parameters for 5 models, each containing λ = (A, B, π) (* `model_01~05.txt`)

### Testing

#### Input
1. A file of paths to the models trained in the previous step (* `modellist.txt`)
2. Observation sequences (* `test_seq.txt`)

#### Output
Best answer labels and P(O | λ) (* `result.txt`)

## Training Details

### Command
```bash
./train <iter> <model_init_path> <seq_path> <output_model_path>
