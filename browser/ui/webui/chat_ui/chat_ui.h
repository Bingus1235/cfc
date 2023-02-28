// Copyright (c) 2023 The Brave Authors. All rights reserved.
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this file,
// you can obtain one at http://mozilla.org/MPL/2.0/.

#ifndef BRAVE_BROWSER_UI_WEBUI_CHAT_UI_CHAT_UI_H_
#define BRAVE_BROWSER_UI_WEBUI_CHAT_UI_CHAT_UI_H_

#include <memory>
#include <string>

#include "brave/components/chat_ui/common/chat_ui.mojom-forward.h"
#include "content/public/browser/web_ui_controller.h"
#include "content/public/browser/webui_config.h"
#include "ui/webui/mojo_bubble_web_ui_controller.h"
#include "ui/webui/mojo_web_ui_controller.h"
#include "ui/webui/untrusted_web_ui_controller.h"

class Browser;
class ChatUI : public ui::UntrustedWebUIController {
 public:
  explicit ChatUI(content::WebUI* web_ui);
  ChatUI(const ChatUI&) = delete;
  ChatUI& operator=(const ChatUI&) = delete;
  ~ChatUI() override;

  void BindInterface(
      mojo::PendingReceiver<chat_ui::mojom::PageHandler> receiver);

  // Set by BubbleContentsWrapperT. MojoBubbleWebUIController provides default
  // implementation for this but we don't use it.
  void set_embedder(
      base::WeakPtr<ui::MojoBubbleWebUIController::Embedder> embedder) {
    embedder_ = embedder;
  }

 private:
  std::unique_ptr<chat_ui::mojom::PageHandler> page_handler_;

  base::WeakPtr<ui::MojoBubbleWebUIController::Embedder> embedder_;
  raw_ptr<Browser> browser_ = nullptr;

  WEB_UI_CONTROLLER_TYPE_DECL();
};

class UntrustedChatUIConfig : public content::WebUIConfig {
 public:
  UntrustedChatUIConfig();
  ~UntrustedChatUIConfig() override = default;

  std::unique_ptr<content::WebUIController> CreateWebUIController(
      content::WebUI* web_ui) override;
};

#endif  // BRAVE_BROWSER_UI_WEBUI_CHAT_UI_CHAT_UI_H_
