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
#include "CarScript.hpp"
#include "minko/MinkoOculus.hpp"
#include "minko/component/Renderer.hpp"
#include "minko/scene/Node.hpp"
#include "trex/Config.hpp"
#include "minko/material/BasicMaterial.hpp"

#if defined(EMSCRIPTEN)
    # include "emscripten/emscripten.h"
#endif

using namespace minko;
using namespace minko::component;
using namespace minko::scene;
using namespace minko::math;
using namespace minko::input;
using namespace minko::animation;
using namespace trex::component;

CarScript::CarScript(Canvas::Ptr canvas, NodePtr root) :
    _target(nullptr),
    _sceneManager(nullptr),
    _speed(0.0f),
    _lane((NUM_LANES - 1) / 2),
    _canvas(canvas),
    _leftDown(false),
    _rightDown(false),
    _leftCameraDown(false),
    _rightCameraDown(false),
    _cameraVAngle(0.f),
    _joystick(nullptr),
    _joyLX(0.f),
    _joyLY(0.f),
    _joyRX(0.f),
    _joyRY(0.f),
    _root(root),
    _lockedLane(-1),
    _obstacleHitCount(0),
    _gameStarted(false),
    _gameOver(false),
    _oculusDetected(false),
    _currentScreen("texture/firstscreen.png"),
    _screenQuad(nullptr),
    _eating(false)
{
}

void
CarScript::initialize()
{
    AbstractScript::initialize();

    _screenQuad = scene::Node::create();

    initJoysticks();
    initCarLaneAnimations();
}

void
CarScript::initJoysticks()
{
    _joystickAdded = _canvas->joystickAdded()->connect([&](AbstractCanvas::Ptr abscanvas, input::Joystick::Ptr joystick)
    {
        if (_joystick != nullptr)
            return;

        _joystick = joystick;

        _joystickAxisMotion = _joystick->joystickAxisMotion()->connect([&](JoystickPtr joystick, int which, int axis, int value)
        {
            auto v = (float)(value) / 33000.f;

            if (axis == AXIS_LX)
                _joyLX = v;
            if (axis == AXIS_LY)
                _joyLY = v;

            if (axis == AXIS_RX)
                _joyRX = v;
            if (axis == AXIS_RY)
                _joyRY = v;
        });
        
        _joystickButtonDown = _joystick->joystickButtonDown()->connect([&](JoystickPtr joystick, int which, int button)
        {
            if (button == (int)Joystick::Button::A || button == (int)Joystick::Button::Start)
                startGame();
        });
    });

    _joystickRemoved = _canvas->joystickRemoved()->connect([&](AbstractCanvas::Ptr canvas, JoystickPtr joystick)
    {
        if (_joystick == joystick)
            _joystick = nullptr;
    });
}

void
CarScript::lockLane(int laneId, bool locked)
{
    _lockedLane = locked ? laneId : -1;
}

void
CarScript::start(scene::Node::Ptr target)
{
    std::cout << "CarScript start" << std::endl;

    if (_target != nullptr)
        throw;

    _target = target;

    if (!_target->hasComponent<Transform>())
        _target->addComponent(Transform::create());

    _sceneManager = _target->root()->component<SceneManager>();

    if (TREX_ENABLE_PARTICLES)
        initParticles();

    initCarSymbol();
    initCamera();

#ifdef CAR_SCORE_ENABLE
    initScore();
#endif

}

