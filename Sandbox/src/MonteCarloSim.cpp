#include "MonteCarloSim.h"
#include "Sandbox2dApp.h"

using namespace Hazel;
static std::vector<float> stockPrice;
std::vector<float> MonteCarloSim::RunSimulation(int num_threads, int inLoops, int outLoops, int timeSteps )
{
    const auto beginTime = std::chrono::system_clock::now();

    // Matrix for stock-price vectors per iteration
    std::vector<std::vector<float>> stock (inLoops,std::vector<float>(timeSteps));

    // Matrix for mean of stock-price vectors per iteration
    std::vector<std::vector<float>> avgStock(outLoops, std::vector<float>(timeSteps));

    static float spotPrice = SandBox2dApp::ClosingPrices[0];  // Spot price (at t = 0)

    // Market volatility (calculated from data.csv)
    const float volatility = calculateVolatility(spotPrice, timeSteps);

    // Welcome message
    HAZEL_CORE_INFO("{} Using market volatility: ", volatility);

    int32_t i;
    // Parallel region with each thread having its own instance of variable 'i',
#pragma omp parallel private(i)
    {
        // Only one thread (irrespective of thread id) handles this region
#pragma omp single
        {
            const int32_t numThreads = omp_get_num_threads();   // Number of threads
            std::cout << "Using " << numThreads << " thread(s)\n\n";
            std::cout << "Have patience! Computing.. ";
            omp_set_num_threads(numThreads);
        }

        /** Parallel for loop with dynamic scheduling, i.e. each thread
            grabs "chunk" iterations until all iterations are done.
            Faster threads are assigned more iterations (not Round Robin) **/
#pragma omp for schedule(dynamic)
        for (i = 0; i < outLoops; i++)
        {
            /** Using Black Scholes model to get stock price every iteration
                Returns data as a column vector having rows=timeSteps  **/
            for (int32_t j = 0; j < inLoops; j++)
            {
                static constexpr float riskRate = 0.01f;   // Risk free interest rate (%)
                stock[j] = runBlackScholesModel(spotPrice, timeSteps, riskRate, volatility); //returns array of predicted prices with length timeSteps
            }

            // Stores average of all estimated stock-price arrays
            avgStock[i] = find2dMean(stock, inLoops, timeSteps);
        }
        //---> Implicit omp barrier <--
    }

    // Average of all the average arrays
    std::vector<float> optStock(timeSteps);     // Vector for most likely outcome stock price
    optStock = find2dMean(avgStock, outLoops, timeSteps);

    return optStock;
}

float MonteCarloSim::calculateVolatility(float spotPrice, int32_t timeSteps)
{
    int32_t len = timeSteps - 1;
    //if (len > SandBox2dApp::ClosingPrices.size())
        //len = SandBox2dApp::ClosingPrices.size();

    float sum = spotPrice;
    // Find mean of the estimated minute-end prices
    for (int i = 0; i < len; i++)
        sum += SandBox2dApp::ClosingPrices[i];
    float meanPrice = sum / (len + 1);

    // Calculate market volatility as standard deviation
    sum = std::powf((spotPrice - meanPrice), 2.0f);
    for (int i = 0; i < len; i++)
        sum += std::powf((SandBox2dApp::ClosingPrices[i] - meanPrice), 2.0f);

    float stdDev = std::sqrtf(sum);

    // Return as percentage
    return stdDev / 100.0f;
}

std::vector<float> MonteCarloSim::find2dMean(std::vector<std::vector<float>>& matrix, int32_t numLoops, int32_t timeSteps)
{
    int32_t j;
    std::vector<float> avg (timeSteps);
    float sum = 0.0f;

    for (int32_t i = 0; i < timeSteps; i++)
    {
        /** A private copy of 'sum' variable is created for each thread.
            At the end of the reduction, the reduction variable is applied to
            all private copies of the shared variable, and the final result
            is written to the global shared variable. **/
#pragma omp parallel for private(j) reduction(+:sum)
        for (j = 0; j < numLoops; j++)
        {
            sum += matrix[j][i];
        }

        // Calculating average across columns
        avg[i] = sum / numLoops;
        sum = 0.0f;
    }

    return avg;
}

float MonteCarloSim::genRand(float mean, float stdDev)
{
    //using time as seed to always get a random value 
    const auto seed = std::chrono::system_clock::now().time_since_epoch().count();
    std::default_random_engine generator(static_cast<uint32_t>(seed));
    std::normal_distribution<float> distribution(mean, stdDev); //using uniform normal distribution
    return distribution(generator);
}

std::vector<float> MonteCarloSim::runBlackScholesModel(float spotPrice, int32_t timeSteps, float riskRate, float volatility)
{
    static constexpr float  mean = 0.0f, stdDev = 1.0f;  // Mean and standard deviation
    float  deltaT = 1.0f / timeSteps;                                              // time between 2-consequitive frames
    std::unique_ptr<float[]> normRand = std::make_unique<float[]>(timeSteps - 1); // Array of normally distributed random nos.
     stockPrice.resize(timeSteps);                                     // Array of stock price at diff. times
    stockPrice[0] = spotPrice;                                                    // Stock price at t=0 is spot price

    // Populate array with random nos.
    for (int32_t i = 0; i < timeSteps - 1; i++)
        normRand[i] = genRand(mean, stdDev);

    // Apply Black Scholes equation to calculate stock price at next timestep
    for (int32_t i = 0; i < timeSteps - 1; i++)
        stockPrice[i + 1] = stockPrice[i] * exp(((riskRate - (std::powf(volatility, 2.0f) / 2.0f)) * deltaT) + (volatility * normRand[i] * std::sqrtf(deltaT)));

    return stockPrice;
}
