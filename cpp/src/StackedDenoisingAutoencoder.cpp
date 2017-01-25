
#include <random>
#include <sstream>
#include <iostream>
#include "StackedDenoisingAutoencoder.h"
#include "DenoisingAutoencoder.h"
#include "AddNoise.h"

using std::string;
using std::to_string;
using std::vector;
using std::stringstream;
using std::stod;
using std::stoul;
using std::ref;
using std::random_device;
using std::mt19937;
using std::uniform_real_distribution;
using std::thread;
using std::cout;
using std::endl;

StackedDenoisingAutoencoder::StackedDenoisingAutoencoder() {
  threads = vector<thread>(num_thread);
}


void StackedDenoisingAutoencoder::build(const vector<vector<double>> &input,
                                          const unsigned long result_num_layer,
                                          const float compression_rate,
                                          const double dropout_rate) {
  sda_neurons.resize(result_num_layer);
  sda_out.resize(result_num_layer);
  sda_learned_out.resize(result_num_layer);
  unsigned long num_sda_layer = 0;

  vector<vector<double>> answer(input);
  vector<vector<double>> noisy_input(add_noise(input, 0.2));

  DenoisingAutoencoder denoisingAutoencoder(noisy_input[0].size(), compression_rate, dropout_rate);

  // Store learned dA middle neurons
  sda_neurons[num_sda_layer] = denoisingAutoencoder.learn(answer, noisy_input);
  sda_out[num_sda_layer].resize(sda_neurons[num_sda_layer].size());
  sda_learned_out[num_sda_layer].resize(sda_neurons[num_sda_layer].size());

  num_sda_layer++;

  while (num_sda_layer < result_num_layer) {
    answer = vector<vector<double>>(noisy_input);
    noisy_input = add_noise(denoisingAutoencoder.getMiddleOutput(noisy_input), 0.2);

    denoisingAutoencoder = DenoisingAutoencoder(noisy_input[0].size(), compression_rate,
                                                dropout_rate);
    sda_neurons[num_sda_layer] = denoisingAutoencoder.learn(answer, noisy_input);
    sda_out[num_sda_layer].resize(sda_neurons[num_sda_layer].size());
    sda_learned_out[num_sda_layer].resize(sda_neurons[num_sda_layer].size());

    num_sda_layer++;
  }
}

void StackedDenoisingAutoencoder::setup(const vector<string> &params, const double dropout_rate) {
  stringstream ss;
  string out_params = params.back();
  ss = stringstream(out_params);

  string item = "";
  vector<string> elems_per_param;
  while (std::getline(ss, item, '|')) if (!item.empty()) elems_per_param.push_back(item);
  item = "";
  ss.str("");

  double bias = stod(elems_per_param.back());
  elems_per_param.pop_back();

  unsigned long iteration = stoul(elems_per_param.back());
  elems_per_param.pop_back();

  vector<double> weight = separate_by_camma(elems_per_param[0]);
  vector<double> m = separate_by_camma(elems_per_param[1]);
  vector<double> nu = separate_by_camma(elems_per_param[2]);

  output_neuron = Neuron(weight.size(), weight, m, nu, iteration, bias, 1, dropout_rate);


  sda_neurons.resize(params.size() - 1);
  sda_out.resize(params.size() - 1);
  sda_learned_out.resize(params.size() - 1);

  vector<string> elems_per_neuron;
  for (unsigned long layer = 0, l_size = sda_neurons.size(); layer < l_size; ++layer) {
    ss = stringstream(params[layer]);

    // split per Neuron
    while (std::getline(ss, item, '\'')) if (!item.empty()) elems_per_neuron.push_back(item);
    sda_neurons[layer].resize(elems_per_neuron.size());
    sda_out[layer].resize(elems_per_neuron.size());
    sda_learned_out[layer].resize(elems_per_neuron.size());

    item = "";
    ss.str("");

    unsigned long charge;
    threads.clear();
    unsigned long elems_per_neuron_size = elems_per_neuron.size();
    if (elems_per_neuron_size <= num_thread) charge = 1;
    else charge = elems_per_neuron_size / num_thread;
    for (int i = 0; i < elems_per_neuron_size; i += charge) {
      if (i != 0 && elems_per_neuron_size / i == 1)
        threads.push_back(thread(&StackedDenoisingAutoencoder::setupMiddleNeuron, this,
                                 ref(elems_per_neuron), dropout_rate, layer,
                                 i, elems_per_neuron_size));
      else
        threads.push_back(thread(&StackedDenoisingAutoencoder::setupMiddleNeuron, this,
                                 ref(elems_per_neuron), dropout_rate, layer,
                                 i, i + charge));
    }
    for (thread &th : threads) th.join();

    elems_per_neuron.clear();
  }
}