void
CarScript::initScore()
{
    auto nodeSet = scene::NodeSet::create(_carSymbol)
        ->descendants(false)
        ->where([=](scene::Node::Ptr n)
    {
        return n->name() == "dummy_counter";
    });

    auto digitDummyNode = nodeSet->nodes()[0];
    auto savePosition = digitDummyNode->component<Transform>()->matrix()->transform(Vector3::create());
    auto digitDummyNodeParent = digitDummyNode->parent();
    digitDummyNodeParent->removeChild(digitDummyNode);

    auto digitBoard = _sceneManager->assets()->symbol("model/counter.scene");
    auto digitBoardTransform = digitBoard->component<Transform>();

    auto digits = scene::NodeSet::create(digitBoard)
        ->descendants(false)
        ->where([=](scene::Node::Ptr n)
    {
        return n->hasComponent<Surface>();
    });
    
    for (auto node : digits->nodes())
    {
        auto basicMaterial = std::static_pointer_cast<minko::material::BasicMaterial>(node->component<Surface>()->material());
        basicMaterial->blendingMode(render::Blending::Mode::ALPHA);
        node->component<Surface>()->effect(_sceneManager->assets()->effect("effect/Basic.effect"));

        if (node->name() == "digit_5")
            _digitNodes[4] = node;
        else if (node->name() == "digit_4")
            _digitNodes[3] = node;
        else if (node->name() == "digit_3")
            _digitNodes[2] = node;
        else if (node->name() == "digit_2")
            _digitNodes[1] = node;
        else if (node->name() == "digit_1")
            _digitNodes[0] = node;

        else if (node->name() == "km_digit_1")
            _kmDigitNodes[0] = node;
        else if (node->name() == "km_digit_2")
            _kmDigitNodes[1] = node;

    }


    digitDummyNodeParent->addChild(digitBoard);

}

void
CarScript::initParticles()
{
    auto dustParticles = _sceneManager->assets()->symbol("model/flying_dust.scene");
    _target->addChild(dustParticles);
}

void
CarScript::initCarSymbol()
{
    _carSymbol = _sceneManager->assets()->symbol("model/vehicle_jeep.scene");

    if (!_carSymbol->hasComponent<Transform>())
        _carSymbol->addComponent(Transform::create());

    _carSymbol->component<Transform>()->matrix()->prependRotationY(float(M_PI));

    _carAnimatedNode->addChild(_carSymbol);
    _target->addChild(_carAnimatedNode);

    _carLaneAnimation->seek(0)->stop();
}

void
CarScript::initCarLaneAnimations()
{
    _carAnimatedNode = Node::create();
    _carAnimatedNode->addComponent(Transform::create());

    auto lane0 = Matrix4x4::create()->appendTranslation(LANE_WIDTH, 0.f, 0.f);
    auto lane1 = Matrix4x4::create();
    auto lane2 = Matrix4x4::create()->appendTranslation(-LANE_WIDTH, 0.f, 0.f);

    auto lane0to1 = Matrix4x4::create()->prependRotationY(float(-M_PI_4) / 5.5f)->appendTranslation(LANE_WIDTH / 2, 0.f, 0.f);
    auto lane1to0 = Matrix4x4::create()->prependRotationY(float(M_PI_4) / 5.5f)->appendTranslation(LANE_WIDTH / 2, 0.f, 0.f);

    auto lane1to2 = Matrix4x4::create()->prependRotationY(float(-M_PI_4) / 5.5f)->appendTranslation(-LANE_WIDTH / 2, 0.f, 0.f);
    auto lane2to1 = Matrix4x4::create()->prependRotationY(float(M_PI_4) / 5.5f)->appendTranslation(-LANE_WIDTH / 2, 0.f, 0.f);

    std::vector<Matrix4x4::Ptr> matrices = { lane1, lane1to0, lane0, lane0to1, lane1, lane1to2, lane2, lane2to1, lane1};
    std::vector<uint> timetable;

    for (int i = 0; i < 9; ++i)
    {
        timetable.push_back(i * LANE_CHANGE_DURATION / 2);
    }

    auto timeline = animation::Matrix4x4Timeline::create("transform.matrix", 3000, timetable, matrices, true);
    _carLaneAnimation = Animation::create({ timeline });

    auto halfDuration = static_cast<unsigned int>(LANE_CHANGE_DURATION * 0.5f);

    _carLaneAnimation->addLabel("1to0_start", 0);
    _carLaneAnimation->addLabel("1to0", halfDuration);
    _carLaneAnimation->addLabel("1to0_end", LANE_CHANGE_DURATION);

    _carLaneAnimation->addLabel("0to1_start", LANE_CHANGE_DURATION);
    _carLaneAnimation->addLabel("0to1", LANE_CHANGE_DURATION + halfDuration);
    _carLaneAnimation->addLabel("0to1_end", LANE_CHANGE_DURATION * 2);

    _carLaneAnimation->addLabel("1to2_start", LANE_CHANGE_DURATION * 2);
    _carLaneAnimation->addLabel("1to2", LANE_CHANGE_DURATION * 2 + halfDuration);
    _carLaneAnimation->addLabel("1to2_end", LANE_CHANGE_DURATION * 3);

    _carLaneAnimation->addLabel("2to1_start", LANE_CHANGE_DURATION * 3);
    _carLaneAnimation->addLabel("2to1", LANE_CHANGE_DURATION * 3 + halfDuration);
    _carLaneAnimation->addLabel("2to1_end", LANE_CHANGE_DURATION * 4);
    
    _carLaneAnimation->isLooping(false);

    _carLaneAnimationLabelHit = _carLaneAnimation->labelHit()->connect([&](AbstractAnimationPtr, std::string name, uint time)
    {
        if (name == "1to0" && _lane != 0)
            _lane = 0;
        else if ((name == "0to1" || name == "2to1") && _lane != 1)
            _lane = 1;
        else if (name == "1to2" && _lane != 2)
            _lane = 2;
        else
            return;
        
        std::cout << "car on lane " << _lane << std::endl;
    });

    _carAnimatedNode->addComponent(_carLaneAnimation->seek(0)->stop());

    _root->addChild(_carAnimatedNode);
}

