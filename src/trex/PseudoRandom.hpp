#pragma once

#include <memory>
#include <vector>
#include <cstdlib>

namespace trex
{
    template <typename T>
    class PseudoRandom
    {
    public:
        typedef std::shared_ptr<PseudoRandom<T>> Ptr;
        static
        std::shared_ptr<PseudoRandom>
        create(std::vector<T> samples)
        {
            return std::shared_ptr<PseudoRandom>(new PseudoRandom(samples));
        }

        T&
        next()
        {
            T* choice = random();

            while (choice == _lastChoice)
                choice = random();

            _lastChoice = choice;

            return *choice;
        }
    private:
        PseudoRandom(const std::vector<T>& samples) :
            _samples(samples),
            _lastChoice(&_samples[0])
        {
        }

        T*
        random()
        {
            return &_samples[std::rand() % _samples.size()];
        }

    private:
        std::vector<T> _samples;
        T* _lastChoice;
    };
}