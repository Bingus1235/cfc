/* Copyright (c) 2023 The Brave Authors. All rights reserved.
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at https://mozilla.org/MPL/2.0/. */

#include "brave/browser/chat_ui/chat_ui_tab_helper.h"

#include <utility>

#include "base/strings/string_number_conversions.h"
#include "brave/browser/ui/brave_browser_window.h"
#include "chrome/browser/ui/browser_finder.h"
#include "content/public/browser/browser_context.h"
#include "content/public/browser/storage_partition.h"
#include "content/public/browser/web_contents.h"
#include "ui/base/l10n/l10n_util.h"

ChatUITabHelper::~ChatUITabHelper() = default;

ChatUITabHelper::ChatUITabHelper(content::WebContents* web_contents)
    : content::WebContentsObserver(web_contents),
      content::WebContentsUserData<ChatUITabHelper>(*web_contents) {
  auto url_loader_factory = web_contents->GetBrowserContext()
                                ->GetDefaultStoragePartition()
                                ->GetURLLoaderFactoryForBrowserProcess();
  api_helper_ = std::make_unique<chat_ui::ChatUIAPIRequest>(url_loader_factory);
}

void ChatUITabHelper::DidStopLoading() {
  Browser* browser = chrome::FindBrowserWithWebContents(web_contents());
  DCHECK(browser);

  auto* active_tab_web_contents =
      browser->tab_strip_model()->GetActiveWebContents();

  if (web_contents() != active_tab_web_contents) {
    return;
  }
}

void ChatUITabHelper::WebContentsDestroyed() {}

WEB_CONTENTS_USER_DATA_KEY_IMPL(ChatUITabHelper);
