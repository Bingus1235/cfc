// Copyright (c) 2023 The Brave Authors. All rights reserved.
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this file,
// You can obtain one at https://mozilla.org/MPL/2.0/.

// hooks
import * as React from 'react'

// types
import { BraveWallet } from '../../constants/types'

// hooks
import { useGetAllNetworksQuery } from '../slices/api.slice'

// selectors
import {
  networkEntityAdapter,
  networkEntityInitialState,
  selectAllNetworksFromQueryResult,
  selectDefaultNetworksFromQueryResult,
  selectMainnetsFromQueryResult,
  selectOnrampNetworksFromQueryResult,
} from '../slices/entities/network.entity'

export const useNetworkQuery = (
  {
    chainId,
    coin
  }: {
    chainId: string
    coin: BraveWallet.CoinType
  },
  opts?: { skip?: boolean }
) => {
  return useGetAllNetworksQuery(undefined, {
    selectFromResult: (res) => ({
      ...res,
      network: res.data
        ? res.data.entities[
            networkEntityAdapter.selectId({
              chainId,
              coin
            })
          ]
        : undefined
    }),
    skip: opts?.skip
  })
}

export const useNetwork = (
  {
    chainId,
    coin
  }: {
    chainId: string
    coin: BraveWallet.CoinType
  },
  opts?: { skip?: boolean }
) => {
  const { data: registry = networkEntityInitialState } = useGetAllNetworksQuery(
    undefined,
    { skip: opts?.skip }
  )

  return React.useMemo(() => {
    return registry.entities[
      networkEntityAdapter.selectId({
        chainId,
        coin
      })
    ]
  }, [registry.entities, chainId, coin])
}

let prevNetsList: BraveWallet.NetworkInfo[]

export const useNetworksListQuery = (
  opts?: { skip?: boolean }
) => {
  const queryResults = useGetAllNetworksQuery(
    undefined,
    {
      selectFromResult: res => ({
        ...res,
        networks: selectAllNetworksFromQueryResult(res),
      }),
      skip: opts?.skip
    }
  )

  return queryResults
}

export const useDefaultNetworksQuery = (
  opts?: { skip?: boolean }
) => {
  const queryResults = useGetAllNetworksQuery(
    undefined,
    {
      selectFromResult: res => ({
        ...res,
        defaultNetworks: selectDefaultNetworksFromQueryResult(res),
      }),
      skip: opts?.skip
    }
  )

  console.log({
    prev: prevNetsList,
    curr: queryResults.defaultNetworks,
    eq: prevNetsList === queryResults.defaultNetworks
  })
  
  prevNetsList = queryResults.defaultNetworks

  return queryResults
}

export const useMainnetsQuery = (
  opts?: { skip?: boolean }
) => {
  const queryResults = useGetAllNetworksQuery(
    undefined,
    {
      selectFromResult: res => ({
        ...res,
        networks: selectMainnetsFromQueryResult(res),
      }),
      skip: opts?.skip
    }
  )

  return queryResults
}

export const useOnrampNetworksQuery = (
  opts?: { skip?: boolean }
) => {
  const queryResults = useGetAllNetworksQuery(
    undefined,
    {
      selectFromResult: res => ({
        ...res,
        networks: selectOnrampNetworksFromQueryResult(res),
      }),
      skip: opts?.skip
    }
  )

  return queryResults
}
