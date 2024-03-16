# wooboard

[![demo](https://img.youtube.com/vi/L3qZYKWTLvg/maxresdefault.jpg)](https://www.youtube.com/watch?v=L3qZYKWTLvg)

in order to run this program
- you'll need a wooting analog keyboard
- you'll need to have the wooting analog sdk installed along with the wooting-analog-plugin, on linux you do it like this: 

install:
- Download & Extract the [latest release](https://github.com/WootingKb/wooting-analog-sdk/releases) `wooting-analog-sdk-v*.*.*-x86_64-unknown-linux-gnu.tar.gz`
- Copy `$extract/wrapper/sdk/libwooting_analog_sdk.so` to `/usr/lib`. (Or to some directory and add that path to the `LD_LIBRARY_PATH` environment variable)
- Now create the correct folder and store the dynamic library for the analog plugin there:
```
sudo mkdir -p /usr/local/share/WootingAnalogPlugins/wooting-analog-plugin
```
and then copy the analog plugin to that location 
```
sudo cp $extract/wrapper/sdk/libwooting_analog_plugin.so /usr/local/share/WootingAnalogPlugins/wooting-analog-plugin/
```

When you run the program you have to run it as sudo.

