
#ifndef MOTIONAUTH_SERVER_DENOISINGAUTOENCODER_H
#define MOTIONAUTH_SERVER_DENOISINGAUTOENCODER_H

#include <string>
#include <vector>
#include <thread>
#include <zconf.h>
#include "Neuron.cuh"

using std::vector;
using std::string;
using std::thread;

class DenoisingAutoencoder {
public:
  DenoisingAutoencoder(const unsigned long num_input, const float compression_rate,
                       const double dropout_rate);
  vector<Neuron> learn(const vector<vector<double>> &input,
                    const vector<vector<double>> &noisy_input);
  vector<vector<double>> getMiddleOutput(const vector<vector<double>> &noisy_input);
  unsigned long getCurrentMiddleNeuronNum();

private:
  static const unsigned int MAX_TRIAL = 200; // 学習上限回数
  constexpr static const double MAX_GAP = 0.1; // 許容する誤差
  unsigned long num_thread = (unsigned long)sysconf(_SC_NPROCESSORS_ONLN) - 1;

  unsigned long input_neuron_num;
  unsigned long middle_neuron_num;
  unsigned long output_neuron_num;

  int middle_layer_type = 1; // 中間層の活性化関数の種類指定：0: identity, 1: sigmoid, 2: tanh, 3: ReLU

  bool successFlg = true;

  vector<double> in;
  vector<double> ans;

  vector<thread> threads;

  vector<Neuron> middle_neurons;
  vector<Neuron> output_neurons;

  vector<double> h; // 中間層の出力値
  vector<double> o; // 出力層の出力値
  vector<double> learnedH;
  vector<double> learnedO;


  void middleForwardThread(const int begin, const int end);
  void outForwardThread(const int begin, const int end);

  void outLearnThread(const int begin, const int end);
  void middleLearnThread(const int begin, const int end);

  void middleOutThread(const int begin, const int end);
  void outOutThread(const int begin, const int end);

  double mean_squared_error(const double output, const double answer);
};

#endif //MOTIONAUTH_SERVER_DENOISINGAUTOENCODER_H
