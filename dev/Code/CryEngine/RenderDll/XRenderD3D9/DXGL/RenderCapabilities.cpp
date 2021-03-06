/*
* All or portions of this file Copyright (c) Amazon.com, Inc. or its affiliates or
* its licensors.
*
* For complete copyright and license terms please see the LICENSE at the root of this
* distribution (the "License"). All use of this software is governed by the License,
* or, if provided, by the license below or the license accompanying this file. Do not
* remove or modify any license notices. This file is distributed on an "AS IS" BASIS,
* WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
*
*/
#include "StdAfx.h"
#include "../../Common/RenderCapabilities.h"
#include "Implementation/GLCommon.hpp"
#include "Interfaces/CCryDXGLDevice.hpp"
#include "Implementation/GLDevice.hpp"

namespace RenderCapabilities
{
    NCryOpenGL::CDevice* GetGLDevice()
    {
        AZ_Assert(gcpRendD3D, "gcpRendD3D is NULL");
        CCryDXGLDevice* pDXGLDevice(CCryDXGLDevice::FromInterface(static_cast<ID3D11Device*>(&gcpRendD3D->GetDevice())));
        AZ_Assert(pDXGLDevice, "CCryDXGLDevice is NULL");
        return pDXGLDevice->GetGLDevice();
    }

    bool SupportsTextureViews()
    {
        return GetGLDevice()->IsFeatureSupported(NCryOpenGL::eF_TextureViews);
    }

    bool SupportsStencilTextures()
    {
        return GetGLDevice()->IsFeatureSupported(NCryOpenGL::eF_StencilTextures);
    }

    bool SupportsDepthClipping()
    {
        return GetGLDevice()->IsFeatureSupported(NCryOpenGL::eF_DepthClipping);
    }

    bool SupportsDualSourceBlending()
    {
        return GetGLDevice()->IsFeatureSupported(NCryOpenGL::eF_DualSourceBlending);
    }

    bool SupportsStructuredBuffer(EShaderStage stageMask)
    {
        AZStd::vector<NCryOpenGL::EShaderType> shaderStages;
        if (stageMask & EShaderStage_Vertex)
        {
            shaderStages.push_back(NCryOpenGL::eST_Vertex);
        }

        if (stageMask & EShaderStage_Pixel)
        {
            shaderStages.push_back(NCryOpenGL::eST_Fragment);
        }

        if (stageMask & EShaderStage_Geometry)
        {
#if DXGL_SUPPORT_GEOMETRY_SHADERS
            shaderStages.push_back(NCryOpenGL::eST_Geometry);
#else
            return false;
#endif
        }

        if (stageMask & EShaderStage_Compute)
        {
#if DXGL_SUPPORT_COMPUTE
            shaderStages.push_back(NCryOpenGL::eST_Compute);
#else
            return false;
#endif
        }

        if (stageMask & EShaderStage_Domain)
        {
#if DXGL_SUPPORT_TESSELLATION
            shaderStages.push_back(NCryOpenGL::eST_TessEvaluation);
#else
            return false;
#endif
        }

        if (stageMask & EShaderStage_Hull)
        {
#if DXGL_SUPPORT_TESSELLATION
            shaderStages.push_back(NCryOpenGL::eST_TessControl);
#else
            return false;
#endif
        }

        const NCryOpenGL::SResourceUnitCapabilities& capabilities = GetGLDevice()->GetAdapter()->m_kCapabilities.m_akResourceUnits[NCryOpenGL::eRUT_StorageBuffer];
        for (AZStd::vector<NCryOpenGL::EShaderType>::iterator it = shaderStages.begin(); it != shaderStages.end(); ++it)
        {
            if (capabilities.m_aiMaxPerStage[*it] == 0)
            {
                return false;
            }
        }

        return true;
    }

#if defined(OPENGL_ES)
    int GetAvailableMRTbpp()
    {
        if (DXGL_GL_EXTENSION_SUPPORTED(EXT_shader_pixel_local_storage))
        {
            GLint availableTiledMem;
            glGetIntegerv(GL_MAX_SHADER_PIXEL_LOCAL_STORAGE_FAST_SIZE_EXT, &availableTiledMem);
            return availableTiledMem * 8;
        }

        return 128;
    }

    bool Supports128bppGmemPath()
    {
        return (SupportsFrameBufferFetches() || SupportsPLSExtension()) && GetAvailableMRTbpp() >= 128;
    }

    bool Supports256bppGmemPath()
    {
        return (SupportsFrameBufferFetches() || SupportsPLSExtension()) && GetAvailableMRTbpp() >= 256;
    }

    bool SupportsHalfFloatRendering()
    {
        return DXGL_GL_EXTENSION_SUPPORTED(EXT_color_buffer_half_float);
    }

    bool SupportsPLSExtension()
    {
        return DXGL_GL_EXTENSION_SUPPORTED(EXT_shader_pixel_local_storage);
    }

    bool SupportsFrameBufferFetches()
    {
        return DXGL_GL_EXTENSION_SUPPORTED(EXT_shader_framebuffer_fetch);
    }

    bool SupportsRenderTargets(int numRTs)
    {
        //static GLint val = 0;
        //glGetIntegerv(GL_MAX_COLOR_ATTACHMENTS, &val);
        //return val >= numRTs;
        return false; //todo: uncomment the above lines after testing on Gmem/PLS device.
    }
#else
    bool SupportsPLSExtension()
    {
        return false;
    }

    bool SupportsFrameBufferFetches()
    {
        return false;
    }
#endif

    uint32 GetDeviceGLVersion()
    {
        return GetGLDevice()->GetFeatureSpec().m_kVersion.ToUint();
    }
}