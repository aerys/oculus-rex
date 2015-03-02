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

#include "minko/log/Logger.hpp"
#include "trex/Config.hpp"
#include "DinoScript.hpp"
#include "CarScript.hpp"
#include "trex/Config.hpp"
#include "trex/PseudoRandom.hpp"

using namespace minko;
using namespace minko::component;
using namespace minko::audio;
using namespace minko::math;
using namespace trex::component;

std::vector<std::string>
DinoScript::_eatSamples =
{
    "sound/trex_eat.ogg"
};

std::vector<std::string>
DinoScript::_roarSamples =
{
    "sound/trex_roar_loud_2.ogg",
    "sound/trex_roar_loud_4.ogg",
    "sound/trex_roar_loud_7.ogg",
    "sound/trex_roar_loud_9.ogg"
};


std::vector<std::string>
DinoScript::_footStepSamples =
{
    "sound/trex_step_close_1.ogg",
    "sound/trex_step_close_2.ogg",
    "sound/trex_step_close_3.ogg",
    "sound/trex_step_close_4.ogg",
    "sound/trex_step_close_5.ogg",
    "sound/trex_step_close_6.ogg"
};

std::vector<std::string>
DinoScript::_attackSamples =
{
    "sound/trex_roar_loud_1.ogg",
    "sound/trex_roar_loud_10.ogg",
    "sound/trex_roar_loud_11.ogg",
    "sound/trex_roar_loud_12.ogg"
};

std::vector<std::string>
DinoScript::_rushSamples =
{
    "sound/trex_roar_middle_1.ogg",
    "sound/trex_roar_middle_2.ogg",
    "sound/trex_roar_middle_3.ogg"
};


std::map<
    audio::PositionalSound::Ptr,
    Signal<SoundChannel::Ptr>::Slot
>
DinoScript::_walkSlots;

const std::string LABEL_DINO_FOOT_STEP_LEFT_START = "footStepLeftStart";
const std::string LABEL_DINO_FOOT_STEP_LEFT_STOP = "footStepLeftStop";
const std::string LABEL_DINO_FOOT_STEP_RIGHT_START = "footStepRightStart";
const std::string LABEL_DINO_FOOT_STEP_RIGHT_STOP = "footStepRightStop";
const std::string LABEL_DINO_SCREAM_START = "screamStart";
const std::string LABEL_DINO_SCREAM_STOP = "screamStop";
const std::string LABEL_DINO_ATTACK_START = "attackStart";
const std::string LABEL_DINO_ATTACK_STOP = "attackStop";
const std::string LABEL_DINO_EAT_START = "eatStart";
const std::string LABEL_DINO_EAT_MOVE_CAMERA = "eatMoveCamera";
const std::string LABEL_DINO_EAT_GAME_OVER = "eatGameOver";
const std::string LABEL_DINO_EAT_STOP = "eatStop";
const std::string LABEL_DINO_RUN_START = "runStart";
const std::string LABEL_DINO_RUN_STOP = "runStop";

std::map<std::string, uint>
DinoScript::_labels = {
    { LABEL_DINO_FOOT_STEP_LEFT_START, 0 },
    { LABEL_DINO_FOOT_STEP_LEFT_STOP, 500 },
    { LABEL_DINO_FOOT_STEP_RIGHT_START, 500 },
    { LABEL_DINO_FOOT_STEP_RIGHT_STOP, 1000 },
    { LABEL_DINO_SCREAM_START, 1500 },
    { LABEL_DINO_SCREAM_STOP, 3500 },
    { LABEL_DINO_ATTACK_START, 5000 },
    { LABEL_DINO_ATTACK_STOP, 7333 },
    { LABEL_DINO_EAT_START, 8000 },
    { LABEL_DINO_EAT_MOVE_CAMERA, 8233 },
    { LABEL_DINO_EAT_GAME_OVER, 11000 },
    { LABEL_DINO_EAT_STOP, 13000 },
    { LABEL_DINO_RUN_START, 14000 },
    { LABEL_DINO_RUN_STOP, 14800 },
    { "footStep2", 2000 },
    { "footStep3", 2500 },
    { "footStep4", 3000 },
    { "footStep5", 3500 },
    { "footStep6", 4833 },
    { "footStep7", 5333 },
    { "footStep8", 5866 },
    { "footStep9", 6333 },
    { "footStep10", 6833 },
    { "footStep11", 7333 },
    { "footStep12", 14400 },
    { "footStep13", 7333 },
    { "footStep14", 7333 }
};

