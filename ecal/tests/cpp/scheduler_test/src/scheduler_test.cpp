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
#include <any>

#include <rarecpp/reflect.h>

void printCurrentTime() {
    // Get the current time point from the high-resolution clock
    auto now = std::chrono::high_resolution_clock::now();
    
    // Get the time since the epoch in milliseconds
    auto duration = now.time_since_epoch();
    auto millis = std::chrono::duration_cast<std::chrono::milliseconds>(duration).count();

    // Calculate the components of the current time
    auto seconds = millis / 1000;
    auto minutes = seconds / 60;

    // Extract the relevant parts
    millis = millis % 1000;
    seconds = seconds % 60;
    minutes = minutes % 60;

    // Print the current time with minute, second, and millisecond
    std::cout << std::setfill('0') << std::setw(2) << minutes << ":"
              << std::setfill('0') << std::setw(2) << seconds << ":"
              << std::setfill('0') << std::setw(3) << millis << std::endl;
}

class IOInterface
{
  public:
    virtual size_t SendOutputs() = 0; 
    virtual void   CreateSubPub() = 0;

    virtual std::any& GetInputs() = 0;
    virtual std::any& GetOutputs() = 0;    
};

struct InputA;
struct OutputA;

template <class INPUT, class OUTPUT>
class InputOutputHandler : public IOInterface
{
  public:
    InputOutputHandler() {
      m_inputs = INPUT{};
      m_outputs = OUTPUT{};
    };

    void CreateSubPub() override {
      // Initialize input subscribers
      RareTs::Members<INPUT>::forEach(std::any_cast<INPUT&>(m_inputs), [&, this](auto member, auto& value) {
        m_input_subscriber.emplace_back(eCAL::CSubscriber(member.name));
            m_input_subscriber.back().AddReceiveCallback([&value](const char* , const struct eCAL::SReceiveCallbackData* data_){            
                memcpy(&value, data_->buf, data_->size);                            
            });
        });

      // Initialize output publishers
      RareTs::Members<OUTPUT>::forEach(std::any_cast<OUTPUT&>(m_outputs), [&](auto member, auto& value) {
          m_output_publisher.emplace_back(eCAL::CPublisher(member.name));
          auto id = m_output_publisher.size() - 1;
          m_send_functions.emplace_back([&, id](){
            std::string buf;
            buf.resize(sizeof(value));
            memcpy(buf.data(), &value, sizeof(value));            
            m_output_publisher[id].Send(buf);            
          });
        });
    };

    std::any& GetInputs() override { return m_inputs; }
    std::any& GetOutputs() override { return m_outputs; }
    
    size_t SendOutputs() override {       
      size_t all_send_bytes{0};
      for(auto& pub : m_send_functions)
      {
        pub();        
      }
      return all_send_bytes;
    };

  private:
    std::any m_inputs;
    std::any m_outputs;
    unsigned int send_counter{0};

    std::vector<eCAL::CSubscriber>     m_input_subscriber;
    std::vector<eCAL::CPublisher>      m_output_publisher;
    std::vector<std::function<void()>> m_send_functions;
};

struct InputA
{
  int   number_1;
  int   number_2;
};

struct OutputA
{
  int out_a;
};

void runA(InputA& inputs_, OutputA& outputs_)
{     
    outputs_.out_a = inputs_.number_1 + inputs_.number_2;
};

struct InputB
{
  int number_3;
};

struct OutputB
{
  int out_b;
};

void runB(InputB& inputs_, OutputB& outputs_)
{  
  outputs_.out_b = inputs_.number_3 % 10;
};

struct InputC
{
  int out_a;
  int out_b;
};

struct OutputC
{
  int out_c;
};


void runC(InputC& inputs_, OutputC& outputs_)
{
    outputs_.out_c = inputs_.out_a + inputs_.out_b;

    if (inputs_.out_b > 0 && inputs_.out_a % inputs_.out_b == 0)
    {
      std::cout << std::to_string(inputs_.out_a) << " is divisible by " << std::to_string(inputs_.out_b) << "\n";
    }
    else{
      std::cout << std::to_string(inputs_.out_a) << " is NOT divisible by " << std::to_string(inputs_.out_b) << "\n";
    }
};

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
                    task();                }
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

