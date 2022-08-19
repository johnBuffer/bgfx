/*
 * Copyright 2011-2022 Branimir Karadzic. All rights reserved.
 * License: https://github.com/bkaradzic/bgfx/blob/master/LICENSE
 */

#include <bx/uint32_t.h>
#include "common.h"
#include "bgfx_utils.h"
#include "logo.h"
#include "imgui/imgui.h"
#include <vector>

namespace
{

// Dummy Vertex struct
struct Vertex {
    // 2D space position
    float x = 0.0f;
    float y = 0.0f;
    // Color value
    uint32_t color = 0;
    // Texture coords;
    float tex_x = 0.0f;
    float tex_y = 0.0f;

    Vertex() = default;

    static void init() {
        // start the attribute declaration
        s_layout.begin()
                    .add(bgfx::Attrib::Position,  2, bgfx::AttribType::Float)
                    .add(bgfx::Attrib::Color0,    4, bgfx::AttribType::Uint8, true)
                    .add(bgfx::Attrib::TexCoord0, 2, bgfx::AttribType::Float)
                .end();
    };

    static bgfx::VertexLayout s_layout;
};

bgfx::VertexLayout Vertex::s_layout;

// Utils functions
void resizeBuffer(uint32_t size, std::vector<Vertex>& vertices, std::vector<uint32_t>& indexes)
{
    vertices.resize(size);
    indexes.resize(size);
}

void updateBufferData(std::vector<Vertex>& vertices, std::vector<uint32_t>& indexes, bgfx::DynamicVertexBufferHandle vbh, bgfx::DynamicIndexBufferHandle ibh)
{
    bgfx::update(vbh, 0, bgfx::copy(vertices.data(), static_cast<uint32_t>(vertices.size()) * sizeof(Vertex)));
    bgfx::update(ibh, 0, bgfx::copy(indexes.data(), static_cast<uint32_t>(indexes.size()) * sizeof(uint32_t)));
}

class ExampleHelloWorld : public entry::AppI
{
public:
    std::vector<Vertex>   vertices;
    std::vector<uint32_t> indexes;
    bgfx::DynamicVertexBufferHandle vbh;
    bgfx::DynamicIndexBufferHandle  ibh;
    
    uint32_t object_count = 53000;

    ExampleHelloWorld(const char* _name, const char* _description, const char* _url)
        : entry::AppI(_name, _description, _url)
    {
    }

    void init(int32_t _argc, const char* const* _argv, uint32_t _width, uint32_t _height) override
    {
        Args args(_argc, _argv);

        m_width  = _width;
        m_height = _height;
        m_debug  = BGFX_DEBUG_TEXT;
        m_reset  = BGFX_RESET_VSYNC;

        bgfx::Init init;
        init.type     = args.m_type;
        init.vendorId = args.m_pciId;
        init.resolution.width  = m_width;
        init.resolution.height = m_height;
        init.resolution.reset  = m_reset;
        bgfx::init(init);

        // Enable debug text.
        bgfx::setDebug(m_debug);

        // Set view 0 clear state.
        bgfx::setViewClear(0
            , BGFX_CLEAR_COLOR|BGFX_CLEAR_DEPTH
            , 0x303030ff
            , 1.0f
            , 0
            );

        imguiCreate();
        
        Vertex::init();
        
        vbh = bgfx::createDynamicVertexBuffer(uint32_t{0}, Vertex::s_layout, BGFX_BUFFER_ALLOW_RESIZE);
        ibh = bgfx::createDynamicIndexBuffer(uint32_t{0}, BGFX_BUFFER_ALLOW_RESIZE | BGFX_BUFFER_INDEX32);
    }

    virtual int shutdown() override
    {
        imguiDestroy();

        // Shutdown bgfx.
        bgfx::shutdown();

        return 0;
    }

    bool update() override
    {
        if (!entry::processEvents(m_width, m_height, m_debug, m_reset, &m_mouseState) )
        {
            imguiBeginFrame(m_mouseState.m_mx
                ,  m_mouseState.m_my
                , (m_mouseState.m_buttons[entry::MouseButton::Left  ] ? IMGUI_MBUT_LEFT   : 0)
                | (m_mouseState.m_buttons[entry::MouseButton::Right ] ? IMGUI_MBUT_RIGHT  : 0)
                | (m_mouseState.m_buttons[entry::MouseButton::Middle] ? IMGUI_MBUT_MIDDLE : 0)
                ,  m_mouseState.m_mz
                , uint16_t(m_width)
                , uint16_t(m_height)
                );

            showExampleDialog(this);

            imguiEndFrame();

            // Set view 0 default viewport.
            bgfx::setViewRect(0, 0, 0, uint16_t(m_width), uint16_t(m_height) );
            
            object_count += object_count < 55000;
            const uint32_t vertex_count = 3 * object_count;
            resizeBuffer(vertex_count, vertices, indexes);
            updateBufferData(vertices, indexes, vbh, ibh);

            // This dummy draw call is here to make sure that view 0 is cleared
            // if no other draw calls are submitted to view 0.
            bgfx::touch(0);
            
            bgfx::dbgTextPrintf(0, 0, 0x0f, "Vertices count %d", object_count);

            // Advance to next frame. Rendering thread will be kicked to
            // process submitted rendering primitives.
            bgfx::frame();

            return true;
        }

        return false;
    }

    entry::MouseState m_mouseState;

    uint32_t m_width;
    uint32_t m_height;
    uint32_t m_debug;
    uint32_t m_reset;
};

} // namespace

ENTRY_IMPLEMENT_MAIN(
      ExampleHelloWorld
    , "00-helloworld"
    , "Initialization and debug text."
    , "https://bkaradzic.github.io/bgfx/examples.html#helloworld"
    );