DinoScript::DinoScript(NodePtr root) :
    _speed(),
    _dinoSymbol(nullptr),
    _currentState(State::NONE),
    _lane((NUM_LANES - 1) / 2),
    _currentTimeStamp(0.0f),
    _hadSameLaneAsCar(false),
    _root(root),
    _wasFollowing(false),
    _isEating(false),
    _gameIsOver(false)
{
    _timers.insert(std::make_pair("stateChanged", 0.0f));
    _timers.insert(std::make_pair("dummyAnimation", 0.0f));
    _timers.insert(std::make_pair("carEnteredLane", 0.0f));
    _timers.insert(std::make_pair("carExitedLane", 0.0f));
    _timers.insert(std::make_pair("dinoAttackEnded", 0.0f));
}

void
DinoScript::initialize()
{
    AbstractScript::initialize();

    _requiredSpeed = TREX_DINO_BASE_SPEED;

    initDinoLaneAnimation();
}

void
DinoScript::initDinoLaneAnimation()
{
    _dinoLaneAnimatedNode = minko::scene::Node::create();
    _dinoLaneAnimatedNode->addComponent(Transform::create());

    auto lane0 = Matrix4x4::create()->appendTranslation(LANE_WIDTH, 0.f, 0.f);
    auto lane1 = Matrix4x4::create();
    auto lane2 = Matrix4x4::create()->appendTranslation(-LANE_WIDTH, 0.f, 0.f);

    auto lane0to1 = Matrix4x4::create()->prependRotationY(float(-M_PI_4) / 5.5f)->appendTranslation(LANE_WIDTH / 2, 0.f, 0.f);
    auto lane1to0 = Matrix4x4::create()->prependRotationY(float(M_PI_4) / 5.5f)->appendTranslation(LANE_WIDTH / 2, 0.f, 0.f);

    auto lane1to2 = Matrix4x4::create()->prependRotationY(float(-M_PI_4) / 5.5f)->appendTranslation(-LANE_WIDTH / 2, 0.f, 0.f);
    auto lane2to1 = Matrix4x4::create()->prependRotationY(float(M_PI_4) / 5.5f)->appendTranslation(-LANE_WIDTH / 2, 0.f, 0.f);

    auto lane0to2 = Matrix4x4::create()->prependRotationY(float(-M_PI_4) / 2.f)->appendTranslation(0.f, 0.f, 0.f);
    auto lane2to0 = Matrix4x4::create()->prependRotationY(float(M_PI_4) / 2.f)->appendTranslation(0.f, 0.f, 0.f);

    std::vector<Matrix4x4::Ptr> matrices = { lane1, lane1to0, lane0, lane0to2, lane2, lane2to0, lane0, lane0to1, lane1, lane1to2, lane2, lane2to1, lane1 };
    std::vector<uint> timetable;

    for (int i = 0; i < 13; ++i)
    {
        timetable.push_back(i * LANE_CHANGE_DURATION / 2);
    }

    auto timeline = animation::Matrix4x4Timeline::create("transform.matrix", 3000, timetable, matrices, true);
    _dinoLaneAnimation = Animation::create({ timeline });

    auto halfDuration = static_cast<unsigned int>(LANE_CHANGE_DURATION * 0.5f);

    _dinoLaneAnimation->addLabel("1to0_start", 0);
    _dinoLaneAnimation->addLabel("1to0", halfDuration);
    _dinoLaneAnimation->addLabel("1to0_end", LANE_CHANGE_DURATION);

    _dinoLaneAnimation->addLabel("0to2_start", LANE_CHANGE_DURATION);
    _dinoLaneAnimation->addLabel("0to2", LANE_CHANGE_DURATION + halfDuration);
    _dinoLaneAnimation->addLabel("0to2_end", LANE_CHANGE_DURATION * 2);

    _dinoLaneAnimation->addLabel("2to0_start", LANE_CHANGE_DURATION * 2);
    _dinoLaneAnimation->addLabel("2to0", LANE_CHANGE_DURATION * 2 + halfDuration);
    _dinoLaneAnimation->addLabel("2to0_end", LANE_CHANGE_DURATION * 3);

    _dinoLaneAnimation->addLabel("0to1_start", LANE_CHANGE_DURATION * 3);
    _dinoLaneAnimation->addLabel("0to1", LANE_CHANGE_DURATION * 3 + halfDuration);
    _dinoLaneAnimation->addLabel("0to1_end", LANE_CHANGE_DURATION * 4);

    _dinoLaneAnimation->addLabel("1to2_start", LANE_CHANGE_DURATION * 4);
    _dinoLaneAnimation->addLabel("1to2", LANE_CHANGE_DURATION * 4 + halfDuration);
    _dinoLaneAnimation->addLabel("1to2_end", LANE_CHANGE_DURATION * 5);

    _dinoLaneAnimation->addLabel("2to1_start", LANE_CHANGE_DURATION * 5);
    _dinoLaneAnimation->addLabel("2to1", LANE_CHANGE_DURATION * 5 + halfDuration);
    _dinoLaneAnimation->addLabel("2to1_end", LANE_CHANGE_DURATION * 6);

    _dinoLaneAnimation->isLooping(false);

    _dinoLaneAnimationLabelHit = _dinoLaneAnimation->labelHit()->connect([&](AbstractAnimationPtr, std::string name, uint time)
    {
        /*if (name == "1to0" && _lane != 0)
            _lane = 0;
        else if ((name == "0to1" || name == "2to1") && _lane != 1)
            _lane = 1;
        else if (name == "1to2" && _lane != 2)
            _lane = 2;
        else
            return;

        std::cout << "car on lane " << _lane << std::endl;*/
    });

    _dinoLaneAnimatedNode->addComponent(_dinoLaneAnimation->seek(0)->stop());

    _root->addChild(_dinoLaneAnimatedNode);
}

