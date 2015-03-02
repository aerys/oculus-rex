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

#include "RoadScript.hpp"
#include "minko/audio/SoundChannel.hpp"

using namespace minko;
using namespace minko::math;
using namespace minko::component;
using namespace minko::audio;
using namespace trex::component;

void
RoadScript::initialize()
{
    AbstractScript::initialize();
}

void
RoadScript::start(scene::Node::Ptr target)
{
    std::cout << "RoadScript start" << std::endl;
    auto root = target->parent();
    auto sceneManager = root->component<SceneManager>();

    _lastChunkSide.push_back(-1);
    _lastChunkSide.push_back(-1);
    initializeProps(sceneManager->assets());
    initializeGround(sceneManager->assets());

    for (int i = 0; i < TREX_CHUNK_POOL_SIZE; i++)
    {
        auto chunk = scene::Node::create();
        initializeChunk(chunk, sceneManager->assets());

#ifdef ROAD_COLLISION_ENABLE
        createObstacle(sceneManager, i);
        chunk->addChild(_obstacles[i]);
#endif

        chunk->addComponent(Transform::create());
        _stockChunks.push_back(chunk);
        target->addChild(chunk);
        chunk->component<Transform>()->matrix()->appendTranslation(0.f, -50.f, 0.f);
    }
}

void
RoadScript::initializeGround(minko::file::AssetLibrary::Ptr assets)
{
    auto fx = assets->effect("effect/Phong.effect");
    _ground = assets->symbol(TREX_ROAD_MAP);

    _ground->component<Transform>()->matrix()
        ->appendTranslation(0.f, 0.f, (float)TREX_ROAD_CHUNK_LENGTH / 2.f);
}

void
RoadScript::initializeProps(minko::file::AssetLibrary::Ptr assets)
{
    _props.push_back(assets->symbol("model/map_block_a.scene"));
    _props.push_back(assets->symbol("model/map_block_b.scene"));
    _props.push_back(assets->symbol("model/map_block_c.scene"));
    _props.push_back(assets->symbol("model/map_block_d.scene"));
    _props.push_back(assets->symbol("model/map_block_e.scene"));
    _trunkModels.push_back(assets->symbol("model/item_trunk_a.scene"));
    _trunkModels.push_back(assets->symbol("model/item_trunk_b.scene"));
    _trunkModels.push_back(assets->symbol("model/item_trunk_c.scene"));
    _trunkModels.push_back(assets->symbol("model/item_trunk_d.scene"));
    _lightWell = assets->symbol("model/misc_lightwell.scene");
    _lianaModel = assets->symbol("model/item_liana.scene");

    for (auto prop : _props)
    {
        auto nodeSet = scene::NodeSet::create(prop)
            ->descendants(false)
            ->where([=](scene::Node::Ptr n)
        {
            return n->hasComponent<Surface>();
        });

        for (auto node : nodeSet->nodes())
        {
            auto material = node->component<Surface>()->material();
            material->set("diffuseMap", assets->texture("texture/map_block_diff.jpg"));
            material->set("normalMap", assets->texture("texture/map_block_nrm.jpg"));
            material->set("alphaMap", assets->texture("texture/map_block_alpha.jpg"));
        }
    }

#ifdef TREX_ENABLE_LIGHTWELL
    auto lightsNodes = scene::NodeSet::create(_lightWell)
        ->descendants(false)
        ->where([](scene::Node::Ptr n)
    {
        return n->hasComponent<Surface>();
    });
    for (scene::Node::Ptr n : lightsNodes->nodes())
        n->component<Surface>()->effect(assets->effect("effect/Basic.effect"));

    auto lianaNodes = scene::NodeSet::create(_lianaModel)
        ->descendants(false)
        ->where([=](scene::Node::Ptr n)
    {
        return n->hasComponent<Surface>();
    });
    for (auto node : lianaNodes->nodes())
    {
        auto material = node->component<Surface>()->material();

        material->set("diffuseMap", assets->texture("texture/map_block_diff.jpg"));
        material->set("normalMap", assets->texture("texture/map_block_nrm.jpg"));
        material->set("alphaMap", assets->texture("texture/map_block_alpha.jpg"));
        material->set("specularMap", assets->texture("texture/map_block_spec.jpg"));
    }
#endif

}

