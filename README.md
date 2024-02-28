
#### build and use
1. Clone the repo. The directory structure is roughly like the following:
```
dd                               // Project root directory
|--- *bin                        // Output path, automatically generated after compilation, all output files are classified and placed here
|--- projects                    // projects directory, each subdirectory inside is a separate project
|    |--- ddbase                 // Output ddbase.lib, this lib encapsulates some commonly used utility functions and classes, such as string processing, file handling, etc.
|    |--- ddhook                 // Output ddhook.lib, this library is injection-related
|    |--- ddtools                // Output ddtools.exe, this exe provides some tools, such as packaging this library, code formatting, etc.
|    |--- ddwin                  // Output ddwin.lib, this lib provides some tools and classes for drawing Windows interface
|    |--- test                   // Output test.exe, this test is a unit test for this project, it's just a manual test, there is no pipeline
|    |--- test_syringe           // Output test_syringe.dll, this file is the testing DLL for ddhook
|--- .gitignore                  // Gitignore file
|--- build.bat                   // Build the entire project
|--- dd.sln                      // .sln file
|--- user.props                  // Defines some paths that will be used by the project (.vcxproj), such as the output path
```
2. Run build.bat to compile. Or open the `dd.sln` with Visual studio, and build it yourself. The build out dir is `dd\bin\`. the directory structure is roughly like the following:
```
dd
|--- bin
|    |--- Debug_Mdd_Win32
|    |--- Debug_Mdd_x64
|    |--- Debug_Win32
|    |--- Debug_x64
|    |--- Release_Md_Win32
|    |--- Release_Md_x64
|    |--- Release_Win32
|    |--- Release_x64
```
3. Add head file dir: `dd\projects`, and add the head file in your code `#include "ddbase/ddmini_include.h"`, this will include some commonly used header files, and other files can be included as needed.
4. Add the library dir: `dd\bin\$(Configuration)_$(PlatForm)\`, and add the needed lib `ddbase.lib`. If you use other libraries in this sln, they also need to be linked.
#### notes
1. Due to the project's use of the STL library in its exported items, it is not recommended to directly send the header files and compiled lib files to others for use. Instead, it is advised to compile and use them locally on your own machine.
2. The dependencies on other third-party libraries have been separated into individual projects, such as ddimage. These libraries will depend on this project, while this project does not rely on any other third-party libraries.