void
DinoScript::start(scene::Node::Ptr target)
{
    std::cout << "DinoScript start" << std::endl;

    if (_target != nullptr)
        return;

    _target = target;

    auto transform = Transform::create();
    transform->matrix()->appendTranslation(0.f, 0.f, -(TREX_DINO_LENGTH / 2) - TREX_DINO_STARTING_DIST);

    _target->addComponent(transform);

    _sceneManager = _target->root()->component<SceneManager>();
    
    auto carNodes = scene::NodeSet::create(target->root())->descendants(true)->where([](scene::Node::Ptr node)
    {
        return node->hasComponent<CarScript>();
    });

    _car = carNodes->nodes().at(0)->component<CarScript>();

    initDinoSymbol();
}

void
DinoScript::initDinoSymbol()
{
    _dinoSymbol = _sceneManager->assets()->symbol("model/char_trex.scene");

    if (!_dinoSymbol->hasComponent<Transform>())
        _dinoSymbol->addComponent(Transform::create());

    _dinoSymbol->component<Transform>()->matrix()->appendRotationY(float(M_PI));

    auto dummyNodes = scene::NodeSet::create(_dinoSymbol)->descendants(true)->where([](scene::Node::Ptr node)
    {
        return node->name() == "Box_Camera_Game_Over";
    });

    _headDummyNode = dummyNodes->nodes()[0];

    _target->addChild(_dinoLaneAnimatedNode);
    _dinoLaneAnimatedNode->addChild(_dinoSymbol);

    auto animNodeSet = scene::NodeSet::create(_dinoSymbol)
        ->descendants(true)
        ->where([](scene::Node::Ptr n)
    {
        if (n->hasComponent<AbstractAnimation>())
        {
            n->component<AbstractAnimation>()->timeFunction([](uint time)
            {
                return time;

                auto ratio = TREX_DINO_BASE_SPEED / TREX_DINO_INTRO_SPEED;
                return (uint)((float)time * ratio);
            });
        }
        return n->hasComponent<MasterAnimation>();
    });

    _dinoSkinnedNode = animNodeSet->nodes().front();


    _dinoSkinnedNode->component<MasterAnimation>()->play();

    _dinoFootStepLabelHitSlot = _dinoSkinnedNode->component<MasterAnimation>()->labelHit()->connect([&](AbstractAnimation::Ptr, std::string name, uint time)
    {
        if (!_car->gameStarted())
            return;

        if (name == LABEL_DINO_FOOT_STEP_LEFT_START ||
            name == LABEL_DINO_FOOT_STEP_RIGHT_START ||
            name == "footStep0" ||
            name == "footStep1" ||
            name == "footStep2" ||
            name == "footStep3" ||
            name == "footStep4" ||
            name == "footStep5" ||
            name == "footStep6" ||
            name == "footStep7" ||
            name == "footStep8" ||
            name == "footStep9" ||
            name == "footStep10" ||
            name == "footStep11" ||
            name == "footStep12" ||
            name == "footStep13" ||
            name == "footStep14")
        {
            step();
        }
        else if (name == LABEL_DINO_ATTACK_STOP)
        {
            if (hasSameLaneAsCar())
                changeCurrentState(TREX_GOD_MODE ? State::RECOVERING : State::AAARGH);
            else
                changeCurrentState(State::RECOVERING);
        }
        else if (name == LABEL_DINO_SCREAM_STOP)
        {
            if (hasSameLaneAsCar())
                changeCurrentState(State::ACCELERATING);
            else
                changeCurrentState(State::WALKING);
        }
        else if (name == LABEL_DINO_EAT_GAME_OVER)
        {
            gameOver();
        }
        else if (name == LABEL_DINO_EAT_MOVE_CAMERA)
        {
            _car->moveCameraToNode(_headDummyNode);
        }
    });

    for (auto& kv : _labels)
        _dinoSkinnedNode->component<MasterAnimation>()->addLabel(kv.first, kv.second);
}

