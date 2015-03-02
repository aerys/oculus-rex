/*
Copyright (c) 2014 Aerys

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and
associated documentation files (the "Software"), to deal in the Software without restriction,
including without limitation the rights to use, copy, modify, merge, publish, distribute,
sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or
substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING
BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM,
DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

#pragma once

#include "minko/Minko.hpp"

namespace trex
{
    namespace component
    {
        class CarScript;
    }
}

namespace trex
{
    namespace component
    {
        class DinoScript : public minko::component::AbstractScript
        {
        public:
            typedef std::shared_ptr<DinoScript>         Ptr;

        private:
            typedef minko::scene::Node::Ptr                                                         NodePtr;

            typedef minko::audio::SoundChannel::Ptr                                                 SoundChannelPtr;
            typedef minko::component::SceneManager::Ptr                                             SceneManagerPtr;
            typedef minko::component::Animation::Ptr                                                AnimationPtr;
            typedef minko::component::AbstractAnimation::Ptr                                        AbstractAnimationPtr;
            typedef minko::Signal<AbstractAnimationPtr, std::string, minko::uint>::Slot             AnimationLabelHitSlot;

            enum class State
            {
                NONE,
                SPAWNING,
                WALKING,
                FOLLOWING,
                SCREAMING, 
                ACCELERATING,
                ATTACKING,
                RECOVERING,
                AAARGH
            };

        private:
            SceneManagerPtr                             _sceneManager;
            NodePtr                                     _target;
            NodePtr                                     _dinoSymbol;
            float                                       _speed;
            float                                       _requiredSpeed;
            State                                       _currentState;
            float                                       _currentTimeStamp;
            std::shared_ptr<CarScript>                  _car;
            int                                         _lane;
            minko::math::Matrix4x4::Ptr                 _destinationMatrix;
            std::map<std::string, float>                _timers;
            bool                                        _hadSameLaneAsCar;
            AnimationLabelHitSlot                       _dinoFootStepLabelHitSlot;

            static
            std::vector<std::string>                    _eatSamples;

            static
            std::vector<std::string>                    _roarSamples;

            static
            std::vector<std::string>                    _footStepSamples;

            static
            std::vector<std::string>                    _attackSamples;

            static
            std::vector<std::string>                    _rushSamples;

            std::shared_ptr<minko::audio::SoundChannel> _footStep;
            NodePtr                                     _root;
            NodePtr                                     _dinoLaneAnimatedNode;
            NodePtr                                     _dinoSkinnedNode;
            NodePtr                                     _headDummyNode;
            AnimationPtr                                _dinoLaneAnimation;
            AnimationLabelHitSlot                       _dinoLaneAnimationLabelHit;

            std::shared_ptr<minko::audio::SoundChannel> _music;

            bool                                        _wasFollowing; // hack
            bool                                        _isEating;
            bool                                        _gameIsOver;

        public:
            static
            std::map<
                minko::audio::PositionalSound::Ptr,
                minko::Signal<SoundChannelPtr>::Slot
            >                                           _walkSlots;

            static
            std::map<std::string, minko::uint>          _labels;

        public:
            ~DinoScript()
            {
            }

            static
            Ptr
            create(NodePtr root)
            {
                auto script = std::shared_ptr<DinoScript>(new DinoScript(root));

                script->initialize();

                return script;
            }

            void
            gameOver();

        protected:
            void
            initialize();

            void
            start(std::shared_ptr<minko::scene::Node> target);

            void
            update(std::shared_ptr<minko::scene::Node> target);

            void
            stop(std::shared_ptr<minko::scene::Node> target);

        private:
            DinoScript(NodePtr);

            void
            initDinoSymbol();

            void
            initDinoLaneAnimation();

            float
            distanceToCar();

            void
            updateState();

            void
            changeCurrentState(const State& state);

            void
            startState(const State& state);

            void
            stopState(const State& state);

            float
            elapsedTime(const std::string& name) const;

            void
            resetTimer(const std::string& name);

            inline
            int
            lane() const
            {
                return _lane;
            }

            bool
            hasSameLaneAsCar() const;

            void
            followCar();

            void
            roar();

            void
            rush();

            void
            attack();

            void
            step();

            void
            carEnteredLane();

            void
            carExitedLane();

            inline
            bool
            dinoIsActive() const
            {
                return _dinoSymbol != nullptr;
            }

            void
            updateSpeed();

            float
            defaultDistanceToCar() const;

            float
            requiredDistanceToCar(float distance) const;

            void
            eat();

            static
            minko::uint
            gameOverTimeFunction(minko::uint time);
        };
    }
}
