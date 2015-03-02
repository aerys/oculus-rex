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

#include "RumbleScript.hpp"

using namespace minko;
using namespace minko::math;
using namespace minko::component;
using namespace trex::component;

void
RumbleScript::initialize()
{
    AbstractScript::initialize();

    _rumbleIntensities = {
        CAR_RUMBLE_LVL,
        CAR_RUMBLE_LVL * 2,
        CAR_RUMBLE_LVL * 3
    };
}

void
RumbleScript::start(scene::Node::Ptr target)
{
    _startRumble = int(time() / 1000.f);
}

void
RumbleScript::update(scene::Node::Ptr target)
{

    _rumbleTime = int(time() / 1000.f) - _startRumble;

#ifdef ROAD_COLLISION_ENABLE
    speedManage();
#endif

    int val = rumbleManage(_rumbleTime % (CAR_RUMBLE_DELAY * 3));
    float rumble = sin(time()) / float(val);
    float newZ = _zRotation + rumble;

    if (newZ > CAR_RUMBLE_THRESHOLD || newZ < -CAR_RUMBLE_THRESHOLD)
        return;

    _car->component<Transform>()->matrix()->prependRotationZ(rumble);

    _zRotation = newZ;
}

int
RumbleScript::rumbleManage(int id)
{
    if (id < CAR_RUMBLE_DELAY)
        id = 0;
    else if (id < CAR_RUMBLE_DELAY * 2)
        id = 1;
    else
        id = 2;

    return _rumbleIntensities[id];
}

void
RumbleScript::speedManage()
{
    auto manageCar = _car->component<trex::component::CarScript>();
    float speed = manageCar->speed();

    if (manageCar->speed() > 0.0f)
    {
        if (_rumbleTime > _rumbleOldTime)
        {
            if (speed + ROAD_COLLISION_ACCELERATION <= CAR_BASE_SPEED)
                manageCar->speed(speed + ROAD_COLLISION_ACCELERATION);
            trunkManage();
        }
        _rumbleOldTime = _rumbleTime;
        if (manageCar->speed() > CAR_BASE_SPEED - ROAD_COLLISION_SLOWDOWN * 1.0f)
            manageCar->obstacleHitCount(std::max(0, manageCar->obstacleHitCount() - 1));
    }
}

void
RumbleScript::trunkManage()
{
    auto manageRoad = _road->component<trex::component::RoadScript>();
    int invisibleTime = manageRoad->invisibleTime();

    if (invisibleTime >= 0)
    {
        std::cout << "invisibleTime: " << invisibleTime << std::endl;
        if (invisibleTime > 2)
        {
            auto trunk = manageRoad->invisibleTrunk();
            auto trunkNodes = scene::NodeSet::create(trunk)
                ->descendants(false)
                ->where([](scene::Node::Ptr n)
            {
                return n->hasComponent<Surface>();
            });

            for (scene::Node::Ptr n : trunkNodes->nodes())
                n->component<Surface>()->visible(true);
            manageRoad->invisibleTime(-1);
            std::cout << "Trunk reappears!" << std::endl;
        }
        else
            manageRoad->invisibleTime(invisibleTime + 1);
    }
}

void
RumbleScript::stop(scene::Node::Ptr target)
{
}
