#ifndef REACTIVE_V3_OBSERVABLEPROPERTY_H
#define REACTIVE_V3_OBSERVABLEPROPERTY_H

#include <memory>
#include <vector>
#include <algorithm>
#include <mutex>
#include <functional>
#include <shared_mutex>

#include "../DelegateTag.h"
#include "../helpers.h"
#include "../threading/SpinLock.h"
#include "../threading/RWSpinLock.h"
#include "../threading/dummy_mutex.h"


namespace reactive{
    namespace nonblocking{

        template<class T, bool m_multi_threaded = true>
        class ObservableProperty {
        public:
            const constexpr static bool multi_threaded = m_multi_threaded;

        private:
            using Self = ObservableProperty<T, m_multi_threaded>;

            class Unsubscriber{
                Self* self;
                const DelegateTag& tag;
            public:
                Unsubscriber(Self* self, const DelegateTag& tag)
                    :self(self)
                    ,tag(tag)
                {}
                Unsubscriber(const Unsubscriber&) = delete;
                Unsubscriber(Unsubscriber&&) = default;

                void operator()(){
                    self->unsubscribe(tag);
                }
            };

            using Observer = std::function<void(Unsubscriber, const T&)>;
            struct Element{
                Observer observer;
                DelegateTag tag;

                Element(Observer&& observer, const DelegateTag& tag)
                    :observer(std::move(observer))
                    ,tag(tag)
                {}

                Element(const Observer& observer, const DelegateTag& tag)
                    :observer(observer)
                    ,tag(tag)
                {}
            };

            using Lock          = std::conditional_t<multi_threaded, threading::SpinLock<>, threading::dummy_mutex> ;
            using ObserversLock = std::conditional_t<multi_threaded, threading::RWSpinLockWriterBiased<>, threading::dummy_mutex>;

            /// data
            std::vector<Element> observers;
            std::vector<Element> defferedActions;

            T value;

            ObserversLock observersLock;
            mutable Lock lock;




            void applyDefferedActions(){
                if (defferedActions.empty()) return;
                std::unique_lock<ObserversLock> l(observersLock);

                for(Element& element : defferedActions){
                    if (!element.observer){
                        // remove
                        auto it = std::find_if(observers.begin(), observers.end(), [&](Element& e) {
                            return (element.tag == e.tag);
                        });

                        if (it != observers.end()) {
                            std::iter_swap(it, observers.end() - 1);
                            observers.pop_back();
                        }
                    } else {
                        // add
                        observers.emplace_back(std::move(element));
                    }
                }
                defferedActions.clear();
            }


        public:
            void pulse(const T& value){
                std::shared_lock<ObserversLock> l(observersLock);

                for(Element& element: observers){
                    element.observer(Unsubscriber{this, element.tag}, value);
                }
            }

            using Value = T;

            ObservableProperty(){}

            // thread-safe move ctr
            ObservableProperty(ObservableProperty&& other)
            {
                std::unique_lock<Lock> l1(other.lock);
                std::unique_lock<ObserversLock> l2(other.observersLock);

                value     = std::move(other.value);
                observers = std::move(other.observers);
            }

            void set(const T& new_value){
                {
                    std::unique_lock<Lock> l(lock);
                    value = new_value;

                    applyDefferedActions();

                    if (observers.empty()) return;
                }

                pulse(new_value);
            }

            T getCopy() const {
                std::unique_lock<Lock> l(lock);
                return value;
            }

            template<class Closure>
            DelegateTag subscribe(Closure&& closure){
                return subscribe_w_unsubscribe(
                    [closure = std::forward<Closure>(closure)](auto&&, const T& value){
                        closure(value);
                    }
                );
            }

            template<class Closure>
            DelegateTag subscribe_w_unsubscribe(Closure&& closure){
                std::unique_lock<Lock> l(lock);

                DelegateTag delegateTag;
                defferedActions.emplace_back(std::forward<Closure>(closure), delegateTag);

                return delegateTag;
            }


            void unsubscribe(const DelegateTag& tag){
                std::unique_lock<Lock> l(lock);
                defferedActions.emplace_back(nullptr, tag);
            }
        };


        template<bool multithreaded, class Closure, class ...Values>
        class Observer{
            Closure closure;
            std::tuple<Values...> values;


            using Lock = std::conditional_t<multithreaded, threading::SpinLock<>, threading::dummy_mutex>;
            Lock lock;

            std::atomic<bool> unsubscribe_scheduled{ false };
        public:
			template<class ClosureT>
            Observer(ClosureT&& closure, const Values&... values)
                    : closure(std::forward<ClosureT>(closure))
                    , values(values...)
            {}

            template<int I, class Unsubscription, class Value>
            void run_w_unsubscribe(Unsubscription& unsubscription, const Value& value){
                if (unsubscribe_scheduled) {
                    unsubscription();
                    return;
                }

                lock.lock();
                    std::get<I>(values) = value;
                    const auto tmp_values = values;     // allow to call multiple observers simuteniously
                lock.unlock();

				reactive::utils::apply([&](const auto& ...args) {
					closure([&]() { unsubscription(); unsubscribe_scheduled = true; }, args...);
				}, std::move(tmp_values));
            }

            void unsubscribe(){
                unsubscribe_scheduled = true;
            }
        };



		template<class Closure, class ...Observables>
		auto observe_w_unsubscribe(Closure&& closure, Observables&...observables) {
			using namespace reactive::utils;

			constexpr const bool multithreaded = and_all(observables.multi_threaded...);

			using ObserverT = Observer<multithreaded, std::decay_t<Closure>, typename Observables::Value...>;

			auto observer = std::make_shared<ObserverT>(std::forward<Closure>(closure), observables.getCopy()...);

			foreach([&](auto index, auto& observable) {
				using I = decltype(index);
				observable.subscribe_w_unsubscribe([observer](auto&& unsubscribe, const auto& value) {
					constexpr const auto i = I::value;
					observer->template run_w_unsubscribe<i>(unsubscribe, value);
				});
			}, observables...);

			return observer;
		}


        template<class Closure, class ...Observables>
        auto observe(Closure&& closure, Observables&...observables){
            return observe_w_unsubscribe([closure = std::forward<Closure>(closure)](auto&& unsubscribe, const auto& ...values){
                closure(values...);
            }, observables...);
        }

    }
}

#endif //REACTIVE_V3_OBSERVABLEPROPERTY_H