void
RoadScript::initializeChunk(minko::scene::Node::Ptr chunk, minko::file::AssetLibrary::Ptr assets)
{
    chunk->addChild(_ground->clone(CloneOption::SHALLOW));
    srand(static_cast<unsigned int>(std::time(NULL)));

    auto leftSide = scene::Node::create("left");
    chunk->addChild(leftSide);
    leftSide->addComponent(Transform::create());
    initializeChunkSide(leftSide, 0);

    auto rightSide = scene::Node::create("right");
    initializeChunkSide(rightSide, 1);
    rightSide->addComponent(Transform::create());

#ifdef TREX_ENABLE_LIGHTWELL
    auto locallightWell = _lightWell->clone(CloneOption::SHALLOW);
    auto liana = _lianaModel->clone(CloneOption::SHALLOW);

    locallightWell->component<Transform>()->matrix()->appendTranslation(
        static_cast<float>(rand() % 5),
        0,
        static_cast<float>(rand() % 100)
    );
    liana->component<Transform>()->matrix()
        ->appendTranslation(0.f, -5.f, 0.f);
    rightSide->addChild(locallightWell);
    leftSide->addChild(liana);
#endif

    chunk->addChild(rightSide);
}

void
RoadScript::initializeChunkSide(minko::scene::Node::Ptr side, int index)
{

    auto numProps = TREX_ROAD_CHUNK_MIN_PROPS
        + (rand() % (TREX_ROAD_CHUNK_MAX_PROPS - TREX_ROAD_CHUNK_MIN_PROPS));

    numProps = 5;

    for (auto propId = 0; propId < numProps; ++propId)
    {
        auto r = 0;
        do{
            r = rand() % 5;
        } while (r == _prevRandomNum);

        _prevRandomNum = r;

        auto prop = _props[r]->clone(CloneOption::SHALLOW);

        // FIXME
        const float propSize = float(TREX_ROAD_CHUNK_LENGTH / numProps);

        auto ratio = 1;
        if (side->name() == "right")
        {
            ratio = -1;
        }

        if (ratio == -1)
            prop->component<Transform>()->matrix()->prependRotationY(float(M_PI));

        prop->component<Transform>()->matrix()
            ->appendTranslation(0.f, 0.f, (propId * propSize));

        side->addChild(prop);
    }

}

void
RoadScript::createObstacle(SceneManager::Ptr sceneManager, int i)
{

    int id = abs(rand() * i) % 4;
    auto pos = (i % 3 - 1) * TREX_ROAD_WIDTH / 3.2;
    auto trans = Transform::create(Matrix4x4::create()
        ->translation(pos, 0.f, TREX_ROAD_CHUNK_LENGTH / 2.f));
    auto mesh = _trunkModels[id]->clone(CloneOption::SHALLOW);

    mesh->component<Transform>()->matrix()
        ->appendScale(2)
        ->appendTranslation(pos, 0.f, TREX_ROAD_CHUNK_LENGTH / 2.f);
    _obstacles.push_back(mesh);
}

void
RoadScript::update(scene::Node::Ptr target)
{
#ifdef ROAD_COLLISION_ENABLE
    auto manageCar = _car->component<trex::component::CarScript>();
    int posCarZ = int(_car->component<Transform>()->z());

    checkCollision(manageCar, posCarZ);
#endif

    manageChunks(_car, target);
}

