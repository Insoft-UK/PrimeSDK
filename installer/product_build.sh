#!/bin/bash
IDENTITY=$(security find-identity -v -p basic | grep "Developer ID Installer" | awk '{print $2}')

pkgbuild --root package-root \
         --identifier uk.insoft.primesdk \
         --version 1.0 --install-location / \
         --scripts scripts \
         primesdk.pkg
         
productsign --sign "$IDENTITY" primesdk.pkg primesdk-signed.pkg

rm -rf primesdk.pkg
mv primesdk-signed.pkg primesdk.pkg

./update_distribution.sh
productbuild --distribution distribution.xml \
             --resources resources \
             --package-path primesdk.pkg \
             PrimeSDK-installer.pkg
             
productsign --sign "$IDENTITY" PrimeSDK-installer.pkg PrimeSDK-installer-signed.pkg

rm -rf primesdk.pkg
rm -rf PrimeSDK-installer.pkg
mv PrimeSDK-installer-signed.pkg PrimeSDK-installer.pkg