vector<double> StackedDenoisingAutoencoder::separate_by_camma(const string &input) {
  vector<double> result;
  stringstream ss = stringstream(input);
  string item;
  while (getline(ss, item, ',')) if (!item.empty()) result.push_back(stod(item));
  item = "";
  ss.str("");
  ss.clear(stringstream::goodbit);

  return result;
}

void StackedDenoisingAutoencoder::setupMiddleNeuron(const vector<string> &elems_per_neuron,
                                                    const double dropout_rate, const int layer,
                                                    const int begin, const int end) {
  for (int neuron = begin; neuron < end; ++neuron) {
    stringstream ss(elems_per_neuron[neuron]);
    string item = "";
    vector<string> elems_per_param;
    while (std::getline(ss, item, '|')) if (!item.empty()) elems_per_param.push_back(item);
    item = "";
    ss.str("");

    double bias = stod(elems_per_param.back());
    elems_per_param.pop_back();

    unsigned long iteration = stoul(elems_per_param.back());
    elems_per_param.pop_back();

    vector<double> weight = separate_by_camma(elems_per_param[0]);
    vector<double> m = separate_by_camma(elems_per_param[1]);
    vector<double> nu = separate_by_camma(elems_per_param[2]);

    sda_neurons[layer][neuron] = Neuron(weight.size(), weight, m, nu, iteration, bias,
                                        1, dropout_rate);
  }
}


