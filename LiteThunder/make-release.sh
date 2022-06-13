
mkdir release
cd release
rm -rf  *
cmake -DCMAKE_BUILD_TYPE=Release ..
make
#ls   | grep -v elx-ctp-trader | xargs -I {} rm -rf {}
cp ../settings.txt ./