void high_precision_sleep(std::chrono::microseconds sleep_duration) {
    if (sleep_duration.count() > 200) {
        auto start_time = std::chrono::high_resolution_clock::now();
        auto end_time = start_time + sleep_duration;
        
        while (std::chrono::high_resolution_clock::now() < end_time - std::chrono::microseconds(200)) {
            std::this_thread::yield(); // Yield to other threads
        }

        while (std::chrono::high_resolution_clock::now() < end_time) {
            // Busy-wait
        }
    } else {
        auto start_time = std::chrono::high_resolution_clock::now();
        auto end_time = start_time + sleep_duration;

        while (std::chrono::high_resolution_clock::now() < end_time) {
            // Busy-wait
        }
    }
}

using AlgoRunMap = std::multimap<unsigned int, std::function<void()>>;

class Scheduler
{
  public:
    Scheduler() : m_thread_pool(std::thread::hardware_concurrency()) {}

    template<class INPUT, class OUTPUT>
    void AddAlgo(unsigned int ms_, const std::function<void(INPUT&, OUTPUT&)>& run_function_) {      
      auto io_handler = std::make_shared<InputOutputHandler<INPUT, OUTPUT>>();
      m_io_vec.emplace_back(io_handler);
      auto handler_id = m_io_vec.size() - 1;
      
      auto& inputs_ref = std::any_cast<INPUT&>(io_handler->GetInputs());
      auto& outputs_ref = std::any_cast<OUTPUT&>(io_handler->GetOutputs());

      // Create a lambda function that captures references to inputs_ref and outputs_ref
      auto algo_func = [&, run_function_, handler_id]() {
          run_function_(inputs_ref, outputs_ref);
          m_io_vec[handler_id]->SendOutputs();
      };
        
      m_algo_map.emplace(ms_, std::move(algo_func));      
    }

    void RunBoyRun() {
      for (auto& io_handler : m_io_vec)
      {
        io_handler->CreateSubPub();
      }
      auto runThread = std::thread(&Scheduler::RunLoop, this);
      runThread.join();
    }

  private:
    void RunLoop() {
      while (m_cycle_count < m_max_runs) {
      m_time.StartTimer();
      for(auto& key_val : m_algo_map) {
        if (m_cycle_count % key_val.first == 0) {
          m_thread_pool.enqueue(
            key_val.second
          );
        }      
      }        
      ++m_cycle_count;
      auto time_diff = m_time.GetTimeDiff();
      
      if (time_diff.count() > 0)
        high_precision_sleep(time_diff); 
    }  
    }
    
    AlgoRunMap   m_algo_map; 
    std::vector<std::shared_ptr<IOInterface>> m_io_vec;   
    TimeCapture  m_time{1U};
    unsigned int m_cycle_count = 0;
    unsigned int m_max_runs = 100000;
    ThreadPool   m_thread_pool;

};

void InputAThreads()
{
  eCAL::CPublisher number_1_pub("number_1");
  eCAL::CPublisher number_2_pub("number_2");

  int counter = 0;
  std::string number_1_buf;
  std::string number_2_buf;

  number_1_buf.resize(sizeof(counter));
  number_2_buf.resize(sizeof(counter));

  while (eCAL::Ok())
  {    
    memcpy(number_1_buf.data(), &counter, sizeof(counter));    
    memcpy(number_2_buf.data(), &++counter, sizeof(counter));

    number_1_pub.Send(number_1_buf);
    number_2_pub.Send(number_2_buf);
    // send every half ms
    std::this_thread::sleep_for(std::chrono::microseconds(500));
  }
}

void InputBThreads()
{
  eCAL::CPublisher number_3_pub("number_3");

  int counter = 0;
  std::string number_3_buf;

  number_3_buf.resize(sizeof(counter));

  while (eCAL::Ok())
  {    
    memcpy(number_3_buf.data(), &++counter, sizeof(counter));    

    number_3_pub.Send(number_3_buf);
    // send every 1 ms
    std::this_thread::sleep_for(std::chrono::microseconds(750));
  }
}

TEST(core_cpp_scheduler, scheduler_test)
{
  eCAL::Initialize();
  eCAL::Util::EnableLoopback(true);
  
  std::thread send_thread_a(&InputAThreads);
  std::thread send_thread_b(&InputBThreads);

  Scheduler scheduler;

  scheduler.AddAlgo<InputA, OutputA>(33U, runA);
  scheduler.AddAlgo<InputB, OutputB>(66U, runB);
  scheduler.AddAlgo<InputC, OutputC>(100U, runC);
  scheduler.RunBoyRun();

  eCAL::Finalize();

  if (send_thread_a.joinable())
    send_thread_a.join();
  if (send_thread_b.joinable())
    send_thread_b.join();
  std::cout << "Test ended." << std::endl;
}