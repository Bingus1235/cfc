// Copyright (c) 2022 The Brave Authors. All rights reserved.
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this file,
// you can obtain one at https://mozilla.org/MPL/2.0/.

import {
  createDraftSafeSelector,
  createEntityAdapter,
  EntityAdapter,
  EntityId
} from '@reduxjs/toolkit'
import { BraveWallet } from '../../../constants/types'
import { getEntitiesListFromEntityState } from '../../../utils/entities.utils'

export type NetworkEntityAdaptor = EntityAdapter<BraveWallet.NetworkInfo> & {
  selectId: (network: { chainId: string, coin: BraveWallet.CoinType }) => EntityId
}

export const networkEntityAdapter: NetworkEntityAdaptor =
  createEntityAdapter<BraveWallet.NetworkInfo>({
    selectId: ({ chainId, coin }): string =>
      chainId === BraveWallet.LOCALHOST_CHAIN_ID
        ? `${chainId}-${coin}`
        : chainId
  })

export type NetworkEntityAdaptorState = ReturnType<
  typeof networkEntityAdapter['getInitialState']
> & {
  defaultIdByCoinType: Record<BraveWallet.CoinType, EntityId>
  defaultIds: string[]
  hiddenIds: string[]
  hiddenIdsByCoinType: Record<BraveWallet.CoinType, EntityId[]>
  idsByCoinType: Record<BraveWallet.CoinType, EntityId[]>
  mainnetIds: string[]
  onRampIds: string[]
  visibleIds: string[]
}

export const networkEntityInitialState: NetworkEntityAdaptorState = {
  ...networkEntityAdapter.getInitialState(),
  defaultIdByCoinType: {},
  defaultIds: [],
  hiddenIds: [],
  hiddenIdsByCoinType: {},
  idsByCoinType: {},
  mainnetIds: [],
  onRampIds: [],
  visibleIds: []
}

//
// Selectors (From Query Results)
//
export const selectNetworksRegistryFromQueryResult = (result: {
  data?: NetworkEntityAdaptorState
}) => {
  return result.data ?? networkEntityInitialState
}

export const {
  selectAll: selectAllNetworksFromQueryResult,
  selectById: selectNetworkByIdFromQueryResult,
  selectEntities: selectNetworkEntitiesFromQueryResult,
  selectIds: selectNetworkIdsFromQueryResult,
  selectTotal: selectTotalNetworksFromQueryResult
} = networkEntityAdapter.getSelectors(selectNetworksRegistryFromQueryResult)

export const makeSelectAllChainIdsForCoinTypeFromQueryResult = () => {
  return createDraftSafeSelector(
    [
      selectNetworksRegistryFromQueryResult,
      (_, coinType: BraveWallet.CoinType) => coinType
    ],
    (registry, coinType) => registry.idsByCoinType[coinType]
  )
}

export const selectVisibleNetworksFromQueryResult = createDraftSafeSelector(
  [selectNetworksRegistryFromQueryResult],
  (registry) => getEntitiesListFromEntityState(registry, registry.visibleIds)
)

export const selectHiddenNetworksFromQueryResult = createDraftSafeSelector(
  [selectNetworksRegistryFromQueryResult],
  (registry) => getEntitiesListFromEntityState(registry, registry.hiddenIds)
)

export const selectDefaultNetworksFromQueryResult = createDraftSafeSelector(
  [selectNetworksRegistryFromQueryResult],
  (registry) => getEntitiesListFromEntityState(registry, registry.defaultIds)
)

export const selectMainnetsFromQueryResult = createDraftSafeSelector(
  [selectNetworksRegistryFromQueryResult],
  (registry) => getEntitiesListFromEntityState(registry, registry.mainnetIds)
)

export const selectOnrampNetworksFromQueryResult = createDraftSafeSelector(
  [selectNetworksRegistryFromQueryResult],
  (registry) => getEntitiesListFromEntityState(registry, registry.onRampIds)
)

// export const selectAllChainIdsForCoinTypeFromQueryResult = () => {
//   return createDraftSafeSelector(
//     [
//       selectNetworksRegistryFromQueryResult,
//       (_, coinType: BraveWallet.CoinType) => coinType
//     ],
//     (registry, coinType) => getEntitiesListFromEntityState(registry, registry.idsByCoinType[coinType])
//   )
// }

// export const selectAllChainIdsForCoinTypeFromQueryResult = () => {
//   return createDraftSafeSelector(
//     [
//       selectNetworksRegistryFromQueryResult,
//       (_, coinType: BraveWallet.CoinType) => coinType
//     ],
//     (registry, coinType) => getEntitiesListFromEntityState(registry, registry.idsByCoinType[coinType])
//   )
// }
