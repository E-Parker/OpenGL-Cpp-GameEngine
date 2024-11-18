#!/bin/bash

echo "Removing assets from build."
cd build
rm -rf assets
cd ..
echo "Copying assets back to build."
cp -r assets build
cd build
echo "Done!"