float
audibilityCurve(float distance)
{
    return 1;

    // FIXME: Return range [0-1] according to distance.
    float volume = 1.f - std::min(1.f, distance / 30.f);
    //std::cout << "distance: " << distance << ", volume: " << volume << std::endl;
    return volume;
}

void
DinoScript::attack()
{
    static Signal<SoundChannel::Ptr>::Slot attackSlot;
    static PseudoRandom<std::string>::Ptr random = PseudoRandom<std::string>::create(_attackSamples);

    auto channel = _sceneManager->assets()->sound(random->next())->play(1);

    if (!channel)
        return;

    auto sound = audio::PositionalSound::create(channel, _car->camera());
    sound->audibilityCurve(audibilityCurve);

    attackSlot = channel->complete()->connect([=](SoundChannel::Ptr channel)
    {
        _dinoSymbol->removeComponent(sound);
    });

    _dinoSymbol->addComponent(sound);
}

void
DinoScript::roar()
{
    static Signal<SoundChannel::Ptr>::Slot _roarSlot;
    static PseudoRandom<std::string>::Ptr random = PseudoRandom<std::string>::create(_roarSamples);

    auto channel = _sceneManager->assets()->sound(random->next())->play(1);

    if (!channel)
        return;

    auto sound = audio::PositionalSound::create(channel, _car->camera());
    sound->audibilityCurve(audibilityCurve);

    _roarSlot = channel->complete()->connect([=](SoundChannel::Ptr channel)
    {
        _dinoSymbol->removeComponent(sound);
    });

    _dinoSymbol->addComponent(sound);
}

