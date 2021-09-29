// Copyright 2020 Samsung Electronics Co., Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef EMBEDDER_TIZEN_RENDERER_H_
#define EMBEDDER_TIZEN_RENDERER_H_

#include <cstdint>
#include <vector>

namespace flutter {

class TizenRenderer {
 public:
  struct WindowGeometry {
    int32_t x{0}, y{0}, w{0}, h{0};
  };

  class Delegate {
   public:
    virtual void OnOrientationChange(int32_t degree) = 0;
  };

  virtual ~TizenRenderer();

  bool IsValid() { return is_valid_; }

  virtual bool OnMakeCurrent() = 0;
  virtual bool OnClearCurrent() = 0;
  virtual bool OnMakeResourceCurrent() = 0;
  virtual bool OnPresent() = 0;
  virtual uint32_t OnGetFBO() = 0;
  virtual void* OnProcResolver(const char* name) = 0;

  virtual WindowGeometry GetCurrentGeometry() = 0;
  virtual int32_t GetDpi() = 0;
  virtual uintptr_t GetWindowId() = 0;
  virtual void* GetWindowHandle() = 0;

  virtual void SetRotate(int angle) = 0;
  virtual void ResizeWithRotation(int32_t x,
                                  int32_t y,
                                  int32_t width,
                                  int32_t height,
                                  int32_t degree) = 0;
  virtual void SetPreferredOrientations(const std::vector<int>& rotations) = 0;
  virtual bool IsSupportedExtention(const char* name) = 0;

 protected:
  explicit TizenRenderer(WindowGeometry geometry,
                         bool transparent,
                         bool focusable,
                         Delegate& delegate);

  WindowGeometry initial_geometry_;
  bool transparent_;
  bool focusable_;
  Delegate& delegate_;

  bool is_valid_ = false;
  bool received_rotation_ = false;
};

}  // namespace flutter

#endif  // EMBEDDER_TIZEN_RENDERER_H_