void
CarScript::initCamera()
{
	auto node = scene::Node::create();
		
	_carSymbol->addChild(node);

    _cameraAnimContainer = scene::Node::create("cameraAnimContainer")->addComponent(Transform::create());
    _cameraContainer = scene::Node::create("cameraContainer")->addComponent(Transform::create());
    _camera = scene::Node::create("camera")->addComponent(Transform::create());
    _oculusDetected = OculusVRCamera::detected();

    if (_oculusDetected)
    {
        node->addComponent(Transform::create(Matrix4x4::create()->lookAt(
            Vector3::create(-CAR_WIDTH / 5.f, 1.5f, -1.f), Vector3::create(-CAR_WIDTH / 5.f, 1.5f, 0.25f)
        )));

        _camera->addComponent(OculusVRCamera::create(_canvas->width(), _canvas->height(), 0.1f, 100.0f));
    }
    else
    {
        node->addComponent(Transform::create(Matrix4x4::create()->lookAt(
            Vector3::create(-CAR_WIDTH / 5.f, 1.4f, -1.f), Vector3::create(-CAR_WIDTH / 5.f, 1.4f, 0.30f)
        )));

        _camera
            ->addComponent(PerspectiveCamera::create(_canvas->aspectRatio(), 1.0f))
            ->addComponent(Renderer::create(0x050514ff));
    }

    _cameraAnimContainer->addChild(_camera);
    _cameraContainer->addChild(_cameraAnimContainer);
    node->addChild(_cameraContainer);
   
   _screenQuad = scene::Node::create("screenQuad")
       ->addComponent(Transform::create(Matrix4x4::create()->appendScale(0.75f)->appendTranslation(0.f, 0.f, -.25f)))
       ->addComponent(Surface::create(
            geometry::QuadGeometry::create(_sceneManager->assets()->context()),
            material::BasicMaterial::create()->diffuseMap(_sceneManager->assets()->texture(_currentScreen)),
            _sceneManager->assets()->effect("effect/Basic.effect")
          )
        );

   _camera->addChild(_screenQuad);


   _resizedSlot = _canvas->resized()->connect([&](AbstractCanvas::Ptr canvas, uint w, uint h)
   {
       if (_camera->hasComponent<OculusVRCamera>())
           _camera->component<OculusVRCamera>()->updateViewport(w, h);
       else
       {
           auto ratio = float(w) / float(h);

           if (_camera->component<PerspectiveCamera>()->aspectRatio() != ratio)
               _camera->component<PerspectiveCamera>()->aspectRatio(ratio);
       }
   });
}

