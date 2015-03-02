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

#include "trex/Config.hpp"
#include "trex/component/CarScript.hpp"

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
        class RoadScript : public minko::component::AbstractScript
        {
        public:
            typedef std::shared_ptr<RoadScript>     Ptr;

        private:
            std::vector<minko::scene::Node::Ptr>    _props;
            minko::scene::Node::Ptr                 _lightWell;
            minko::scene::Node::Ptr                 _ground;
            minko::scene::Node::Ptr                 _car;
            minko::scene::Node::Ptr                 _lianaModel;
            minko::scene::Node::Ptr                 _invisibleTrunk;
            std::vector<minko::scene::Node::Ptr>    _trunkModels;
            std::vector<minko::scene::Node::Ptr>    _activeChunks;
            std::vector<minko::scene::Node::Ptr>    _stockChunks;
            std::vector<minko::scene::Node::Ptr>    _obstacles;
            std::vector<int>                        _lastChunkSide;
            int                                     _lastCollision;
            int                                     _prevRandomNum;
            int                                     _invisibleTime;

        private:
            RoadScript(minko::scene::Node::Ptr car) :
                _car(car),
                _lastCollision(-1),
                _prevRandomNum(0),
                _invisibleTime(-1)
            {
            }

        public:
            ~RoadScript()
            {
            }

            static
            Ptr
            create(minko::scene::Node::Ptr car)
            {
                auto script = std::shared_ptr<RoadScript>(new RoadScript(car));

                script->initialize();

                return script;
            }

            inline
            int
            invisibleTime() const
            {
                return _invisibleTime;
            }

            inline
            void
            invisibleTime(int val)
            {
                _invisibleTime = val;
            }

            inline
            minko::scene::Node::Ptr
            invisibleTrunk() const
            {
                return _invisibleTrunk;
            }

        protected:
            void
            initialize();

            void
            start(minko::scene::Node::Ptr target);

            void
            update(minko::scene::Node::Ptr target);

            void
            stop(minko::scene::Node::Ptr target);

            void
            initializeProps(minko::file::AssetLibrary::Ptr assets);

            void
            initializeGround(minko::file::AssetLibrary::Ptr assets);

            void
            initializeChunk(minko::scene::Node::Ptr chunk, minko::file::AssetLibrary::Ptr );

            void
            initializeChunkSide(minko::scene::Node::Ptr side, int index);

            void
            createObstacle(minko::component::SceneManager::Ptr, int i);

            void
            manageChunks(minko::scene::Node::Ptr camera, minko::scene::Node::Ptr target);

            void
            addFrontChunk(minko::scene::Node::Ptr target);

            void
            removeBackChunk(minko::scene::Node::Ptr target);

            void
            checkCollision(trex::component::CarScript::Ptr manageCar, int posCarZ);

            void
            manageCollision(CarScript::Ptr manageCar, minko::scene::Node::Ptr collision);

            void
            playHitSound();
        };
    }
}