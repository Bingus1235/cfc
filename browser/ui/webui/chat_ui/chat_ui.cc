// Copyright (c) 2023 The Brave Authors. All rights reserved.
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this file,
// you can obtain one at http://mozilla.org/MPL/2.0/.

#include "brave/browser/ui/webui/chat_ui/chat_ui.h"

#include <utility>

#include "brave/browser/ui/webui/brave_webui_source.h"
#include "brave/browser/ui/webui/chat_ui/chat_ui_page_handler.h"
#include "brave/components/chat_ui/resources/page/grit/chat_ui_generated_map.h"
#include "brave/components/constants/webui_url_constants.h"
#include "chrome/browser/ui/webui/webui_util.h"
#include "components/grit/brave_components_resources.h"
#include "content/public/browser/web_contents.h"
#include "content/public/browser/web_ui_data_source.h"
#include "content/public/common/url_constants.h"
#include "services/network/public/mojom/content_security_policy.mojom.h"

ChatUI::ChatUI(content::WebUI* web_ui) : ui::UntrustedWebUIController(web_ui) {
  // Create a URLDataSource and add resources.
  content::WebUIDataSource* untrusted_source =
      content::WebUIDataSource::CreateAndAdd(
          web_ui->GetWebContents()->GetBrowserContext(), kChatUIURL);

  webui::SetupWebUIDataSource(
      untrusted_source, base::make_span(kChatUiGenerated, kChatUiGeneratedSize),
      IDR_CHAT_UI_HTML);

  untrusted_source->OverrideContentSecurityPolicy(
      network::mojom::CSPDirectiveName::ScriptSrc,
      std::string("script-src 'self' chrome-untrusted://resources;"));
  untrusted_source->OverrideContentSecurityPolicy(
      network::mojom::CSPDirectiveName::StyleSrc,
      std::string(
          "style-src 'self' 'unsafe-inline' chrome-untrusted://resources;"));
  untrusted_source->OverrideContentSecurityPolicy(
      network::mojom::CSPDirectiveName::FontSrc,
      std::string("font-src 'self' data: chrome-untrusted://resources;"));
}

ChatUI::~ChatUI() = default;

void ChatUI::BindInterface(
    mojo::PendingReceiver<chat_ui::mojom::PageHandler> receiver) {
  page_handler_ = std::make_unique<ChatUIPageHandler>(
      web_ui()->GetWebContents(), std::move(receiver));
}

std::unique_ptr<content::WebUIController>
UntrustedChatUIConfig::CreateWebUIController(content::WebUI* web_ui) {
  return std::make_unique<ChatUI>(web_ui);
}

UntrustedChatUIConfig::UntrustedChatUIConfig()
    : WebUIConfig(content::kChromeUIUntrustedScheme, kChatUIHost) {}

WEB_UI_CONTROLLER_TYPE_IMPL(ChatUI)