void
CarScript::moveCameraToNode(NodePtr node)
{
    auto previousPosition = _camera->component<Transform>()->modelToWorldMatrix(true)->translation();

    auto worldParentMatrix = node->parent()->component<Transform>()->modelToWorldMatrix(true);
    auto worldToModelContainer = _cameraContainer->parent()->component<Transform>()->modelToWorldMatrix(true)->invert();

    worldToModelContainer = worldParentMatrix->append(worldToModelContainer);

    _cameraContainer->component<Transform>()->matrix()->copyFrom(worldToModelContainer);
    _cameraAnimContainer->component<Transform>()->matrix()->copyFrom(node->component<Transform>()->matrix());

    auto anim = node->component<Animation>();
    node->removeComponent(anim);
    _cameraAnimContainer->addComponent(anim);

    auto pos = Vector3::create(previousPosition->x(), 1.5f, _target->component<Transform>()->z() - 0.3f);
    auto lookAt = Vector3::create(previousPosition->x(), 1.5f, _target->component<Transform>()->z() - 10.0f);
    
    auto transform = _cameraAnimContainer->component<Transform>();
    transform->modelToWorldMatrix(true);

    pos = transform->worldToModel(pos);
    lookAt = transform->worldToModel(lookAt);

    _camera->component<Transform>()->matrix()->copyFrom(Matrix4x4::create()->lookAt(lookAt, pos));
    auto worldMatrix = _camera->component<Transform>()->modelToWorldMatrix(true);

    _camera->component<Transform>()->matrix()->prependScale(-1.0f / worldMatrix->data()[0], 1.0f / worldMatrix->data()[5], -1.0f / worldMatrix->data()[10]);
 

    std::cout << _camera->component<Transform>()->modelToWorldMatrix(true)->toString() << std::endl;
}

void
CarScript::handleControls()
{
    auto leftPressed = _canvas->keyboard()->keyIsDown(Keyboard::Key::LEFT) || (-_joyLX > MOVE_THRESHOLD);
    auto rightPressed = _canvas->keyboard()->keyIsDown(Keyboard::Key::RIGHT) || (_joyLX > MOVE_THRESHOLD);

    if (!_gameOver)
    {
        if (_canvas->keyboard()->keyIsDown(Keyboard::Key::F))
            startGame();

        if (_canvas->keyboard()->keyIsDown(Keyboard::Key::END))
        {
            _gameOver = true;
        }
    }

    if (_canvas->keyboard()->keyIsDown(Keyboard::Key::ESCAPE) && _gameOver)
    {
        auto position = _carSymbol->component<Transform>()->modelToWorld(Vector3::create());
        auto z = int(position->z());

#if defined(EMSCRIPTEN)
        std::string eval = "gameOver(" + std::to_string(z) + ");";
        emscripten_run_script_int(eval.c_str());
#endif
    }

    if (_canvas->keyboard()->keyIsDown(Keyboard::Key::ESCAPE))
    {
        _canvas->quit();
    }

    if (_canvas->keyboard()->keyIsDown(Keyboard::Key::E) && !_leftCameraDown)
    {
        _leftCameraDown = true;
        _joyRX = -1.0f;
    }
    else if (!_canvas->keyboard()->keyIsDown(Keyboard::Key::E) && _leftCameraDown)
    {
        _joyRX = 0.f;
        _leftCameraDown = false;
    }
    

    if (_canvas->keyboard()->keyIsDown(Keyboard::Key::R) && !_rightCameraDown)
    {
        _joyRX = 1.0f;
        _rightCameraDown = true;
    }
    else if (!_canvas->keyboard()->keyIsDown(Keyboard::Key::R) && _rightCameraDown)
    {
        _joyRX = 0.f;
        _rightCameraDown = false;
    }
    
    if (leftPressed && !_leftDown)
    {
        turnLeft();
    }
    else if (rightPressed && !_rightDown)
    {
        turnRight();
    }

    _leftDown = leftPressed;
    _rightDown = rightPressed;

    if (_oculusDetected)
        return;

    auto dtRatio = (deltaTime() / 16.f) / 18.f;

    auto axis = Vector3::create(0.f, 0.f, 0.f);

    if (std::abs(_joyRX) > CAMERA_THRESHOLD)
    {
        axis->y(-_joyRX);
        axis = _camera->component<Transform>()->deltaWorldToModel(axis)->normalize();
    }

    auto angle = dtRatio * -_joyRY;

    if (std::abs(_joyRY) > CAMERA_THRESHOLD)// && !((_cameraVAngle >= CAMERA_V_LIMIT && angle > 0.f) || (_cameraVAngle <= 0.f && angle < 0.f)))
    {
        /*if (_cameraVAngle + angle > CAMERA_V_LIMIT)
        {
            angle = CAMERA_V_LIMIT - _cameraVAngle;
            _cameraVAngle = CAMERA_V_LIMIT;
        }
        else if (_cameraVAngle + angle < 0.f)
        {
            angle = 0.f - _cameraVAngle;
            _cameraVAngle = 0.f;
        }
        else
        {
            _cameraVAngle += angle;
        }*/

        _camera->component<Transform>()->matrix()->prependRotationX(angle);
    }

    if (axis->length() != 0.f)
        _camera->component<Transform>()->matrix()->prependRotation(dtRatio, axis);
}

