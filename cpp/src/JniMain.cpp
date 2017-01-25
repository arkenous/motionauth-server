#include <vector>
#include <iostream>
#include <string>
#include "JniMain.h"
#include "StackedDenoisingAutoencoder.h"
#include "Normalize.h"
#include "AddNoise.h"

using std::vector;
using std::string;
using std::cout;
using std::endl;

#ifdef __cplusplus
extern "C" {
#endif

#include <jni.h>
#include "JniCppUtil.h"

JNIEXPORT jobjectArray JNICALL Java_SocketProcessor_learn
  (JNIEnv *env, jobject thiz, jobjectArray x) {
  cout << "Java_SocketProcessor_learn" << endl;
  vector<vector<double>> xVector = jobjectArrayToTwoDimenDoubleVector(env, x);
  cout << "Data converted" << endl;

  for (unsigned long i = 0, size = xVector.size(); i < size; ++i)
    normalize(&xVector[i]);
  cout << "Data normalized" << endl;

  vector<vector<double>> train;
  for (unsigned long i = 0, size = xVector.size(); i < size; ++i)
    train.push_back(xVector[i]);

  cout << "Start building SdA" << endl;
  StackedDenoisingAutoencoder stackedDenoisingAutoencoder;
  stackedDenoisingAutoencoder.build(train, num_sda_layer, sda_compression_rate, dropout_rate);
  cout << "Finish Building SdA" << endl;

  vector<vector<double>> answer;
  answer.resize(xVector.size());
  for (unsigned long i = 0, size = xVector.size(); i < size; ++i) {
    answer[i].push_back(0.0);
  }

  cout << "Start learning NN" << endl;
  vector<string> resultString = stackedDenoisingAutoencoder.learn(train, answer, dropout_rate);
  cout << "Finish learning NN" << endl;

  jobjectArray result = oneDimenStringVectorToJObjectArray(env, resultString);
  cout << "Result converted" << endl;

  return result;
}


JNIEXPORT jdouble JNICALL Java_SocketProcessor_out
  (JNIEnv *env, jobject thiz, jobjectArray neuronParams, jdoubleArray x) {
  cout << "Java_SocketProcessor_out" << endl;
  vector<string> neuronParamsVector = jobjectArrayToOneDimenStringVector(env, neuronParams);
  vector<double> xVector = jdoubleArrayToOneDimenDoubleVector(env, x);
  cout << "Data converted" << endl;

  normalize(&xVector);
  cout << "Data normalized" << endl;

  StackedDenoisingAutoencoder stackedDenoisingAutoencoder;
  cout << "Start setup NN" << endl;
  stackedDenoisingAutoencoder.setup(neuronParamsVector, dropout_rate);

  cout << "Start feed forwarding NN" << endl;
  double result = stackedDenoisingAutoencoder.out(xVector);
  cout << "Finish feed forwarding NN" << endl;

  jdouble resultJdouble = result;
  cout << "Result converted" << endl;

  return resultJdouble;
}


#ifdef __cplusplus
}
#endif
