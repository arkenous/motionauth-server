
#include "AddNoise.h"
#include <vector>
#include <random>
#include <algorithm>

using std::vector;
using std::random_device;
using std::mt19937;
using std::uniform_real_distribution;

/**
 * データごとに0.0以上1.0未満の乱数を生成し，rate未満であればそのデータを0.0にする
 * @param input ノイズをのせるデータ
 * @param rate ノイズをのせる確率
 * @return ノイズをのせたデータ
 */
vector<vector<double>> max_min_noise(const vector<vector<double>> &input, const float rate) {
  random_device rnd;
  mt19937 mt;
  mt.seed(rnd());
  uniform_real_distribution<double> real_rnd(0.0, 1.0);
  double rnd_value = 0.0;

  vector<vector<double>> result(input);

  for (unsigned long i = 0, i_s = result.size(); i < i_s; ++i) {
    double max = *std::max_element(result[i].begin(), result[i].end());
    double min = *std::min_element(result[i].begin(), result[i].end());
    for (unsigned long j = 0, j_s = result[i].size(); j < j_s; ++j) {
      //if (real_rnd(mt) < rate) result[i][j] = 0.0;
      rnd_value = real_rnd(mt);
      if (rnd_value <= rate) {

        //std::cout << "before: " << result[i][j] << std::endl;
        if (rnd_value <= rate / 2.0) {
          result[i][j] = min;
        } else {
          result[i][j] = max;
        }
        //std::cout << "after: " << result[i][j] << std::endl;

      }
    }
  }

  return result;
}

/**
 * データごとに0.0以上1.0未満の乱数を生成し，rate未満であればそのデータをデータセットの最小値以上最大値未満の乱数にする
 * @param input ノイズをのせるデータ
 * @param rate ノイズをのせる確率
 * @return ノイズをのせたデータ
 */
vector<vector<double>> gen_random_noise(const vector<vector<double>> &input, const float rate) {
  random_device rnd;
  mt19937 mt;
  mt.seed(rnd());
  uniform_real_distribution<double> rnd_zero_one(0.0, 1.0);
  double rnd_zero_one_val = 0.0;

  vector<vector<double>> result(input);

  for (unsigned long i = 0, i_s = result.size(); i < i_s; ++i) {
    double max = *std::max_element(result[i].begin(), result[i].end());
    double min = *std::min_element(result[i].begin(), result[i].end());
    uniform_real_distribution<double> rnd_val(min, max);
    for (unsigned long j = 0, j_s = result[i].size(); j < j_s; ++j) {
      rnd_zero_one_val = rnd_zero_one(mt);
      if (rnd_zero_one_val <= rate) {
        //std::cout << "before: " << result[i][j] << std::endl;
        result[i][j] = rnd_val(mt);
        //std::cout << "after: " << result[i][j] << std::endl;
      }
    }
  }

  return result;
}

vector<vector<double>> zero_noise(const vector<vector<double>> &input, const float rate) {
  random_device rnd;
  mt19937 mt;
  mt.seed(rnd());
  uniform_real_distribution<double> rnd_zero_one(0.0, 1.0);
  double rnd_zero_one_val = 0.0;

  vector<vector<double>> result(input);

  for (unsigned long i = 0, i_s = result.size(); i < i_s; ++i) {
    for (unsigned long j = 0, j_s = result[i].size(); j < j_s; ++j) {
      rnd_zero_one_val = rnd_zero_one(mt);
      if (rnd_zero_one_val <= rate) {
        //std::cout << "before: " << result[i][j] << std::endl;
        result[i][j] = 0.0;
        //std::cout << "after: " << result[i][j] << std::endl;
      }
    }
  }

  return result;
}

vector<vector<double>> tiny_noise(const vector<vector<double>> &input, const float rate) {
  random_device rnd;
  mt19937 mt;
  mt.seed(rnd());
  uniform_real_distribution<double> rnd_zero_one(0.0, 1.0);
  double rnd_zero_one_val = 0.0;
  double tiny_val = 0.0;


  vector<vector<double>> result(input);

  for (unsigned long i = 0, i_s = result.size(); i < i_s; ++i) {
      tiny_val = 0.0;
    for (unsigned long j = 0, j_s = result[i].size(); j < j_s; ++j) {
      tiny_val += std::abs(result[i][j]);
    }
    tiny_val = (tiny_val / result[i].size()) / 1.0;
    //std::cout << "tiny_val: " << tiny_val << std::endl;

    for (unsigned long j = 0, j_s = result[i].size(); j < j_s; ++j) {
      rnd_zero_one_val = rnd_zero_one(mt);
      if (rnd_zero_one_val <= rate) {
        //std::cout << "tiny_val: " << tiny_val << std::endl;
        //std::cout << "before: " << result[i][j] << std::endl;
        if (rnd_zero_one_val <= rate / 2.0) {
          result[i][j] -= tiny_val;
        } else {
          result[i][j] += tiny_val;
        }
        //std::cout << "after: " << result[i][j] << std::endl;
      }
    }
  }

  return result;
}

vector<vector<double>> gaussian_noise(const vector<vector<double>> &input, const double mean, const double stddev, const float rate) {
  vector<vector<double>> result(input);
  random_device rnd;
  mt19937 mt;
  mt.seed(rnd());
  uniform_real_distribution<double> real_rnd(0.0, 1.0);
  std::normal_distribution<double> dist(mean, stddev);

  for (unsigned long i = 0, i_s = result.size(); i < i_s; ++i) {
    for (unsigned long j = 0, j_s = result[i].size(); j < j_s; ++j) {
      if (real_rnd(mt) <= rate) {
        //std::cout << "before: " << result[i][j] << std::endl;
        result[i][j] += dist(mt);
        //std::cout << "after: " << result[i][j] << std::endl;
      }
    }
  }

  return result;
}
