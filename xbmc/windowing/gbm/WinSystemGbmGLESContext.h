/*
 *  Copyright (C) 2005-2018 Team Kodi
 *  This file is part of Kodi - https://kodi.tv
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 *  See LICENSES/README.md for more information.
 */

#pragma once

#include "utils/EGLUtils.h"
#include "rendering/gles/RenderSystemGLES.h"
#include "WinSystemGbmEGLContext.h"
#include <memory>

class CVaapiProxy;

namespace KODI
{
namespace WINDOWING
{
namespace GBM
{

class CWinSystemGbmGLESContext : public CWinSystemGbmEGLContext, public CRenderSystemGLES
{
public:
  CWinSystemGbmGLESContext();
  virtual ~CWinSystemGbmGLESContext() = default;

  // Implementation of CWinSystemBase via CWinSystemGbm
  CRenderSystemBase *GetRenderSystem() override { return this; }
  bool InitWindowSystem() override;
  bool SetFullScreen(bool fullScreen, RESOLUTION_INFO& res, bool blankOtherDisplays) override;
  void PresentRender(bool rendered, bool videoLayer) override;
protected:
#ifndef TARGET_RPI4_GBM
  void SetVSyncImpl(bool enable) override { return; };
#else
  void SetVSyncImpl(bool enable) override;
  virtual std::unique_ptr<CVideoSync> GetVideoSync(void *clock) override;
#endif
  void PresentRenderImpl(bool rendered) override {};
  bool CreateContext() override;
};

}
}
}
