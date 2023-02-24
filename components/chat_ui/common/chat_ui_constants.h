/* Copyright (c) 2022 The Brave Authors. All rights reserved.
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at https://mozilla.org/MPL/2.0/. */

#ifndef BRAVE_COMPONENTS_CHAT_UI_COMMON_CHAT_UI_CONSTANTS_H_
#define BRAVE_COMPONENTS_CHAT_UI_COMMON_CHAT_UI_CONSTANTS_H_

#include "build/buildflag.h"

namespace chat_ui {
constexpr char kHumanPrompt[] = "\n\nHuman:";
constexpr char kAIPrompt[] = "\n\nAssistant:";
constexpr char kUrlBase[] = "api.anthropic.com/";
constexpr char kCompletionPath[] = "v1/complete";
}  // namespace chat_ui

#endif  // BRAVE_COMPONENTS_CHAT_UI_COMMON_CHAT_UI_CONSTANTS_H_