void
DinoScript::rush()
{
    static Signal<SoundChannel::Ptr>::Slot _rushSlot;
    static PseudoRandom<std::string>::Ptr random = PseudoRandom<std::string>::create(_rushSamples);

    auto channel = _sceneManager->assets()->sound(random->next())->play(1);

    if (!channel)
        return;

    auto sound = audio::PositionalSound::create(channel, _car->camera());
    sound->audibilityCurve(audibilityCurve);

    _rushSlot = channel->complete()->connect([=](SoundChannel::Ptr channel)
    {
        _dinoSymbol->removeComponent(sound);
    });

    _dinoSymbol->addComponent(sound);
}

void
DinoScript::step()
{
    static PseudoRandom<std::string>::Ptr random = PseudoRandom<std::string>::create(_footStepSamples);
    auto channel = _sceneManager->assets()->sound(random->next())->play(1);

    if (!channel)
        return;

    auto sound = audio::PositionalSound::create(channel, _car->camera());
    sound->audibilityCurve(audibilityCurve);

    _walkSlots[sound] = channel->complete()->connect([=](SoundChannel::Ptr channel)
    {
        _walkSlots.erase(sound);
        _dinoSymbol->removeComponent(sound);
    });

    _dinoSymbol->addComponent(sound);
}

void
DinoScript::update(scene::Node::Ptr target)
{
    if (target != _target)
        return;

    if (_car->gameStarted())
    {
        if (_currentState == State::NONE)
            changeCurrentState(State::SPAWNING);

        auto dz = (_speed / 3600.f) * deltaTime();

        _target->component<Transform>()->matrix()->appendTranslation(0.f, 0.f, dz);

        if (dinoIsActive())
        {
            if (_hadSameLaneAsCar && !hasSameLaneAsCar())
            {
                carExitedLane();
            }
            else if (!_hadSameLaneAsCar && hasSameLaneAsCar())
            {
                carEnteredLane();
            }

            _hadSameLaneAsCar = hasSameLaneAsCar();
        }

        updateSpeed();

        updateState();
    }
}

float
DinoScript::distanceToCar()
{
    return (_car->getTarget(0)->component<Transform>()->modelToWorld(Vector3::create()) -
            _target->component<Transform>()->modelToWorld(Vector3::create()))->length();
}

void
DinoScript::stop(scene::Node::Ptr)
{
    std::cout << "DinoScript stop" << std::endl;
}

float
DinoScript::elapsedTime(const std::string& name) const
{
    return _timers.at(name);
}

void
DinoScript::resetTimer(const std::string& name)
{
    _timers[name] = 0.0f;
}

void
DinoScript::updateState()
{
    const auto previousTime = _currentTimeStamp;
    const auto currentTime = time();

    const auto dt = (currentTime - previousTime) / 1000.0f;

    for (auto& timer : _timers)
        timer.second += dt;

    _currentTimeStamp = currentTime;

    switch (_currentState)
    {
    case State::SPAWNING:

        if (distanceToCar() > 0.0f && distanceToCar() <= requiredDistanceToCar(defaultDistanceToCar()))
            changeCurrentState(State::WALKING);

        break;

    case State::WALKING:

        if (!hasSameLaneAsCar() && elapsedTime("carExitedLane") > TREX_DINO_FOLLOWING_STATE_DELAY)
        {
            _wasFollowing = true;

            changeCurrentState(State::FOLLOWING);
        }
        else if (hasSameLaneAsCar() &&
                 elapsedTime("carEnteredLane") > TREX_DINO_WALKING_TO_SCREAMING_STATE_DELAY &&
                 elapsedTime("dinoAttackEnded") > TREX_DINO_AFTER_ATTACKING_WALKING_STATE_DURATION)
            changeCurrentState(State::SCREAMING);

        break;

    case State::FOLLOWING:

        if (elapsedTime("stateChanged") > LANE_CHANGE_DURATION / 1000.0f)
            changeCurrentState(State::WALKING);

        break;

    case State::SCREAMING:

        break;

    case State::ACCELERATING:

        if (distanceToCar() < requiredDistanceToCar(TREX_DINO_ATTACKING_DIST))
            changeCurrentState(State::ATTACKING);

        break;

    case State::ATTACKING:

        break;

    case State::RECOVERING:

        if (distanceToCar() > requiredDistanceToCar(defaultDistanceToCar()))
            changeCurrentState(State::WALKING);

        break;

    default:
        break;
    }
}