void
RoadScript::manageChunks(scene::Node::Ptr camera, scene::Node::Ptr target)
{

    auto currentPosition = camera->component<Transform>()->z();

    auto furtherFrontChunkPosition = 0;
    auto furtherBackChunkPosition = 10000;
    if (_activeChunks.size() > 0)
    {
        furtherFrontChunkPosition = int(_activeChunks.back()->component<Transform>()->z());
        furtherBackChunkPosition = int(_activeChunks.front()->component<Transform>()->z());
    }
    else
    {
        addFrontChunk(target);
        addFrontChunk(target);
        addFrontChunk(target);
        addFrontChunk(target);
        addFrontChunk(target);
        addFrontChunk(target);
    }

    if (currentPosition + TREX_FRONT_VIEW_DISTANCE >  furtherFrontChunkPosition + TREX_ROAD_CHUNK_LENGTH * 2)
    {
        removeBackChunk(target);
        addFrontChunk(target);
    }

}

void
RoadScript::addFrontChunk(scene::Node::Ptr target)
{
    auto index = rand() % _stockChunks.size();
    auto furtherFrontChunkPosition = float(-TREX_ROAD_CHUNK_LENGTH * 4);
    if (_activeChunks.size() > 0)
    {
        furtherFrontChunkPosition = std::floor(_activeChunks.back()->component<Transform>()->modelToWorldMatrix(true)->transform(Vector3::create())->z());
    }
    auto newPosZ = furtherFrontChunkPosition + (float)TREX_ROAD_CHUNK_LENGTH;
    auto chunkAdded = _stockChunks[index];
    chunkAdded->component<Transform>()->matrix()->identity();
    chunkAdded->component<Transform>()->matrix()->appendTranslation(0., 0., newPosZ);
    _activeChunks.push_back(chunkAdded);

    _stockChunks.erase(_stockChunks.begin() + index);
}

void
RoadScript::removeBackChunk(scene::Node::Ptr target)
{
    _stockChunks.push_back(_activeChunks[0]);
    _activeChunks[0]->component<Transform>()->matrix()
        ->identity()
        ->appendTranslation(0.f, -50.f, 0.f);

    _activeChunks.erase(_activeChunks.begin());
}

void
RoadScript::playHitSound()
{
    static auto sceneManager = _car->root()->component<SceneManager>();
    static auto channel = sceneManager->assets()->sound("sound/car_hit_1.ogg")->play(1);
}

void
RoadScript::checkCollision(CarScript::Ptr manageCar, int posCarZ)
{
    int lane = manageCar->lane() - 1;

    if (posCarZ > 0 && posCarZ != _lastCollision)
    {
        for (unsigned int i = 0; i < _obstacles.size(); i++)
        {
            if (posCarZ == int(_obstacles[i]->component<Transform>()->z()))
            {
                int posObstacleX = int(_obstacles[i]->component<Transform>()->x());

                if (posObstacleX * lane < 0 || (posObstacleX == 0 && lane == 0))
                    manageCollision(manageCar, _obstacles[i]);
                _lastCollision = posCarZ;
                break;
            }
        }
    }
}

void
RoadScript::manageCollision(CarScript::Ptr manageCar, scene::Node::Ptr obstacle)
{
    float speed = manageCar->speed();
    auto obstacleNodes = scene::NodeSet::create(obstacle)
        ->descendants(false)
        ->where([](scene::Node::Ptr n)
    {
        return n->hasComponent<Surface>();
    });
    for (scene::Node::Ptr n : obstacleNodes->nodes())
        n->component<Surface>()->visible(false);
    _invisibleTrunk = obstacle;
    _invisibleTime = 0;

    playHitSound();

    if (speed - ROAD_COLLISION_SLOWDOWN > 0)
        manageCar->speed(float(speed - ROAD_COLLISION_SLOWDOWN));

    manageCar->obstacleHitCount(manageCar->obstacleHitCount() + 1);

    if (manageCar->obstacleHitCount() >= 2)
    {
        LOG_INFO("death by obstacle hit");

        manageCar->gameOver();
    }
}

void
RoadScript::stop(scene::Node::Ptr target)
{
    std::cout << "RoadScript stop" << std::endl;
}
