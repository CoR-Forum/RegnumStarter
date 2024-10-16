#include <thread>
#include <chrono>
#include "chrono.h"

void UncheckFakelagAfterDelay(bool& optionFakelag) {
    std::this_thread::sleep_for(std::chrono::seconds(2));
    optionFakelag = false;
}