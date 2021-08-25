#include <iomanip>
#include <iostream>
#include <sstream>
#include "tests/test.h"

#include "timewheel/timewheel.h"

static std::string now_time_string()
{
    auto now = std::chrono::system_clock::now().time_since_epoch();
    auto milli = std::chrono::duration_cast<std::chrono::milliseconds>(now).count();
    time_t t = milli / 1000;
    std::stringstream s;
    s << std::put_time(std::localtime(&t), "%Y-%m-%d %H:%M:%S") << '.' << milli % 1000;
    return s.str();
}

static void cycle_timer_function()
{
    std::cout << now_time_string() << ", cycle timer: 1s" << std::endl;
}

void Test_TimeWheel()
{
    try
    {
        auto tp = std::make_shared<ThreadPool>(1);
        TimeWheel tw(tp);

        std::cout << now_time_string() << " start timer" << std::endl;

        // timer once
        int64_t start = TimeWheel::now_milli();
        tw.Add(100, [start] {
            std::cout << now_time_string()
                << " timer once: 100ms, fact: " << TimeWheel::now_milli() - start
                << "ms" << std::endl;
        });

        start = TimeWheel::now_milli();
        tw.Add(1000, [start] {
            std::cout << now_time_string()
                << " timer once: 1s, fact:" << TimeWheel::now_milli() - start
                << "ms" << std::endl;
        });

        start = TimeWheel::now_milli();
        tw.Add(5000, [start] {
            std::cout << now_time_string()
                << " timer once: 5s, fact:" << TimeWheel::now_milli() - start
                << "ms" << std::endl;
        });

        //cycle timer, execute the callback function immediately and every 1 seconds
        int64_t timerid = tw.Add(0, 1000, &cycle_timer_function);

        std::this_thread::sleep_for(std::chrono::seconds(40));

        // erase timer
        tw.Erase(timerid);

        std::this_thread::sleep_for(std::chrono::seconds(2));
        tw.Stop();
        tp->Stop();
    }
    catch (const std::exception& e)
    {
        std::cerr << e.what() << std::endl;
    }
    catch (...)
    {
        std::cerr << "unknown error" << std::endl;
    }
}
