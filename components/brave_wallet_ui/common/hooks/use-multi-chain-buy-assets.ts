// Copyright (c) 2022 The Brave Authors. All rights reserved.
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this file,
// you can obtain one at https://mozilla.org/MPL/2.0/.

import * as React from 'react'

// utils
import {
  getRampAssetSymbol,
  isSelectedAssetInAssetOptions
} from '../../utils/asset-utils'
import { networkEntityAdapter, networkEntityInitialState } from '../slices/entities/network.entity'
import { getEntitiesListFromEntityState } from '../../utils/entities.utils'

// types
import {
  BraveWallet,
  BuyOption,
  SupportedTestNetworks
} from '../../constants/types'
import { WalletSelectors } from '../selectors'
import { PageSelectors } from '../../page/selectors'

// options
import { BuyOptions } from '../../options/buy-with-options'

// hooks
import { useIsMounted } from './useIsMounted'
import { useLib } from './useLib'
import {
  useUnsafePageSelector,
  useUnsafeWalletSelector
} from './use-safe-selector'
import {
  useGetAllNetworksQuery,
  useGetSelectedChainQuery
} from '../slices/api.slice'


export const useMultiChainBuyAssets = () => {
  // redux
  const selectedCurrency = useUnsafeWalletSelector(WalletSelectors.selectedCurrency)
  const reduxSelectedAsset = useUnsafePageSelector(PageSelectors.selectedAsset)
  
  // queries
  const { data: selectedNetwork } = useGetSelectedChainQuery()
  const { data: networksRegistry = networkEntityInitialState } =
    useGetAllNetworksQuery()

  // custom hooks
  const isMounted = useIsMounted()
  const { getAllBuyAssets, getBuyAssetUrl } = useLib()

  // state
  const [buyAmount, setBuyAmount] = React.useState<string>('')
  const [selectedAsset, setSelectedAsset] = React.useState<BraveWallet.BlockchainToken | undefined>()
  const [options, setOptions] = React.useState<
    {
      rampAssetOptions: BraveWallet.BlockchainToken[]
      sardineAssetOptions: BraveWallet.BlockchainToken[]
      transakAssetOptions: BraveWallet.BlockchainToken[]
      allAssetOptions: BraveWallet.BlockchainToken[]
    }
  >({
    rampAssetOptions: [],
    sardineAssetOptions: [],
    transakAssetOptions: [],
    allAssetOptions: []
  })

  // memos
  const buyAssetNetworks: BraveWallet.NetworkInfo[] = React.useMemo(() => {
    return getEntitiesListFromEntityState(
      networksRegistry,
      networksRegistry.onRampIds
    )
  }, [networksRegistry])

  const selectedAssetNetwork: BraveWallet.NetworkInfo | undefined =
    React.useMemo(() => {
      return selectedAsset
        ? networksRegistry.entities[
            networkEntityAdapter.selectId(selectedAsset)
          ]
        : undefined
    }, [selectedAsset, networksRegistry])

  const selectedAssetBuyOptions: BuyOption[] = React.useMemo(() => {
    const { rampAssetOptions, sardineAssetOptions, transakAssetOptions } = options
    const onRampAssetMap = {
      [BraveWallet.OnRampProvider.kRamp]: rampAssetOptions,
      [BraveWallet.OnRampProvider.kSardine]: sardineAssetOptions,
      [BraveWallet.OnRampProvider.kTransak]: transakAssetOptions
    }
    return selectedAsset
      ? [...BuyOptions]
        .filter(buyOption => isSelectedAssetInAssetOptions(selectedAsset, onRampAssetMap[buyOption.id]))
        .sort((optionA, optionB) => optionA.name.localeCompare(optionB.name))
      : []
  }, [selectedAsset, options])

  const isSelectedNetworkSupported = React.useMemo(() => {
    if (!selectedNetwork) return false

    // Test networks are not supported in buy tab
    if (SupportedTestNetworks.includes(selectedNetwork.chainId.toLowerCase())) {
      return false
    }

    return options.allAssetOptions
      .map(asset => asset.chainId.toLowerCase())
      .includes(selectedNetwork.chainId.toLowerCase())
  }, [options.allAssetOptions, selectedNetwork])

  const assetsForFilteredNetwork = React.useMemo(() => {
    return options.allAssetOptions.filter(asset => selectedNetwork?.chainId === asset.chainId)
  }, [selectedNetwork, options.allAssetOptions])

  // methods
  const getAllBuyOptionsAllChains = React.useCallback(() => {
    getAllBuyAssets()
      .then(result => {
        if (isMounted && result) {
          setOptions(result)
        }
      })
  }, [getAllBuyAssets, isMounted])

  const openBuyAssetLink = React.useCallback(({ buyOption, depositAddress }: {
    buyOption: BraveWallet.OnRampProvider
    depositAddress: string
  }) => {
    if (!selectedAsset || !selectedAssetNetwork) {
      return
    }

    const asset = {
      ...selectedAsset,
      symbol:
        buyOption === BraveWallet.OnRampProvider.kRamp ? getRampAssetSymbol(selectedAsset)
            : selectedAsset.symbol
    }

    getBuyAssetUrl({
      asset,
      onRampProvider: buyOption,
      chainId: selectedAssetNetwork.chainId,
      address: depositAddress,
      amount: buyAmount,
      currencyCode: selectedCurrency ? selectedCurrency.currencyCode : 'USD'
    })
      .then(url => {
        chrome.tabs.create({ url }, () => {
          if (chrome.runtime.lastError) {
            console.error('tabs.create failed: ' + chrome.runtime.lastError.message)
          }
        })
      })
      .catch(e => console.error(e))
  }, [
    selectedAsset,
    selectedAssetNetwork,
    getBuyAssetUrl,
    buyAmount,
    selectedCurrency
  ])

  const isReduxSelectedAssetBuySupported = React.useMemo(() => {
    return options.allAssetOptions.some((asset) => asset.symbol.toLowerCase() === reduxSelectedAsset?.symbol.toLowerCase())
  }, [options.allAssetOptions, reduxSelectedAsset?.symbol])

  return {
    allAssetOptions: options.allAssetOptions,
    rampAssetOptions: options.rampAssetOptions,
    selectedAsset,
    setSelectedAsset,
    selectedAssetNetwork,
    selectedAssetBuyOptions,
    buyAssetNetworks,
    getAllBuyOptionsAllChains,
    buyAmount,
    setBuyAmount,
    openBuyAssetLink,
    isReduxSelectedAssetBuySupported,
    isSelectedNetworkSupported,
    assetsForFilteredNetwork
  }
}