void
DinoScript::startState(const State& state)
{
    switch (state)
    {
    case State::SPAWNING:

        _car->speed(CAR_INTRO_SPEED);
        _requiredSpeed = TREX_DINO_INTRO_SPEED - std::min(_car->deltaSpeed(), 0.0f);

        _dinoSkinnedNode->component<MasterAnimation>()
            ->setPlaybackWindow(
                LABEL_DINO_FOOT_STEP_LEFT_START,
                LABEL_DINO_FOOT_STEP_RIGHT_STOP,
                true
            );

        break;

    case State::WALKING:

        LOG_INFO("start WALKING");

        _requiredSpeed = TREX_DINO_BASE_SPEED;

        if (!_wasFollowing)
        {
            _dinoSkinnedNode->component<MasterAnimation>()
                ->setPlaybackWindow(
                    LABEL_DINO_FOOT_STEP_LEFT_START,
                    LABEL_DINO_FOOT_STEP_RIGHT_STOP,
                    true
            );
        }
        else
            _wasFollowing = false;

        break;

    case State::FOLLOWING:

        LOG_INFO("start FOLLOWING");

        followCar();

        break;

    case State::SCREAMING:

         LOG_INFO("start SCREAMING");

        _dinoSkinnedNode->component<MasterAnimation>()
            ->setPlaybackWindow(
                LABEL_DINO_SCREAM_START,
                LABEL_DINO_SCREAM_STOP,
                true
        );

         roar();

        break;

    case State::ACCELERATING:

        LOG_INFO("start ACCELERATING");

        _requiredSpeed = TREX_DINO_ACCELERATING_STATE_SPEED;

        _dinoSkinnedNode->component<MasterAnimation>()
            ->setPlaybackWindow(
                LABEL_DINO_RUN_START,
                LABEL_DINO_RUN_STOP,
                true
            );

        rush();

        break;

    case State::ATTACKING:

        LOG_INFO("start ATTACKING");

        _requiredSpeed = TREX_DINO_BASE_SPEED;

        _car->lockLane(_lane, true);

        _dinoSkinnedNode->component<MasterAnimation>()
            ->setPlaybackWindow(
                LABEL_DINO_ATTACK_START,
                LABEL_DINO_ATTACK_STOP,
                true
            );

        attack();

        break;

    case State::RECOVERING:

        LOG_INFO("start RECOVERING");

        _dinoSkinnedNode->component<MasterAnimation>()
            ->setPlaybackWindow(
                LABEL_DINO_FOOT_STEP_LEFT_START,
                LABEL_DINO_FOOT_STEP_RIGHT_STOP,
                true
            );

        _requiredSpeed = TREX_DINO_RECOVERING_STATE_SPEED;

        break;

    case State::AAARGH:

        eat();

        _dinoSkinnedNode->component<MasterAnimation>()
            ->setPlaybackWindow(
                LABEL_DINO_EAT_START,
                LABEL_DINO_EAT_STOP,
                true
            )->isLooping(true);


        _dinoSkinnedNode->component<MasterAnimation>()
            ->timeFunction(gameOverTimeFunction);

        break;

    default:
        break;
    }
}

