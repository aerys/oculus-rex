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

#include "trex/Config.hpp"
#include "MirrorScript.hpp"
#include "minko/component/Renderer.hpp"
#include "minko/geometry/Geometry.hpp"
#include "minko/material/Material.hpp"
#include "minko/scene/Node.hpp"
#include "minko/math/Matrix4x4.hpp"
#include "minko/component/PerspectiveCamera.hpp"
#include "minko/file/AssetLibrary.hpp"
#include "minko/math/Vector3.hpp"
#include "minko/scene/Node.hpp"
#include "minko/component/Transform.hpp"
#include "minko/component/Renderer.hpp"
#include "minko/render/Texture.hpp"
#include "minko/render/Effect.hpp"
#include "minko/component/SceneManager.hpp"
#include "CarScript.hpp"

using namespace minko;
using namespace minko::component;
using namespace minko::scene;
using namespace minko::math;
using namespace minko::input;
using namespace minko::animation;
using namespace minko::geometry;
using namespace minko::material;
using namespace trex::component;

MirrorScript::MirrorScript(
    std::shared_ptr<file::AssetLibrary> assets,
    std::shared_ptr<minko::component::SceneManager> sceneManager,
    Canvas::Ptr canvas,
    NodePtr root,
    NodePtr car) :
    _target(nullptr),
    _assets(assets),
    _sceneManager(sceneManager),
    _canvas(canvas),
    _root(root),
    _car(car),
    _leftMirror(nullptr),
    _rightMirror(nullptr),
    _frameBeginSlot(nullptr)
{
}

void
MirrorScript::initialize()
{
    AbstractScript::initialize();

    _renderTarget = render::Texture::create(
        _assets->context(),
        128, 128, false, true
        );
}

void
MirrorScript::start(scene::Node::Ptr target)
{
    std::cout << "MirrorScript start" << std::endl;

    if (_target != nullptr)
        throw;

    _target = target;

    if (!_target->hasComponent<Transform>())
        _target->addComponent(Transform::create());

    initMirrors();
}

void
MirrorScript::initMirrors()
{
    auto carSymbol = _car->component<CarScript>()->getCarSymbol();

    auto nodeSet = scene::NodeSet::create(carSymbol)
        ->descendants(false)
        ->where([=](scene::Node::Ptr n)
    {
        return n->name() == "left_mirror";
    });

    auto nodeSetRight = scene::NodeSet::create(carSymbol)
        ->descendants(false)
        ->where([=](scene::Node::Ptr n)
    {
        return n->name() == "right_mirror";
    });

    auto leftMirrorNode = nodeSet->nodes()[0];
    auto leftMirrorTransform = leftMirrorNode->component<Transform>();
    auto rightMirrorNode = nodeSetRight->nodes()[0];
    auto rightMirrorTransform = rightMirrorNode->component<Transform>();

    auto applyReflectionEffect = _assets->effect("effect/Mirror.effect");

    // Create a virtual camera
    auto virtualPerspectiveCameraComponent = PerspectiveCamera::create(
        (float)_canvas->width() / (float)_canvas->height(), 0.78f, .01f, 1000.f);

    auto clearColor = 0x050514ff;
    _reflectionEffect = _assets->effect("effect/Phong.effect");

    auto renderer = Renderer::create(clearColor, _renderTarget, _reflectionEffect, 1000000.f, "Reflection");

    auto leftMirrorPosition = leftMirrorTransform->matrix()->transform(Vector3::create());
    leftMirrorPosition->scaleBy(.01f);
    leftMirrorPosition->x(-1.f * leftMirrorPosition->x());
    leftMirrorPosition->z((-1.f * leftMirrorPosition->z()) + 0.2f);
    leftMirrorPosition->y(leftMirrorPosition->y() + 0.2f);
    auto leftMirrorTarget = Vector3::create(leftMirrorPosition);
    leftMirrorTarget->z(leftMirrorTarget->z() + 1.f);

    _virtualCamera = scene::Node::create("virtualCamera")
        ->addComponent(renderer)
        ->addComponent(virtualPerspectiveCameraComponent)
        ->addComponent(Transform::create(
            Matrix4x4::create()->lookAt(
                leftMirrorTarget,
                leftMirrorPosition
            )
        ));

    carSymbol->addChild(_virtualCamera);

    auto leftMirrorSurface = leftMirrorNode->component<Surface>();
    auto rightMirrorSurface = rightMirrorNode->component<Surface>();

    leftMirrorSurface->material()->set("diffuseMap", _renderTarget);
    leftMirrorSurface->effect(applyReflectionEffect);
    rightMirrorSurface->material()->set("diffuseMap", _renderTarget);
    rightMirrorSurface->effect(applyReflectionEffect);
}

void
MirrorScript::update(scene::Node::Ptr target)
{
    if (target != _target)
        return;
}

void
MirrorScript::stop(scene::Node::Ptr target)
{
    std::cout << "MirrorScript stop" << std::endl;

    if (_target == target)
        _target = nullptr;
}
