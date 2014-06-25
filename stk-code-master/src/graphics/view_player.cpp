#include "graphics/view_player.hpp"

#include "config/user_config.hpp"
#include "graphics/irr_driver.hpp"
#include "io/file_manager.hpp"
#include "utils/log.hpp"

#include <sstream>
#include <cstring>
#include <cmath>
#include <string>

// ShaderPath is for the pixel shader
ViewPlayer::ViewPlayer(IrrlichtDevice *device, int nbViews, bool leftInterlacing, int viewsPerTexture) :
                                                            m_device(device),
                                                            m_nbViews(nbViews),
                                                            m_3DOn(true),
                                                            m_interocularDistance(0.05),
                                                            m_SVAlg(false),
                                                            m_firstView(0),
                                                            m_viewsPerTexture(viewsPerTexture)
{
    if (leftInterlacing)
        leftInterlacing = 1;
    else
        leftInterlacing = -1;

    if (nbViews == 5)
        m_leftInterlacing = true;

    core::dimension2du screenSize = device->getVideoDriver()->getScreenSize() / sqrt(nbViews);

    for(int i = 0 ; i < nbViews ; i++)
    {
        m_textures[i] = m_device->getVideoDriver()->addRenderTargetTextureWithDepthBuffer(&m_zBuffers[i], screenSize);
        beginCapture(i);
    }

    endCapture();

    video::IGPUProgrammingServices* gpu = m_device->getVideoDriver()->getGPUProgrammingServices();

    s32 interlacingShader = 0;

    if (gpu)
    {
        if (m_SVAlg)
        {
            interlacingShader = gpu->addHighLevelShaderMaterialFromFiles(   (file_manager->getShaderDir() + "shader3D.vert").c_str(),  "main", irr::video::EVST_VS_1_1,
                                                                            (file_manager->getShaderDir() + "SVAlg.frag").c_str(),     "main", irr::video::EPST_PS_1_1,
                                                                            this, irr::video::EMT_SOLID);
        }

        else
        {
            interlacingShader = gpu->addHighLevelShaderMaterialFromFiles(   (file_manager->getShaderDir() + "shader3D.vert").c_str(), "main", irr::video::EVST_VS_1_1,
                                                                            (file_manager->getShaderDir() + "shader3D.frag").c_str(), "main", irr::video::EPST_PS_1_1,
                                                                            this, irr::video::EMT_SOLID);
        }
    }



    m_material.MaterialType = (video::E_MATERIAL_TYPE)interlacingShader;

    for (int i = 0 ; i < m_nbViews ; i++)
    {
        m_material.setTexture(i,m_textures[i]);
    }

    m_material.Wireframe = false;
    m_material.Lighting = false;
    m_material.ZWriteEnable = true;
}

ViewPlayer::~ViewPlayer()
{
    for (int i = 0 ; i < m_nbViews ; i++)
    {
        m_textures[i]->drop();
        m_zBuffers[i]->drop();
    }
}

void ViewPlayer::reset()
{


        //const core::recti &vp = Camera::getCamera(i)->getViewport();
        // Map viewport to [-1,1] x [-1,1]. First define the coordinates
        // left, right, top, bottom:
        float right  = 1.0f;
        float left   = -1.0f;
        float top    = 1.0f;
        float bottom = -1.0f;

        // Use left etc to define 4 vertices on which the rendered screen
        // will be displayed:
        m_vertices.v0.Pos = core::vector3df(left,  bottom, 0);
        m_vertices.v1.Pos = core::vector3df(left,  top,    0);
        m_vertices.v2.Pos = core::vector3df(right, top,    0);
        m_vertices.v3.Pos = core::vector3df(right, bottom, 0);
        // Define the texture coordinates of each vertex, which must
        // be in [0,1]x[0,1]
        m_vertices.v0.TCoords  = core::vector2df(left  ==-1.0f ? 0.0f : 0.5f,
                                                    bottom==-1.0f ? 0.0f : 0.5f);
        m_vertices.v1.TCoords  = core::vector2df(left  ==-1.0f ? 0.0f : 0.5f,
                                                    top   == 1.0f ? 1.0f : 0.5f);
        m_vertices.v2.TCoords  = core::vector2df(right == 0.0f ? 0.5f : 1.0f,
                                                    top   == 1.0f ? 1.0f : 0.5f);
        m_vertices.v3.TCoords  = core::vector2df(right == 0.0f ? 0.5f : 1.0f,
                                                    bottom==-1.0f ? 0.0f : 0.5f);
        // Set normal and color:
        core::vector3df normal(0,0,1);
        m_vertices.v0.Normal = m_vertices.v1.Normal =
        m_vertices.v2.Normal = m_vertices.v3.Normal = normal;
        video::SColor white(0xFF, 0xFF, 0xFF, 0xFF);
        m_vertices.v0.Color  = m_vertices.v1.Color  =
        m_vertices.v2.Color  = m_vertices.v3.Color  = white;
}

void ViewPlayer::beginCapture(unsigned int views)
{
    irr_driver->getVideoDriver()->setRenderTarget(m_textures[views], true, true, 0);
    /*irr_driver->getVideoDriver()->setViewPort(core::recti(0, 0,
                                                UserConfigParams::m_width/sqrt(m_nbViews),
                                                UserConfigParams::m_height/sqrt(m_nbViews)));*/
}

// ----------------------------------------------------------------------------
// Restore the framebuffer render target.

void ViewPlayer::endCapture()
{
    irr_driver->getVideoDriver()->setRenderTarget(video::ERT_FRAME_BUFFER,
                                                  true, true, 0);
}

void ViewPlayer::render3D()
{
    /*if(!m_supported || !UserConfigParams::m_postprocess_enabled)
        return;*/


    u16 indices[6] = {0, 1, 2, 3, 0, 2};

    video::IVideoDriver*    video_driver = irr_driver->getVideoDriver();


        // Draw the fullscreen quad while applying the corresponding
        // post-processing shaders
        video_driver->setMaterial(m_material);
        video_driver->drawIndexedTriangleList(&(m_vertices.v0),
                                              4, &indices[0], 2);

}

//Est appelé automatiquement dans render3D
void ViewPlayer::OnSetConstants(video::IMaterialRendererServices* services, s32 userData)
{
    int index[8];
    services->setPixelShaderConstant((std::string("nbviews")).c_str() ,(&m_nbViews), 1);
    services->setPixelShaderConstant((std::string("leftInterlacing")).c_str() ,(&m_leftInterlacing), 1);

    for(int i = 0 ; i < m_nbViews ; i++)
    {
        // The texture are named "tex0", "tex1"... in the shader
        index[i]=((i+m_firstView) % m_nbViews) / m_viewsPerTexture;


        std::ostringstream oss;
        oss << i;
        std::string iString = oss.str();
        services->setPixelShaderConstant((std::string("tex") + iString).c_str() ,(&index[i]), 1);
        /*oss << m_index[i];
        iString = oss.str();
        Log::info("shader tex", iString.c_str());*/
    }


}

