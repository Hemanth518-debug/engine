// Copyright 2022 Samsung Electronics Co., Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef EMBEDDER_TIZEN_VIEW_BASE_H_
#define EMBEDDER_TIZEN_VIEW_BASE_H_

#include <cstdint>
#include <memory>
#include <string>
#include <vector>

#include "flutter/shell/platform/tizen/tizen_input_method_context.h"

namespace flutter {

class FlutterTizenView;

struct TizenGeometry {
  int32_t left = 0, top = 0, width = 0, height = 0;
};

enum class TizenViewType { kView, kWindow };

class TizenViewBase {
 public:
  TizenViewBase() = default;
  virtual ~TizenViewBase() = default;

  // Returns a pointer to the platform object that rendering can be bound to
  // by the rendering backend.
  virtual void* GetRenderTarget() = 0;

  virtual uintptr_t GetWindowId() = 0;

  // Returns the geometry of the view.
  virtual TizenGeometry GetGeometry() = 0;

  // Set the geometry of the view.
  virtual void SetGeometry(TizenGeometry geometry) = 0;

  // Returns the dpi of the screen.
  virtual int32_t GetDpi() = 0;

  // Sets the delegate used to communicate state changes from render target to
  // view such as key presses, mouse position updates etc.
  void SetView(FlutterTizenView* view) { view_ = view; }

  virtual void BindKeys(const std::vector<std::string>& keys) = 0;

  virtual void ResizeWithRotation(TizenGeometry geometry, int32_t degree) = 0;

  // FIXME
  // This is a temporary implementation that is only used by the window channel.
  virtual void OnGeometryChanged(TizenGeometry geometry) = 0;

  virtual void Show() = 0;

  virtual TizenViewType GetType() = 0;

  TizenInputMethodContext* input_method_context() {
    return input_method_context_.get();
  }

 protected:
  FlutterTizenView* view_ = nullptr;

  // The Tizen input method context. nullptr if not set.
  std::unique_ptr<TizenInputMethodContext> input_method_context_;
};

}  // namespace flutter

#endif  // EMBEDDER_TIZEN_VIEW_BASE_H_
