#!/bin/bash

# Your AppleID, TeamID and Password (An app-specific password NOT! AppleID password)
APPLE_ID="apple_id@icloud.com"
TEAM_ID="0AB11C3DEF"
PASSWORD="aaaa-bbbb-cccc-dddd"


NAME=pplmin
IDENTIFIER=your.domain.$NAME
BIN=Applications/HP/PrimeSDK/bin
YOUR_NAME="Your Name"

# re-sign all binarys
find "package-root/$BIN" -type f -exec codesign --remove-signature {} \;
find "package-root/$BIN" -type f -exec codesign --sign "Developer ID Application: $YOUR_NAME ($TEAM_ID)" --options runtime --timestamp {} \;


pkgbuild --root package-root \
         --identifier $IDENTIFIER \
         --version 1.0 --install-location / \
         --scripts scripts \
         $NAME.pkg
         
productsign --sign "Developer ID Installer: $YOUR_NAME ($TEAM_ID)" $NAME.pkg $NAME-signed.pkg

xcrun notarytool submit --apple-id $APPLE_ID \
                        --password $PASSWORD \
                        --team-id $TEAM_ID \
                        --wait $NAME-signed.pkg
             
rm -r $NAME.pkg
rm -r $NAME-signed.pkg

spctl -a -v $NAME-signed.pkg
mv $NAME-signed.pkg $NAME.pkg
