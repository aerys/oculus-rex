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
#include "minko/Signal.hpp"
#include "minko/MinkoSDL.hpp"

// Minko forward declarations
namespace minko
{
    namespace render
    {
        class Texture;
        class Renderer;
        class Effect;
    }

    namespace scene
    {
        class Node;
    }

    namespace component
    {
        class PerspectiveCamera;
        class Transform;
        class SceneManager;
    }
}

namespace trex
{

    namespace component
    {

        class MirrorScript : public minko::component::AbstractScript
        {
        public:
            typedef std::shared_ptr<MirrorScript>    Ptr;

        private:
            typedef minko::Signal<minko::AbstractCanvas::Ptr, minko::uint, minko::uint>::Slot       ResizedSlot;
            typedef minko::scene::Node::Ptr                                                         NodePtr;

            MirrorScript(
                std::shared_ptr<minko::file::AssetLibrary>, 
                std::shared_ptr<minko::component::SceneManager>,
                minko::Canvas::Ptr,
                NodePtr, 
                NodePtr
            );

        public:
            ~MirrorScript()
            {
            }

            static
            Ptr
            create(std::shared_ptr<minko::file::AssetLibrary> assets, std::shared_ptr<minko::component::SceneManager> sceneManager, minko::Canvas::Ptr canvas, NodePtr root, NodePtr car)
            {
                auto script = std::shared_ptr<MirrorScript>(new MirrorScript(assets, sceneManager, canvas, root, car));
                
                script->initialize();

                return script;
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
            initMirrors();

        private:
            std::shared_ptr<minko::file::AssetLibrary>                  _assets;
            minko::Canvas::Ptr                                          _canvas;
            std::shared_ptr<minko::component::SceneManager>             _sceneManager;
            NodePtr                                                     _target;
            NodePtr                                                     _root;

            NodePtr                                                     _camera;
            ResizedSlot                                                 _resizedSlot;
            NodePtr                                                     _car;
            NodePtr                                                     _leftMirror;
            NodePtr                                                     _rightMirror;

            std::shared_ptr<minko::render::Texture>                    _renderTarget;
            std::shared_ptr<minko::scene::Node>                        _virtualCamera;
            std::shared_ptr<minko::scene::Node>                        _virtualCameraRight;
            std::shared_ptr<minko::component::PerspectiveCamera>       _perspectiveCamera;
            std::shared_ptr<minko::component::Transform>               _cameraTransform;
            std::shared_ptr<minko::component::Transform>               _virtualCameraTransform;
            std::shared_ptr<minko::render::Renderer>                   _reflectionRenderer;
            std::shared_ptr<minko::math::Matrix4x4>                    _reflectedViewMatrix;
            std::shared_ptr<minko::render::Effect>                     _reflectionEffect;

            minko::Signal<std::shared_ptr<minko::component::SceneManager>, float, float>::Slot _frameBeginSlot;
        };
    }
}