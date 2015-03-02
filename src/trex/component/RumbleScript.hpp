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
#include "trex/component/RoadScript.hpp"

namespace trex
{
    namespace component
    {
        class CarScript;
        class RoadScript;
    }
}

namespace trex
{
    namespace component
    {
        class RumbleScript : public minko::component::AbstractScript
        {

        private:
            RumbleScript(minko::scene::Node::Ptr car, minko::scene::Node::Ptr road) :
                _car(car),
                _road(road),
                _zRotation(0.f),
                _rumbleTime(0),
                _rumbleOldTime(0)
            {
            }

        public:
            ~RumbleScript()
            {
            }

            static
            Ptr
            create(minko::scene::Node::Ptr car, minko::scene::Node::Ptr road)
            {
                auto script = std::shared_ptr<RumbleScript>(new RumbleScript(car, road));

                script->initialize();

                return script;
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

            int
            rumbleManage(int id);

            void
            speedManage();

            void
            trunkManage();

        private:
            minko::scene::Node::Ptr     _car;
            minko::scene::Node::Ptr     _road;
            float                       _zRotation;
            int                         _startRumble;
            int                         _rumbleTime;
            int                         _rumbleOldTime;

            std::vector<int>            _rumbleIntensities;
        };
    }
}