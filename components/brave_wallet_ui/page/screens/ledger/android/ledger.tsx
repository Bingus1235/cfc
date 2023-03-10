// Copyright (c) 2023 The Brave Authors. All rights reserved.
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this file,
// You can obtain one at https://mozilla.org/MPL/2.0/.

import TransportWebUSB from '@backpacker69/hw-transport-webusb'

function initialize () {
  var button = document.getElementById('clickme');
  if (button != null) {
      button.addEventListener ("click", function() {
        TransportWebUSB.create().then(transport => {console.log('!!!isSupported == ' + (transport != null));});
      });
  }
}

document.addEventListener('DOMContentLoaded', initialize)