void
CarScript::startGame()
{
    if (!_gameStarted && !_gameOver)
    {
        _gameStarted = true;
        if (_camera->contains(_screenQuad))
            _screenQuad->component<Surface>()->visible(false);
    }
}

void
CarScript::turnLeft()
{
    if (_speed <= 0.0f)
        return;

    if (_lane - 1 != _lockedLane && _lane > 0 && !_carLaneAnimation->isPlaying())
    {
        std::string laneChange = std::to_string(_lane) + "to" + std::to_string((_lane - 1));

        _carLaneAnimation->stop()->setPlaybackWindow(laneChange + "_start", laneChange + "_end")->seek(laneChange + "_start")->play();
    }

}

void
CarScript::turnRight()
{
    if (_speed <= 0.0f)
        return;

    if (_lane + 1 != _lockedLane && _lane < (NUM_LANES - 1) && !_carLaneAnimation->isPlaying())
    {
        std::string laneChange = std::to_string(_lane) + "to" + std::to_string((_lane + 1));

        _carLaneAnimation->stop()->setPlaybackWindow(laneChange + "_start", laneChange + "_end")->seek(laneChange + "_start")->play();
    }
}

void
CarScript::update(scene::Node::Ptr target)
{
    if (target != _target)
        return;

    auto dz = (_speed / 3600.f) * deltaTime();

    if (_gameStarted && !_eating)
        _target->component<Transform>()->matrix()->appendTranslation(0.f, 0.f, dz);
    else if (_eating)
    {
        _carSymbol->component<Transform>()->matrix()->appendTranslation(0.f, 0.f, -dz);

        _speed -= 0.5f;
    }

    handleControls();

#ifdef CAR_SCORE_ENABLE
    updateScoreBoard();
    updateKmBoard();
#endif
    static bool displayquad = true;

    if (_gameOver && displayquad)
    {
        std::cout << "gameover" << std::endl;
        _screenQuad->component<Surface>()->material()->set("diffuseMap", _sceneManager->assets()->texture("texture/endscreen.png"));
        _screenQuad->component<Surface>()->visible(true);
        //_camera->addChild(_screenQuad);
        _gameOver = false;
    }
}

void
CarScript::stop(scene::Node::Ptr target)
{
    std::cout << "CarScript stop" << std::endl;

    if (_target == target)
        _target = nullptr;
}

void
CarScript::updateScoreBoard()
{
    auto position = _carSymbol->component<Transform>()->modelToWorld(Vector3::create());
    auto z = (int)position->z();
    
    int counter = 0;
    while(z != 0)
    {
        auto digit = z % 10;

        auto basicMaterial = std::static_pointer_cast<minko::material::BasicMaterial>(
            _digitNodes[counter]->component<Surface>()->material()
        );
        basicMaterial->uvOffset(0.1f * digit, 0);

        counter++;
        z /= 10;
    }
}

void
CarScript::updateKmBoard()
{
    auto speed = int(_speed);
    int counter = 0;
    while (speed != 0)
    {
        auto digit = speed % 10;

        auto basicMaterial = std::static_pointer_cast<minko::material::BasicMaterial>(
            _kmDigitNodes[counter]->component<Surface>()->material()
        );
        basicMaterial->uvOffset(0.1f * digit, 0.5f);

        counter++;
        speed /= 10;
    }
}
