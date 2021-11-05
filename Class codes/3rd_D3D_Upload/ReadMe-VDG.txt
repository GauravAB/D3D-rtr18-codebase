01) Download From https://github.com/Microsoft/DirectXTK.git
    Eg : DirectXTK-master.zip

02) Unzip It Into Its Own Directory.
    Eg : DirectXTK-master

03) Inside It There Is A .sln/.vcxproj File For Respective Versions Of Visual Studio.

04) Open Respective .sln/.vcxproj File In Respective Visual Studio.
    Eg : I Chose 'DirectXTK_Desktop_2015_Win10.vcxproj'

05) It Will Open With 3 Projects In Solution Explorer.
    a) DirectXTK_Desktop_2015_Win10 ( C++ Project )
    b) MakeSpriteFont ( C# Project )
    c) XWBTool_Desktop_2015 ( C++ Project )

06) By Default Project Is 32 Bit. 
    If Required Convert Project To 64 Bit.

07) Right Click On 'a' i.e. DirectXTK_Desktop_2015_Win10, And From Context Menu, Choose 'Rebuild'.

08) The Resulting 'DirectXTK-master\Bin\Desktop_2015_Win10\Win32\Debug' Path Contains 'DirectXTK.lib' File.

09) Copy 'DirectXTK.lib' And Its Associated Other 2 Files : 'DirectXTK.pdb' And 'DirectXTK.pch'
    To The Project's Directory And Then Link 'DirectXTK.lib' With The Project By Using #pragma Directive.

10) Similarly Copy 'DirectXTK-master\Inc\WICTextureLoader.h' To Project's Directory.

11) Add It To Project By 'Add Existing Item' Method And Then Include As :
    #include "WICTextureLoader.h"

12) As Functions In Above File Are Inside 'DirectX' Namespace,
    Either Add 'using namespace DirectX;' Statement After Above Header File's Inclusion
    Or Use Any Function From It By 'DirectX::<function name> Syntax.
============================================================================================================
IMPORTANT About Command Line Building :
=======================================
If Using 'Debug' Version Of DirectXTK.lib Creates Linking Errors,
Use 'Release' Version.
This May Raise 'LNK4098' Warning Of defaultLib 'MSVCRT'
Remedy Is To Use : /NODEFAULTLIB:msvcrt.lib Linker Command Line Switch.
So Change 'Compile.bat' Accordingly.
============================================================================================================
