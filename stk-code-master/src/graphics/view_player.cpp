#include "graphics/view_player.hpp"

#include "config/user_config.hpp"
#include "graphics/irr_driver.hpp"
#include "io/file_manager.hpp"
#include "modes/world.hpp"
#include "tracks/track.hpp"
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
                                                            m_SVAlg(true),
                                                            m_firstView(0),
                                                            m_viewsPerTexture(viewsPerTexture),
                                                            m_leftInterlacing(leftInterlacing),
                                                            m_znear(1.0),
                                                            m_zfar(5000.0)
{
    if (nbViews == 5)
        m_leftInterlacing = true;

    core::dimension2du screenSize = device->getVideoDriver()->getScreenSize() / sqrt(nbViews);
    core::dimension2du screenSizePlayerView = device->getVideoDriver()->getScreenSize();

    for(int i = 0 ; i < m_nbViews ; i++)
    {
        m_textures[i] = m_device->getVideoDriver()->addRenderTargetTextureWithDepthBuffer(&m_zBuffers[i], screenSize);
        m_textures_player_view[i] = m_device->getVideoDriver()->addRenderTargetTexture(screenSizePlayerView, "playerView"+i);
        beginCapture(i);
    }

    endCapture();

    video::IGPUProgrammingServices* gpu = m_device->getVideoDriver()->getGPUProgrammingServices();

    s32 interlacingShader = 0;
    s32 renderfromdepthShader = 0;

    if (gpu)
    {
        renderfromdepthShader = gpu->addHighLevelShaderMaterialFromFiles(   (file_manager->getShaderDir() + "shader3D.vert").c_str(),  "main", irr::video::EVST_VS_1_1,
                                                                            (file_manager->getShaderDir() + "SVAlg.frag").c_str(),     "main", irr::video::EPST_PS_1_1,
                                                                            this, irr::video::EMT_SOLID);

        interlacingShader = gpu->addHighLevelShaderMaterialFromFiles(   (file_manager->getShaderDir() + "shader3D.vert").c_str(), "main", irr::video::EVST_VS_1_1,
                                                                        (file_manager->getShaderDir() + "shader3D.frag").c_str(), "main", irr::video::EPST_PS_1_1,
                                                                        this, irr::video::EMT_SOLID);
    }



    m_material.MaterialType = (video::E_MATERIAL_TYPE)interlacingShader;

    for (int i = 0 ; i < m_nbViews ; i++)
    {
        // m_material.setTexture(i,m_zBuffers[i]);
    	m_material.setTexture(i,m_textures[i]);
    }

    m_material.Wireframe = false;
    m_material.Lighting = false;
    m_material.ZWriteEnable = false;

    //Shader render from depth
    m_material_fromDepth.MaterialType = (video::E_MATERIAL_TYPE)renderfromdepthShader;
    for (int i = 0 ; i < m_nbViews/2 ; i++)
    {
        m_material_fromDepth.setTexture(i*2,m_textures[i]);
        m_material_fromDepth.setTexture(i*2+1,m_zBuffers[i]);
    }
}

ViewPlayer::~ViewPlayer()
{
    // TODO: do we have to delete/drop anything?
}   // ~ViewPlayer

void ViewPlayer::reset()
{
        m_znear = 1.0;
        m_zfar = World::getWorld()->getTrack()->getCameraFar();


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
    //m_zBuffers[views] = m_device->getVideoDriver()->createDepthTexture(m_textures[views]);

    /*irr_driver->getVideoDriver()->setViewPort(core::recti(0, 0,
                                                UserConfigParams::m_width/sqrt(m_nbViews),
                                                UserConfigParams::m_height/sqrt(m_nbViews)));*/
}

void ViewPlayer::beginCapturePlayerView(unsigned int views)
{
    irr_driver->getVideoDriver()->setRenderTarget(m_textures_player_view[views], true, true, 0);
    //m_zBuffers[views] = m_device->getVideoDriver()->createDepthTexture(m_textures[views]);

    /*irr_driver->getVideoDriver()->setViewPort(core::recti(0, 0,
                                                UserConfigParams::m_width/sqrt(m_nbViews),
                                                UserConfigParams::m_height/sqrt(m_nbViews)));*/
}

void ViewPlayer::drawPlayerViewToTexture(unsigned int views)
{
    u16 indices[6] = {0, 1, 2, 3, 0, 2};
    video::IVideoDriver*    video_driver = irr_driver->getVideoDriver();

    video::ITexture &textures = *(m_textures[views]);
    video::ITexture &textures_player_view = *(m_textures_player_view[views]);
    video_driver->setRenderTarget(&textures, false, false, 0);

    // video::SMaterial origMaterial = video_driver->getMaterial2D();
    // video::SMaterial viewPlayer;

    // viewPlayer.setTexture(0,&textures_player_view);
    // viewPlayer.ZWriteEnable = false;

    u32 targetWidth = textures.getOriginalSize().Width;
    u32 targetHeight = textures.getOriginalSize().Height;
    u32 texWidth = textures_player_view.getOriginalSize().Width;
    u32 texHeight = textures_player_view.getOriginalSize().Height;

    core::rect<s32> rectTexture(0,0,targetWidth,targetHeight);
    core::rect<s32> rectTexturePlayerView(0,texHeight,texWidth,0);

    // video_driver->setMaterial(viewPlayer);
    //video_driver->setTransform(video::ETS_PROJECTION, core::IdentityMatrix);
    // video_driver->drawIndexedTriangleList(&(m_vertices.v0),
    //                                           4, &indices[0], 2);
    irr_driver->getVideoDriver()->draw2DImage(&textures_player_view, rectTexture, rectTexturePlayerView,
                                                NULL, NULL, true);
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
    if(m_SVAlg)
        video_driver->setMaterial(m_material_fromDepth);
    else
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
    
    if(m_SVAlg)
    {
        index[0] = 0;
        index[1] = 1;
        float Width = (float)m_textures[0]->getOriginalSize().Width;
        services->setPixelShaderConstant((std::string("width")).c_str() ,(&Width), 1);
        services->setPixelShaderConstant((std::string("ZFAR")).c_str() ,(&m_zfar), 1);
        services->setPixelShaderConstant((std::string("ZNEAR")).c_str() ,(&m_znear), 1);
        services->setPixelShaderConstant((std::string("tex0")).c_str() ,(&index[0]), 1);
        services->setPixelShaderConstant((std::string("depth0")).c_str() ,(&index[1]), 1);
    }
    else
    {
    
        for(int i = 0 ; i < m_nbViews ; i++)
        {
            // The texture are named "tex0", "tex1"... in the shader
            index[i]=((i+m_firstView) % m_nbViews) / m_viewsPerTexture;
            std::ostringstream oss;
            oss << i;
            std::string iString = oss.str();
            services->setPixelShaderConstant((std::string("tex") + iString).c_str() ,(&index[i]), 1);
        }
    }


}

