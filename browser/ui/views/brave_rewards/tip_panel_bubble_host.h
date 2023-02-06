/* Copyright (c) 2023 The Brave Authors. All rights reserved.
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at https://mozilla.org/MPL/2.0/. */

#ifndef BRAVE_BROWSER_UI_VIEWS_BRAVE_REWARDS_TIP_PANEL_BUBBLE_HOST_H_
#define BRAVE_BROWSER_UI_VIEWS_BRAVE_REWARDS_TIP_PANEL_BUBBLE_HOST_H_

#include <memory>
#include <string>

#include "chrome/browser/ui/browser_user_data.h"

class WebUIBubbleManager;

namespace brave_rewards {

// A browser helper responsible for displaying the tipping panel for the creator
// currently displayed in the browser. Instances own the displayed bubble, and
// are owned by a Browser.
class TipPanelBubbleHost : public BrowserUserData<TipPanelBubbleHost> {
 public:
  explicit TipPanelBubbleHost(Browser* browser);

  TipPanelBubbleHost(const TipPanelBubbleHost&) = delete;
  TipPanelBubbleHost& operator=(const TipPanelBubbleHost&) = delete;

  ~TipPanelBubbleHost() override;

  static void MaybeCreateForBrowser(Browser* browser);

  void ShowPanelForPublisher(const std::string& publisher_id);

  const std::string& publisher_id() { return publisher_id_; }

 private:
  friend class BrowserUserData<TipPanelBubbleHost>;

  std::unique_ptr<WebUIBubbleManager> bubble_manager_;
  std::string publisher_id_;

  BROWSER_USER_DATA_KEY_DECL();
};

}  // namespace brave_rewards

#endif  // BRAVE_BROWSER_UI_VIEWS_BRAVE_REWARDS_TIP_PANEL_BUBBLE_HOST_H_
