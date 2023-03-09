// Copyright (c) 2022 The Brave Authors. All rights reserved.
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this file,
// you can obtain one at https://mozilla.org/MPL/2.0/.
import * as React from 'react'

import styles from './style.module.scss'
import Icon from '@brave/leo/react/icon'

interface InputBoxProps {
  onInputChange?: Function
  onSubmit?: Function
  onSummaryClick?: Function
  value: string
}

function InputBox (props: InputBoxProps) {
  const handleChange = (e: any) => {
    props.onInputChange?.(e)
  }

  const handleClick = (e: any) => {
    props.onSubmit?.(e)
  }

  const handleSummaryClick = (e: any) => {
    props.onSummaryClick?.(e)
  }

  return (
    <div className={styles.container}>
      <button className={styles.summaryButton} onClick={handleSummaryClick}> Summarize</button>
      <form className={styles.form}>
        <textarea
          className={styles.textbox}
          placeholder="Ask Brave AI"
          onChange={handleChange}
          value={props.value}
        />
        <div>
          <button className={styles.button} onClick={handleClick}>
            <Icon name='send' />
          </button>
        </div>
      </form>
    </div>
  )
}

export default InputBox
