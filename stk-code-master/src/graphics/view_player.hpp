#ifndef VIEWPLAYER_H
#define VIEWPLAYER_H

#include <irrlicht.h>
#include <string>
#include <vector>

#include "SMaterial.h"
#include "S3DVertex.h"
//#include "graphics/irr_driver.hpp"



using namespace irr;

class ViewPlayer : public video::IShaderConstantSetCallBack
{
    public:
        ViewPlayer(IrrlichtDevice *device, int nbViews = 8);

        video::ITexture *getTexture(int playerIndex) {return m_textures[playerIndex];}

        virtual void OnSetConstants(video::IMaterialRendererServices* services, s32 userData);

        video::SMaterial getProcessedTexture() {return m_material;}

        virtual ~ViewPlayer();

        void reset();

        void render3D();

        void beginCapture(unsigned int views);

        void endCapture();

    private:
        IrrlichtDevice *m_device;
        int m_nbViews;

        video::ITexture *m_textures[8];
        video::SMaterial m_material;

        struct Quad { video::S3DVertex v0, v1, v2, v3; };

        /** The vertices for the rectangle used for each camera. This includes
         *  the vertex position, normal, and texture coordinate. */
        Quad m_vertices;

};

#endif // VIEWPLAYER_H
