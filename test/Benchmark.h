#ifndef REACTIVE_PROPERTYOBSERVER_BENCHMARK_H_H
#define REACTIVE_PROPERTYOBSERVER_BENCHMARK_H_H

#include <vector>
#include <algorithm>
#include <numeric>
#include <chrono>
#include <string>
#include <thread>


#include <reactive/nonblocking/ObservableProperty.h>

struct Benchmark{
    const int count = 100'000;

    template<class T, class R>
    struct DataCopyableOnly {
        T x1, x2, x3, x4;
        R sum;

		DataCopyableOnly(){
            // test small update on 4 observables
			reactive::nonblocking::observe([&](const auto& x1, const auto& x2, const auto& x3, const auto& x4) {
            //reactive::copyable_only::observe([&](const std::string& x1, const std::string& x2, const std::string& x3, const std::string& x4){
                //std::this_thread::sleep_for(std::chrono::milliseconds(1));
                sum.set( x1+x2+x3+x4 );
            }, x1, x2, x3, x4);
        }

        template<class I1, class I2, class I3, class I4>
        void update(I1&& x1, I2&& x2, I3&& x3, I4&& x4) {
            this->x1.set(x1);
            this->x2.set(x2);
            this->x3.set(x3);
            this->x4.set(x4);
        }

        auto getSum() const{
            sum.getCopy();
        }
    };

    template<class Container>
    void benchmark_fill(Container& container) {
        using namespace std::chrono;
        high_resolution_clock::time_point t1 = high_resolution_clock::now();

        container.reserve(count);
        for (int i = 0; i < count; i++) {
            container.emplace_back();
        }
        /*for (auto&& p : container) {
            p.update(100,20,40,600);
        }*/


        high_resolution_clock::time_point t2 = high_resolution_clock::now();
        auto duration = duration_cast<milliseconds>(t2 - t1).count();
        std::cout << "filled in : " << duration << std::endl;
    }

    template<class Container>
    void benchmark_update(Container& container, int threads_num = 1) {
        using namespace std::chrono;
        high_resolution_clock::time_point t1 = high_resolution_clock::now();

		std::vector<long long> sums;
		for (int t = 0; t < threads_num; t++) {
			sums.emplace_back(0);
		}

        std::vector<std::thread> threads;
        for(int t = 0; t<threads_num;t++){
			//sums.emplace_back(0);
            threads.emplace_back([&, t](){
                for (int i = 0; i < count; i++) {
					
                    //container[i].update(""+std::to_string(i), ""+std::to_string(i+1), ""+std::to_string(i+2), ""+std::to_string(i+3));
                    //container[i].update("AAAAAAAAAAAA", "VBVVVVVVVVVVV", "CCCCCCCCCCCCC", "DDDDDDDDDDDDDDDDDDDDDDDDDDD");
                    container[i].update(i, i + 1, i + 2, i + 3);

					sums[t] +=  container[i].sum.getCopy();
                    //sum += container[i].sum.getCopy();
                }
            });
        }

		for(auto& t : threads){
            t.join();
        }


        high_resolution_clock::time_point t2 = high_resolution_clock::now();
        auto duration = duration_cast<milliseconds>(t2 - t1).count();

		long long sum = std::accumulate(sums.begin(), sums.end(), 0);

        std::cout << "updated in : " << duration
                  << " (" << sum << ")"
                  << std::endl;
    }


    void benchmark_all(){
		const int threads_count = 1;

        //using T = std::string;
		using T = long long;
        /*{
			std::cout << "reactive::Property" << std::endl;
			using Element = Data<reactive::Property<T>, reactive::Property<T> >;
			std::vector<Element> list;
			benchmark_fill(list);
			benchmark_update(list, threads_count);
			benchmark_update(list, threads_count);
			benchmark_update(list, threads_count);
			benchmark_update(list, threads_count);
			std::cout << "---" << std::endl;
		}*/
		{
            std::cout << "reactive::copyable_only::Property"  << std::endl;
            using Element = DataCopyableOnly<reactive::nonblocking::ObservableProperty<T>, reactive::nonblocking::ObservableProperty<T> >;
            std::vector<Element> list;
            benchmark_fill(list);
            benchmark_update(list, threads_count);
			benchmark_update(list, threads_count);
			benchmark_update(list, threads_count);
			benchmark_update(list, threads_count);
            std::cout << "---"  << std::endl;
        }
    }
};

#endif //REACTIVE_PROPERTYOBSERVER_BENCHMARK_H_H
