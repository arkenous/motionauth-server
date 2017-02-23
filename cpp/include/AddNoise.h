
#ifndef MOTIONAUTH_SERVER_ADDNOISE_H
#define MOTIONAUTH_SERVER_ADDNOISE_H

#include <vector>

using std::vector;

vector<vector<double>> max_min_noise(const vector<vector<double>> &input, const float rate);
vector<vector<double>> gen_random_noise(const vector<vector<double>> &input, const float rate);
vector<vector<double>> zero_noise(const vector<vector<double>> &input, const float rate);
vector<vector<double>> tiny_noise(const vector<vector<double>> &input, const float rate);
vector<vector<double>> gaussian_noise(const vector<vector<double>> &input, const double mean, const double stddev, const float rate);

#endif //MOTIONAUTH_SERVER_ADDNOISE_H
