// Copyright (c) 2023 The Brave Authors. All rights reserved.
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this file,
// you can obtain one at http://mozilla.org/MPL/2.0/.

#ifndef BRAVE_BROWSER_UI_WEBUI_CHAT_UI_CHAT_UI_H_
#define BRAVE_BROWSER_UI_WEBUI_CHAT_UI_CHAT_UI_H_

#include <memory>
#include <string>

#include "content/public/browser/web_ui_controller.h"
#include "ui/webui/mojo_web_ui_controller.h"
#include "ui/webui/untrusted_web_ui_controller.h"
#include "content/public/browser/webui_config.h"

class ChatUI : public ui::UntrustedWebUIController {
 public:
  explicit ChatUI(content::WebUI* web_ui);
  ChatUI(const ChatUI&) = delete;
  ChatUI& operator=(const ChatUI&) = delete;
  ~ChatUI() override;
};

class UntrustedChatUIConfig : public content::WebUIConfig {
 public:
  UntrustedChatUIConfig();
  ~UntrustedChatUIConfig() override = default;

  std::unique_ptr<content::WebUIController> CreateWebUIController(
      content::WebUI* web_ui) override;
};

#endif  // BRAVE_BROWSER_UI_WEBUI_CHAT_UI_CHAT_UI_H_
