/* Copyright (c) 2023 The Brave Authors. All rights reserved.
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at https://mozilla.org/MPL/2.0/. */

#ifndef BRAVE_BROWSER_CHAT_UI_CHAT_UI_TAB_HELPER_H_
#define BRAVE_BROWSER_CHAT_UI_CHAT_UI_TAB_HELPER_H_

#include <memory>
#include <string>

#include "brave/components/chat_ui/browser/chat_ui_api_request.h"
#include "content/public/browser/web_contents_observer.h"
#include "content/public/browser/web_contents_user_data.h"

class ChatUITabHelper : public content::WebContentsObserver,
                        public content::WebContentsUserData<ChatUITabHelper> {
 public:
  ChatUITabHelper(const ChatUITabHelper&) = delete;
  ChatUITabHelper& operator=(const ChatUITabHelper&) = delete;
  ~ChatUITabHelper() override;

 private:
  friend class content::WebContentsUserData<ChatUITabHelper>;

  explicit ChatUITabHelper(content::WebContents* web_contents);

  // content::WebContentsObserver
  void DidStopLoading() override;
  void WebContentsDestroyed() override;

  std::unique_ptr<chat_ui::ChatUIAPIRequest> api_helper_ = nullptr;

  WEB_CONTENTS_USER_DATA_KEY_DECL();
};

#endif  // BRAVE_BROWSER_CHAT_UI_CHAT_UI_TAB_HELPER_H_