vector<string> StackedDenoisingAutoencoder::learn(const vector <vector<double>> &input,
                                        const vector <vector<double>> &answer,
                                        const double dropout_rate) {
  vector<double> empty_vector;
  output_neuron = Neuron(sda_neurons.back().size(), empty_vector, empty_vector, empty_vector,
                         0, 0.0, 1, dropout_rate);
  output_neuron.dropout(1.0); // disable dropout

  // Learn
  int succeed = 0; // 連続正解回数のカウンタを初期化

  random_device rnd;
  mt19937 mt;
  mt.seed(rnd());
  uniform_real_distribution<double> real_rnd(0.0, 1.0);

  for (int trial = 0; trial < MAX_TRIAL; ++trial) {
    cout << "-----   trial: " << trial << "   -----" << endl;

    // Set SdA dropout
    for (unsigned long layer = 0, l_size = sda_neurons.size(); layer < l_size; ++layer) {
      for (unsigned long neuron = 0, n_size = sda_neurons[layer].size(); neuron < n_size; ++neuron) {
        sda_neurons[layer][neuron].dropout(real_rnd(mt));
      }
    }

    in = input[trial % answer.size()];
    ans = answer[trial % answer.size()];

    // Feed Forward
    // SdA First Layer
    unsigned long charge;
    threads.clear();
    if (sda_neurons[0].size() <= num_thread) charge = 1;
    else charge = sda_neurons[0].size() / num_thread;
    for (unsigned long i = 0, num_neuron = sda_neurons[0].size(); i < num_neuron; i += charge) {
      if (i != 0 && num_neuron / i == 1) {
        threads.push_back(thread(&StackedDenoisingAutoencoder::sdaFirstLayerForwardThread, this,
                                 i, num_neuron));
      } else {
        threads.push_back(thread(&StackedDenoisingAutoencoder::sdaFirstLayerForwardThread, this,
                                 i, i + charge));
      }
    }
    for (thread &th : threads) th.join();

    // SdA Other Layer
    if (sda_neurons.size() > 1) {
      for (unsigned long layer = 1, last_layer = sda_neurons.size() - 1;
           layer <= last_layer; ++layer) {
        threads.clear();
        if (sda_neurons[layer].size() <= num_thread) charge = 1;
        else charge = sda_neurons[layer].size() / num_thread;
        for (unsigned long i = 0, num_neuron = sda_neurons[layer].size();
             i < num_neuron; i += charge) {
          if (i != 0 && num_neuron / i == 1) {
            threads.push_back(thread(&StackedDenoisingAutoencoder::sdaOtherLayerForwardThread, this,
                                     layer, i, num_neuron));
          } else {
            threads.push_back(thread(&StackedDenoisingAutoencoder::sdaOtherLayerForwardThread, this,
                                     layer, i, i + charge));
          }
        }
        for (thread &th : threads) th.join();
      }
    }

    // 出力値を推定
    threads.clear();
    if (output_neuron_num <= num_thread) charge = 1;
    else charge = output_neuron_num / num_thread;
    for (int i = 0; i < output_neuron_num; i += charge) {
      if (i != 0 && output_neuron_num / i == 1) {
        threads.push_back(thread(&StackedDenoisingAutoencoder::outForwardThread, this,
                                 i, output_neuron_num));
      } else {
        threads.push_back(thread(&StackedDenoisingAutoencoder::outForwardThread, this,
                                 i, i + charge));
      }
    }
    for (thread &th : threads) th.join();

    successFlg = true;

    // Back Propagation (learn phase)
    //region 出力層を学習する
    threads.clear();
    if (output_neuron_num <= num_thread) charge = 1;
    else charge = output_neuron_num / num_thread;
    for (int i = 0; i < output_neuron_num; i += charge) {
      if (i != 0 && output_neuron_num / i == 1) {
        threads.push_back(thread(&StackedDenoisingAutoencoder::outLearnThread, this,
                                 i, output_neuron_num));
      } else {
        threads.push_back(thread(&StackedDenoisingAutoencoder::outLearnThread, this,
                                 i, i + charge));
      }
    }
    for (thread &th : threads) th.join();
    //endregion

    // 連続成功回数による終了判定
    if (successFlg) {
      succeed++;
      if (succeed >= input.size()) break;
      else continue;
    } else succeed = 0;

    // learn SdA
//    if (sda_neurons.size() > 1) {
//      threads.clear();
//      if (sda_neurons[sda_neurons.size() - 1].size() <= num_thread) charge = 1;
//      else charge = sda_neurons[sda_neurons.size() - 1].size() / num_thread;
//      for (int i = 0; i < sda_neurons[sda_neurons.size() - 1].size(); i += charge) {
//        if (i != 0 && sda_neurons[sda_neurons.size() - 1].size() / i == 1) {
//          threads.push_back(std::thread(&StackedDenoisingAutoencoder::sdaLastLayerLearnThread, this,
//                                        i, sda_neurons[sda_neurons.size() - 1].size()));
//        } else {
//          threads.push_back(std::thread(&StackedDenoisingAutoencoder::sdaLastLayerLearnThread, this,
//                                        i, i + charge));
//        }
//      }
//      for (std::thread &th : threads) th.join();
//    }
//
//    for (int layer = sda_neurons.size() - 2; layer >= 1; --layer) {
//      if (sda_neurons[layer].size() <= num_thread) charge = 1;
//      else charge = sda_neurons[layer].size() / num_thread;
//      threads.clear();
//      for (int i = 0; i < sda_neurons[layer].size(); i += charge) {
//        if (i != 0 && sda_neurons[layer].size() / i == 1) {
//          threads.push_back(std::thread(&StackedDenoisingAutoencoder::sdaMiddleLayerLearnThread, this,
//                                        layer, i, sda_neurons[layer].size()));
//        } else {
//          threads.push_back(std::thread(&StackedDenoisingAutoencoder::sdaMiddleLayerLearnThread, this,
//                                        layer, i, i + charge));
//        }
//      }
//      for (std::thread &th : threads) th.join();
//    }
//
//    threads.clear();
//    if (sda_neurons[0].size() <= num_thread) charge = 1;
//    else charge = sda_neurons[0].size() / num_thread;
//    for (int i = 0; i < sda_neurons[0].size(); i += charge) {
//      if (i != 0 && sda_neurons[0].size() / i == 1) {
//        threads.push_back(std::thread(&StackedDenoisingAutoencoder::sdaFirstLayerLearnThread, this,
//                                      i, sda_neurons[0].size()));
//      } else {
//        threads.push_back(std::thread(&StackedDenoisingAutoencoder::sdaFirstLayerLearnThread, this,
//                                      i, i + charge));
//      }
//    }
//    for (std::thread &th : threads) th.join();
  }

  // 全ての教師データで正解を出すか，学習上限回数を超えた場合に終了
  //レイヤ毎にニューロンのパラメータを取得し，vectorにつめて返す
  vector<string> result(sda_neurons.size() + 1, "");
  string params_per_layer = "";
  for (unsigned long layer = 0, l_size = sda_neurons.size(); layer < l_size; ++layer) {
    for (unsigned long neuron = 0, n_size = sda_neurons[layer].size(); neuron < n_size; ++neuron ) {
      unsigned long num_input = sda_neurons[layer][neuron].getNumInput();

      for (unsigned long num = 0; num < num_input; ++num)
        params_per_layer += to_string(sda_neurons[layer][neuron].getInputWeightIndexOf(num)) + ',';
      params_per_layer.pop_back();
      params_per_layer += '|';

      for (unsigned long num = 0; num < num_input; ++num)
        params_per_layer += to_string(sda_neurons[layer][neuron].getMIndexOf(num)) + ',';
      params_per_layer.pop_back();
      params_per_layer += '|';

      for (unsigned long num = 0; num < num_input; ++num)
        params_per_layer += to_string(sda_neurons[layer][neuron].getNuIndexOf(num)) + ',';
      params_per_layer.pop_back();
      params_per_layer += '|';

      params_per_layer += to_string(sda_neurons[layer][neuron].getIteration()) + '|';

      params_per_layer += to_string(sda_neurons[layer][neuron].getBias()) + '\'';
    }

    params_per_layer.pop_back();

    result[layer] = params_per_layer;
    params_per_layer = "";
  }

  string params = "";
  unsigned long num_input = output_neuron.getNumInput();
  for (unsigned long num = 0; num < num_input; ++num)
    params += to_string(output_neuron.getInputWeightIndexOf(num)) + ',';
  params.pop_back();
  params += '|';

  for (unsigned long num = 0; num < num_input; ++num)
    params += to_string(output_neuron.getMIndexOf(num)) + ',';
  params.pop_back();
  params += '|';

  for (unsigned long num = 0; num < num_input; ++num)
    params += to_string(output_neuron.getNuIndexOf(num)) + ',';
  params.pop_back();
  params += '|';

  params += to_string(output_neuron.getIteration()) + '|';

  params += to_string(output_neuron.getBias());

  result[result.size() - 1] = params;

  return result;
}




