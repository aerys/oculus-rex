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
#include "minko/MinkoSDL.hpp"
#include "trex/Config.hpp"

namespace trex
{
    namespace component
    {
        class CarScript : public minko::component::AbstractScript
        {
        public:
            typedef std::shared_ptr<CarScript>    Ptr;

        private:
            typedef minko::Signal<minko::AbstractCanvas::Ptr, minko::uint, minko::uint>::Slot       ResizedSlot;
            typedef minko::scene::Node::Ptr                                                         NodePtr;

            typedef minko::Signal<minko::AbstractCanvas::Ptr, minko::input::Joystick::Ptr>::Slot    JoystickSlot;
            typedef minko::Signal<minko::input::Joystick::Ptr, int, int, int>::Slot                 JoystickAxisMotionSlot;
            typedef minko::Signal<minko::input::Joystick::Ptr, int, int>::Slot                      JoystickButtonDownSlot;
            typedef minko::input::Joystick::Ptr                                                     JoystickPtr;

            typedef minko::component::Animation::Ptr                                                AnimationPtr;
            typedef minko::component::AbstractAnimation::Ptr                                        AbstractAnimationPtr;
            typedef minko::Signal<AbstractAnimationPtr, std::string, minko::uint>::Slot             AnimationLabelHitSlot;

            CarScript(minko::Canvas::Ptr, NodePtr);

        public:
            ~CarScript()
            {
            }

            static
            Ptr
            create(minko::Canvas::Ptr canvas, NodePtr root)
            {
                auto script = std::shared_ptr<CarScript>(new CarScript(canvas, root));
                
                script->initialize();

                return script;
            }

            inline
            float
            speed() const
            {
                return _speed;
            }

            inline
            void
            speed(float speed)
            {
                _speed = speed;
            }

            inline
            float
            deltaSpeed() const
            {
                return _speed - CAR_BASE_SPEED;
            }

            inline
            int
            lane()
            {
                return _lane;
            }

            void
            lockLane(int laneId, bool locked);

            minko::scene::Node::Ptr
            camera()
            {
                return _camera;
            }

            inline
            void
            obstacleHitCount(int obstacleHitCount)
            {
                _obstacleHitCount = obstacleHitCount;
            }

            inline
            int
            obstacleHitCount()
            {
                return _obstacleHitCount;
			}
			
            minko::scene::Node::Ptr
            getCarSymbol()
            {
                return _carSymbol;
            }

            inline
            void
            gameOver()
            {
                _gameOver = true;
            }

            inline
            bool
            gameStarted() const
            {
                return _gameStarted;
            }

            void
            moveCameraToNode(NodePtr);

            void
            eating(bool v)
            {
                _eating = v;
            }

        protected:
            void
            initialize();

            void
            start(NodePtr target);

            void
            update(NodePtr target);

            void
            stop(NodePtr target);

        private:
            void
            initCarSymbol();

            void
            initCarLaneAnimations();

            void
            initScore();

            void
            initCamera();

            void
            initParticles();
            
            void
            initJoysticks();

            void
            handleControls();

            void
            startGame();

            void
            turnLeft();

            void
            turnRight();

            void
            updateScoreBoard();

            void
            updateKmBoard();

        private:
            minko::component::SceneManager::Ptr     _sceneManager;
            minko::Canvas::Ptr                      _canvas;

            NodePtr                                 _target;
            NodePtr                                 _root;

            NodePtr                                 _carAnimatedNode;
            NodePtr                                 _carSymbol;

            AnimationPtr                            _carLaneAnimation;
            AnimationLabelHitSlot                   _carLaneAnimationLabelHit;

            NodePtr                                 _camera;
            NodePtr                                 _cameraContainer;
            NodePtr                                 _cameraAnimContainer;
            ResizedSlot                             _resizedSlot;

            JoystickPtr                             _joystick;
            JoystickSlot                            _joystickAdded;
            JoystickSlot                            _joystickRemoved;
            JoystickAxisMotionSlot                  _joystickAxisMotion;
            JoystickButtonDownSlot                  _joystickButtonDown;

            std::string                             _currentScreen;

            minko::scene::Node::Ptr                 _screenQuad;

            bool                                    _gameStarted;
            bool                                    _gameOver;
            bool                                    _oculusDetected;

            float                                   _joyLX;
            float                                   _joyLY;
            float                                   _joyRX;
            float                                   _joyRY;

            float                                   _speed; //in km/h
            int                                     _lane;

            bool                                    _leftDown;
            bool                                    _rightDown;

            bool                                    _leftCameraDown;
            bool                                    _rightCameraDown;

            float                                   _cameraVAngle;

            int                                     _lockedLane;

            int                                     _obstacleHitCount;
            std::array<NodePtr, 5>                  _digitNodes;
            std::array<NodePtr, 5>                  _kmDigitNodes;

            bool                                    _eating;
        };
    }
}