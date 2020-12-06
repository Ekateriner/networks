#include <iostream>
#include <vector>
#include <thread>
#include <mutex>
#include <string>
#include <chrono>
#include <ctime>

std::mutex mtx_wr;
std::mutex mtx_out;
std::vector<char> cable;

typedef std::chrono::time_point<std::chrono::system_clock, std::chrono::nanoseconds> Time;
typedef std::chrono::milliseconds milisec;
typedef std::chrono::nanoseconds nanosec;
typedef std::chrono::duration<float> duration;

std::vector<char> make_frame(char* destination, char* source, std::vector<char> &data) {
    std::vector<char> frame;
    std::vector<char> check_sum(4, 0);

    //preamble
    for (int i = 0; i < 7; i++) {
        frame.push_back(char(0x10101010));
        check_sum[i % 4] ^= char(0x10101010);
    }
    // Start frame detector
    frame.push_back(char(0x10101011));
    check_sum[3] ^= char(0x10101011);

    //MACs
    for (int i = 0; i < 6; i++) {
        frame.push_back(destination[i]);
        check_sum[i % 4] ^= destination[i];
    }

    for (int i = 0; i < 6; i++) {
        frame.push_back(source[i]);
        check_sum[(i + 2) % 4] ^= source[i];
    }

    // len

    frame.push_back(char(data.size() / 256));
    frame.push_back(char(data.size() % 256));
    check_sum[0] ^= char(data.size() / 256);
    check_sum[1] ^= char(data.size() % 256);

    //data
    for (int i = 0; i < data.size(); i++) {
        frame.push_back(data[i]);
        check_sum[(i + 2) % 4] ^= data[i];
    }

    // check sum
    for (int i = 0; i < 4; i++)
        frame.push_back(check_sum[i]);

    return frame;
}

void thread_flow(int num, Time start){
    char error = char(0b11110000);
    char destination[7] = "master";

    char source[6] = "thr ";
    source[4] = char(num);
    source[5] = '\0';

    std::string str = "Some random data from this thread";
    std::vector<char> data(str.begin(), str.end());

    for (size_t i = 0; i < data.size(); i += 6) {
        std::vector<char> part_data(data.begin() + i, data.begin() + std::min(i + 6, data.size()));
        std::vector<char> frame = make_frame(destination, source, part_data);

        bool success = false;
        while (!success) {
            success = true;
            for (size_t j = 0; j < frame.size(); j++) {
                char last;
                if (mtx_wr.try_lock()){
                    cable.push_back(frame[i]);
                    mtx_wr.unlock();
                }
                else {
                    success = false;
                    break;
                }
                if (mtx_wr.try_lock()) {
                    last = cable.back();
                    mtx_wr.unlock();
                }
                else {
                    success = false;
                    break;
                }
                if (last != frame[i]) {
                    success = false;
                    break;
                }
            }
            if (!success) {
                bool wait = false;

                int counter = 0;
                while (!wait) {
                    counter += 1;
                    mtx_wr.lock();
                    cable.push_back(error);
                    mtx_wr.unlock();

                    mtx_wr.lock();
                    char last = cable.back();
                    mtx_wr.unlock();
                    if (last == error) {
                        wait = true;
                    }
                }

                std::srand(std::time(nullptr));
                int random = std::rand() % counter + 1;
                std::this_thread::sleep_for(std::chrono::nanoseconds(int(51200 * random)));
            }
            else {
                Time current = std::chrono::system_clock::now();

                mtx_out.lock();
                duration dur = (current - start);
                std::cout << "Success part " << i / 6 << " in thread " << num << " at " << std::chrono::duration_cast<nanosec>(dur).count() / 51200 << std::endl;
                mtx_out.unlock();
            }
        }
        std::this_thread::sleep_for(std::chrono::nanoseconds(51200));
    }
}

int main() {
    int N;
    std::cin >> N;
    std::vector<std::thread> threads(N);

    Time start = std::chrono::system_clock::now();
    for (int i = 0; i < N; i++) {
        threads[i] = std::thread(thread_flow, i, start);
    }

    for (int i = 0; i < N; i++) {
        threads[i].join();
    }

    Time end = std::chrono::system_clock::now();

    std::chrono::duration<double> elapsed_seconds = end-start;

    std::cout << "All work takes: " << std::chrono::duration_cast<milisec>(elapsed_seconds).count() << "ms";

    return 0;
}
