#include <iostream>
#include <random>
#include <algorithm>
#include <vector>
#include <functional>
#include <map>
#include <thread>
#include <future>

using namespace std;


template <typename DataT, typename Key, typename Value>
class MapReduce
{
public:
    using ResT = vector<std::pair<Key, Value> >;
    using DataTIter = typename vector<DataT>::const_iterator;
    using MapperT = function<ResT(DataTIter, DataTIter)>;
    using ReducerT = function<ResT(const ResT&)>;
public:
    MapReduce(const vector<DataT>& data,
              MapperT map,
              ReducerT reducer) :
        _data(data),
        _map(map),
        _reducer(reducer)
    {
    }
    ResT run()
    {
        //map
        unsigned int number_of_threads = thread::hardware_concurrency();
        unsigned int part_size = _data.size()/ number_of_threads;

        std::vector< future<ResT> > mapResults;
        for (int i = 0; i < number_of_threads; ++i)
        {
            auto partStart = _data.begin() + i*part_size;
            auto partEnd = partStart + part_size;
            mapResults.push_back(
                        async(std::launch::async, _map, partStart, partEnd));
        }
        std::vector<ResT> shuffledArray(number_of_threads);
        //shuffle
        for (int i = 0; i < mapResults.size(); ++i)
        {
            ResT mapRes = mapResults[i].get();
            for (auto& intermediate_value : mapRes)
            {
                int targedReducer = std::hash<Key>{}(intermediate_value.first) % number_of_threads;
                shuffledArray[targedReducer].push_back(intermediate_value);
            }
        }
        //reducer
        std::vector< future<ResT> > reducerResults;
        for (int i = 0; i < number_of_threads; ++i)
        {
            reducerResults.push_back(
                        async(std::launch::async, _reducer, cref(shuffledArray[i])));
        }
        //combining results
        ResT finalSolution;
        for (int i = 0; i < number_of_threads; ++i)
        {
            ResT reducerRes = reducerResults[i].get();
            finalSolution.insert( finalSolution.end(),
                                  reducerRes.begin(),
                                  reducerRes.end() );
        }
        return finalSolution;
    }
private:
    const vector<DataT>& _data;
    MapperT _map;
    ReducerT _reducer;
};

using IntCountMapReduce = MapReduce<int,int,int>;

IntCountMapReduce::ResT mapper(IntCountMapReduce::DataTIter begin,
                               IntCountMapReduce::DataTIter end)
{
    map<int, int> counter;
    for (auto it = begin; it != end; ++it)
    {
        counter[*it] += 1;
    }
    return IntCountMapReduce::ResT(counter.begin(), counter.end());
}

IntCountMapReduce::ResT reducer(const IntCountMapReduce::ResT& intermediate_values)
{
    map<int, int> counter;
    for(auto& intermediate_value: intermediate_values)
    {
        counter[intermediate_value.first] += intermediate_value.second;
    }
    return IntCountMapReduce::ResT(counter.begin(), counter.end());
}

int main()
{
    random_device rd;
    mt19937 gen(rd());
    uniform_int_distribution<> dis(0, 9);

    size_t size = std::pow (2, 10);
    vector<int> arr(size);
    generate(arr.begin(), arr.end(), [&](){return dis(gen);});

    IntCountMapReduce mapReduce(arr, mapper, reducer);
    IntCountMapReduce::ResT result = mapReduce.run();

    cout << "parallel result" << endl;
    for (auto item : result)
    {
        cout << item.first << " " << item.second << endl;
    }

    cout << "expected result" << endl;
    IntCountMapReduce::ResT checkResr = mapper(arr.begin(), arr.end());

    for (auto item : checkResr)
    {
        cout << item.first << " " << item.second << endl;
    }
    return 0;
}

