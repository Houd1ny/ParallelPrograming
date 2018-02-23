#include <iostream>
#include <thread>
#include <random>
#include <vector>
#include <algorithm>
#include <cmath>  
#include <chrono>

#include "ThreadPool.h"


using namespace std;
using namespace std::chrono;


void printArr(std::vector<int>& arr)
{
	for (int& item: arr)
    {
    	cout << item << " ";
    }
    cout << endl;
}


void parallelPrefixSum(std::vector<int>& arr)
{
	size_t size = arr.size();
	unsigned int n = std::thread::hardware_concurrency();
    ThreadPool pool(n);
    std::vector< std::future<void> > results;

    size_t depth = log2 (size);
    for(size_t d = 0; d < depth; ++d) {

    	size_t works = std::pow (2, depth - d - 1);

    	for(size_t k = 0; k < works; ++k)
    	{
    		results.emplace_back(
	            pool.enqueue(
            	[d, k, &arr] 
	            {
	            	int arraySize = std::pow (2, d + 1);
	            	int arrayStart = k*arraySize; 	
	            	arr[arrayStart + arraySize - 1] += arr[arrayStart + arraySize/2 - 1];
	                return;
	            })
        	);
    	}
    	for(auto && result: results)
    		result.get();
    	results.clear();
    }
    
    arr[size-1] = 0;

    for(int d = depth-1; d >= 0; --d) {

    	int works = std::pow (2, depth - d - 1);

    	for(int k = 0; k < works; ++k)
    	{
    		results.emplace_back(
	            pool.enqueue(
            	[d, k, &arr] 
	            {
	            	int arraySize = std::pow (2, d + 1);
	            	int arrayStart = k*arraySize;
	            	int temp =  arr[arrayStart + arraySize - 1];
	            	arr[arrayStart + arraySize - 1] += arr[arrayStart + arraySize/2 - 1];
	            	arr[arrayStart + arraySize/2 - 1] = temp;
	                return;
	            })
        	);
    	}
    	for(auto && result: results)
    		result.get();
    	results.clear();
    }
    
}


void normalPrefixSum(std::vector<int>& arr)
{
	for (size_t i = 1; i < arr.size(); ++i)
	{
		arr[i] += arr[i-1];
	}
}

int main() 
{
    unsigned int n = std::thread::hardware_concurrency();
    std::cout << n << " concurrent threads are supported.\n";

    std::random_device rd; 
    std::mt19937 gen(rd()); 
    std::uniform_int_distribution<> dis(1, 1000);


    size_t size = std::pow (2, 20);
    std::vector<int> arr(size);
    std::generate(arr.begin(), arr.end(), [&](){return 1/*dis(gen)*/;});

    std::vector<int> copy1(arr.begin(), arr.end());

    //printArr(copy1);
    high_resolution_clock::time_point start1 = high_resolution_clock::now();
    parallelPrefixSum(copy1);
    high_resolution_clock::time_point end1 = high_resolution_clock::now();
    //printArr(copy1);
    copy1.clear();
    cout << "parallelPrefixSum execution time " << duration_cast<microseconds>( end1 - start1 ).count() << endl;
    
    std::vector<int> copy2(arr.begin(), arr.end());
    //printArr(copy2);
    high_resolution_clock::time_point start2 = high_resolution_clock::now();
    normalPrefixSum(copy2);
    high_resolution_clock::time_point end2 = high_resolution_clock::now();
    //printArr(copy2);
    cout << "normalPrefixSum execution time " << duration_cast<microseconds>( end2 - start2 ).count() << endl;

    std::cout.flush();
}