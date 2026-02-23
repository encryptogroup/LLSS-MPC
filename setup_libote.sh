mkdir thirdparty
cd thirdparty
git clone https://github.com/osu-crypto/libOTe.git
cd libOTe
python3 build.py --all --boost --sodium -D ENABLE_SILENT_VOLE=ON -D CMAKE_C_COMPILER=clang -D CMAKE_CXX_COMPILER=clang++
python3 build.py --install
