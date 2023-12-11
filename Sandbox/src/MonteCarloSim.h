#pragma once
#include "Hazel.h"
#include <chrono>
#include <cmath>
#include <cstdio>
#include <fstream>
#include <iostream>
#include <memory>
#include <omp.h>
#include <random>
#include <sstream>
#include <string>
using namespace Hazel;

class MonteCarloSim
{
public:
	std::vector<float> RunSimulation(int num_threads,int inLoop,int outLoop,int timestep);
private:
	float calculateVolatility(float spotPrice, int32_t timeSteps);
	std::vector<float> find2dMean(std::vector<std::vector<float>>& matrix, int32_t numLoops, int32_t timeSteps);
	float genRand(float mean, float stdDev);
	std::vector<float> runBlackScholesModel(float spotPrice, int32_t timeSteps, float riskRate, float volatility);

};