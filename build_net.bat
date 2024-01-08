@REM Doc
@REM https://docs.godotengine.org/en/stable/contributing/development/compiling/introduction_to_the_buildsystem.html
@REM https://docs.godotengine.org/en/stable/contributing/development/compiling/compiling_with_dotnet.html#enable-the-net-module
@REM https://docs.godotengine.org/en/stable/contributing/development/compiling/compiling_for_windows.html

scons platform=windows target=editor tools=yes module_mono_enabled=yes mono_glue=no vsproj=yes dev_build=yes

"bin/godot.windows.editor.dev.x86_64.mono.console.exe" --headless --generate-mono-glue modules/mono/glue

@REM # dotnet nuget add source GodotNugetSource --name GodotNugetSource
@REM # dotnet nuget list source
@REM # dotnet nuget remove source GodotNugetSource

python ./modules/mono/build_scripts/build_assemblies.py --godot-output-dir ./bin --push-nupkgs-local C:\Robyn\Nuget\GodotNugetSource
