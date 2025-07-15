# 包描述文件

[Defines]
  DSC_SPECIFICATION              = 0x00010005
  PLATFORM_NAME                  = MyHttpClientPkg
  PLATFORM_GUID                  = 6cc67b22-de5a-4f44-a69e-007c47d522a3
  PLATFORM_VERSION               = 0.1
  SUPPORTED_ARCHITECTURES        = IA32|X64
  BUILD_TARGETS                  = DEBUG|RELEASE|NOOPT
  OUTPUT_DIRECTORY               = Build/MyHttpClientPkg

[LibraryClasses]
  UefiApplicationEntryPoint|MdePkg/Library/UefiApplicationEntryPoint/UefiApplicationEntryPoint.inf
  UefiLib|MdePkg/Library/UefiLib/UefiLib.inf
  DebugLib|MdePkg/Library/UefiDebugLibConOut/UefiDebugLibConOut.inf
  NetLib|NetworkPkg/Library/DxeNetLib/DxeNetLib.inf
  HttpLib|NetworkPkg/Library/DxeHttpLib/DxeHttpLib.inf
  DpcLib|MdeModulePkg/Library/DxeDpcLib/DxeDpcLib.inf

[Components]
  # 添加你的应用程序
  MyHttpClientPkg/Applications/HttpPostApp/HttpPostApp.inf