/* Copyright (c) 2020 The Brave Authors. All rights reserved.
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at https://mozilla.org/MPL/2.0/. */

#ifndef BRAVE_COMPONENTS_BRAVE_ADS_CORE_INTERNAL_GEOGRAPHIC_SUBDIVISION_GET_SUBDIVISION_URL_REQUEST_BUILDER_H_
#define BRAVE_COMPONENTS_BRAVE_ADS_CORE_INTERNAL_GEOGRAPHIC_SUBDIVISION_GET_SUBDIVISION_URL_REQUEST_BUILDER_H_

#include "brave/components/brave_ads/common/interfaces/ads.mojom-forward.h"
#include "brave/components/brave_ads/core/internal/server/url/url_request_builder_interface.h"

namespace brave_ads::geographic {

class GetSubdivisionUrlRequestBuilder final
    : public UrlRequestBuilderInterface {
 public:
  mojom::UrlRequestInfoPtr Build() override;
};

}  // namespace brave_ads::geographic

#endif  // BRAVE_COMPONENTS_BRAVE_ADS_CORE_INTERNAL_GEOGRAPHIC_SUBDIVISION_GET_SUBDIVISION_URL_REQUEST_BUILDER_H_
