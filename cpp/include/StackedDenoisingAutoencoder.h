
#ifndef MOTIONAUTH_SERVER_STACKEDDENOISINGAUTOENCODER_H
#define MOTIONAUTH_SERVER_STACKEDDENOISINGAUTOENCODER_H

#include <vector>
#include <string>
#include <thread>
#include <zconf.h>
#include "Neuron.cuh"

using std::string;
using std::vector;
using std::thread;

class StackedDenoisingAutoencoder {
public:
  StackedDenoisingAutoencoder();

  void build(const vector<vector<double>> &input,
             const unsigned long result_num_dimen, const float compression_rate,
             const double dropout_rate);
  void setup(const vector<string> &params, const double dropout_rate);
  vector<string> learn(const vector<vector<double>> &input, const vector<vector<double>> &answer,
             const double dropout_rate);
  double out(const vector<double> &input);

private:
  static const unsigned int MAX_TRIAL = 10000; // 学習上限回数
  constexpr static const double MAX_GAP = 0.1; // 許容する誤差の域値
  bool successFlg = true;
  unsigned long num_thread = (unsigned long) sysconf(_SC_NPROCESSORS_ONLN);
  unsigned long num_middle_neurons;
  unsigned long output_neuron_num = 1;

  vector<vector<Neuron>> sda_neurons;
  vector<vector<double>> sda_out;
  vector<vector<double>> sda_learned_out;
  Neuron output_neuron;
  double o;
  double learned_o;

  vector<thread> threads;
  vector<double> in;
  vector<double> ans;

  vector<double> separate_by_camma(const string &input);
  void setupMiddleNeuron(const vector<string> &elems_per_neuron, const double dropout_rate,
                         const int layer, const int begin, const int end);
  void sdaFirstLayerForwardThread(const int begin, const int end);
  void sdaOtherLayerForwardThread(const int layer, const int begin, const int end);
  void outForwardThread(const int begin, const int end);

  void sdaFirstLayerOutThread(const int begin, const int end);
  void sdaOtherLayerOutThread(const int layer, const int begin, const int end);
  void outOutThread(const int begin, const int end);

  void outLearnThread(const int begin, const int end);

  void sdaLastLayerLearnThread(const int begin, const int end);
  void sdaMiddleLayerLearnThread(const int layer, const int begin, const int end);
  void sdaFirstLayerLearnThread(const int begin, const int end);

  double crossEntropy(const double output, const double answer);
};

#endif //MOTIONAUTH_SERVER_STACKEDDENOISINGAUTOENCODER_H