void StackedDenoisingAutoencoder::sdaFirstLayerForwardThread(const int begin,
                                                             const int end) {
  for (int neuron = begin; neuron < end; ++neuron)
    sda_out[0][neuron] = sda_neurons[0][neuron].learn_output(in);
}

void StackedDenoisingAutoencoder::sdaOtherLayerForwardThread(const int layer,
                                                             const int begin, const int end) {
  for (int neuron = begin; neuron < end; ++neuron)
    sda_out[layer][neuron] = sda_neurons[layer][neuron].learn_output(sda_out[layer - 1]);
}

void StackedDenoisingAutoencoder::outForwardThread(const int begin, const int end) {
  for (int neuron = begin; neuron < end; ++neuron)
    o = output_neuron.learn_output(sda_out.back());
}

void StackedDenoisingAutoencoder::outLearnThread(const int begin, const int end) {
  for (int neuron = begin; neuron < end; ++neuron) {
    // 出力層ニューロンのdeltaの計算
    double delta = o - ans[neuron];

    // 教師データとの誤差が十分小さい場合は学習しない．そうでなければ正解フラグをfalseに
    if (crossEntropy(o, ans[neuron]) < MAX_GAP) continue;
    else {
      cout << "MLP ce: " << crossEntropy(o, ans[neuron]) << endl;
      successFlg = false;
    }

    // 出力層の学習
    output_neuron.learn(delta, sda_out.back());
  }
}

void StackedDenoisingAutoencoder::sdaLastLayerLearnThread(const int begin, const int end) {
  for (int neuron = begin; neuron < end; ++neuron) {
    double sumDelta = 0.0;
    sumDelta = output_neuron.getInputWeightIndexOf(neuron) * output_neuron.getDelta();

    double delta;
    // sigmoid
    delta = (sda_out[sda_out.size() - 1][neuron]
             * (1.0 - sda_out[sda_out.size() - 1][neuron])) * sumDelta;

    sda_neurons[sda_neurons.size() - 1][neuron].learn(delta, sda_out[sda_out.size() - 2]);
  }
}

void StackedDenoisingAutoencoder::sdaMiddleLayerLearnThread(const int layer,
                                                            const int begin, const int end) {
  for (int neuron = begin; neuron < end; ++neuron) {
    double sumDelta = 0.0;
    for (int k = 0; k < sda_neurons[layer + 1].size(); ++k) {
      sumDelta += sda_neurons[layer + 1][k].getInputWeightIndexOf(neuron)
                  * sda_neurons[layer + 1][k].getDelta();
    }

    double delta;
    // sigmoid
    delta = (sda_out[layer][neuron] * (1.0 - sda_out[layer][neuron])) * sumDelta;

    sda_neurons[layer][neuron].learn(delta, sda_out[layer - 1]);
  }
}

