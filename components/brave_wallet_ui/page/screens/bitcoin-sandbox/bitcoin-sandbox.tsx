// Copyright (c) 2023 The Brave Authors. All rights reserved.
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this file,
// you can obtain one at https://mozilla.org/MPL/2.0/.

import getAPIProxy from '../../../common/async/bridge'
import * as React from 'react'
import styled from 'styled-components'
import { useState } from 'react'
import { BraveWallet } from '../../../constants/types'
import { LoadingSkeleton } from '../../../components/shared'

const StyledWrapper = styled.div`
  display: flex;
  flex-direction: column;
  align-items: center;
  justify-content: center;
  width: 100%;
  padding-top: 32px;
`

const AddressesSection = styled.h2`
  margin-bottom: 0;
`

const AddressLine = styled.div`
  display: flex;
  flex-direction: row;
  align-items: center;
  justify-content: start;
`

const Address = styled.div`
  font-family: monospace;
  margin-right: 30px;
`

const Balance = styled.div`
  font-family: monospace;
`

export const BitcoinSandbox = () => {
  //  const [utxoList, setUtxoList] = useState<string>('')
  // const [receivingAddresses, setReceivingAddresses] = useState<
  //   BitcoinAddressInfo[]
  // >([])
  // const [changeAddresses, setChangeAddresses] = useState<BitcoinAddressInfo[]>(
  //   []
  // )

  const [loading, setLoading] = useState<boolean>(true)

  const [accountInfo, setAccountInfo] =
    useState<BraveWallet.BitcoinAccountInfo | null>(null)

  React.useEffect(() => {
    // const fetchUtxoList = async () => {
    //   const { resultJson, errorMessage } =
    //     await getAPIProxy().bitcoinRpcService.getUtxoList(
    //       BraveWallet.BITCOIN_TESTNET_KEYRING_ID,
    //       'address'
    //     )
    //   console.log(resultJson, errorMessage)
    //   setUtxoList(resultJson)
    // }

    // fetchUtxoList()

    // const fetchAddresses = async () => {
    //   const addresses = await getAPIProxy().keyringService.getBitcoinAddresses(
    //     BraveWallet.BITCOIN_TESTNET_KEYRING_ID,
    //     0
    //   )
    //   setReceivingAddresses(addresses.receivingAddresses)
    //   setChangeAddresses(addresses.changeAddresses)
    // }

    // fetchAddresses()

    const fetchAccountInfo = async () => {
      const accountInfo =
        await getAPIProxy().bitcoinRpcService.getBitcoinAccountInfo(
          BraveWallet.BITCOIN_TESTNET_KEYRING_ID,
          0
        )
      setAccountInfo(accountInfo.accountInfo)
      setLoading(false)
      // setReceivingAddresses(accountInfo.receiveAddresses)
      // setChangeAddresses(accountInfo.changeAddresses)
    }

    fetchAccountInfo()
  }, [])

  if (loading) {
    return (
      <StyledWrapper>
        <LoadingSkeleton useLightTheme={true} width={300} height={500} />
      </StyledWrapper>
    )
  }

  return (
    <StyledWrapper>
      <h2>Balance: {accountInfo?.balance}</h2>
      <AddressesSection>Receiving Addresses</AddressesSection>
      <ul>
        {accountInfo?.addressInfos
          .filter((a) => !a.change)
          .map((a) => {
            return (
              <li key={a.address}>
                <AddressLine>
                  <Address>{a.address}</Address>
                  <Balance>{a.balance}</Balance>
                </AddressLine>
              </li>
            )
          })}
      </ul>
      <AddressesSection>Change Addresses</AddressesSection>
      <ul>
        {accountInfo?.addressInfos
          .filter((a) => a.change)
          .map((a) => {
            return (
              <li key={a.address}>
                <AddressLine>
                  <Address>{a.address}</Address>
                  <Balance>{a.balance}</Balance>
                </AddressLine>
              </li>
            )
          })}
      </ul>
    </StyledWrapper>
  )
}

export default BitcoinSandbox
