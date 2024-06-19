/* ========================= eCAL LICENSE =================================
 *
 * Copyright (C) 2016 - 2024 Continental Corporation
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 * 
 *      http://www.apache.org/licenses/LICENSE-2.0
 * 
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 * ========================= eCAL LICENSE =================================
*/

#include <ecal/ecal_core.h>
#include <ecal/ecal.h>
#include <ecal/ecal_config.h>

#include <gtest/gtest.h>

#include <stdexcept>
#include <fstream>
#include <string>
#include <cstdio>

#include <thread>
#include <chrono>
#include <iostream>
#include <queue>
#include <condition_variable>
#include <typeinfo>

#include <rarecpp/reflect.h>

struct InputA
{
  int gear;
  float speed;
  std::vector<float> points;
};

struct OutputA
{
  std::string message;
  size_t      message_size;
};

template <typename KeyType, typename ValueType>
class MyMap {
public:
    void insert(const KeyType& key, const ValueType& value) {
        map[key] = value;
    }

    void print() const {
        for (const auto& pair : map) {
            std::cout << pair.first << ": " << pair.second << std::endl;
        }
    }

private:
    std::map<KeyType, ValueType> map;
};

template <class INPUT, class OUTPUT>
class InputOutputHandler
{
  public:
    InputOutputHandler() {
        // Initialize input subscribers
      RareTs::Members<INPUT>::forEach(m_inputs, [&](auto member, auto& value) {
        m_input_subscriber.emplace_back(eCAL::CSubscriber(member.name));
            m_input_subscriber.back().AddReceiveCallback([&value, &member](const char* , const struct eCAL::SReceiveCallbackData* data_){
            std::cout << "Received something on inputSubscriber for membername " << member.name << "\n";

                // Assign received data to the correct member
                if constexpr (std::is_same_v<decltype(value), int&>) {
                    value = std::stoi(std::string(static_cast<const char*>(data_->buf), data_->size));
                } else if constexpr (std::is_same_v<decltype(value), float&>) {
                    value = std::stof(std::string(static_cast<const char*>(data_->buf), data_->size));
                } else if constexpr (std::is_same_v<decltype(value), std::vector<float>&>) {
                    // Deserialize the data buffer into the vector<float>
                    value.clear();
                    std::istringstream iss(std::string(static_cast<const char*>(data_->buf), data_->size));
                    std::copy(std::istream_iterator<float>(iss), std::istream_iterator<float>(), std::back_inserter(value));
                } else {
                    std::cerr << "Unsupported type for member " << member.name << "\n";
                }
            });
        });

        // Initialize output publishers
      RareTs::Members<OUTPUT>::forEach(m_outputs, [&](auto member, auto& value) {
          m_output_publisher.emplace_back(eCAL::CPublisher(member.name));
        });
    }

    INPUT GetInputs() const { return m_inputs; }
    
    size_t SendOutputs() {       
      size_t all_send_bytes{0};
      for(auto& pub : m_output_publisher)
      {
        all_send_bytes = pub.Send("I send something to you!" + std::to_string(send_counter++));        
      }
      return all_send_bytes;
    };

  private:
    INPUT  m_inputs;
    OUTPUT m_outputs;
    unsigned int send_counter{0};

    std::vector<eCAL::CSubscriber> m_input_subscriber;
    std::vector<eCAL::CPublisher>  m_output_publisher;
};

using AlgoRunMap = std::multimap<unsigned int, std::function<void()>>;

struct TimeCapture
{
  TimeCapture(unsigned int ms_) { m_cycle_time = std::chrono::microseconds(ms_ * 1000); }
    
  std::chrono::microseconds GetTimeDiff() { 
        auto elapsed = std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::high_resolution_clock::now() - m_start_time);
        if (m_cycle_time > elapsed) 
            return m_cycle_time - elapsed;
        else 
            return std::chrono::microseconds(0);
    }
    
    void StartTimer() { m_start_time = std::chrono::high_resolution_clock::now(); }

  private:
    std::chrono::high_resolution_clock::time_point  m_start_time;
    std::chrono::microseconds                       m_cycle_time;
};

class ThreadPool {
public:
    ThreadPool(size_t numThreads) : stop(false) {
        for (size_t i = 0; i < numThreads; ++i) {
            workers.emplace_back([this] {
                while (true) {
                    std::function<void()> task;
                    {
                        std::unique_lock<std::mutex> lock(this->queueMutex);
                        this->condition.wait(lock, [this] { return this->stop || !this->tasks.empty(); });
                        if (this->stop && this->tasks.empty())
                            return;
                        task = std::move(this->tasks.front());
                        this->tasks.pop();
                    }
                    task();
                }
            });
        }
    }

    template <class F>
    void enqueue(F&& f) {
        {
            std::unique_lock<std::mutex> lock(queueMutex);
            if (stop)
                throw std::runtime_error("enqueue on stopped ThreadPool");
            tasks.emplace(std::forward<F>(f));
        }
        condition.notify_one();
    }

    ~ThreadPool() {
        {
            std::unique_lock<std::mutex> lock(queueMutex);
            stop = true;
        }
        condition.notify_all();
        for (std::thread& worker : workers)
            worker.join();
    }

private:
    std::vector<std::thread> workers;
    std::queue<std::function<void()>> tasks;
    std::mutex queueMutex;
    std::condition_variable condition;
    bool stop;
};

class Scheduler
{
  public:
    Scheduler() : m_thread_pool(std::thread::hardware_concurrency()) {}

    void AddAlgo(unsigned int ms_, const std::function<void()>& run_function_) {
      m_algo_map.emplace(ms_, run_function_);
    }

    void RunBoyRun() {
      auto runThread = std::thread(&Scheduler::RunLoop, this);
      runThread.join();
    }

  private:
    void RunLoop() {
        while (m_cycle_count < m_max_runs) {
        m_time.StartTimer();
            for(auto& key_val : m_algo_map) {
                if (m_cycle_count % key_val.first == 0) {
            m_thread_pool.enqueue([&key_val] {
              key_val.second();
            });
          }      
        }        
        ++m_cycle_count;
        auto time_diff = m_time.GetTimeDiff();
        // std::cout << "Time Diff: " << time_diff.count() << "\n";
        if (time_diff.count() > 0)
          std::this_thread::sleep_for(time_diff); 
      }  
    }
    
    AlgoRunMap   m_algo_map;
    TimeCapture  m_time{33U}; // 33ms cycle time
    unsigned int m_cycle_count = 0;
    unsigned int m_max_runs = 1000;
    ThreadPool   m_thread_pool;

};

TEST(core_cpp_scheduler, scheduler_test)
{
  eCAL::Initialize();
    Scheduler scheduler;

  InputOutputHandler<InputA, OutputA> myAlgo;
  
    std::function<void()> run = [&] {
        auto inputs = myAlgo.GetInputs();
        std::cout << "InputA gear: " << inputs.gear << "\n";
        std::cout << "InputA speed: " << inputs.speed << "\n";
        for (const auto& point : inputs.points)
      std::cout << "InputA points: " << point << "\n";

        // Fill outputs and send
    myAlgo.SendOutputs();
  };

    scheduler.AddAlgo(33U, run);
    scheduler.RunBoyRun();

  std::cout << "Test ended." << std::endl;
}