void StackedDenoisingAutoencoder::sdaFirstLayerLearnThread(const int begin, const int end) {
  for (int neuron = begin; neuron < end; ++neuron) {
    double sumDelta = 0.0;

    if (sda_neurons.size() > 1) {
      for (int k = 0; k < sda_neurons[1].size(); ++k) {
        sumDelta += sda_neurons[1][k].getInputWeightIndexOf(neuron) * sda_neurons[1][k].getDelta();
      }
    } else {
      sumDelta = output_neuron.getInputWeightIndexOf(neuron) * output_neuron.getDelta();
    }

    double delta;
    // sigmoid
    delta = (sda_out[0][neuron] * (1.0 - sda_out[0][neuron])) * sumDelta;

    sda_neurons[0][neuron].learn(delta, in);
  }
}


double StackedDenoisingAutoencoder::crossEntropy(const double output, const double answer) {
  return -answer * log(output) - (1.0 - answer) * log(1.0 - output);
}


double StackedDenoisingAutoencoder::out(const vector<double> &input) {
  in = input;

  // Feed Forward
  // SdA First Layer
  unsigned long charge;
  threads.clear();
  if (sda_neurons[0].size() <= num_thread) charge = 1;
  else charge = sda_neurons[0].size() / num_thread;
  for (unsigned long i = 0, num_neuron = sda_neurons[0].size(); i < num_neuron; i += charge) {
    if (i != 0 && num_neuron / i == 1) {
      threads.push_back(thread(&StackedDenoisingAutoencoder::sdaFirstLayerOutThread, this,
                               i, num_neuron));
    } else {
      threads.push_back(thread(&StackedDenoisingAutoencoder::sdaFirstLayerOutThread, this,
                               i, i + charge));
    }
  }
  for (thread &th : threads) th.join();

  // SdA Other Layer
  if (sda_neurons.size() > 1) {
    for (unsigned long layer = 1, last_layer = sda_neurons.size() - 1;
         layer <= last_layer; ++layer) {
      threads.clear();
      if (sda_neurons[layer].size() <= num_thread) charge = 1;
      else charge = sda_neurons[layer].size() / num_thread;
      for (unsigned long i = 0, num_neuron = sda_neurons[layer].size();
           i < num_neuron; i += charge) {
        if (i != 0 && num_neuron / i == 1) {
          threads.push_back(thread(&StackedDenoisingAutoencoder::sdaOtherLayerOutThread, this,
                                   layer, i, num_neuron));
        } else {
          threads.push_back(thread(&StackedDenoisingAutoencoder::sdaOtherLayerOutThread, this,
                                   layer, i, i + charge));
        }
      }
      for (thread &th : threads) th.join();
    }
  }

  // 出力値を推定
  threads.clear();
  if (output_neuron_num <= num_thread) charge = 1;
  else charge = output_neuron_num / num_thread;
  for (int i = 0; i < output_neuron_num; i += charge) {
    if (i != 0 && output_neuron_num / i == 1) {
      threads.push_back(thread(&StackedDenoisingAutoencoder::outOutThread, this,
                               i, output_neuron_num));
    } else {
      threads.push_back(thread(&StackedDenoisingAutoencoder::outOutThread, this,
                               i, i + charge));
    }
  }
  for (thread &th : threads) th.join();


  return learned_o;
}

void StackedDenoisingAutoencoder::sdaFirstLayerOutThread(const int begin, const int end) {
  for (int neuron = begin; neuron < end; ++neuron)
    sda_learned_out[0][neuron] = sda_neurons[0][neuron].output(in);
}

void StackedDenoisingAutoencoder::sdaOtherLayerOutThread(const int layer,
                                                         const int begin, const int end) {
  for (int neuron = begin; neuron < end; ++neuron) {
    sda_learned_out[layer][neuron] = sda_neurons[layer][neuron].output(sda_learned_out[layer - 1]);
  }
}

void StackedDenoisingAutoencoder::outOutThread(const int begin, const int end) {
  for (int neuron = begin; neuron < end; ++neuron) {
    learned_o = output_neuron.output(sda_learned_out.back());
  }
}
