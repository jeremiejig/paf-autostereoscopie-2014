#ifndef VIEWPLAYER_H
#define VIEWPLAYER_H

#include <irrlicht.h>
#include <string>
#include <vector>

#include "SMaterial.h"
#include "S3DVertex.h"
//#include "graphics/irr_driver.hpp"
#include "utils/log.hpp"



using namespace irr;

class ViewPlayer : public video::IShaderConstantSetCallBack
{
    public:
        ViewPlayer(IrrlichtDevice *device, int nbViews = 8, float interocularDistance = 0.05, bool leftInterlacing = false, int viewsPerTexture = 1);

        virtual ~ViewPlayer();


        //! Getters

        video::ITexture *getTexture(int playerIndex) {return m_textures[playerIndex];}
        video::SMaterial getProcessedTexture() {return m_material;}
        float getInterocularDistance() {return m_interocularDistance;}
        bool is3DOn()   {return m_3DOn;}
        bool isActiveSVAlg() {return (m_3DOn & m_SVAlg);}


        //! Setters

        void switch3D      ()   {m_3DOn = !m_3DOn;}
        static void switchSVAlg   ()    {m_SVAlg = !m_SVAlg;}
        void setInterocularDistancePlus   (float amount)    {m_interocularDistance += amount;}
        void setInterocularDistanceMinus   (float amount)    {m_interocularDistance -= amount;}
        void setFirstView       (int firstView)     {m_firstView = firstView;}
        void setViewsPerTexture (int viewsPerTexture){m_viewsPerTexture = viewsPerTexture;}



        virtual void OnSetConstants(video::IMaterialRendererServices* services, s32 userData);

        void reset();

        void render3D();

        void beginCapture(unsigned int views);
        void beginCapturePlayerView(unsigned int views);
        void drawPlayerViewToTexture(unsigned int views);
        void endCapture();

    private:
        IrrlichtDevice *m_device;
        int m_nbViews;
        bool m_3DOn;
        float m_interocularDistance;

        // Use Sormain-Vaulet algorithm
        static bool m_SVAlg;

        // Index of the first view (left)
        int m_firstView;

        int m_viewsPerTexture;

        // 1 if it's towards the left, else -1
        bool m_leftInterlacing;

        float m_znear;
        float m_zfar;

        video::ITexture *m_textures[8];
        video::ITexture *m_textures_player_view[8];
        video::ITexture *m_zBuffers[8];
        video::SMaterial m_material;
        video::SMaterial m_material_fromDepth;

        struct Quad { video::S3DVertex v0, v1, v2, v3; };

        /** The vertices for the rectangle used for each camera. This includes
         *  the vertex position, normal, and texture coordinate. */
        Quad m_vertices;

};

#endif // VIEWPLAYER_H
