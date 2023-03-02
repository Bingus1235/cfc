// Copyright (c) 2023 The Brave Authors. All rights reserved.
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this file,
// You can obtain one at https://mozilla.org/MPL/2.0/.
import * as React from 'react'
import styled from 'styled-components'
import Keys from './Keys'
import { keysToString, stringToKeys } from '../utils/accelerator'
import { color, effect, radius, spacing } from '@brave/leo/tokens/css'
import Button from '@brave/leo/react/button'

const Dialog = styled.dialog`
  border: none;
  border-radius: ${radius[16]};

  ::backdrop {
    backdrop-filter: blur(2px);
  }
`

const Container = styled.div`
  background: ${color.white};
  width: 400px;
  height: 252px;
  margin: auto;

  display: flex;
  flex-direction: column;
  align-items: stretch;
  justify-content: center;

  box-shadow: ${effect.elevation[5]};
`

const KeysContainer = styled.div`
  align-self: stretch;
  flex: 1;
  
  display: flex;
  justify-content: center;
  align-items: center;

  gap: ${spacing[8]};
`

const ActionsContainer = styled.div`
  display: flex;
  flex-direction: row;
  justify-content: stretch;
  gap: ${spacing[8]};
  margin: 0 ${spacing[24]} ${spacing[32]} ${spacing[24]};

  > * { flex: 1; }
`

const modifiers = ['Control', 'Alt', 'Shift', 'Meta']

class AcceleratorInfo {
  codes: string[] = []
  keys: string[] = []

  add(e: KeyboardEvent) {
    if (e.ctrlKey) {
      this.codes.push('Control')
      this.keys.push('Control')
    }
    if (e.altKey) {
      this.codes.push('Alt')
      this.keys.push('Alt')
    }

    if (e.shiftKey) {
      this.codes.push('Shift')
      this.keys.push('Shift')
    }

    if (e.metaKey) {
      this.codes.push('Meta')
      this.keys.push('Meta')
    }

    if (!modifiers.includes(e.key)) {
      this.codes.push(e.code)
      this.keys.push(e.key)
    }

    this.keys = Array.from(new Set(this.keys))
    this.codes = Array.from(new Set(this.codes))
  }

  isValid() {
    // There needs to be at least one non-modifier key
    return this.keys.some((k) => !modifiers.includes(k))
  }
}

export default function ConfigureShortcut(props: {
  value?: string
  onChange: (info: { codes: string; keys: string }) => void
  onCancel?: () => void
}) {
  const [, setCurrentKeys] = React.useState<string[]>([])
  const maxKeys = React.useRef<AcceleratorInfo>(new AcceleratorInfo())

  React.useEffect(() => {
    const onDown = (e: KeyboardEvent) => {
      e.preventDefault()
      setCurrentKeys((keys) => {
        if (keys.length === 0) {
          maxKeys.current = new AcceleratorInfo()
        }

        const newKeys = Array.from(new Set([...keys, e.key]))
        maxKeys.current.add(e)
        return newKeys
      })
    }

    const onUp = (e: KeyboardEvent) => {
      e.preventDefault()
      setCurrentKeys((keys) => {
        const newKeys = keys.filter((k) => k !== e.key)
        return newKeys
      })
    }
    document.addEventListener('keyup', onUp)
    document.addEventListener('keydown', onDown)
    return () => {
      document.removeEventListener('keyup', onUp)
      document.removeEventListener('keydown', onDown)
    }
  })

  const keys = maxKeys.current.keys.length
    ? maxKeys.current.keys
    : stringToKeys(props.value ?? '')

  const dialogRef = React.useRef<HTMLDialogElement>()
  React.useEffect(() => {
    dialogRef.current?.showModal()
  }, [])
  return (
    <Dialog ref={dialogRef as any}>
      <Container>
        <KeysContainer>
          <Keys keys={keys} large />
        </KeysContainer>
        <ActionsContainer>
          <Button
            size='large'
            kind='quaternary'
            onClick={() => {
              setCurrentKeys([])
              maxKeys.current = new AcceleratorInfo()
              props.onCancel?.()
            }}
          >
            Cancel
          </Button>
          <Button
            size='large'
            kind='primary'
            disabled={!maxKeys.current.isValid()}
            onClick={() => {
              props.onChange({
                codes: keysToString(maxKeys.current.codes),
                keys: keysToString(maxKeys.current.keys)
              })
            }}
          >
            Save
          </Button>
        </ActionsContainer>
      </Container>
    </Dialog>
  )
}
