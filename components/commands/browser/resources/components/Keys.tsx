// Copyright (c) 2023 The Brave Authors. All rights reserved.
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this file,
// You can obtain one at https://mozilla.org/MPL/2.0/.
import * as React from 'react'
import styled, { css } from 'styled-components'
import { color, font, radius, spacing } from '@brave/leo/tokens/css'

const Kbd = styled.div<{ large?: boolean }>`
  display: inline-block;

  font-family: ${font.components.label};

  border: 1px solid ${color.divider.subtle};
  border-radius: ${radius[12]};
  padding: 6px 10px;
  background: linear-gradient(180deg, #F6F7F9 0%, #FFFFFF 100%);
  box-shadow: 0px 1px 0px rgba(0, 0, 0, 0.05), inset 0px 2px 0px #FFFFFF;
  color: ${color.text.tertiary};
  text-transform: capitalize;
  text-shadow: 0px 1px 0px #FFFFFF;

  ${props => props.large && css`
    border-radius: ${radius[12]};
    padding: ${spacing[16]};
    font: var(--leo-font-heading-h3);
    box-shadow: 0px 2px 0px rgba(0, 0, 0, 0.05), inset 0px 3px 0px #FFFFFF;
    background: linear-gradient(180deg, #F4F6F8 0%, #FFFFFF 100%);
  `}
`

export default function Keys({ keys, large }: { keys: string[], large?: boolean }) {
  return (
    <>
      {keys.map((k, i) => (
          <Kbd key={i} large={large}>{k}</Kbd>
      ))}
    </>
  )
}
