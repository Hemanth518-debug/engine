// Copyright 2020 Samsung Electronics Co., Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "platform_channel.h"

#include <app.h>

#include <map>

#include "flutter/shell/platform/common/json_method_codec.h"
#include "flutter/shell/platform/tizen/channels/feedback_manager.h"
#include "flutter/shell/platform/tizen/channels/tizen_shell.h"
#include "flutter/shell/platform/tizen/logger.h"

namespace flutter {

namespace {

constexpr char kChannelName[] = "flutter/platform";

constexpr char kGetClipboardDataMethod[] = "Clipboard.getData";
constexpr char kSetClipboardDataMethod[] = "Clipboard.setData";
constexpr char kClipboardHasStringsMethod[] = "Clipboard.hasStrings";
constexpr char kPlaySoundMethod[] = "SystemSound.play";
constexpr char kHapticFeedbackVibrateMethod[] = "HapticFeedback.vibrate";
constexpr char kSystemNavigatorPopMethod[] = "SystemNavigator.pop";
#ifdef COMMON_PROFILE
constexpr char kRestoreSystemUiOverlaysMethod[] =
    "SystemChrome.restoreSystemUIOverlays";
constexpr char kSetEnabledSystemUiOverlaysMethod[] =
    "SystemChrome.setEnabledSystemUIOverlays";
#endif
constexpr char kSetPreferredOrientationsMethod[] =
    "SystemChrome.setPreferredOrientations";

constexpr char kTextKey[] = "text";
constexpr char kValueKey[] = "value";
constexpr char kTextPlainFormat[] = "text/plain";
constexpr char kUnknownClipboardFormatError[] =
    "Unknown clipboard format error";
constexpr char kUnknownClipboardError[] =
    "Unknown error during clipboard data retrieval";

constexpr char kSoundTypeClick[] = "SystemSoundType.click";
constexpr char kSystemUiOverlayBottom[] = "SystemUiOverlay.bottom";
constexpr char kPortraitUp[] = "DeviceOrientation.portraitUp";
constexpr char kPortraitDown[] = "DeviceOrientation.portraitDown";
constexpr char kLandscapeLeft[] = "DeviceOrientation.landscapeLeft";
constexpr char kLandscapeRight[] = "DeviceOrientation.landscapeRight";

// Naive implementation using std::string as a container of internal clipboard
// data.
std::string text_clipboard = "";

}  // namespace

PlatformChannel::PlatformChannel(BinaryMessenger* messenger,
                                 TizenRenderer* renderer)
    : channel_(std::make_unique<MethodChannel<rapidjson::Document>>(
          messenger,
          kChannelName,
          &JsonMethodCodec::GetInstance())),
      renderer_(renderer) {
  channel_->SetMethodCallHandler(
      [this](const MethodCall<rapidjson::Document>& call,
             std::unique_ptr<MethodResult<rapidjson::Document>> result) {
        HandleMethodCall(call, std::move(result));
      });
}

PlatformChannel::~PlatformChannel() {}

void PlatformChannel::HandleMethodCall(
    const MethodCall<rapidjson::Document>& method_call,
    std::unique_ptr<MethodResult<rapidjson::Document>> result) {
  const auto& method = method_call.method_name();
  const auto* arguments = method_call.arguments();

  if (method == kSystemNavigatorPopMethod) {
    SystemNavigatorPop();
    result->Success();
  } else if (method == kPlaySoundMethod) {
    PlaySystemSound(arguments[0].GetString());
    result->Success();
  } else if (method == kHapticFeedbackVibrateMethod) {
    std::string type;
    if (arguments->IsString()) {
      type = arguments[0].GetString();
    }
    HapticFeedbackVibrate(type);
    result->Success();
  } else if (method == kGetClipboardDataMethod) {
    // https://api.flutter.dev/flutter/services/Clipboard/kTextPlain-constant.html
    // The API supports only kTextPlain format.
    if (strcmp(arguments[0].GetString(), kTextPlainFormat) != 0) {
      result->Error(kUnknownClipboardFormatError,
                    "Clipboard API only supports text.");
      return;
    }
    rapidjson::Document document;
    document.SetObject();
    rapidjson::Document::AllocatorType& allocator = document.GetAllocator();
    document.AddMember(rapidjson::Value(kTextKey, allocator),
                       rapidjson::Value(text_clipboard, allocator), allocator);
    result->Success(document);
  } else if (method == kSetClipboardDataMethod) {
    const rapidjson::Value& document = *arguments;
    auto iter = document.FindMember(kTextKey);
    if (iter == document.MemberEnd()) {
      result->Error(kUnknownClipboardError, "Invalid message format.");
      return;
    }
    text_clipboard = iter->value.GetString();
    result->Success();
  } else if (method == kClipboardHasStringsMethod) {
    rapidjson::Document document;
    document.SetObject();
    rapidjson::Document::AllocatorType& allocator = document.GetAllocator();
    document.AddMember(rapidjson::Value(kValueKey, allocator),
                       rapidjson::Value(!text_clipboard.empty()), allocator);
    result->Success(document);
#ifdef COMMON_PROFILE
  } else if (method == kRestoreSystemUiOverlaysMethod) {
    RestoreSystemUiOverlays();
    result->Success();
  } else if (method == kSetEnabledSystemUiOverlaysMethod) {
    const auto& list = arguments[0];
    std::vector<std::string> overlays;
    for (auto iter = list.Begin(); iter != list.End(); ++iter) {
      overlays.push_back(iter->GetString());
    }
    SetEnabledSystemUiOverlays(overlays);
    result->Success();
#endif
  } else if (method == kSetPreferredOrientationsMethod) {
    const auto& list = arguments[0];
    std::vector<std::string> orientations;
    for (auto iter = list.Begin(); iter != list.End(); ++iter) {
      orientations.push_back(iter->GetString());
    }
    SetPreferredOrientations(orientations);
    result->Success();
  } else {
    FT_LOG(Info) << "Unimplemented method: " << method;
    result->NotImplemented();
  }
}

void PlatformChannel::SystemNavigatorPop() {
  ui_app_exit();
}

void PlatformChannel::PlaySystemSound(const std::string& sound_type) {
  if (sound_type == kSoundTypeClick) {
    FeedbackManager::GetInstance().PlayTapSound();
  } else {
    FeedbackManager::GetInstance().PlaySound();
  }
}

void PlatformChannel::HapticFeedbackVibrate(const std::string& feedback_type) {
  FeedbackManager::GetInstance().Vibrate();
}

void PlatformChannel::RestoreSystemUiOverlays() {
  if (!renderer_) {
    return;
  }
  auto& shell = TizenShell::GetInstance();
  shell.InitializeSoftkey(renderer_->GetWindowId());

  if (shell.IsSoftkeyShown()) {
    shell.ShowSoftkey();
  } else {
    shell.HideSoftkey();
  }
}

void PlatformChannel::SetEnabledSystemUiOverlays(
    const std::vector<std::string>& overlays) {
  if (!renderer_) {
    return;
  }
  auto& shell = TizenShell::GetInstance();
  shell.InitializeSoftkey(renderer_->GetWindowId());

  if (std::find(overlays.begin(), overlays.end(), kSystemUiOverlayBottom) !=
      overlays.end()) {
    shell.ShowSoftkey();
  } else {
    shell.HideSoftkey();
  }
}

void PlatformChannel::SetPreferredOrientations(
    const std::vector<std::string>& orientations) {
  if (!renderer_) {
    return;
  }
  static const std::map<std::string, int> orientation_mapping = {
      {kPortraitUp, 0},
      {kLandscapeLeft, 90},
      {kPortraitDown, 180},
      {kLandscapeRight, 270},
  };
  std::vector<int> rotations;
  for (const auto& orientation : orientations) {
    rotations.push_back(orientation_mapping.at(orientation));
  }
  if (rotations.empty()) {
    // The empty list causes the application to defer to the operating system
    // default.
    rotations = {0, 90, 180, 270};
  }
  renderer_->SetPreferredOrientations(rotations);
}

}  // namespace flutter
