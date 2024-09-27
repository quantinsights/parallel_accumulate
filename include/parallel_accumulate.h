#ifndef PARALLEL_ACCUMULATE_H
#define PARALLEL_ACCUMULATE_H

#include <numeric>
#include <functional>
#include <iterator>
#include <thread>
#include <vector>

// auto sequential_fold = []<typename T, typename Iterator, typename Container>(Iterator first, Iterator last, T init, T& result)
// {
//     result = std::accumulate(first, last, init);
// };

auto sequential_fold = []<typename T_elem, typename T_accum, typename Iterator, typename Container>(Iterator first, Iterator last, T_accum init, std::function<T_accum(T_elem, T_accum)> func, T_accum& result)
{
    result = std::accumulate(first, last, init, func);
};

template <typename T_elem, typename T_accum, typename Iterator, typename Container>
T_accum parallel_accumulate(Iterator first, Iterator last, T_elem init, std::function<T_accum(T_elem, T_accum)> func)
{
    unsigned long const length = std::distance(first, last);
    if (!length)
        return init;
    unsigned const int min_block_size = 25;

    // This bit is a clever piece of code. To calculate the maximum number of threads that should be spawned
    // you can divide the total number of elements (length) / by the minimum block size. If the container
    // has #elements in the interval [1,min_block_size), then atleast 1 thread will be spawned. 
    unsigned long const max_threads = static_cast<unsigned long const>((length + min_block_size - 1)/min_block_size);

    //The actual number of threads to run is the minimum of your calculated maximum and the 
    //number of hardware threads. You don't want to run more threads than the hardware can support(oversubscription),
    //because context switching will mean that more threads will decrease the performance. 
    
    // If the call to std::threads::hardware_concurrency() returned 0, you'd substitute a number of
    // your choice; in this case, I have chosen 2. You don't want to run too many threads, 
    // because that would slow things down on a single-core machine, but likewise you don't want
    // to run too few threads, you'd be passing up on available concurrency.
    unsigned long const hardware_threads = std::thread::hardware_concurrency();
    unsigned long const num_threads = std::min(hardware_threads == 0 ? 2 : hardware_threads, max_threads);
    unsigned long const block_size = static_cast<unsigned long const>(length / num_threads);

    // Now, that we know how many threads we have, we can create a vector<T_accum> object for the intermediate
    // results and a std::vector<std::thread> for the threads. 
    std::vector<T_accum> results(num_threads);
    std::vector<std::thread> threads(num_threads);
    Iterator it = first;
    for(int i{0};i<num_threads; ++i)
    {
        threads[i] = std::thread(sequential_fold, it, it + block_size, func, std::ref(results[i]));
        it += block_size;
    }

    for(auto& t : threads)
        t.join();
    
    return std::accumulate(results.begin(), results.end(), init, func);
}

#endif