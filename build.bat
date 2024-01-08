@REM Doc
@REM https://docs.godotengine.org/en/stable/contributing/development/compiling/introduction_to_the_buildsystem.html
@REM https://docs.godotengine.org/en/stable/contributing/development/compiling/compiling_with_dotnet.html#enable-the-net-module
@REM https://docs.godotengine.org/en/stable/contributing/development/compiling/compiling_for_windows.html

scons platform=windows target=editor dev_build=yes optimize=none
@REM  vsproj=yes

@REM "c:\Robyn\godot\godot\bin\godot.windows.editor.dev.x86_64.exe"
