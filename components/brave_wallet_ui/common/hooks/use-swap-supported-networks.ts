// Copyright (c) 2023 The Brave Authors. All rights reserved.
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this file,
// You can obtain one at https://mozilla.org/MPL/2.0/.

import * as React from 'react'
import { NetworkInfo } from '@brave/swap-interface'

// adapters
import { makeNetworkInfo } from '../../page/screens/swap/adapters'

// hooks
import {
  useGetAllNetworksQuery,
  useGetSwapSupportedChainIdsQuery
} from '../slices/api.slice'

// utils
import { getEntitiesListFromEntityState } from '../../utils/entities.utils'
import { BraveWallet } from '../../constants/types'

const emptyNetworks: BraveWallet.NetworkInfo[] = []
const emptyIds: string[] = []

export const useSwapSupportedNetworksQuery = () => {
  // queries
  const { data: networksRegistry } =
    useGetAllNetworksQuery()
  const { data: swapSupportedChainIds = emptyIds } =
    useGetSwapSupportedChainIdsQuery(undefined, { skip: !networksRegistry })

  // memos
  const supportedNetworks = React.useMemo(() => {
    if (!networksRegistry) {
      return emptyNetworks
    }

    return getEntitiesListFromEntityState(
      networksRegistry,
      swapSupportedChainIds
    )
  }, [networksRegistry, swapSupportedChainIds])

  return supportedNetworks
}

export const useSwapSupportedNetworkInfos = () => {
  // queries
  const networks = useSwapSupportedNetworksQuery()

  // memos
  const supportedNetInfos: NetworkInfo[] = React.useMemo(() => {
    return networks.map(makeNetworkInfo)
  }, [networks])

  return supportedNetInfos
}
