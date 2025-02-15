#!/bin/bash
pkgbuild --root package-root \
         --identifier uk.insoft.primesdk \
         --version 1.0 --install-location / \
         --scripts scripts \
         primesdk.pkg

./update_distribution.sh
productbuild --distribution distribution.xml \
             --resources resources \
             --package-path primesdk.pkg \
             PrimeSDK-installer.pkg
