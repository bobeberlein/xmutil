if [ ! -e ./tinyxml2 ]; then
    git clone https://github.com/leethomason/tinyxml2.git
fi

cd tinyxml2
git checkout 8.0.0

MSBuild.exe $PWD/tinyxml2/tinyxml2.sln /t:tinyxml2:rebuild /property:Configuration=Release-Lib

mkdir -p ./../../win/lib
cp -a $PWD/tinyxml2/bin/tinyxml2/x64-Release-Lib/tinyxml2.lib ./../../win/lib
cp -a tinyxml2.h ./../../include
