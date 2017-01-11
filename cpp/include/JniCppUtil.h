
#ifndef MOTIONAUTH_SERVER_JNICPPUTIL_H
#define MOTIONAUTH_SERVER_JNICPPUTIL_H

#include <vector>
#include <string>

#ifdef __cplusplus
extern "C" {
#endif

#include <jni.h>

using namespace std;

vector<vector<double>> jobjectArrayToTwoDimenDoubleVector(JNIEnv *env, jobjectArray input);
vector<double> jdoubleArrayToOneDimenDoubleVector(JNIEnv *env, jdoubleArray input);
vector<string> jobjectArrayToOneDimenStringVector(JNIEnv *env, jobjectArray input);
string jstringToString(JNIEnv *env, jstring input);
jobjectArray twoDimenDoubleVectorToJOBjectArray(JNIEnv *env, vector<vector<double>> &input);
jdoubleArray oneDimenDoubleVectorToJDoubleArray(JNIEnv *env, vector<double> &input);
jobjectArray oneDimenStringVectorToJObjectArray(JNIEnv *env, vector<string> &input);
jstring stringToJString(JNIEnv *env, string &input);

#ifdef __cplusplus
}
#endif
#endif //MOTIONAUTH_SERVER_JNICPPUTIL_H
