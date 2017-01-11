
#include "JniCppUtil.h"

#ifdef __cplusplus
extern "C" {
#endif

using namespace std;

/**
 * double型二次元配列が入ったjobjectArrayをdouble型二次元vectorに変換する
 */
vector<vector<double>> jobjectArrayToTwoDimenDoubleVector(JNIEnv *env, jobjectArray input) {
  int len1 = env->GetArrayLength(input); // 配列一次元目の長さ取得

  vector<vector<double>> output;
  if (len1 <= 0) return output;

  // 配列0番目のオブジェクトをjdoubleArrayにキャストして取得
  jdoubleArray dim = (jdoubleArray) env->GetObjectArrayElement(input, 0);

  int len2 = env->GetArrayLength(dim); // 配列二次元目の長さ取得

  vector<double> tmp;
  for (int i = 0; i < len1; ++i) {
    // 配列i番目のオブジェクトをjdoubleArrayにキャストして取得
    jdoubleArray oneDim = (jdoubleArray) env->GetObjectArrayElement(input, i);
    jdouble *element = env->GetDoubleArrayElements(oneDim, 0); // jdoubleArray要素を取得する
    for (int j = 0; j < len2; ++j) tmp.push_back(element[j]);
    output.push_back(tmp);
    tmp.clear();
    env->ReleaseDoubleArrayElements(oneDim, element, 0);
  }

  return output;
}

/**
 * double型一次元配列が入ったjdoubleArrayをdouble型一次元vectorに変換する
 */
vector<double> jdoubleArrayToOneDimenDoubleVector(JNIEnv *env, jdoubleArray input) {
  int len = env->GetArrayLength(input); // 配列の長さ取得

  vector<double> output;
  if (len <= 0) return output;

  jdouble *element = env->GetDoubleArrayElements(input, 0);
  for (int i = 0; i < len; ++i) output.push_back(element[i]);
  env->ReleaseDoubleArrayElements(input, element, 0);

  return output;
}

/**
 * String型一次元配列が入ったjobjectArrayをstring型一次元vectorに変換する
 */
vector<string> jobjectArrayToOneDimenStringVector(JNIEnv *env, jobjectArray input) {
  int len = env->GetArrayLength(input); // 配列の長さ取得

  vector<string> output;
  if (len <= 0) return output;

  jstring tmp;
  for (int i = 0; i < len; ++i) {
    tmp = (jstring) env->GetObjectArrayElement(input, i);
    const char *c = env->GetStringUTFChars(tmp, 0);
    output.push_back(c);
    env->ReleaseStringUTFChars(tmp, c);
  }

  return output;
}

/**
 * jstring型をstring型に変換する
 */
string jstringToString(JNIEnv *env, jstring input) {
  string output;
  const char *c = env->GetStringUTFChars(input, 0);
  output = c;
  env->ReleaseStringUTFChars(input, c);
  return output;
}

/**
 * double型二次元vectorをdouble型二次元配列が入ったjobjectArrayに変換する
 */
jobjectArray twoDimenDoubleVectorToJOBjectArray(JNIEnv *env, vector<vector<double>> &input) {
  double tmp[input.size()][input[0].size()];
  for (unsigned long i = 0, m = input.size(); i < m; ++i)
    for (unsigned long j = 0, n = input[i].size(); j < n; ++j)
      tmp[i][j] = input[i][j];

  int len1 = sizeof(tmp) / sizeof(tmp[0]);
  int len2 = sizeof(tmp[0]) / sizeof(tmp[0][0]);

  jclass doubleArray1DClass = env->FindClass("[D");

  // 二次元配列オブジェクトの作成
  jobjectArray array2D = env->NewObjectArray(len1, doubleArray1DClass, NULL);
  for (jint i = 0; i < len1; ++i) {
    jdoubleArray array1D = env->NewDoubleArray(len2); // 一次元配列オブジェクトの作成
    env->SetDoubleArrayRegion(array1D, 0, len2, tmp[i]); // 一次元配列オブジェクトに配列をセット
    env->SetObjectArrayElement(array2D, i, array1D);
  }

  return array2D;
}

/**
 * double型一次元vectorをjdoubleArray型に変換する
 */
jdoubleArray oneDimenDoubleVectorToJDoubleArray(JNIEnv *env, vector<double> &input) {
  double tmp[input.size()];
  for (unsigned long i = 0, n = input.size(); i < n; ++i) tmp[i] = input[i];

  int len = sizeof(tmp) / sizeof(tmp[0]);

  jdoubleArray array1D = env->NewDoubleArray(len); // 一次元配列オブジェクトの作成
  env->SetDoubleArrayRegion(array1D, 0, len, tmp); // 一次元配列オブジェクトに配列をセット

  return array1D;
}

/**
 * string型一次元vectorをString型一次元配列が入ったjobjectArrayに変換する
 */
jobjectArray oneDimenStringVectorToJObjectArray(JNIEnv *env, vector<string> &input) {
  int len = (int) input.size();
  jclass stringClass = env->FindClass("java/lang/String");
  jobjectArray array1D = env->NewObjectArray(len, stringClass, NULL);

  jstring tmp;
  for (int i = 0; i < len; ++i) {
    tmp = env->NewStringUTF(input[i].c_str());
    env->SetObjectArrayElement(array1D, i, tmp);
  }

  return array1D;
}

/**
 * string型をjstring型に変換する
 */
jstring stringToJString(JNIEnv *env, string &input) {
  return env->NewStringUTF(input.c_str());
}

#ifdef __cplusplus
}
#endif
