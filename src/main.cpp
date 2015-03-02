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

#include "minko/Minko.hpp"
#include "minko/MinkoSDL.hpp"
#include "minko/MinkoPNG.hpp"
#include "minko/MinkoJPEG.hpp"
#include "minko/MinkoSerializer.hpp"
#include "minko/MinkoParticles.hpp"

#include "trex/component/CarScript.hpp"
#include "trex/component/DinoScript.hpp"
#include "trex/component/RoadScript.hpp"
#include "trex/component/RumbleScript.hpp"
#include "trex/component/MirrorScript.hpp"

#if defined(EMSCRIPTEN)
    # include "emscripten/emscripten.h"
#endif

using namespace minko;
using namespace minko::component;
using namespace minko::math;
using namespace minko::extension;

using namespace trex;
using namespace trex::component;

int main(int argc, char** argv)
{
    if (TREX_ENABLE_PARTICLES)
    {
        SerializerExtension::activeExtension<extension::ParticlesExtension>();
    }

    std::srand((unsigned int) (std::time(nullptr)));

    auto canvas = Canvas::create("Oculus Rex", 1280, 720, Canvas::RESIZABLE);
    canvas->desiredFramerate(120.f);

    auto sceneManager = SceneManager::create(canvas->context());

    sceneManager->assets()->loader()->options()
        ->generateMipmaps(true)
        ->registerParser<file::PNGParser>("png")
        ->registerParser<file::JPEGParser>("jpg")
        ->registerParser<audio::SoundParser>("ogg")
        ->registerParser<file::SceneParser>("scene");

    auto fxLoader = file::Loader::create(sceneManager->assets()->loader());

    fxLoader
        ->queue("effect/Basic.effect")
        ->queue("effect/Phong.effect")
        ->queue("effect/Mirror.effect");

    if (TREX_ENABLE_PARTICLES)
    {
        fxLoader->queue("effect/Particles.effect");
    }

    auto fxComplete = fxLoader->complete()->connect([&](file::Loader::Ptr loader)
    {
        auto options = sceneManager->assets()->loader()->options();

        options->materialFunction([](const std::string&, material::Material::Ptr m)
        {
            auto phongMaterial = material::PhongMaterial::create();

            phongMaterial->copyFrom(m);
            phongMaterial
                ->fogType(render::FogType::Exponential)
                ->fogColor(TREX_FOG_COLOR)
                ->fogStart(TREX_ROAD_CHUNK_LENGTH / 2)
                ->fogEnd(TREX_FRONT_VIEW_DISTANCE - TREX_ROAD_CHUNK_LENGTH)
                ->fogDensity(1.0f);

            return phongMaterial;
        });

        options->effect(sceneManager->assets()->effect("effect/Phong.effect"));
        options->disposeTextureAfterLoading(true);
        //options->disposeVertexBufferAfterLoading(true);
        //options->disposeIndexBufferAfterLoading(true);
        sceneManager->assets()->loader()
            ->queue(TREX_ROAD_MAP)
            ->queue("sound/music.ogg")
            ->queue("sound/car_engine_loop_1.ogg")
            ->queue("sound/car_road_loop_1.ogg")
            ->queue("sound/car_hit_1.ogg")
            ->queue("sound/trex_eat.ogg")
            ->queue("sound/trex_step_close_1.ogg")
            ->queue("sound/trex_step_close_2.ogg")
            ->queue("sound/trex_step_close_3.ogg")
            ->queue("sound/trex_step_close_4.ogg")
            ->queue("sound/trex_step_close_5.ogg")
            ->queue("sound/trex_step_close_6.ogg")
            ->queue("sound/trex_roar_loud_1.ogg")
            ->queue("sound/trex_roar_loud_2.ogg")
            ->queue("sound/trex_roar_loud_4.ogg")
            ->queue("sound/trex_roar_loud_7.ogg")
            ->queue("sound/trex_roar_loud_9.ogg")
            ->queue("sound/trex_roar_loud_10.ogg")
            ->queue("sound/trex_roar_loud_11.ogg")
            ->queue("sound/trex_roar_loud_12.ogg")
            ->queue("sound/trex_roar_middle_1.ogg")
            ->queue("sound/trex_roar_middle_2.ogg")
            ->queue("sound/trex_roar_middle_3.ogg")
            ->queue("model/char_trex.scene")
            ->queue("model/map_block_a.scene")
            ->queue("model/map_block_b.scene")
            ->queue("model/map_block_c.scene")
            ->queue("model/map_block_d.scene")
            ->queue("model/map_block_e.scene")
            ->queue("model/item_trunk_a.scene")
            ->queue("model/item_trunk_b.scene")
            ->queue("model/item_trunk_c.scene")
            ->queue("model/item_trunk_d.scene")
            ->queue("model/misc_lightwell.scene")
            ->queue("model/vehicle_jeep.scene")
            ->queue("model/item_liana.scene")
            ->queue("texture/map_block_diff.jpg")
            ->queue("texture/map_block_nrm.jpg")
            ->queue("texture/map_block_alpha.jpg")
            ->queue("texture/map_block_spec.jpg")
            ->queue("texture/endscreen.png")
            ->queue("texture/firstscreen.png")
            ->queue("model/counter.scene");

        if (TREX_ENABLE_PARTICLES)
        {
            sceneManager->assets()->loader()
                ->queue("model/flying_dust.scene");
        }
        
        sceneManager->assets()->loader()->load();
    });

    auto root = scene::Node::create("root")
        ->addComponent(sceneManager);

    auto car     = scene::Node::create("car");
    auto dino    = scene::Node::create("dino");
    auto road    = scene::Node::create("road");
    auto mirrors = scene::Node::create("mirrors");

    car->addComponent(CarScript::create(canvas, root));
    dino->addComponent(DinoScript::create(root));
    road->addComponent(RoadScript::create(car));
    mirrors->addComponent(MirrorScript::create(sceneManager->assets(), sceneManager, canvas, root, car));

    auto ambientLight = scene::Node::create("ambientLight");
    ambientLight->addComponent(AmbientLight::create(0.05f));
    ambientLight->component<AmbientLight>()->color(0xE8EBFFFF);

    auto moonLight = scene::Node::create("moonLight");
    moonLight->addComponent(DirectionalLight::create(0.1f, 0.1f));
    moonLight->component<DirectionalLight>()->color(0xE8EBFFFF);

    moonLight->addComponent(Transform::create());
    moonLight->component<Transform>()->matrix()->lookAt(Vector3::create(0.f, 0.f, 0.f), Vector3::create(1.f, 1.f, 1.f));

    //root->addChild(ambientLight);
    //root->addChild(moonLight);

    std::shared_ptr<audio::SoundChannel> engine;
    std::shared_ptr<audio::SoundChannel> mud;

    sceneManager->assets()->geometry("cube", geometry::CubeGeometry::create(sceneManager->assets()->context()));
    sceneManager->assets()->geometry("quad", geometry::QuadGeometry::create(sceneManager->assets()->context()));

    auto _ = sceneManager->assets()->loader()->complete()->connect([&](file::Loader::Ptr loader)
    {
        auto symbol = sceneManager->assets()->symbol("model/char_trex.scene");
        root->addChild(symbol);

        root->addChild(car);
        root->addChild(dino);
        root->addChild(road);
        root->addChild(mirrors);

#ifdef CAR_RUMBLE_ENABLE
        auto rumble = scene::Node::create("rumble");
        rumble->addComponent(RumbleScript::create(car, road));
        root->addChild(rumble);
#endif

        engine = sceneManager->assets()->sound("sound/car_engine_loop_1.ogg")->play(0);
        mud = sceneManager->assets()->sound("sound/car_road_loop_1.ogg")->play(0);
    });

    auto keyDown = canvas->keyboard()->keyDown()->connect([&](input::Keyboard::Ptr k)
    {
        if (k->keyIsDown(input::Keyboard::R))
        {
#if defined(EMSCRIPTEN)
            emscripten_run_script("location.reload();");
#endif
        }
    });

    auto enterFrame = canvas->enterFrame()->connect([&](Canvas::Ptr canvas, float time, float deltaTime)
    {
        sceneManager->nextFrame(time, deltaTime);
    });

    fxLoader->load();
    canvas->run();

    return 0;
}