void
DinoScript::stopState(const State& state)
{
    switch (state)
    {
    case State::SPAWNING:
    {
        LOG_INFO("stop spawning");

        _car->speed(CAR_BASE_SPEED);

        _music = _sceneManager->assets()->sound("sound/music.ogg")->play(0);
        _music->transform(SoundTransform::create(.4f));

        break;
    }
    case State::WALKING:

        LOG_INFO("stop WALKING");

        break;

    case State::FOLLOWING:

        LOG_INFO("stop FOLLOWING");

        break;

    case State::SCREAMING:

         LOG_INFO("stop SCREAMING");

        break;

    case State::ACCELERATING:

        LOG_INFO("stop ACCELERATING");

        break;

    case State::ATTACKING:

        LOG_INFO("stop ATTACKING");

        _car->lockLane(_lane, false);

        resetTimer("dinoAttackEnded");

        break;

    case State::RECOVERING:

        LOG_INFO("stop RECOVERING");

        break;

    default:
        break;
    }
}

void
DinoScript::changeCurrentState(const State& state)
{
    if (state == _currentState)
        return;

    stopState(_currentState);

    _currentState = state;

    startState(state);

    resetTimer("stateChanged");
}

bool
DinoScript::hasSameLaneAsCar() const
{
    return _car->lane() == lane();
}

void
DinoScript::followCar()
{
    auto deltaLane = _car->lane() - lane();

    auto distance = deltaLane * LANE_WIDTH;
    
    _lane = lane() + deltaLane;

    if (deltaLane != 0)
    {
        std::string laneChange = std::to_string(_lane - deltaLane) + "to" + std::to_string(_lane);

        _dinoLaneAnimation->stop()->setPlaybackWindow(laneChange + "_start", laneChange + "_end")->seek(laneChange + "_start")->play();
    }
}

void
DinoScript::carEnteredLane()
{
    LOG_INFO("car entered lane");

    resetTimer("carEnteredLane");
}

void
DinoScript::carExitedLane()
{
    LOG_INFO("car exited lane");

    resetTimer("carExitedLane");
}

float
DinoScript::defaultDistanceToCar() const
{
    return _car->obstacleHitCount() > 0 ? TREX_DINO_DIST_WHEN_CAR_IS_SLOWN : TREX_DINO_DIST;
}

float
DinoScript::requiredDistanceToCar(float distance) const
{
    return (TREX_DINO_LENGTH / 2) + distance;
}

void
DinoScript::updateSpeed()
{
    if (_isEating)
    {
        _speed = 0.0f;

        return;
    }

    _speed = _requiredSpeed + _car->deltaSpeed();
}

void
DinoScript::eat()
{
    _isEating = true;

    _car->speed(0.0f);
    
    _requiredSpeed = 0.0f;

    static Signal<SoundChannel::Ptr>::Slot _eatSlot;
    static PseudoRandom<std::string>::Ptr random = PseudoRandom<std::string>::create(_eatSamples);

    auto channel = _sceneManager->assets()->sound("sound/trex_eat.ogg")->play(1);

    if (!channel)
        return;

    auto sound = audio::PositionalSound::create(channel, _car->camera());
    sound->audibilityCurve(audibilityCurve);

    _eatSlot = channel->complete()->connect([=](SoundChannel::Ptr channel)
    {
        _dinoSymbol->removeComponent(sound);
    });

    _dinoSymbol->addComponent(sound);
}

void
DinoScript::gameOver()
{
    if (_gameIsOver)
        return;

    _dinoSkinnedNode->component<MasterAnimation>()
        ->stop();

    _gameIsOver = true;

    LOG_INFO("Game Is Over");

    _car->gameOver();

    _target->component<Transform>()->matrix()->copyFrom(Matrix4x4::create());
}

uint
DinoScript::gameOverTimeFunction(uint time)
{
    static uint firstCalledAt = time;

    auto diff = time - firstCalledAt;

    time = firstCalledAt + int(diff / 3.f);

    return time;
}