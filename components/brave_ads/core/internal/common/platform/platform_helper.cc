/* Copyright (c) 2020 The Brave Authors. All rights reserved.
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at https://mozilla.org/MPL/2.0/. */

#include "brave/components/brave_ads/core/internal/common/platform/platform_helper.h"

#include "build/build_config.h"
#if BUILDFLAG(IS_ANDROID)
#include "brave/components/brave_ads/core/internal/common/platform/platform_helper_android.h"
#elif BUILDFLAG(IS_IOS)
#include "brave/components/brave_ads/core/internal/common/platform/platform_helper_ios.h"
#elif BUILDFLAG(IS_LINUX)
#include "brave/components/brave_ads/core/internal/common/platform/platform_helper_linux.h"
#elif BUILDFLAG(IS_MAC)
#include "brave/components/brave_ads/core/internal/common/platform/platform_helper_mac.h"
#elif BUILDFLAG(IS_WIN)
#include "brave/components/brave_ads/core/internal/common/platform/platform_helper_win.h"
#endif

namespace brave_ads {

namespace {

constexpr char kPlatformName[] = "unknown";

PlatformHelper* g_platform_helper_for_testing = nullptr;

}  // namespace

PlatformHelper::PlatformHelper() = default;

PlatformHelper::~PlatformHelper() = default;

// static
void PlatformHelper::SetForTesting(PlatformHelper* platform_helper) {
  g_platform_helper_for_testing = platform_helper;
}

bool PlatformHelper::IsMobile() const {
  return false;
}

std::string PlatformHelper::GetName() const {
  return kPlatformName;
}

PlatformType PlatformHelper::GetType() const {
  return PlatformType::kUnknown;
}

// static
PlatformHelper* PlatformHelper::GetInstance() {
  if (g_platform_helper_for_testing) {
    return g_platform_helper_for_testing;
  }

  return GetInstanceImpl();
}

// static
PlatformHelper* PlatformHelper::GetInstanceImpl() {
#if BUILDFLAG(IS_ANDROID)
  static base::NoDestructor<PlatformHelperAndroid> platform_helper;
#elif BUILDFLAG(IS_IOS)
  static base::NoDestructor<PlatformHelperIos> platform_helper;
#elif BUILDFLAG(IS_LINUX)
  static base::NoDestructor<PlatformHelperLinux> platform_helper;
#elif BUILDFLAG(IS_MAC)
  static base::NoDestructor<PlatformHelperMac> platform_helper;
#elif BUILDFLAG(IS_WIN)
  static base::NoDestructor<PlatformHelperWin> platform_helper;
#else
  // Default platform helper for unsupported platforms
  static base::NoDestructor<PlatformHelper> platform_helper;
#endif  // BUILDFLAG(IS_ANDROID)

  return platform_helper.get();
}

}  // namespace brave_ads
