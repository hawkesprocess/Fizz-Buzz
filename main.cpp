#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <unistd.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sched.h>
#include <x86intrin.h>

static inline uint32_t remainderDiv3(uint32_t number) {
    uint32_t quotient = (uint32_t)(((uint64_t)number * 0xAAAAAAABULL) >> 33);
    return number - quotient * 3;
}

static inline uint32_t remainderDiv5(uint32_t number) {
    uint32_t quotient = (uint32_t)(((uint64_t)number * 0xCCCCCCCDULL) >> 34);
    return number - quotient * 5;
}

static inline char* convertUint32ToString(uint32_t number, char* buffer) {
    char reversedDigits[11];
    int digitCount = 0;
    static const char digitTable[201] =
        "00010203040506070809"
        "10111213141516171819"
        "20212223242526272829"
        "30313233343536373839"
        "40414243444546474849"
        "50515253545556575859"
        "60616263646566676869"
        "70717273747576777879"
        "80818283848586878889"
        "90919293949596979899";

    if (number == 0) {
        buffer[0] = '0';
        return buffer + 1;
    }
    while (number >= 100) {
        unsigned twoDigits = number % 100;
        number /= 100;
        reversedDigits[digitCount++] = digitTable[twoDigits * 2 + 1];
        reversedDigits[digitCount++] = digitTable[twoDigits * 2];
    }
    if (number < 10)
        reversedDigits[digitCount++] = char('0' + number);
    else {
        unsigned twoDigits = number;
        reversedDigits[digitCount++] = digitTable[twoDigits * 2 + 1];
        reversedDigits[digitCount++] = digitTable[twoDigits * 2];
    }

    for (int i = 0; i < digitCount; i++) {
        buffer[i] = reversedDigits[digitCount - i - 1];
    }
    return buffer + digitCount;
}

int main() {

    struct sched_param fizzBuzzSchedParam;
    fizzBuzzSchedParam.sched_priority = 99;
    sched_setscheduler(0, SCHED_RR, &fizzBuzzSchedParam);

    struct stat stdinStats;
    fstat(STDIN_FILENO, &stdinStats);
    uint32_t* fizzBuzzNumbers = (uint32_t*)mmap(nullptr, stdinStats.st_size, PROT_READ, MAP_PRIVATE, STDIN_FILENO, 0);
    size_t totalNumbers = stdinStats.st_size / sizeof(uint32_t);

    size_t maxOutputSize = totalNumbers * 11;
    ftruncate(STDOUT_FILENO, maxOutputSize);
    char* outputBuffer = (char*)mmap(nullptr, maxOutputSize, PROT_WRITE, MAP_SHARED, STDOUT_FILENO, 0);
    size_t outputOffset = 0;

    for (size_t index = 0; index < totalNumbers; index++) {
        uint32_t currentNumber = fizzBuzzNumbers[index];
        uint32_t mod3 = remainderDiv3(currentNumber);
        uint32_t mod5 = remainderDiv5(currentNumber);

        if (mod3 == 0 && mod5 == 0) {
            memcpy(outputBuffer + outputOffset, "FizzBuzz\n", 9);
            outputOffset += 9;
        } else if (mod3 == 0) {
            memcpy(outputBuffer + outputOffset, "Fizz\n", 5);
            outputOffset += 5;
        } else if (mod5 == 0) {
            memcpy(outputBuffer + outputOffset, "Buzz\n", 5);
            outputOffset += 5;
        } else {
            char numberString[12];
            char* endOfString = convertUint32ToString(currentNumber, numberString);
            *endOfString++ = '\n';
            size_t stringLength = endOfString - numberString;
            memcpy(outputBuffer + outputOffset, numberString, stringLength);
            outputOffset += stringLength;
        }
    }

    ftruncate(STDOUT_FILENO, outputOffset);

    munmap(fizzBuzzNumbers, stdinStats.st_size);
    munmap(outputBuffer, maxOutputSize);

    return 0;
}
