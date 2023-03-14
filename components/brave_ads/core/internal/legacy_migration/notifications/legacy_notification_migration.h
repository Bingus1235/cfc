/* Copyright (c) 2022 The Brave Authors. All rights reserved.
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at https://mozilla.org/MPL/2.0/. */

#ifndef BRAVE_COMPONENTS_BRAVE_ADS_CORE_INTERNAL_LEGACY_MIGRATION_NOTIFICATIONS_LEGACY_NOTIFICATION_MIGRATION_H_
#define BRAVE_COMPONENTS_BRAVE_ADS_CORE_INTERNAL_LEGACY_MIGRATION_NOTIFICATIONS_LEGACY_NOTIFICATION_MIGRATION_H_

#include "brave/components/brave_ads/core/ads_callback.h"

namespace brave_ads::notifications {

void Migrate(InitializeCallback callback);

}  // namespace brave_ads::notifications

#endif  // BRAVE_COMPONENTS_BRAVE_ADS_CORE_INTERNAL_LEGACY_MIGRATION_NOTIFICATIONS_LEGACY_NOTIFICATION_MIGRATION_H_
