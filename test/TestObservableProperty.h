#ifndef REACTIVE_V3_TEST_TESTOBSERVABLEPROPERTY_H
#define REACTIVE_V3_TEST_TESTOBSERVABLEPROPERTY_H

#include "reactive/nonblocking/ObservableProperty.h"

struct TestObservableProperty{
    void test_1(){
        using namespace reactive::nonblocking;

        ObservableProperty<int> x;
    }


    void test_unsubscribe(){
        using namespace reactive::nonblocking;

        ObservableProperty<int> x;

        x.subscribe_w_unsubscribe([](auto unsubscribe, int x){
            std::cout << "*";
            if (x>=100){
                return unsubscribe();
            }

            std::cout << x << std::endl;
        });

        x.set(1);
        x.set(101);
        x.set(102);
    }

    void test_unsubscribe_observe(){
        using namespace reactive::nonblocking;

        ObservableProperty<int> x;
        ObservableProperty<int> y;
        x.set(0);
        y.set(0);

        auto observer = observe_w_unsubscribe([](auto unsubscribe, int x, int y){
            std::cout << "*";
            if (x>=100){
                return unsubscribe();
            }

            std::cout << x << ":" << y << std::endl;
        }, x, y);
        //observer->unsubscribe();


        x.set(1);
        y.set(20);
        x.set(101);
        x.set(102);
        y.set(2);
    }

    void test_all(){
        //test_1();
        //test_unsubscribe();
        test_unsubscribe_observe();
    }
};

#endif //REACTIVE_V3_TEST_TESTOBSERVABLEPROPERTY_H
